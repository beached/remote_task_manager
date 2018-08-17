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

#include <vector>

#include "wmi_impl.h"
#include "wmi_process.h"

namespace daw {
	template<typename Integer>
	constexpr VARENUM get_integral_vt_type( ) noexcept {
		static_assert( std::is_integral_v<Integer>,
		               "UnsignedInteger must be an integer type" );

		if constexpr( std::is_signed_v<Integer> ) {
			switch( sizeof( Integer ) ) {
			case 1:
				return VT_I1;
			case 2:
				return VT_I2;
			case 4:
				return VT_I4;
			case 8:
				return VT_I8;
			default:
				std::terminate( );
			}
		} else {
			switch( sizeof( Integer ) ) {
			case 1:
				return VT_UI1;
			case 2:
				return VT_UI2;
			case 4:
				return VT_UI4;
			case 8:
				return VT_UI8;
			default:
				std::terminate( );
			}
		}
	}

	struct ArgVal {
		std::wstring name;

		ArgVal( std::wstring_view name_ );

		virtual VARENUM get_type( ) noexcept = 0;

		virtual ~ArgVal( ) = default;
		ArgVal( ArgVal const & ) = default;
		ArgVal( ArgVal && ) noexcept = default;
		ArgVal &operator=( ArgVal const & ) = default;
		ArgVal &operator=( ArgVal && ) noexcept = default;
	};

	template<typename Integer,
	         VARENUM val_size = get_integral_vt_type<Integer>( )>
	struct IntArg : public ArgVal {
		static_assert( std::is_integral_v<Integer>,
		               "UnsignedInteger must be an integer type" );
		static_assert( std::is_signed_v<Integer>,
		               "UnsignedInteger must be signed" );

		Integer value;

		IntArg( Integer val, std::wstring_view desc )
		  : ArgVal( desc )
		  , value( val ) {}

		VARENUM get_type( ) noexcept override {
			std::vector<int> a{};
			return val_size;
		}
	};

	template<typename UnsignedInteger,
	         VARENUM val_size = get_integral_vt_type<UnsignedInteger>( )>
	struct UIntArg : public ArgVal {
		static_assert( std::is_integral_v<UnsignedInteger>,
		               "UnsignedInteger must be an integer type" );
		static_assert( !std::is_signed_v<UnsignedInteger>,
		               "UnsignedInteger cannot be signed" );

		UnsignedInteger value;

		UIntArg( UnsignedInteger val, std::wstring_view desc )
		  : ArgVal( desc )
		  , value( val ) {}

		VARENUM get_type( ) noexcept override {
			return val_size;
		}
	};

	/*
	template<typename Float>
	struct FloatVal : public ArgVal {};
	template<typename... OutArgs, typename... InArgs>
	std::tuple<OutArgs...> exec_method( wmi_state_t &wmi_state,
	                                    std::wstring_view class_name,
	                                    std::wstring_view method_name,
	                                    std::initializer_list<std::wstring_view>
	out_params, InArgs &&... in_args ) {}
	                                    (
	                                    */
} // namespace daw
