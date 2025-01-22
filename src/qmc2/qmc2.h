#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <span>
#include <string_view>
#include <vector>

namespace QMC2 {

class QMC2_Base {
   public:
    QMC2_Base() = default;
    virtual ~QMC2_Base() = default;

    virtual void Decrypt(std::span<uint8_t> data, size_t offset) const = 0;
};

constexpr size_t kMapIndexOffset = 71214;
constexpr size_t kMapKeySize = 128;

class QMC2_MAP : public QMC2_Base {
   public:
    explicit QMC2_MAP(std::span<uint8_t> key);

    void Decrypt(std::span<uint8_t> data, size_t offset) const override;

   private:
    std::array<uint8_t, kMapKeySize> key_{};
};

constexpr size_t kFirstSegmentSize = 0x0080;
constexpr size_t kOtherSegmentSize = 0x1400;
constexpr size_t kRC4StreamSize = kOtherSegmentSize + 512;

class QMC2_RC4 : public QMC2_Base {
   public:
    explicit QMC2_RC4(std::span<uint8_t> key);

    void Decrypt(std::span<uint8_t> data, size_t offset) const override;

   private:
    std::vector<uint8_t> key_{};
    double hash_{0};
    std::array<uint8_t, kRC4StreamSize> key_stream_{};

    [[nodiscard]] size_t DecryptFirstSegment(std::span<uint8_t> data, size_t offset) const;
    [[nodiscard]] size_t DecryptOtherSegment(std::span<uint8_t> data, size_t offset) const;
};

std::unique_ptr<QMC2_Base> Create(std::string_view ekey);

}  // namespace QMC2
