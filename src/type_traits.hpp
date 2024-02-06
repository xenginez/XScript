#pragma once

#include <any>
#include <map>
#include <span>
#include <list>
#include <array>
#include <string>
#include <vector>

namespace x
{
    template<typename T> struct is_null : public std::is_same<T, std::nullptr_t>
    {
    };
    template<typename T> struct is_null<T &> : public is_null<T>
    {
    };
    template<typename T> struct is_null<const T> : public is_null<T>
    {
    };
    template<typename T> static constexpr bool is_null_v = is_null<T>::value;

    template<typename T> struct is_byte : public std::conditional_t< std::is_same_v<T, uint8_t> || std::is_same_v<T, std::byte>, std::true_type, std::false_type >
    {
    };
    template<typename T> struct is_byte<T &> : public is_byte<T>
    {
    };
    template<typename T> struct is_byte<const T> : public is_byte<T>
    {
    };
    template<typename T> static constexpr bool is_byte_v = is_byte<T>::value;

    template<typename T> struct is_boolean : public std::is_same<T, bool>
    {
    };
    template<typename T> struct is_boolean<T &> : public is_boolean<T>
    {
    };
    template<typename T> struct is_boolean<const T> : public is_boolean<T>
    {
    };
    template<typename T> static constexpr bool is_boolean_v = is_boolean<T>::value;

    template<typename T> struct is_integer : public std::is_integral<T>
    {
    };
    template<typename T> struct is_integer<T &> : public is_integer<T>
    {
    };
    template<typename T> struct is_integer<const T> : public is_integer<T>
    {
    };
    template<typename T> static constexpr bool is_integer_v = is_integer<T>::value;

    template<typename T> struct is_floating : public std::is_floating_point<T>
    {
    };
    template<typename T> struct is_floating<T &> : public is_floating<T>
    {
    };
    template<typename T> struct is_floating<const T> : public is_floating<T>
    {
    };
    template<typename T> static constexpr bool is_floating_v = is_floating<T>::value;



    template<typename T> struct is_basic_string : public std::false_type
    {
    };
    template<typename T> struct is_basic_string<T &> : public is_basic_string<T>
    {
    };
    template<typename T> struct is_basic_string<const T> : public is_basic_string<T>
    {
    };
    template<typename ... T> struct is_basic_string< std::basic_string<T...> > : public std::true_type
    {
    };
    template<typename T> static constexpr bool is_basic_string_v = is_basic_string<T>::value;

    template<typename T> struct is_basic_string_view : public std::false_type
    {
    };
    template<typename T> struct is_basic_string_view<T &> : public is_basic_string_view<T>
    {
    };
    template<typename T> struct is_basic_string_view<const T> : public is_basic_string_view<T>
    {
    };
    template<typename ... T> struct is_basic_string_view< std::basic_string_view<T...> > : public std::true_type
    {
    };
    template<typename T> static constexpr bool is_basic_string_view_v = is_basic_string_view<T>::value;

    template<typename T> struct is_string : public std::false_type
    {
    };
    template<typename T> struct is_string<T &> : public is_string<T>
    {
    };
    template<typename T> struct is_string<const T> : public is_string<T>
    {
    };
    template<> struct is_string<char *> : public std::true_type
    {
    };
    template<typename T> static constexpr bool is_string_v = is_string<T>::value || is_basic_string_v<T> || is_basic_string_view_v<T>;
    


    template<typename T> struct is_vector : public std::false_type
    {
    };
    template<typename T> struct is_vector<T &> : public is_vector<T>
    {
    };
    template<typename T> struct is_vector<const T> : public is_vector<T>
    {
    };
    template<typename ... T> struct is_vector< std::vector<T...> > : public std::true_type
    {
    };
    template<typename T> static constexpr bool is_vector_v = is_vector<T>::value;

    template<typename T> struct is_span : public std::false_type
    {
    };
    template<typename T> struct is_span<T &> : public is_span<T>
    {
    };
    template<typename T> struct is_span<const T> : public is_span<T>
    {
    };
    template<typename T, size_t N> struct is_span< std::span<T, N> > : public std::true_type
    {
    };
    template<typename T> static constexpr bool is_span_v = is_span<T>::value;

    template<typename T> struct is_array : public std::is_array<T>
    {
    };
    template<typename T> struct is_array<T &> : public is_array<T>
    {
    };
    template<typename T> struct is_array<const T> : public is_array<T>
    {
    };
    template<typename T, size_t N> struct is_array< std::array<T, N> > : public std::true_type
    {
    };
    template<typename T> static constexpr bool is_array_v = is_array<T>::value || is_vector_v<T> || is_span_v<T>;

}
