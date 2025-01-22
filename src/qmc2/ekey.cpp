#include "ekey.h"
#include "base64.h"
#include "tc_tea.h"

#include <algorithm>
#include <string>
#include <array>
#include <vector>

const static std::string kEKeyV2Prefix = "UVFNdXNpYyBFbmNWMixLZXk6";
const static std::array<uint8_t, 16> kEKeyV2Key1{
    0x33, 0x38, 0x36, 0x5A, 0x4A, 0x59, 0x21, 0x40,
    0x23, 0x2A, 0x24, 0x25, 0x5E, 0x26, 0x29, 0x28,
};
const static std::array<uint8_t, 16> kEKeyV2Key2{
    0x2A, 0x2A, 0x23, 0x21, 0x28, 0x23, 0x24, 0x25,
    0x26, 0x5E, 0x61, 0x31, 0x63, 0x5A, 0x2C, 0x54,
};

template <typename T> std::span<T> ss2span(std::string_view sv) {
  auto *data = reinterpret_cast<const T *>(sv.data());
  return std::span<T>(const_cast<T *>(data), sv.size());
}

template <typename T> std::string_view span2ss(std::span<T> span) {
  return std::string_view(reinterpret_cast<char *>(span.data()), span.size());
}

void remove_trailing_zeros(std::vector<uint8_t> &vec) {
  auto it = std::find_if(vec.rbegin(), vec.rend(),
                         [](uint8_t value) { return value != 0; });
  vec.erase(it.base(), vec.end());
}

std::vector<uint8_t> decrypt_ekey_v1(std::string_view ekey) {
  std::vector<uint8_t> result =
      b64_decode(reinterpret_cast<const uint8_t *>(ekey.data()), ekey.size());
  remove_trailing_zeros(result);

  uint32_t tea_key[4] = {
      0x69005600 | static_cast<uint32_t>(result[0] << 16) | (result[1]),
      0x46003800 | static_cast<uint32_t>(result[2] << 16) | (result[3]),
      0x2b002000 | static_cast<uint32_t>(result[4] << 16) | (result[5]),
      0x15000b00 | static_cast<uint32_t>(result[6] << 16) | (result[7]),
  };
  auto decrypted = tc_tea_cbc_decrypt(std::span(result).subspan(8), tea_key);
  if (decrypted.empty()) {
    return {};
  }
  result.resize(8);
  result.insert(result.end(), decrypted.begin(), decrypted.end());
  return result;
}

std::vector<uint8_t> decrypt_ekey_v2(std::string_view ekey) {
  std::vector<uint8_t> result;
  result = tc_tea_cbc_decrypt(ss2span<uint8_t>(ekey), kEKeyV2Key1.data());
  result = tc_tea_cbc_decrypt(std::span(result), kEKeyV2Key2.data());
  return decrypt_ekey_v1(span2ss(std::span(result)));
}

std::vector<uint8_t> decrypt_ekey(std::string_view ekey) {
  if (ekey.starts_with(kEKeyV2Prefix)) {
    ekey.remove_prefix(kEKeyV2Prefix.size());
    return decrypt_ekey_v2(ekey);
  }

  return decrypt_ekey_v1(ekey);
}
