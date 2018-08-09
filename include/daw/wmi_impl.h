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

#include <atlcomcli.h>
#include <comdef.h>
#include <string>
#include <wbemidl.h>

namespace daw {
	struct wmi_error_t {
		std::wstring message;
		long code;
	};

	struct wmi_state_co {
		bool has_init = false;

		wmi_state_co( DWORD const dwCoInit );
		wmi_state_co( wmi_state_co const & ) noexcept = default;	
		wmi_state_co( wmi_state_co && ) noexcept = default;	
		wmi_state_co &operator=( wmi_state_co const & ) noexcept = default;	
		wmi_state_co &operator=( wmi_state_co && ) noexcept = default;	
		~wmi_state_co( );
	};

	struct wmi_state_t {
		CComPtr<IWbemLocator> locator = nullptr;
		CComPtr<IWbemServices> service = nullptr;
		wmi_state_co init;

		wmi_state_t( wmi_state_t && ) noexcept = default;
		wmi_state_t &operator=( wmi_state_t && ) noexcept = default;
		wmi_state_t( wmi_state_t const & ) = delete;
		wmi_state_t &operator=( wmi_state_t const & ) = delete;
		~wmi_state_t( ) = default;

		wmi_state_t( DWORD const dwCoInit );
		void connect( std::wstring const &path, std::wstring machine = L"" ); 
		void set_proxy_blanket( ); 
		CComPtr<IEnumWbemClassObject> query( std::wstring const &query_str ); 
	};
} // namespace daw
