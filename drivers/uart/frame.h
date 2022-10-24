#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace Nilai::Drivers::Uart {

struct Frame {
#if 0
        uint8_t* data      = nullptr;
    size_t   len       = 0;
#endif
    std::vector<uint8_t> data;
    size_t               len       = 0;
    uint32_t             timestamp = 0;

    Frame() = default;

#if 0
        Frame(const std::vector<uint8_t>& d, uint32_t t) : timestamp(t)
    {
        data = new uint8_t[d.size()];
        CEP_ASSERT(data != nullptr, "Unable to allocate memory for data!");
        len = d.size();
        for (size_t i = 0; i < d.size(); i++)
        {
            data[i] = d[i];
        }
    }
    ~Frame()
    {
        delete[] data;
        data = nullptr;
        len  = 0;
    }
#endif

    Frame(const std::vector<uint8_t>& d, uint32_t t) : timestamp(t) {
        data = d;
        len  = data.size();
    }

    [[nodiscard]] std::string ToStr() const { return std::string{(char*)data.data()}; }

    bool operator==(const std::string& s) const { return (std::string((char*)data.data(), len) == s); }

    bool operator!=(const std::string& s) const { return (std::string((char*)data.data(), len) != s); }
};

}    // namespace Nilai::Drivers::Uart