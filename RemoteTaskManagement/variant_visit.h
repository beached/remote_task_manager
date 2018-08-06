// MIT License
//
// Copyright (c) 2018 Darrell Wright
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
#pragma once

#include <cstddef>
#include <exception>
#include <type_traits>
#include <variant>
#include <wtypes.h>

namespace daw {
	template<class... Ts>
	struct overloaded_t {};

	template<class T0>
	struct overloaded_t<T0> : T0 {
		using T0::operator( );

		constexpr overloaded_t( T0 &&t0 )
		  : T0( std::forward<T0>( t0 ) ) {}
	};

	template<class T0, class T1, class... Ts>
	struct overloaded_t<T0, T1, Ts...> : T0, overloaded_t<T1, Ts...> {
		using T0::operator( );
		using overloaded_t<T1, Ts...>::operator( );

		constexpr overloaded_t( T0 &&t0, T1 &&t1, Ts &&... ts )
		  : T0( std::move( t0 ) )
		  , overloaded_t<T1, Ts...>( std::forward<T1>( t1 ),
		                             std::forward<Ts>( ts )... ) {}
	};

	template<class... Ts>
	constexpr overloaded_t<std::decay_t<Ts>...> overload( Ts &&... ts ) {
		return {std::forward<Ts>( ts )...};
	}

	// A visitor for Win32 VARIANT

	namespace impl {
		struct empty_variant {};
		struct null_variant {};

		template<typename T>
		constexpr bool not_void = !std::is_same_v<void, std::decay_t<T>>;

		// VT_EMPTY
		template<
		  typename Result, typename Visitor,
		  std::enable_if_t<(not_void<Result> && std::is_invocable_v<Visitor>),
		                   std::nullptr_t> = nullptr>
		constexpr Result variant_visitor( empty_variant, Visitor &&visitor ) {
			return static_cast<Result>( visitor( ) );
		}

		// Others
		template<typename Result, typename Value, typename Visitor,
		         typename std::enable_if_t<
		           (not_void<Result> &&
		            !std::is_same_v<std::decay_t<Value>, empty_variant> &&
		            std::is_invocable_v<Visitor, Value>),
		           std::nullptr_t> = nullptr>
		constexpr Result variant_visitor( Value &&value, Visitor &&visitor ) {
			return static_cast<Result>(
			  std::forward<Visitor>( visitor )( std::forward<Value>( value ) ) );
		}

		template<typename Result>
		Result variant_visitor( ... ) {
			throw std::bad_variant_access( );
		}
	} // namespace impl

	template<typename Result, typename Variant, typename... Visitors,
	         std::enable_if_t<!std::is_same_v<void, std::decay_t<Result>>,
	                          std::nullptr_t> = nullptr>
	constexpr Result variant_visit( Variant &&var, Visitors &&... visitors ) {
		auto v = overload( std::forward<Visitors>( visitors )... );
		switch( var.vt ) {
		case VT_EMPTY:
		case VT_NULL:
			return impl::variant_visitor<Result>( impl::empty_variant{},
			                                      std::move( v ) );
		case VT_I2:
			return impl::variant_visitor<Result>( std::forward<Variant>( var ).iVal,
			                                      std::move( v ) );
		case VT_I4:
			return impl::variant_visitor<Result>( std::forward<Variant>( var ).lVal,
			                                      std::move( v ) );
		case VT_R4:
			return impl::variant_visitor<Result>( std::forward<Variant>( var ).fltVal,
			                                      std::move( v ) );
		case VT_R8:
			return impl::variant_visitor<Result>( std::forward<Variant>( var ).dblVal,
			                                      std::move( v ) );
		case VT_CY:
			return impl::variant_visitor<Result>( std::forward<Variant>( var ).cyVal,
			                                      std::move( v ) );
		case VT_DATE:
			return impl::variant_visitor<Result>( std::forward<Variant>( var ).date,
			                                      std::move( v ) );
		case VT_BSTR:
			return impl::variant_visitor<Result>(
			  std::forward<Variant>( var ).bstrVal, std::move( v ) );
		case VT_DISPATCH:
			return impl::variant_visitor<Result>(
			  std::forward<Variant>( var ).pdispVal, std::move( v ) );
		case VT_ERROR:
			return impl::variant_visitor<Result>( std::forward<Variant>( var ).scode,
			                                      std::move( v ) );
		case VT_BOOL:
			return impl::variant_visitor<Result>(
			  std::forward<Variant>( var ).boolVal, std::move( v ) );
		case VT_VARIANT:
			return impl::variant_visitor<Result>(
			  std::forward<Variant>( var ).pvarVal, std::move( v ) );
		case VT_UNKNOWN:
			return impl::variant_visitor<Result>(
			  std::forward<Variant>( var ).punkVal, std::move( v ) );
		case VT_DECIMAL:
			return impl::variant_visitor<Result>( std::forward<Variant>( var ).decVal,
			                                      std::move( v ) );
		case VT_I1:
			return impl::variant_visitor<Result>( std::forward<Variant>( var ).cVal,
			                                      std::move( v ) );
		case VT_UI1:
			return impl::variant_visitor<Result>( std::forward<Variant>( var ).bVal,
			                                      std::move( v ) );
		case VT_UI2:
			return impl::variant_visitor<Result>( std::forward<Variant>( var ).uiVal,
			                                      std::move( v ) );
		case VT_UI4:
			return impl::variant_visitor<Result>( std::forward<Variant>( var ).ulVal,
			                                      std::move( v ) );
		case VT_I8:
			return impl::variant_visitor<Result>( std::forward<Variant>( var ).llVal,
			                                      std::move( v ) );
		case VT_UI8:
			return impl::variant_visitor<Result>( std::forward<Variant>( var ).ullVal,
			                                      std::move( v ) );
		case VT_INT:
			return impl::variant_visitor<Result>( std::forward<Variant>( var ).intVal,
			                                      std::move( v ) );
		case VT_UINT:
			return impl::variant_visitor<Result>(
			  std::forward<Variant>( var ).uintVal, std::move( v ) );
		case VT_RECORD:
			return impl::variant_visitor<Result>(
			  std::forward<Variant>( var ).pvRecord, std::move( v ) );
		case VT_ARRAY:
			return impl::variant_visitor<Result>( std::forward<Variant>( var ).parray,
			                                      std::move( v ) );
		default:
			// Unknown Type
			std::terminate( );
		}
	}
} // namespace daw
