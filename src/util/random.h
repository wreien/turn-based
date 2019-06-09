#ifndef RANDOM_H_INCLUDED
#define RANDOM_H_INCLUDED

#include <random>
#include <type_traits>
#include <initializer_list>
#include <stdexcept>

namespace util {


namespace _detail::random {
    inline auto& generator() {
        static auto gen = [](){
            std::random_device dev;
            // TODO: better random device initialization?
            // TODO: generate different random functions for different subsystems?
            return std::mt19937{ dev() };
        }();
        return gen;
    }
}

// Generates a random number of type (C = T union U) in the given range.
// Given a common type "C", then:
//  - if "C" is integral, return a value in range [min, max]
//  - else if "C" is floating point, return a value in range [min, max)
template <typename T, typename U>
auto random(T min, U max) {
    using C = std::common_type_t<T, U>;
    static_assert(std::is_arithmetic_v<C>);

    auto dist = [](C min, C max) {
        if constexpr (std::is_integral_v<C>)
            return std::uniform_int_distribution<C>{ min, max };
        else
            return std::uniform_real_distribution<C>{ min, max };
    }(min, max);

    return dist(_detail::random::generator());
}


// Shortcut for calling random(0, max), with the same semantics as above.
template <typename T>
std::enable_if_t<std::is_arithmetic_v<T>, T> random(T max) {
    return random(T{}, max);
}

namespace _detail::random {
    template <typename C>
    decltype(auto) fromContainer(C&& container) {
        const auto size = std::size(container);
        if (size == 0)
            throw std::invalid_argument("random: empty container");

        auto it = std::begin(container);
        using diff_t = typename std::iterator_traits<decltype(it)>::difference_type;
        return *std::next(it, static_cast<diff_t>(util::random(size - 1)));
    }
}

// Select a random element from a container.
template <typename T, typename = std::void_t<
    decltype(std::begin(std::declval<T>())), decltype(std::size(std::declval<T>()))>>
decltype(auto) random(T&& container) {
    return _detail::random::fromContainer(std::forward<T>(container));
}

// Select a random element from an initializer list of options.
// Always returns by value (does it make sense to return reference to il?)
template <typename T>
auto random(std::initializer_list<T> il) {
    return _detail::random::fromContainer(il);
}

// Catch all clause for better error messages
template <typename T = void>
void random(...) {
    // can't static_assert(false), so workaround ;)
    constexpr auto fail = [](){ return false; }();
    static_assert(fail, "must provide a number or container type to 'random'");
}


} // namespace util

#endif // RANDOM_H_INCLUDED
