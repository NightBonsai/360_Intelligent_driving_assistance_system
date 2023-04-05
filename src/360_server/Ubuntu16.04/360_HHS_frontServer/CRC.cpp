#include "CRC.h"

uint32_t CRC::calculate_crc32(const char* data, size_t length)
{
    uint32_t crc = 0xFFFFFFFF;

    for (size_t i = 0; i < length; ++i) {
        crc = (crc >> 8) ^ crc32_table[(crc ^ data[i]) & 0xFF];
    }

    return crc ^ 0xFFFFFFFF;
}
