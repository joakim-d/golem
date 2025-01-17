#pragma once

#include <functional>
#include <list>

#include <utils/shared_reference.h>

namespace utils {

using observable_callback_id = size_t;

class observable_handle {
    template <typename... Args>
    friend class observable;

public:
    observable_handle() = default;

    ~observable_handle()
    {
        if (unregister_callback) {
            unregister_callback();
        }
    }

    observable_handle(const observable_handle& other) = delete;
    observable_handle& operator=(const observable_handle& other) = delete;

    observable_handle(observable_handle&& other) = default;
    observable_handle& operator=(observable_handle&& other) = default;

private:
    observable_handle(
        std::function<void()> unregister_callback)
        : unregister_callback(std::move(unregister_callback))
    {
    }

    std::function<void()> unregister_callback;
};

template <typename... Args>
class observable {
public:
    observable()
        : self_reference(make_shared_reference(*this))
    {
    }

    observable_handle register_callback(std::function<void(Args...)>&& callback)
    {
        callbacks.push_back(
            { current_id, std::move(callback) });

        auto weak_ref = utils::weak_reference<observable<Args...>> { self_reference };

        observable_handle handle {
            [weak_ref, current_id = current_id]() {
                if (auto ref = weak_ref.lock()) {
                    ref->get().unregister_callback(current_id);
                }
            }
        };

        current_id++;

        return handle;
    }

    void operator()(Args... args) const
    {
        for (const auto& [id, callback] : callbacks) {
            callback(args...);
        }
    }

private:
    void unregister_callback(observable_callback_id id)
    {
        auto callback_iterator = std::find_if(
            callbacks.begin(),
            callbacks.end(),
            [id](const std::pair<observable_callback_id, std::function<void(Args...)>>& callback) {
                return id == callback.first;
            });

        if (callback_iterator != callbacks.end()) {
            callbacks.erase(callback_iterator);
        }
    }

    const utils::shared_reference<observable<Args...>>
        self_reference;
    observable_callback_id current_id { 0 };
    std::list<std::pair<observable_callback_id, std::function<void(Args...)>>> callbacks;
};
}
