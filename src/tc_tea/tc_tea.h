#pragma once

#include "endian_helper.h"

#include <cstdint>
#include <span>
#include <vector>

std::vector<uint8_t> tc_tea_cbc_decrypt(std::span<uint8_t> cipher,
                                        const uint32_t *key);

inline std::vector<uint8_t> tc_tea_cbc_decrypt(std::span<uint8_t> cipher,
                                               const uint8_t *key) {
  uint32_t key_u32[4];
  for (int i = 0; i < 4; i++) {
    key_u32[i] = Endian::be_u32_read(key + i * 4);
  }
  return tc_tea_cbc_decrypt(cipher, key_u32);
}
