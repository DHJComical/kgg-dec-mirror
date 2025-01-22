#include "infra/infra.h"
#include "infra/sqlite_error.h"
#include "jobs.hpp"
#include "qmc2/qmc2.h"

// clang-format off
#include <windows.h>
#include <shlobj.h>
#include <initguid.h>
#include <knownfolders.h>
// clang-format on

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <span>
#include <utility>

using Infra::kgm_ekey_db_t;

void WalkFileOrDir(KggTaskQueue& queue, const std::filesystem::path& input_path, bool scan_all_exts) {
    std::queue<std::filesystem::path> file_queue;
    file_queue.push(absolute(input_path));

    while (!file_queue.empty()) {
        auto target_path = std::move(file_queue.front());
        file_queue.pop();

        if (is_regular_file(target_path)) {
            if (!scan_all_exts && target_path.extension() != L".kgg") {
                continue;
            }

            queue.Push(std::make_unique<KggTask>(target_path, target_path.parent_path()));
            continue;
        }

        if (is_directory(target_path)) {
            for (auto const& dir_entry : std::filesystem::directory_iterator{target_path}) {
                file_queue.push(dir_entry.path());
            }
            continue;
        }

        fwprintf(stderr, L"[WARN] invalid path: %s\n", target_path.c_str());
    }
}

std::pair<std::vector<std::wstring>, std::unordered_map<std::wstring, std::wstring>> ParseCommandLine() {
    int argc;
    wchar_t** argv = CommandLineToArgvW(GetCommandLineW(), &argc);

    std::vector<std::wstring> positional_args{};
    std::unordered_map<std::wstring, std::wstring> named_args{};

    bool positional_only{false};
    for (int i = 1; i < argc; i++) {
        std::wstring arg{argv[i]};
        if (arg == L"--") {
            positional_only = true;
            continue;
        }

        if (!positional_only && arg.starts_with(L"--")) {
            auto pos = arg.find(L'=');
            if (pos != std::wstring::npos) {
                named_args[arg.substr(2, pos - 2)] = arg.substr(pos + 1);
            } else if (++i < argc) {
                named_args[arg.substr(2)] = argv[i];
            } else {
                named_args[arg.substr(2)] = L"";
            }
        } else {
            positional_args.push_back(arg);
        }
    }

    return std::make_pair(positional_args, named_args);
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    setlocale(LC_ALL, ".UTF8");

    fprintf(stderr, "kgg-dec v" KGGDEC_PROJECT_VERSION " by LSR\n");
    fputs(
        "Usage: kgg-dec "
        "[--infra-dll infra.dll] "
        "[--scan-all-file-ext 0] "
        "[--db /path/to/KGMusicV3.db] "
        "[--] [kgg-dir... = '.']\n\n",
        stderr);

    auto [positional_args, named_args] = ParseCommandLine();
    if (positional_args.empty()) {
        positional_args.emplace_back(L".");
    }

    std::filesystem::path kgm_db_path{};
    std::filesystem::path infra_dll_path{L"infra.dll"};
    if (auto it = named_args.find(L"infra-dll"); it != named_args.end()) {
        infra_dll_path = std::filesystem::path{it->second};
    }
    infra_dll_path = absolute(infra_dll_path);
    if (!exists(infra_dll_path)) {
        fputs("[ERR ] infra.dll not found\n", stderr);
        return 1;
    }

    bool scan_all_exts{false};
    if (auto it = named_args.find(L"scan-all-file-ext"); it != named_args.end()) {
        scan_all_exts = it->second == L"1";
    }

    if (auto it = named_args.find(L"db"); it != named_args.end()) {
        kgm_db_path = std::filesystem::path{it->second};
    } else {
        PWSTR pAppDirPath{};
        SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &pAppDirPath);
        kgm_db_path = std::filesystem::path{pAppDirPath} / L"Kugou8" / L"KGMusicV3.db";
        CoTaskMemFree(pAppDirPath);
    }
    if (!exists(kgm_db_path)) {
        fputs("[ERR ] KGMusicV3.db not found\n", stderr);
        return 1;
    }

    int error{-1};
    Infra::KugouDb db{infra_dll_path, kgm_db_path};
    if (!db.IsOpen()) {
        fprintf(stderr, "[ERR ] db init error: is infra.dll ok?\n");
        return 1;
    }
    auto ekey_db = db.dump_ekey(error);
    if (error != 0) {
        fprintf(stderr, "[ERR ] dump ekey failed %d (%s)", error, sqlite_get_error(error));
        return 1;
    }
    db.Close();

#ifndef NDEBUG
    fprintf(stderr, "ekey_db:\n");
    for (auto& [a, b] : ekey_db) {
        fprintf(stderr, "%s --> %s\n", a.c_str(), b.c_str());
    }
#endif

    KggTaskQueue queue(ekey_db);
    auto thread_count =
#ifndef NDEBUG
        1;
#else
        std::max(static_cast<int>(std::thread::hardware_concurrency()) - 2, 2);
#endif

    for (int i = 0; i < thread_count; i++) {
        queue.AddWorkerThread();
    }

    for (auto& positional_arg : positional_args) {
        WalkFileOrDir(queue, positional_arg, scan_all_exts);
    }
    queue.Join();

    return 0;
}
