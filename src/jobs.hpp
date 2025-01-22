#pragma once
#include <windows.h>

#include <array>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <queue>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "qmc2/qmc2.h"

class KggTask {
   public:
    explicit KggTask(std::filesystem::path kgg_path, std::filesystem::path out_dir)
        : kgg_path_(std::move(kgg_path)), out_dir_(std::move(out_dir)) {}

    void warning(const wchar_t* msg) const { fwprintf(stderr, L"[WARN] %s (%s)\n", msg, kgg_path_.filename().c_str()); }
    void warning(const std::wstring& msg) const { warning(msg.c_str()); }

    void error(const wchar_t* msg) const { fwprintf(stderr, L"[ERR ] %s (%s)\n", msg, kgg_path_.filename().c_str()); }
    void error(const std::wstring& msg) const { error(msg.c_str()); }

    void info(const wchar_t* msg) const { fwprintf(stderr, L"[INFO] %s (%s)\n", msg, kgg_path_.filename().c_str()); }
    void info(const std::wstring& msg) const { info(msg.c_str()); }

    void Execute(const Infra::kgm_ekey_db_t& ekey_db) {
        constexpr static std::array<uint8_t, 16> kMagicHeader{0x7C, 0xD5, 0x32, 0xEB, 0x86, 0x02, 0x7F, 0x4B,
                                                              0xA8, 0xAF, 0xA6, 0x8E, 0x0F, 0xFF, 0x99, 0x14};

        std::ifstream kgg_stream_in(kgg_path_, std::ios::binary);
        char header[0x100]{};
        kgg_stream_in.read(header, sizeof(kgg_stream_in));
        if (std::equal(kMagicHeader.cbegin(), kMagicHeader.cend(), header)) {
            warning(L"invalid kgg header");
            return;
        }
        uint32_t offset_to_audio = *reinterpret_cast<uint32_t*>(&header[0x10]);
        uint32_t encrypt_mode = *reinterpret_cast<uint32_t*>(&header[0x14]);
        if (encrypt_mode != 5) {
            warning(std::format(L"unsupported enc_version (expect=0x05, got 0x{:02x})", encrypt_mode));
            return;
        }
        uint32_t audio_hash_len = *reinterpret_cast<uint32_t*>(&header[0x44]);
        if (audio_hash_len != 0x20) {
            warning(std::format(L"audio hash length invalid (expect=0x20, got 0x{:02x})", audio_hash_len));
            return;
        }
        std::string audio_hash(&header[0x48], &header[0x48 + audio_hash_len]);
        std::string ekey{};
        if (auto it = ekey_db.find(audio_hash); it != ekey_db.end()) {
            ekey = it->second;
        } else {
            warning(L"ekey not found");
            return;
        }

        auto qmc2 = QMC2::Create(ekey);

        std::string magic(4, 0);
        kgg_stream_in.seekg(offset_to_audio, std::ios::beg);
        kgg_stream_in.read(magic.data(), 4);
        qmc2->Decrypt(std::span(reinterpret_cast<uint8_t*>(magic.data()), 4), 0);
        auto real_ext = DetectRealExt(magic);
        auto out_path = out_dir_ / std::format(L"{}_kgg-dec.{}", kgg_path_.stem().wstring(), real_ext);

        if (exists(out_path)) {
            warning(std::format(L"output file already exists: {}", out_path.filename().wstring()));
            return;
        }

        kgg_stream_in.seekg(offset_to_audio, std::ios::beg);
        std::ofstream ofs_decrypted(out_path, std::ios::binary);
        if (!ofs_decrypted.is_open()) {
            error(L"failed to open output file");
            return;
        }

        size_t offset{0};
        thread_local std::vector<uint8_t> temp_buffer(1024 * 1024, 0);
        auto buf_signed = std::span(reinterpret_cast<char*>(temp_buffer.data()), temp_buffer.size());
        auto buf_unsigned = std::span(temp_buffer);

        while (!kgg_stream_in.eof()) {
            kgg_stream_in.read(buf_signed.data(), buf_signed.size());
            const auto n = static_cast<size_t>(kgg_stream_in.gcount());
            qmc2->Decrypt(buf_unsigned.subspan(0, n), offset);
            ofs_decrypted.write(buf_signed.data(), n);
            offset += n;
        }

        info(std::format(L"** OK **  -> {}", out_path.filename().wstring()));
    }

   private:
    std::filesystem::path kgg_path_;
    std::filesystem::path out_dir_;

    static const wchar_t* DetectRealExt(std::string_view magic) {
        if (magic == "fLaC") {
            return L"flac";
        }
        if (magic == "OggS") {
            return L"ogg";
        }
        return L"mp3";
    }
};

class KggTaskQueue {
   public:
    explicit KggTaskQueue(Infra::kgm_ekey_db_t ekey_db) : ekey_db_(std::move(ekey_db)) {}

    void Push(std::unique_ptr<KggTask> task) {
        std::lock_guard lock(mutex_);
        tasks_.push(std::move(task));
        signal_.notify_one();
    }

    std::unique_ptr<KggTask> Pop() {
        std::unique_lock lock(mutex_);
        signal_.wait(lock, [this] { return !tasks_.empty() || thread_end_; });
        if (tasks_.empty()) {
            return {};
        }

        auto task = std::move(tasks_.front());
        tasks_.pop();
        return task;
    }

    [[nodiscard]] bool Finished() {
        std::lock_guard lock(mutex_);
        return tasks_.empty();
    }

    void AddWorkerThread() { threads_.emplace(&KggTaskQueue::WorkerThreadBody, this); }

    void Join() {
        thread_end_ = true;
        signal_.notify_all();

        for (int i = 1; !threads_.empty(); i++) {
            threads_.front().join();
            threads_.pop();
#ifndef NDEBUG
            fprintf(stderr, "[INFO] thread %d joined\n", i);
#endif
        }
    }

   private:
    bool thread_end_{false};
    Infra::kgm_ekey_db_t ekey_db_;
    void WorkerThreadBody() {
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);

        std::unique_ptr<KggTask> task{nullptr};
        while ((task = Pop())) {
            task->Execute(ekey_db_);
        }
    }

    std::mutex mutex_{};
    std::condition_variable signal_;
    std::queue<std::unique_ptr<KggTask>> tasks_{};
    std::queue<std::thread> threads_{};
};
