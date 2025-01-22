#include "tc_tea.h"

#include "endian_helper.h"
#include "tea_ecb.h"

#include <algorithm>
#include <cstdint>

constexpr size_t kTeaBlockSize = 8;
constexpr size_t kFixedSaltLen = 2;
constexpr size_t kZeroPadLen = 7;

inline void decrypt_round(uint8_t *p_plain, const uint8_t *p_cipher,
                          uint64_t *iv1, uint64_t *iv2, const uint32_t *key) {
  uint64_t iv1_next = Endian::be_u64_read(p_cipher);
  uint64_t iv2_next = tc_tea_ecb_decrypt(iv1_next ^ *iv2, key);
  uint64_t plain = iv2_next ^ *iv1;
  *iv1 = iv1_next;
  *iv2 = iv2_next;
  Endian::be_u64_write(p_plain, plain);
}

std::vector<uint8_t> tc_tea_cbc_decrypt(std::span<uint8_t> cipher,
                                        const uint32_t *key) {
  // It needs to have at least 2 blocks long, due to the nature of the padding
  // scheme used.
  if (cipher.size() % kTeaBlockSize != 0 || cipher.size() < kTeaBlockSize * 2) {
    return {};
  }

  uint64_t iv1 = 0;
  uint64_t iv2 = 0;
  uint8_t header[kTeaBlockSize * 2];
  const uint8_t *in_cipher = cipher.data();
  decrypt_round(header, in_cipher, &iv1, &iv2, key);
  in_cipher += kTeaBlockSize;
  decrypt_round(header + kTeaBlockSize, in_cipher, &iv1, &iv2, key);
  in_cipher += kTeaBlockSize;

  size_t hdr_skip_len = 1 + (header[0] & 7) + kFixedSaltLen;
  size_t real_plain_len = cipher.size() - hdr_skip_len - kZeroPadLen;
  std::vector<uint8_t> result(real_plain_len);

  auto p_output = result.data();

  // copy first block of plain text
  size_t copy_len = std::min(sizeof(header) - hdr_skip_len, real_plain_len);
  std::copy_n(header + hdr_skip_len, real_plain_len, p_output);
  p_output += copy_len;

  if (real_plain_len != copy_len) {
    // Decrypt the rest of the blocks
    for (size_t i = cipher.size() - kTeaBlockSize * 3; i != 0;
         i -= kTeaBlockSize) {
      decrypt_round(p_output, in_cipher, &iv1, &iv2, key);
      in_cipher += kTeaBlockSize;
      p_output += kTeaBlockSize;
    }

    decrypt_round(header + kTeaBlockSize, in_cipher, &iv1, &iv2, key);
    p_output[0] = header[kTeaBlockSize];
  }
  // Validate zero padding
  auto verify = Endian::be_u64_read(header + kTeaBlockSize) << 8;
  if (verify != 0) {
    result.resize(0);
  }

  return result;
}
