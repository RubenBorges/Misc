/*Usage example

generator<int> counter(int start, int stop) {
    for (int i = start; i <= stop; ++i) {
        co_yield i;
    }
}

int main() {
    for (int value : counter(1, 5)) {
        std::cout << value << "\n";
    }
}
OUTPUT:
1
2
3
4
5

*/

#include <coroutine>
#include <exception>
#include <iostream>

template<typename T>
struct generator {
    struct promise_type {
        T current_value;

        generator get_return_object() {
            return generator{
                std::coroutine_handle<promise_type>::from_promise(*this)
            };
        }

        std::suspend_always initial_suspend() noexcept { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }

        std::suspend_always yield_value(T value) noexcept {
            current_value = value;
            return {};
        }

        void return_void() {}
        void unhandled_exception() { std::terminate(); }
    };

    using handle_type = std::coroutine_handle<promise_type>;
    handle_type coro;

    explicit generator(handle_type h) : coro(h) {}
    generator(const generator&) = delete;
    generator(generator&& other) noexcept : coro(other.coro) {
        other.coro = nullptr;
    }
    ~generator() {
        if (coro) coro.destroy();
    }

    struct iterator {
        handle_type coro;

        iterator() : coro(nullptr) {}
        iterator(handle_type h) : coro(h) {}

        iterator& operator++() {
            coro.resume();
            if (coro.done()) coro = nullptr;
            return *this;
        }

        const T& operator*() const {
            return coro.promise().current_value;
        }

        bool operator==(std::default_sentinel_t) const {
            return !coro || coro.done();
        }

        bool operator!=(std::default_sentinel_t s) const {
            return !(*this == s);
        }
    };

    iterator begin() {
        if (coro) {
            coro.resume();
            if (coro.done()) return iterator{nullptr};
        }
        return iterator{coro};
    }

    std::default_sentinel_t end() { return {}; }
};
