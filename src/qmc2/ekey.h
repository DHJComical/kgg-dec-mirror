#pragma once
#include <cstdint>
#include <string_view>
#include <vector>

std::vector<uint8_t> decrypt_ekey(std::string_view ekey);
