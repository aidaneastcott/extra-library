/*
	File:           extra.h
	Project:        Extra Library
	Programmer:     Aidan Eastcott
	Last Update:    2021-02-19
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
#define XTR_VERSION 2021'02'19


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

template <typename Type, ::std::size_t I, ::std::size_t ...J>
struct multiarray_base {
	using nested = typename multiarray_base<Type, J...>::type;
	using type = ::std::array<nested, I>;
};

template <typename Type, ::std::size_t I>
struct multiarray_base<Type, I> {
	using type = ::std::array<Type, I>;
};

} // namespace xtr::detail

template <typename Type, ::std::size_t I, ::std::size_t ...J>
using multiarray = typename detail::multiarray_base<Type, I, J...>::type;

} // namespace xtr

#endif // XTR_MULTIARRAY


// Enumerate function for ranged for loops in C++
#if defined(XTR_ENUMERATE) && defined(__cplusplus)

namespace xtr {

namespace detail {

template <typename T>
using get_stored_t = ::std::conditional_t<::std::is_rvalue_reference_v<T>, ::std::remove_reference_t<T>, T>;

template <typename Iterable>
struct enumerator_base {

	using iterable_type = Iterable;

	static_assert(::std::is_reference_v<iterable_type>, "iterable_type must be a reference");

	get_stored_t<iterable_type> m_iterable;


	using is_constructor_noexcept =
		::std::conditional_t<::std::is_reference_v<get_stored_t<iterable_type>>,
		::std::true_type, ::std::is_nothrow_move_constructible<get_stored_t<iterable_type>>>;

	XTR_CONSTEXPR enumerator_base(iterable_type iterable)
		noexcept(is_constructor_noexcept::value) :
		m_iterable{::std::forward<iterable_type>(iterable)} {}


	template <typename ...Parameters>
	using is_variadic_constructor_noexcept =
		::std::is_nothrow_constructible<
		get_stored_t<iterable_type>, ::std::add_rvalue_reference_t<Parameters>...>;

	template <typename ...Parameters>
	XTR_CONSTEXPR enumerator_base(Parameters &&...parameters)
		noexcept(is_variadic_constructor_noexcept<Parameters...>::value) :
		m_iterable{::std::forward<Parameters>(parameters)...} {}
};

template <typename Iterable, typename Index>
struct enumerator : enumerator_base<Iterable> {

	using iterable_type = typename enumerator_base<Iterable>::iterable_type;
	using index_type = Index;

	static_assert(::std::negation_v<::std::is_reference<index_type>>, "index_type must not be a reference");


	using is_constructor_noexcept =
		::std::is_nothrow_constructible<
		enumerator_base<Iterable>, ::std::add_rvalue_reference_t<iterable_type>>;

	XTR_CONSTEXPR enumerator(iterable_type iterable)
		noexcept(is_constructor_noexcept::value) :
		enumerator_base<Iterable>{::std::forward<iterable_type>(iterable)} {}


	template <typename ...Parameters>
	using is_variadic_constructor_noexcept =
		::std::is_nothrow_constructible<
		enumerator_base<Iterable>, ::std::add_rvalue_reference_t<Parameters>...>;

	template <typename ...Parameters>
	XTR_CONSTEXPR enumerator(Parameters &&...parameters)
		noexcept(is_variadic_constructor_noexcept<Parameters...>::value) :
		enumerator_base<Iterable>{::std::forward<Parameters>(parameters)...} {}
};

template <typename Iterator>
struct indexed_iterator_base {

	using iterator_type = Iterator;

	static_assert(::std::is_reference_v<iterator_type>, "iterator_type must be a reference");

	get_stored_t<iterator_type> m_iterator;


	using is_constructor_noexcept =
		::std::conditional_t<::std::is_reference_v<get_stored_t<iterator_type>>,
		::std::true_type, ::std::is_nothrow_move_constructible<get_stored_t<iterator_type>>>;

	XTR_CONSTEXPR indexed_iterator_base(iterator_type iterator)
		noexcept(is_constructor_noexcept::value) :
		m_iterator{::std::forward<iterator_type>(iterator)} {}


	template <typename EndIterator>
	using compare_result_type =
		decltype(::std::declval<::std::add_lvalue_reference_t<iterator_type>>() != ::std::declval<EndIterator>());

	template <typename EndIterator>
	using is_compare_noexcept =
		::std::bool_constant<noexcept(::std::declval<::std::add_lvalue_reference_t<iterator_type>>() != ::std::declval<EndIterator>())>;

	template <typename EndIterator>
	XTR_CONSTEXPR compare_result_type<EndIterator> operator!=(EndIterator &&end) const
		noexcept(is_compare_noexcept<EndIterator>::value) {
		return m_iterator != ::std::forward<EndIterator>(end);
	}
};

template <typename Iterator, typename Index>
struct indexed_iterator : indexed_iterator_base<Iterator> {

	using iterator_type = typename indexed_iterator_base<Iterator>::iterator_type;
	using index_type = Index;

	static_assert(::std::negation_v<::std::is_reference<index_type>>, "index_type must not be a reference");

	index_type m_index;


	using is_constructor_noexcept =
		::std::conjunction<
		::std::is_nothrow_default_constructible<index_type>,
		::std::is_nothrow_constructible<indexed_iterator_base<Iterator>, ::std::add_rvalue_reference_t<iterator_type>>>;

	XTR_CONSTEXPR indexed_iterator(iterator_type iterator)
		noexcept(is_constructor_noexcept::value) :
		indexed_iterator_base<Iterator>{::std::forward<iterator_type>(iterator)},
		m_index{} {}


	using is_increment_noexcept =
		::std::conjunction<
		::std::bool_constant<noexcept(++::std::declval<::std::add_lvalue_reference_t<iterator_type>>())>,
		::std::bool_constant<noexcept(++::std::declval<::std::add_lvalue_reference_t<index_type>>())>>;

	XTR_CONSTEXPR void operator++()
		noexcept(is_increment_noexcept::value) {
		++indexed_iterator_base<Iterator>::m_iterator;
		++m_index;
	}


	using dereference_result_type =
		::std::tuple<::std::add_const_t<index_type>,
		decltype(*::std::declval<::std::add_lvalue_reference_t<iterator_type>>())>;

	using is_dereference_noexcept =
		::std::conjunction<
		::std::bool_constant<noexcept(*::std::declval<::std::add_lvalue_reference_t<iterator_type>>())>,
		::std::is_nothrow_constructible<dereference_result_type, index_type,
		decltype(*::std::declval<::std::add_lvalue_reference_t<iterator_type>>())>>;

	XTR_CONSTEXPR dereference_result_type operator*()
		noexcept(is_dereference_noexcept::value) {
		return {m_index, *indexed_iterator_base<Iterator>::m_iterator};
	}
};


using ::std::begin;
using ::std::end;


template <typename Iterable, typename Index>
using begin_result_type =
	indexed_iterator<::std::add_rvalue_reference_t<
	decltype(begin(::std::declval<::std::add_lvalue_reference_t<Iterable>>()))>, Index>;

template <typename Iterable, typename Index>
using is_begin_noexcept =
	::std::conjunction<
	::std::bool_constant<noexcept(begin(::std::declval<::std::add_lvalue_reference_t<Iterable>>()))>,
	::std::is_nothrow_constructible<begin_result_type<Iterable, Index>,
	decltype(begin(::std::declval<::std::add_lvalue_reference_t<Iterable>>()))>>;

template <typename Iterable, typename Index>
XTR_CONSTEXPR begin_result_type<Iterable, Index>
begin(enumerator<Iterable, Index> &object)
	noexcept(is_begin_noexcept<Iterable, Index>::value) {
	return {begin(object.m_iterable)};
}


template <typename Iterable>
using end_result_type =
	decltype(end(::std::declval<::std::add_lvalue_reference_t<Iterable>>()));

template <typename Iterable>
using is_end_noexcept =
	::std::bool_constant<noexcept(end(::std::declval<::std::add_lvalue_reference_t<Iterable>>()))>;

template <typename Iterable>
XTR_CONSTEXPR end_result_type<Iterable>
end(enumerator_base<Iterable> &object)
	noexcept(is_end_noexcept<Iterable>::value) {
	return end(object.m_iterable);
}


template <typename, typename = void>
struct get_index {
	using type = ::std::size_t;
};

template <typename Iterable>
struct get_index<Iterable, ::std::void_t<typename ::std::remove_reference_t<Iterable>::size_type>> {
	using type = typename ::std::remove_reference_t<Iterable>::size_type;
};

template <typename Iterable>
using get_index_t = typename get_index<Iterable>::type;


template <typename Iterable, typename Index>
using enumerate_result_type =
enumerator<Iterable, Index>;

template <typename Iterable, typename Index>
using is_enumerate_noexcept =
	::std::is_nothrow_constructible<
	enumerate_result_type<::std::add_rvalue_reference_t<Iterable>, Index>,
	::std::add_rvalue_reference_t<Iterable>>;

template <typename Iterable, typename Index, typename ...Parameters>
using is_variadic_enumerate_noexcept =
	::std::is_nothrow_constructible<
	enumerate_result_type<::std::add_rvalue_reference_t<Iterable>, Index>,
	::std::add_rvalue_reference_t<Parameters>...>;

} // namespace detail


template <typename Iterable, typename Index = detail::get_index_t<Iterable>>
XTR_NODISCARD XTR_CONSTEXPR detail::enumerate_result_type<::std::add_rvalue_reference_t<Iterable>, Index>
enumerate(Iterable &&iterable)
	noexcept(detail::is_enumerate_noexcept<Iterable, Index>::value) {
	return {::std::forward<Iterable>(iterable)};
}

template <typename Iterable, typename Index = detail::get_index_t<Iterable>, typename ...Parameters>
XTR_NODISCARD XTR_CONSTEXPR detail::enumerate_result_type<::std::add_rvalue_reference_t<Iterable>, Index>
enumerate(Parameters &&...parameters)
	noexcept(detail::is_variadic_enumerate_noexcept<Iterable, Index, Parameters...>::value) {
	return {::std::forward<Parameters>(parameters)...};
}

} // namespace xtr

#endif // XTR_ENUMERATE


#endif // EXTRA_H
