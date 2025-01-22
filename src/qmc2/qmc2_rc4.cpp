#include <cstdint>
#include <utility>
#include <vector>
#include "qmc2.h"

namespace QMC2 {

inline double hash(const uint8_t* key, size_t len) {
    uint32_t hash = {1};
    const uint8_t* end = key + len;
    for (; key < end; ++key) {
        if (*key == 0) {
            continue;
        }

        // Overflow check.
        uint32_t next_hash = hash * static_cast<uint32_t>(*key);
        if (next_hash <= hash) {
            break;
        }
        hash = next_hash;
    }
    return (double)hash;
}

class RC4 {
   public:
    inline RC4(const uint8_t* key, size_t key_len) {
        state_.resize(key_len);
        for (size_t i = 0; i < key_len; i++) {
            state_[i] = static_cast<uint8_t>(i);
        }

        for (size_t i = 0, j = 0; i < key_len; i++) {
            j = (j + state_[i] + key[i]) % key_len;
            std::swap(state_[i], state_[j]);
        }

        state_len_ = key_len;
    }

    inline void Derive(std::span<uint8_t> buffer) {
        size_t i = i_;
        size_t j = j_;
        uint8_t* s = state_.data();
        const size_t n = state_len_;

        for (auto& it : buffer) {
            i = (i + 1) % n;
            j = (j + s[i]) % n;
            std::swap(s[i], s[j]);

            const size_t final_idx = (s[i] + s[j]) % n;
            it ^= s[final_idx];
        }

        i_ = i;
        j_ = j;
    }

   private:
    std::vector<uint8_t> state_{};
    size_t state_len_{0};
    size_t i_{0};
    size_t j_{0};
};

inline size_t get_segment_key(double key_hash, size_t segment_id, uint8_t seed) {
    if (seed == 0) {
        return 0;
    }

    const double result = key_hash / static_cast<double>(seed * (segment_id + 1)) * 100.0;
    return static_cast<size_t>(result);
}

QMC2_RC4::QMC2_RC4(std::span<uint8_t> key) {
    hash_ = hash(key.data(), key.size());
    key_ = std::vector(key.begin(), key.end());

    RC4 rc4(key.data(), key.size());
    rc4.Derive(std::span<uint8_t>(key_stream_));
}

void QMC2_RC4::Decrypt(std::span<uint8_t> data, size_t offset) const {
    if (offset < kFirstSegmentSize) {
        const auto n = DecryptFirstSegment(data, offset);
        offset += n;
        data = data.subspan(n);
    }

    while (!data.empty()) {
        const auto n = DecryptOtherSegment(data, offset);
        offset += n;
        data = data.subspan(n);
    }
}

size_t QMC2_RC4::DecryptFirstSegment(std::span<uint8_t> data, size_t offset) const {
    const uint8_t* key = key_.data();
    const size_t n = this->key_.size();

    size_t process_len = std::min(data.size(), kFirstSegmentSize - offset);
    for (auto& it : data.subspan(0, process_len)) {
        const auto idx = get_segment_key(hash_, offset, key[offset % n]) % n;
        it ^= key[idx];
        offset++;
    }
    return process_len;
}

size_t QMC2_RC4::DecryptOtherSegment(std::span<uint8_t> data, size_t offset) const {
    const size_t n = this->key_.size();

    size_t segment_idx = offset / kOtherSegmentSize;
    size_t segment_offset = offset % kOtherSegmentSize;

    size_t skip_len = get_segment_key(hash_, segment_idx, key_[segment_idx % n]) & 0x1FF;
    size_t process_len = std::min(data.size(), kOtherSegmentSize - segment_offset);
    const uint8_t* rc4_stream = &key_stream_[skip_len + segment_offset];
    for (auto& it : data.subspan(0, process_len)) {
        it ^= *rc4_stream++;
    }
    return process_len;
}

}  // namespace QMC2
