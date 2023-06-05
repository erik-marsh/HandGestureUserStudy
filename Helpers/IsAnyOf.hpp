#include <concepts>

// https://stackoverflow.com/questions/73377786/c-concept-that-a-type-is-same-as-any-one-of-several-types
template <typename T, typename... U>
concept IsAnyOf = (std::same_as<T, U> || ...);
