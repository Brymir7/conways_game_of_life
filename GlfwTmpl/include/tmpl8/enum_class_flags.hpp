#pragma once

#include <type_traits>

namespace tmpl8
{
	template <typename T>
	struct is_enum_class_flags : std::false_type { };

	template <typename T> typename std::enable_if<is_enum_class_flags<T>::value, T>::type operator&(T a, T b)
	{ return static_cast<T>(static_cast<typename std::underlying_type<T>::type>(a) & static_cast<typename std::underlying_type<T>::type>(b)); }
	template <typename T> typename std::enable_if<is_enum_class_flags<T>::value, T>::type operator|(T a, T b)
	{ return static_cast<T>(static_cast<typename std::underlying_type<T>::type>(a) | static_cast<typename std::underlying_type<T>::type>(b)); }
	template <typename T> typename std::enable_if<is_enum_class_flags<T>::value, T>::type operator^(T a, T b)
	{ return static_cast<T>(static_cast<typename std::underlying_type<T>::type>(a) ^ static_cast<typename std::underlying_type<T>::type>(b)); }

	template <typename T> typename std::enable_if<is_enum_class_flags<T>::value, T&>::type operator&=(T& a, T b) { return a = a & b; }
	template <typename T> typename std::enable_if<is_enum_class_flags<T>::value, T&>::type operator|=(T& a, T b) { return a = a | b; }
	template <typename T> typename std::enable_if<is_enum_class_flags<T>::value, T&>::type operator^=(T& a, T b) { return a = a ^ b; }

	template <typename T> typename std::enable_if<is_enum_class_flags<T>::value, bool>::type operator!(T t)
	{ return !static_cast<typename std::underlying_type<T>::type>(t); }
	template <typename T> typename std::enable_if<is_enum_class_flags<T>::value, T>::type operator~(T t) 
	{ return static_cast<T>(~static_cast<typename std::underlying_type<T>::type>(t)); }
}
