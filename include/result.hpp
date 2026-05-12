#pragma once
#include <expected>
#include "error.hpp"

template<typename T>
using Result = std::expected<T, Error>;
using VoidResult = std::expected<void, Error>;
