#pragma once

#include <memory>

namespace utils {

template <typename T>
using shared_reference = std::shared_ptr<std::reference_wrapper<T>>;

template <typename T>
using weak_reference = std::weak_ptr<std::reference_wrapper<T>>;

template <typename T>
shared_reference<T> make_shared_reference(T& reference)
{
    return std::make_shared<std::reference_wrapper<T>>(reference);
}

}
