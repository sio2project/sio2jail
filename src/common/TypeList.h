#pragma once

namespace s2j {

template<typename... Arg>
struct TypeList;

template<>
struct TypeList<> {
    const static bool empty = true;
};

template<typename Arg, typename... Args>
struct TypeList<Arg, Args...> {
    const static bool empty = false;
};

} // namespace s2j
