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

#include <combaseapi.h>
#include <utility>

#include "daw/wmi_impl.h"

namespace daw {
	wmi_error_t::wmi_error_t( char const *msg, long err_code )
	  : std::exception( msg )
	  , code( err_code ) {}

	wmi_state_co::wmi_state_co( DWORD const dwCoInit ) {
		if( auto const hres = CoInitializeEx( nullptr, dwCoInit );
		    FAILED( hres ) ) {
			throw wmi_error_t{"Failed to init COM library", hres};
		}
		has_init = true;
	}

	wmi_state_co::~wmi_state_co( ) {
		if( std::exchange( has_init, false ) ) {
			CoUninitialize( );
		}
	}

	wmi_state_t::wmi_state_t( DWORD co_init )
	  : init( co_init ) {

		static auto const sec_res = []( ) {
			// Can only be called once per process
			auto const hres = CoInitializeSecurity(
			  nullptr,
			  -1,                          // COM authentication
			  nullptr,                     // Authentication services
			  nullptr,                     // Reserved
			  RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication
			  RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation
			  nullptr,                     // Authentication info
			  EOAC_NONE,                   // Additional capabilities
			  nullptr                      // Reserved
			);
			if( FAILED( hres ) ) {
				throw wmi_error_t{"Failed to initialize security", hres};
			}
			return hres;
		}( );
		auto hres = CoCreateInstance( CLSID_WbemLocator, nullptr,
		                              CLSCTX_INPROC_SERVER, IID_IWbemLocator,
		                              reinterpret_cast<LPVOID *>( &locator ) );

		if( FAILED( hres ) ) {
			CoUninitialize( );
			throw wmi_error_t{"Failed to create IWbemLocator object",
			                  hres};
		}
	}

	void wmi_state_t::connect( std::wstring const &path, std::wstring machine ) {
		if( machine.empty( ) ) {
			machine = path;
		} else {
			machine = L"\\\\" + machine + L"\\" + path;
		}

		auto const result = locator->ConnectServer(
		  _bstr_t( machine.c_str( ) ), // Object path of WMI namespace
		  nullptr,                     // User name. NULL = current user
		  nullptr,                     // User password. NULL = current
		  nullptr,                     // Locale. NULL indicates current
		  0,                           // Security flags.
		  nullptr,                     // Authority (for example, Kerberos)
		  nullptr,                     // Context object
		  &service                     // pointer to IWbemServices proxy
		);
		if( FAILED( result ) ) {
			throw wmi_error_t{"Could not connect", result};
		}
		set_proxy_blanket( );
	}

	void wmi_state_t::set_proxy_blanket( ) {
		auto const result =
		  CoSetProxyBlanket( service,                // Indicates the proxy to set
		                     RPC_C_AUTHN_WINNT,      // RPC_C_AUTHN_xxx
		                     RPC_C_AUTHZ_NONE,       // RPC_C_AUTHZ_xxx
		                     nullptr,                // Server principal name
		                     RPC_C_AUTHN_LEVEL_CALL, // RPC_C_AUTHN_LEVEL_xxx
		                     RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
		                     nullptr,                     // client identity
		                     EOAC_NONE                    // proxy capabilities
		  );
		if( FAILED( result ) ) {
			throw wmi_error_t{"Could not set proxy blanket", result};
		}
	}

	CComPtr<IEnumWbemClassObject>
	wmi_state_t::query( std::wstring const &query_str ) {
		CComPtr<IEnumWbemClassObject> result;
		auto const hres = service->ExecQuery(
		  bstr_t( "WQL" ), bstr_t( query_str.c_str( ) ),
		  WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, nullptr, &result );

		if( FAILED( hres ) ) {
			throw wmi_error_t{"Query for Win32_Process failed", hres};
		}
		return result;
	}
} // namespace daw
