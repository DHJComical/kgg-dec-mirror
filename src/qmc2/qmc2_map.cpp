#include "qmc2.h"

namespace QMC2 {

constexpr size_t kMapOffsetBoundary = 0x7FFF;

QMC2_MAP::QMC2_MAP(std::span<uint8_t> key) {
    auto n = key.size();
    for (size_t i = 0; i < kMapKeySize; i++) {
        size_t j = (i * i + kMapIndexOffset) % n;
        const auto shift = (j + 4) % 8;
        key_[i] = (key[j] << shift) | (key[j] >> shift);
    }
}

void QMC2_MAP::Decrypt(std::span<uint8_t> data, size_t offset) const {
    for (auto& it : data) {
        size_t idx = (offset <= kMapOffsetBoundary) ? offset : (offset % kMapOffsetBoundary);
        it ^= key_[idx % key_.size()];
        offset++;
    }
}

}  // namespace QMC2
