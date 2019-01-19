#ifndef RESULT_H
#define RESULT_H

#include <optional>

template<typename T>
using Result = std::pair<std::optional<T>, std::optional<std::string>>;

using Error = std::optional<std::string>;

#endif
