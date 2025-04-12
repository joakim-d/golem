#pragma once

#include <functional>

namespace utils
{

class ScopeGuard
{
public:
    ScopeGuard(std::function<void()> callback);
    ~ScopeGuard();

private:
    std::function<void()> m_callback;
};

}
