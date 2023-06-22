#pragma once

#include <type_traits>
#include <stdexcept>

/// @brief Home-grown implementation of C++23's std::expected.
///        Nowhere near compliant to the C++23 standard, just something usable for this project.
/// @tparam ValueType 
/// @tparam ErrorType
template <typename ValueType, typename ErrorType>
requires std::is_default_constructible_v<ValueType> && std::is_default_constructible_v<ErrorType>
class Expected
{
   public:
    static_assert(!std::is_same_v<ValueType, ErrorType>,
                  "ValueType and ErrorType cannot be the same.");

    constexpr Expected(ValueType&& value);
    constexpr Expected(ErrorType&& error);

    constexpr ValueType& Value() &;
    constexpr const ValueType& Value() const&;
    constexpr ValueType&& Value() &&;
    constexpr const ValueType&& Value() const&&;

    constexpr ErrorType& Error() &;
    constexpr const ErrorType& Error() const&;
    constexpr ErrorType&& Error() &&;
    constexpr const ErrorType&& Error() const&&;

    constexpr explicit operator bool() const noexcept;
    constexpr bool HasValue() const noexcept;

   private:
    bool m_hasValue;
    ValueType m_value;
    ErrorType m_error;
};

template <typename ValueType, typename ErrorType>
requires std::is_default_constructible_v<ValueType> && std::is_default_constructible_v<ErrorType>
inline constexpr Expected<ValueType, ErrorType>::Expected(ValueType&& value)
    : m_hasValue(true), m_value(value)
{
}

template <typename ValueType, typename ErrorType>
requires std::is_default_constructible_v<ValueType> && std::is_default_constructible_v<ErrorType>
inline constexpr Expected<ValueType, ErrorType>::Expected(ErrorType&& error)
    : m_hasValue(false), m_error(error)
{
}

template <typename ValueType, typename ErrorType>
requires std::is_default_constructible_v<ValueType> && std::is_default_constructible_v<ErrorType>
inline constexpr ValueType& Expected<ValueType, ErrorType>::Value() &
{
    if (!m_hasValue) throw std::runtime_error("Cannot retrieve value: this Expected type has no value!");
    return m_value;
}

template <typename ValueType, typename ErrorType>
requires std::is_default_constructible_v<ValueType> && std::is_default_constructible_v<ErrorType>
inline constexpr const ValueType& Expected<ValueType, ErrorType>::Value() const&
{
    if (!m_hasValue) throw std::runtime_error("Cannot retrieve value: this Expected type has no value!");
    return m_value;
}

template <typename ValueType, typename ErrorType>
requires std::is_default_constructible_v<ValueType> && std::is_default_constructible_v<ErrorType>
inline constexpr ValueType&& Expected<ValueType, ErrorType>::Value() &&
{
    if (!m_hasValue) throw std::runtime_error("Cannot retrieve value: this Expected type has no value!");
    return std::move(m_value);
}

template <typename ValueType, typename ErrorType>
requires std::is_default_constructible_v<ValueType> && std::is_default_constructible_v<ErrorType>
inline constexpr const ValueType&& Expected<ValueType, ErrorType>::Value() const&&
{
    if (!m_hasValue) throw std::runtime_error("Cannot retrieve value: this Expected type has no value!");
    return std::move(m_value);
}

template <typename ValueType, typename ErrorType>
requires std::is_default_constructible_v<ValueType> && std::is_default_constructible_v<ErrorType>
inline constexpr ErrorType& Expected<ValueType, ErrorType>::Error() &
{
    if (m_hasValue) throw std::runtime_error("Cannot retrieve error: this Expected type has a value!");
    return m_error;
}

template <typename ValueType, typename ErrorType>
requires std::is_default_constructible_v<ValueType> && std::is_default_constructible_v<ErrorType>
inline constexpr const ErrorType& Expected<ValueType, ErrorType>::Error() const&
{
    if (m_hasValue) throw std::runtime_error("Cannot retrieve error: this Expected type has a value!");
    return m_error;
}

template <typename ValueType, typename ErrorType>
requires std::is_default_constructible_v<ValueType> && std::is_default_constructible_v<ErrorType>
inline constexpr ErrorType&& Expected<ValueType, ErrorType>::Error() &&
{
    if (m_hasValue) throw std::runtime_error("Cannot retrieve error: this Expected type has a value!");
    return std::move(m_error);
}

template <typename ValueType, typename ErrorType>
requires std::is_default_constructible_v<ValueType> && std::is_default_constructible_v<ErrorType>
inline constexpr const ErrorType&& Expected<ValueType, ErrorType>::Error() const&&
{
    if (m_hasValue) throw std::runtime_error("Cannot retrieve error: this Expected type has a value!");
    return std::move(m_error);
}

template <typename ValueType, typename ErrorType>
requires std::is_default_constructible_v<ValueType> && std::is_default_constructible_v<ErrorType>
inline constexpr Expected<ValueType, ErrorType>::operator bool() const noexcept
{
    return m_hasValue;
}

template <typename ValueType, typename ErrorType>
requires std::is_default_constructible_v<ValueType> && std::is_default_constructible_v<ErrorType>
inline constexpr bool Expected<ValueType, ErrorType>::HasValue() const noexcept
{
    return m_hasValue;
}
