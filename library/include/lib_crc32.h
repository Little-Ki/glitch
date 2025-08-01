#pragma once

#include <cstdint>

#include <array>

namespace cl::crc32 {    
    
    template <uint32_t Poly>
    constexpr std::array<uint32_t, 256> crcTable() {
        std::array<uint32_t, 256> result{}; 
        for (auto i = 0; i < 256; i += 1) {
            uint32_t crc = i;
            for (int j = 0; j < 8; j += 1)  {
                crc = (crc >> 1) ^ ((crc & 1) ? Poly : 0);
            }
            result[i] = crc;
        }
        return result;
    }

    template <typename T, typename = typename std::enable_if_t<std::is_integral_v<T>>>
    uint32_t hash(const T *data, uint32_t len, uint32_t crc = 0) {
        const static auto table = crcTable<0xEDB88320>();

        const auto raw_data = reinterpret_cast<uint8_t*>(data);
        const auto raw_size = len * sizeof(T);

        crc = crc ^ 0xFFFFFFFFU;
        for(uint32_t i = 0; i < raw_size; i++) {
            crc = table[(crc ^ raw_data[i]) & 0xFF] ^ (crc >> 8);
        }
        return crc ^ 0xFFFFFFFFU;
    }

    #define CT_CRC32(STR) std::integral_constant<uint32_t, ::hash(STR, sizeof(STR))>::value
    
}