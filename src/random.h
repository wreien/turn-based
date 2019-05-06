#ifndef RANDOM_H_INCLUDED
#define RANDOM_H_INCLUDED

#include <random>
#include <type_traits>
#include <initializer_list>

namespace _detail::random {
    auto& generator() {
        static auto gen = [](){
            std::random_device dev;
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
    template <typename T>
    auto& fromContainer(T&& container) {
        auto s = std::size(container);
        auto v = ::random(s - 1);
        if constexpr (std::is_const_v<T>)
            return *std::next(std::cbegin(container), v);
        else
            return *std::next(std::begin(container), v);
    }
}

// Select a random element from a container glvalue. Returns a reference.
template <typename T>
auto random(T& container) -> decltype(*std::begin(container)) {
    return _detail::random::fromContainer(container);
}

// Select a random element from a const ref to a container. Returns a const reference.
template <typename T>
auto random(const T& container) -> decltype(*std::cbegin(container)) {
    return _detail::random::fromContainer(container);
}

// Select a random element from a container xvalue. Returns a value.
template <typename T>
auto random(T&& container)
    -> std::remove_reference_t<decltype(*std::begin(container))>
{
    return _detail::random::fromContainer(container);
}

// Select a random element from an initializer list of options. Returns a value.
template <typename T>
auto random(std::initializer_list<T> il) -> typename decltype(il)::value_type {
    return _detail::random::fromContainer(il);
}

// Catch all clause for better error messages
template <typename T = void>
void random(...) {
    // can't static_assert(false), so workaround ;)
    constexpr auto fail = [](){ return false; }();
    static_assert(fail, "must provide a number or container type to 'random'");
}

#endif // RANDOM_H_INCLUDED
