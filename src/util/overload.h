#ifndef UTIL_H_INCLUDED
#define UTIL_H_INCLUDED

namespace util {

template <typename... Ts>
struct overload : Ts... { using Ts::operator()...; };

template <typename... Ts>
overload(Ts...) -> overload<Ts...>;

} // namespace util

#endif // UTIL_H_INCLUDED
