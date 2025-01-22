#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

constexpr static auto kBase64Table = ([]() {
    const char* table_str =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789"
        "+/";
    std::array<uint8_t, 64> table{};
    for (auto& item : table) {
        item = *table_str++;
    }
    return table;
})();

// NOLINTBEGIN(*-magic-numbers)

constexpr static auto kBase64ReverseTable = ([]() {
    std::array<uint8_t, 256> reverse_table{};

    for (size_t i = 0; i < kBase64Table.size(); i++) {
        reverse_table[kBase64Table[i]] = static_cast<uint8_t>(i);
    }

    // url-safe varient
    reverse_table['-'] = 62;
    reverse_table['_'] = 63;

    return reverse_table;
})();

inline size_t b64_decode_buffer_len(size_t len) {
    // Every 4 bytes in, it will yield 3 bytes output
    return (len + 3) / 4 * 3;
}

inline size_t b64_decode(uint8_t* output, const uint8_t* input, size_t input_len) {
    auto* p_out = output;
    size_t total_decoded = 0;

    auto decode_block = [&p_out](const uint8_t* p_in) {
        // NOLINTBEGIN(*-identifier-length)
        uint8_t a{kBase64ReverseTable[p_in[0]]};
        uint8_t b{kBase64ReverseTable[p_in[1]]};
        uint8_t c{kBase64ReverseTable[p_in[2]]};
        uint8_t d{kBase64ReverseTable[p_in[3]]};
        // NOLINTEND(*-identifier-length)

        *p_out++ = (a << 2) | (b >> 4);
        *p_out++ = (b << 4) | (c >> 2);
        *p_out++ = (c << 6) | (d >> 0);

        if (p_in[2] == '=') {
            p_out -= 2;
            return true;
        }

        if (p_in[3] == '=') {
            p_out -= 1;
            return true;
        }

        return false;
    };

    for (const auto* p_input_end = input + input_len - 4; input <= p_input_end; input += 4) {
        if (decode_block(input)) {
            return p_out - output;
        }
    }

    input_len %= 4;
    if (input_len != 0) {
        std::array<uint8_t, 4> buffer{0, '=', '=', '='};
        std::copy_n(input, input_len, buffer.begin());
        decode_block(buffer.data());
    }
    return p_out - output;
}

std::vector<uint8_t> b64_decode(const uint8_t* input, size_t len) {
    std::vector<uint8_t> result(b64_decode_buffer_len(len));
    result.resize(b64_decode(result.data(), input, len));
    return result;
}
