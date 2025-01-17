#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include <model/Register.h>

enum class Register;

namespace model {

class Channel {
public:
    size_t size() const;
    void setValue(size_t register_index, uint8_t value);

    static Channel channel1();
    static Channel channel2();
    static Channel channel3();
    static Channel channel4();

private:
    Channel(std::initializer_list<Register> registers);
    static constexpr size_t maxRegisterSize = 5;

    std::array<Register, maxRegisterSize> m_registers;
    std::array<uint8_t, maxRegisterSize> m_values;
};

}
