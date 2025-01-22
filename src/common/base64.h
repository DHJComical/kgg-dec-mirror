#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

std::vector<uint8_t> b64_decode(const uint8_t *input, size_t len);
