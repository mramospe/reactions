/*! \file
 * \brief Conversion of C++ types to python types
 */
#pragma once

#include <optional>
#include <string>
#include <tuple>

/// Define the conversion of a C++ type to a python type
template <class T> struct cpp_to_py_type;

/// Define compile-time attributes to convert floating point numbers
template <> struct cpp_to_py_type<float> {
  static constexpr char descriptor = 'f';
  using type = float;
};

/// Define compile-time attributes to convert floating point numbers
template <> struct cpp_to_py_type<double> {
  static constexpr char descriptor = 'd';
  using type = double;
};

/// Define compile-time attributes to convert integral numbers
template <> struct cpp_to_py_type<int> {
  static constexpr char descriptor = 'i';
  using type = int;
};

/// Define compile-time attributes to convert boolean numbers
template <> struct cpp_to_py_type<bool> {
  static constexpr char descriptor = 'p';
  using type = int;
};

/// Define compile-time attributes to convert strings
template <> struct cpp_to_py_type<std::string> {
  static constexpr char descriptor = 's';
  using type = const char *;
};

/// Define compile-time attributes to convert optional objects
template <class T> struct cpp_to_py_type<std::optional<T>> {
  static constexpr char descriptor = 'O'; // object
  using type = std::optional<T>;
};

/// Define compile-time attributes to convert tuple-based objects
template <class... T> struct cpp_to_py_type<std::tuple<T...>> {
  // descriptor when parsing only "args"
  static constexpr char descriptor_args[] = {cpp_to_py_type<T>::descriptor...,
                                             '\0'};
  // descriptor when parsing only "kwargs"
  static constexpr char descriptor_kwargs[] = {
      '|', '$', cpp_to_py_type<T>::descriptor..., '\0'};
  using type = std::tuple<typename cpp_to_py_type<T>::type...>;
};

/// Alias for a python type associated to a C++ type
template <class T> using cpp_to_py_type_t = typename cpp_to_py_type<T>::type;

/// Alias for a python descriptor associated to a C++ type
template <class T>
static constexpr auto cpp_to_py_type_d = cpp_to_py_type<T>::descriptor;
