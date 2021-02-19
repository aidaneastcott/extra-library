/*
	File:           extra.h
	Project:        Extra Library
	Programmer:     Aidan Eastcott
	Last Update:    2021-02-18
	Description:
		A header-only library containing miscellaneous utility macros, functions, and types for C and C++

	Macro Options:
		XTR_MINIMAL          Disables all functionality except macro definitions
		XTR_LOGGING          Enables debug_log function
		XTR_MULTIARRAY       Enables xtr::multiarray type in C++
		XTR_ENUMERATE        Enables xtr::enumerate function in C++
		XTR_NO_CONSTEXPR     Disables use of constexpr specifier in C++
*/

#pragma once
#ifndef EXTRA_H
#define EXTRA_H


// Extra library version
#define XTR_VERSION 2021'02'18


// Enable extra functionality
#if !defined(XTR_MINIMAL)
#define XTR_LOGGING
#define XTR_MULTIARRAY
#define XTR_ENUMERATE
#endif


// Include the correct headers for the language
#ifdef __cplusplus
#include <cassert>
#include <cstddef>
#else
#include <assert.h>
#include <stddef.h>
#include <stdbool.h>
#endif


// Logging headers
#if defined(XTR_LOGGING)
#if defined(__cplusplus)
#include <cstdio>
#else
#include <stdio.h>
#endif
#endif


// Multiarray headers
#if defined(XTR_MULTIARRAY) && defined(__cplusplus)
#include <array>
#endif


// Enumerate headers
#if defined(XTR_ENUMERATE) && defined(__cplusplus)
#include <type_traits>
#include <tuple>
#include <iterator>
#include <utility>
#endif


// Define empty xtr namespace in C++
#if defined(__cplusplus)
namespace xtr {}
#endif


// Detect compiler vendor
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#define XTR_COMPILER_MSVC
#elif defined(__GNUC__)
#define XTR_COMPILER_GNUC
#endif


// Namespace qualifier for C++
#if defined(__cplusplus)
#define XTR_NAMESPACE_STD ::std::
#else
#define XTR_NAMESPACE_STD
#endif


// Nodiscard attribute for C++
#if defined(__cplusplus) && (__cplusplus >= 201703L)
#define XTR_NODISCARD [[nodiscard]]
#else
#define XTR_NODISCARD
#endif


// Constexpr specifier for C++
#if defined(__cplusplus) && !defined(XTR_NO_CONSTEXPR)
#define XTR_CONSTEXPR constexpr
#else
#define XTR_CONSTEXPR
#endif


// Casting to void
#if defined(__cplusplus)
#define XTR_CAST_VOID(expression) static_cast<void>(expression)
#else
#define XTR_CAST_VOID(expression) ((void)(expression))
#endif


// Macros for meta string literal conversion
#if !defined(concat_string) && !defined(literal_string) && !defined(macro_string)

#define concat_string(string1, string2) string1 ## string2
#define literal_string(macro) #macro
#define macro_string(macro) literal_string(macro)

#endif // macro_string, literal_string, concat_string


// Assume macro for MSVC compiler specific __assume behavior
#if !defined(assume)

#if defined(XTR_COMPILER_MSVC)
#define assume(expression) __assume(!!(expression))
#elif defined(XTR_COMPILER_GNUC)
#define assume(expression) XTR_CAST_VOID(!!(expression) ? XTR_CAST_VOID(0) : __builtin_unreachable())
#else
#define assume(expression) XTR_CAST_VOID(0)
#endif

#endif // assume


// Macro to combine assert and assume macros
#if !defined(assert_assume)

#if defined(NDEBUG)
#define assert_assume(expression) do { assert(expression); assume(expression); } while(0)
#else
#define assert_assume(expression) assume(expression)
#endif

#endif // assert_assume


// Macros for GNU likely and unlikely builtins
#if !defined(likely) && !defined(unlikely)

#if defined(XTR_COMPILER_GNUC)
#define likely(expression) XTR_CAST_VOID(__builtin_expect(!!(expression), 1))
#define unlikely(expression) XTR_CAST_VOID(__builtin_expect(!!(expression), 0))
#else
#define likely(expression) XTR_CAST_VOID(0)
#define unlikely(expression) XTR_CAST_VOID(0)
#endif

#endif // likely, unlikely


// Macro to portably enable MSVC compiler specific __restrict in C++
#if !defined(restrict) && defined(__cplusplus)

#if defined(XTR_COMPILER_MSVC)
#define restrict  __restrict
#elif defined(XTR_COMPILER_GNUC)
#define restrict __restrict__
#else
#define restrict
#endif

#endif // restrict


// Debug logging macro in the style of assert()
#if defined(XTR_LOGGING)

#if !defined(debug_log)

#if defined(NDEBUG)
#define debug_log(message) XTR_CAST_VOID(0)
#else
#define debug_log(message) XTR_NAMESPACE_STD puts("Debug message: " message ", file " __FILE__ ", line " macro_string(__LINE__) "\n")
#endif

#endif // debug_log

#endif // XTR_LOGGING


// Multidimensional array alias for std::array in C++
#if defined(XTR_MULTIARRAY) && defined(__cplusplus)

namespace xtr {

namespace detail {

template <typename T, ::std::size_t I, ::std::size_t... J>
struct multiarray_base {
	using nested = typename multiarray_base<T, J...>::type;
	using type = ::std::array<nested, I>;
};

template <typename T, ::std::size_t I>
struct multiarray_base<T, I> {
	using type = ::std::array<T, I>;
};

} // namespace xtr::detail

template <typename T, ::std::size_t I, ::std::size_t... J>
using multiarray = typename detail::multiarray_base<T, I, J...>::type;

} // namespace xtr

