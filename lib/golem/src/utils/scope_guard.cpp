#include <golem/utils/scope_guard.h>

namespace utils
{

ScopeGuard::ScopeGuard(std::function<void()> callback)
    : m_callback(std::move(callback))
{
}

ScopeGuard::~ScopeGuard()
{
    m_callback();
}

}
