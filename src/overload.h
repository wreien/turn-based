#ifndef UTIL_H_INCLUDED
#define UTIL_H_INCLUDED

template <typename... Ts>
struct overload : Ts... { using Ts::operator()...; };

template <typename... Ts>
overload(Ts...) -> overload<Ts...>;

#endif // UTIL_H_INCLUDED