#endif // XTR_MULTIARRAY


// Enumerate function for ranged for loops in C++
#if defined(XTR_ENUMERATE) && defined(__cplusplus)

namespace xtr {

namespace detail {

template <typename, typename = void>
struct get_index_type {
	using type = ::std::size_t;
};

template <typename T>
struct get_index_type<T, ::std::void_t<typename T::size_type>> {
	using type = typename T::size_type;
};

template <typename T>
using get_index_type_t = typename get_index_type<T>::type;


template <typename T>
using get_iterable_type_t = ::std::conditional_t<::std::is_rvalue_reference_v<T>, ::std::remove_reference_t<T>, T>;


template <typename Iterable>
struct enumerator_base {

	using iterable_type = Iterable;

	get_iterable_type_t<iterable_type> m_iterable;

	XTR_CONSTEXPR enumerator_base(iterable_type iterable)
		noexcept(::std::conditional_t<::std::is_reference_v<iterable_type>,
			::std::true_type, ::std::is_nothrow_move_constructible<iterable_type>>::value) :
		m_iterable{::std::forward<iterable_type>(iterable)} {}
};

template <typename Iterable, typename Index>
struct enumerator : enumerator_base<Iterable> {

	using iterable_type = typename enumerator_base<Iterable>::iterable_type;
	using index_type = Index;

	XTR_CONSTEXPR enumerator(iterable_type iterable)
		noexcept(::std::is_nothrow_constructible_v<
			enumerator_base<Iterable>, ::std::add_rvalue_reference_t<iterable_type>>) :
		enumerator_base<Iterable>{::std::forward<iterable_type>(iterable)} {}
};

template <typename Iterator>
struct indexed_iterator_base {

	using iterator_type = Iterator;

	iterator_type m_iterator;

	XTR_CONSTEXPR indexed_iterator_base(iterator_type iterator)
		noexcept(::std::conditional_t<::std::is_reference_v<iterator_type>,
			::std::true_type, ::std::is_nothrow_move_constructible<iterator_type>>::value) :
		m_iterator{::std::forward<iterator_type>(iterator)} {}

	template <typename EndIterator>
	XTR_CONSTEXPR auto operator!=(EndIterator &&end) const
		noexcept(noexcept(m_iterator != end))
		-> decltype(m_iterator != end) {
		return m_iterator != end;
	}
};

template <typename Iterator, typename Index>
struct indexed_iterator : indexed_iterator_base<Iterator> {

	using iterator_type = typename indexed_iterator_base<Iterator>::iterator_type;
	using index_type = Index;

	index_type m_index;

	XTR_CONSTEXPR indexed_iterator(iterator_type iterator)
		noexcept(::std::is_nothrow_default_constructible_v<index_type> &&
			::std::is_nothrow_constructible_v<
				indexed_iterator_base<Iterator>, ::std::add_rvalue_reference_t<iterator_type>>) :
		indexed_iterator_base<Iterator>{::std::forward<iterator_type>(iterator)}, m_index{} {}

	XTR_CONSTEXPR void operator++()
		noexcept(noexcept(++indexed_iterator_base<Iterator>::m_iterator) && noexcept(++m_index)) {
		++indexed_iterator_base<Iterator>::m_iterator;
		++m_index;
	}

	XTR_CONSTEXPR auto operator*()
		noexcept(noexcept(*indexed_iterator_base<Iterator>::m_iterator) &&
			::std::is_nothrow_constructible_v<
				::std::tuple<::std::add_const_t<index_type>, decltype(*indexed_iterator_base<Iterator>::m_iterator)>,
				index_type, decltype(*indexed_iterator_base<Iterator>::m_iterator)>)
		-> ::std::tuple<::std::add_const_t<index_type>, decltype(*indexed_iterator_base<Iterator>::m_iterator)> {
		return {m_index, *indexed_iterator_base<Iterator>::m_iterator};
	}
};


using ::std::begin;
using ::std::end;

template <typename Iterable, typename Index>
XTR_CONSTEXPR auto begin(enumerator<Iterable, Index> &object)
	noexcept(noexcept(begin(object.m_iterable)) &&
		::std::is_nothrow_constructible_v<
			indexed_iterator<decltype(begin(object.m_iterable)), Index>, decltype(begin(object.m_iterable))>)
	-> indexed_iterator<decltype(begin(object.m_iterable)), Index> {
	return {begin(object.m_iterable)};
}

template <typename Iterable, typename Index>
XTR_CONSTEXPR auto end(enumerator<Iterable, Index> &object)
	noexcept(noexcept(end(object.m_iterable)))
	-> decltype(end(object.m_iterable)) {
	return end(object.m_iterable);
}

} // namespace detail

template <typename Iterable,
	typename Index = detail::get_index_type_t<::std::remove_reference_t<Iterable>>>
XTR_NODISCARD XTR_CONSTEXPR detail::enumerator<::std::add_rvalue_reference_t<Iterable>, Index>
enumerate(Iterable &&iterable)
	noexcept(::std::is_nothrow_constructible_v<
		detail::enumerator<::std::add_rvalue_reference_t<Iterable>, Index>,
		::std::add_rvalue_reference_t<Iterable>>) {
	return {::std::forward<Iterable>(iterable)};
}

} // namespace xtr

#endif // XTR_ENUMERATE


#endif // EXTRA_H
