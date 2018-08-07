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
#include <array>
#include <atlcomcli.h>
#include <chrono>
#include <comdef.h>
#include <string>
#include <utility>
#include <vector>
#include <wbemidl.h>
#include <wx/datetime.h>
#include <wx/string.h>

#include "daw/variant_visit.h"
#include "daw/wmi_process.h"

#pragma comment( lib, "wbemuuid.lib" )

namespace daw {

	std::array<wxString, 14> const wmi_process::column_names = []( ) {
		std::array<wxString, 14> result;
		result[static_cast<size_t>( column_number::Name )] = L"Name";
		result[static_cast<size_t>( column_number::ProcessId )] = L"Process Id";
		result[static_cast<size_t>( column_number::ParentProcessId )] =
		  L"Parent Process Id";
		result[static_cast<size_t>( column_number::SessionId )] = L"Session Id";
		result[static_cast<size_t>( column_number::CreationDate )] =
		  L"Creation Date";
		result[static_cast<size_t>( column_number::ThreadCount )] = L"Thread Count";
		result[static_cast<size_t>( column_number::PageFaults )] = L"Page Faults";
		result[static_cast<size_t>( column_number::PageFileUsage )] = L"Page File";
		result[static_cast<size_t>( column_number::PeakPageFileUsage )] =
		  L"Peak Page File";
		result[static_cast<size_t>( column_number::WorkingSetSize )] =
		  L"Working Set";
		result[static_cast<size_t>( column_number::PeakWorkingSetSize )] =
		  L"Peak Working Set";
		result[static_cast<size_t>( column_number::ReadTransferCount )] =
		  L"Read Transfer";
		result[static_cast<size_t>( column_number::WriteTransferCount )] =
		  L"Write Transfer";
		result[static_cast<size_t>( column_number::CommandLine )] = L"CommandLine";
		return result;
	}( );

	namespace {
		struct wmi_state_co {
			bool has_init = false;

			wmi_state_co( DWORD const dwCoInit ) {
				if( auto const hres = CoInitializeEx( nullptr, dwCoInit );
				    FAILED( hres ) ) {
					throw wmi_error_t{std::wstring( L"Failed to init COM library" ),
					                  hres};
				}
				has_init = true;
			}

			~wmi_state_co( ) {
				if( std::exchange( has_init, false ) ) {
					CoUninitialize( );
				}
			}
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

			wmi_state_t( DWORD const dwCoInit )
			  : init( dwCoInit ) {

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
						throw wmi_error_t{std::wstring( L"Failed to initialize security" ),
						                  hres};
					}
					return hres;
				}( );
				auto hres = CoCreateInstance( CLSID_WbemLocator, nullptr,
				                              CLSCTX_INPROC_SERVER, IID_IWbemLocator,
				                              reinterpret_cast<LPVOID *>( &locator ) );

				if( FAILED( hres ) ) {
					CoUninitialize( );
					throw wmi_error_t{
					  std::wstring( L"Failed to create IWbemLocator object" ), hres};
				}
			}

			void connect( std::wstring const &path, std::wstring machine = L"" ) {
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
					throw wmi_error_t{std::wstring( L"Could not connect" ), result};
				}
				set_proxy_blanket( );
			}

			void set_proxy_blanket( ) {
				auto const result =
				  CoSetProxyBlanket( service,           // Indicates the proxy to set
				                     RPC_C_AUTHN_WINNT, // RPC_C_AUTHN_xxx
				                     RPC_C_AUTHZ_NONE,  // RPC_C_AUTHZ_xxx
				                     nullptr,           // Server principal name
				                     RPC_C_AUTHN_LEVEL_CALL, // RPC_C_AUTHN_LEVEL_xxx
				                     RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
				                     nullptr,                     // client identity
				                     EOAC_NONE                    // proxy capabilities
				  );
				if( FAILED( result ) ) {
					throw wmi_error_t{std::wstring( L"Could not set proxy blanket" ),
					                  result};
				}
			}

			CComPtr<IEnumWbemClassObject> query( std::wstring const &query_str ) {
				CComPtr<IEnumWbemClassObject> result;
				auto const hres = service->ExecQuery(
				  bstr_t( "WQL" ), bstr_t( query_str.c_str( ) ),
				  WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, nullptr,
				  &result );

				if( FAILED( hres ) ) {
					throw wmi_error_t{std::wstring( L"Query for Win32_Process failed" ),
					                  hres};
				}
				return result;
			}
		};

		CComVariant get_property( CComPtr<IWbemClassObject> const &obj,
		                          std::wstring const &property ) {

			CComVariant v;
			auto const hr = obj->Get( property.c_str( ), 0, &v, nullptr, nullptr );
			if( FAILED( hr ) ) {
				throw wmi_error_t{L"Error retrieving property", hr};
			}
			return v;
		}

		std::wstring get_wstring( CComPtr<IWbemClassObject> const &obj,
		                          std::wstring const &property ) {
			return daw::variant_visit<std::wstring>(
			  get_property( obj, property ),
			  []( BSTR str ) { return std::wstring( str ); },
			  []( ) { return std::wstring{}; } );
		}

		constexpr bool is_number( wchar_t c ) noexcept {
			return L'0' <= c && c <= L'9';
		}

		template<typename T>
		constexpr T from_char_range( wchar_t const *first,
		                             wchar_t const *const last ) noexcept {
			T result = 0;
			while( first != last && is_number( *first ) ) {
				result *= 10;
				result += static_cast<T>( *first - L'0' );
				++first;
			}
			return result;
		}

		template<typename UnsignedInteger>
		struct unsigned_from_bstr {
			constexpr UnsignedInteger operator( )( BSTR str ) const noexcept {
				auto const first = &str[0];
				return from_char_range<UnsignedInteger>( first, first + SysStringLen( str ) );
			}
		};

		struct from_variant_date {
			wxDateTime operator( )( DATE var_dte ) const {
				SYSTEMTIME st = {0};
				auto const ret_val = VariantTimeToSystemTime( var_dte, &st );
				if( ret_val == FALSE ) {
					throw std::runtime_error( "Error converting DATE to wxDateTime" );
				}
				std::tm tm = {0};
				tm.tm_sec = st.wSecond;
				tm.tm_min = st.wMinute;
				tm.tm_hour = st.wHour;
				tm.tm_mday = st.wDay;
				tm.tm_mon = st.wMonth - 1;
				tm.tm_year = st.wYear - 1900;
				tm.tm_isdst = -1;
				return wxDateTime( tm );
			}
		};

		template<typename Integer>
		Integer get_integer( CComPtr<IWbemClassObject> const &obj,
		                     std::wstring const &property ) {
			// Integer must be convertible from the value type stored in
			// the VARIANT
			return variant_visit<Integer>(
			  get_property( obj, property ),
			  []( Integer const &value ) { return value; },
			  []( Integer &&value ) { return std::move( value ); },
			  unsigned_from_bstr<Integer>{}, /* some ints are encoded as strings */
			  []( ) { return 0; }            /* do not fail */
			);
		}

		wxDateTime get_datetime( CComPtr<IWbemClassObject> const &obj,
		                         std::wstring const &property ) {
			return variant_visit<wxDateTime>(
			  get_property( obj, property ),
			  []( BSTR str ) {
				  wxDateTime result;
				  if( !result.ParseFormat( str, L"%Y%m%d%H%M%S%Z" ) ) {
					  return wxDateTime::Now( );
				  }
				  return result;
			  },
			  from_variant_date{} );
		}

		template<typename T>
		constexpr T from_kilobytes( T kilobyte_value ) noexcept {
			return kilobyte_value * static_cast<T>( 1024 );
		}

		struct make_wmi_process {
			wmi_process operator( )( CComPtr<IWbemClassObject> &record ) const {
				auto item = wmi_process{};
				item.name = get_wstring( record, L"Name" );
				item.command_line = get_wstring( record, L"CommandLine" );
				item.process_id = get_integer<uint32_t>( record, L"ProcessId" );
				item.parent_process_id =
				  get_integer<uint32_t>( record, L"ParentProcessId" );
				item.session_id = get_integer<uint32_t>( record, L"SessionId" );
				item.creation_date = get_datetime( record, L"CreationDate" );
				item.thread_count = get_integer<uint32_t>( record, L"ThreadCount" );
				item.page_faults = get_integer<uint32_t>( record, L"PageFaults" );
				item.page_file_usage =
				  from_kilobytes( get_integer<uint64_t>( record, L"PageFileUsage" ) );

				item.peak_page_file_usage = from_kilobytes(
				  get_integer<uint64_t>( record, L"PeakPageFileUsage" ) );

				item.working_set_size =
				  get_integer<uint32_t>( record, L"WorkingSetSize" );

				item.peak_working_set_size = from_kilobytes(
				  get_integer<uint64_t>( record, L"PeakWorkingSetSize" ) );

				item.read_transfer_count =
				  get_integer<uint64_t>( record, L"ReadTransferCount" );
				item.write_transfer_count =
				  get_integer<uint64_t>( record, L"WriteTransferCount" );
				return item;
			}
		};

		template<typename Enumerator, typename OutputIterator, typename Function>
		void transform( Enumerator &&enumerator, OutputIterator iter,
		                Function &&func ) {
			while( enumerator ) {
				CComPtr<IWbemClassObject> current_record;
				unsigned long record_count = 0;
				auto const hr =
				  enumerator->Next( WBEM_INFINITE, 1, &current_record, &record_count );
				if( record_count == 0 || FAILED( hr ) ) {
					// We have an error or no more records left
					break;
				}
				*iter++ = func( current_record );
			}
		}
	} // namespace

	std::vector<wmi_process>
	get_wmi_win32_process( std::wstring const &machine ) {
		wmi_state_t wmi_state( COINIT_APARTMENTTHREADED );
		wmi_state.connect( L"ROOT\\CIMV2", machine );

		auto result = std::vector<wmi_process>( );
		transform( wmi_state.query( L"SELECT * FROM Win32_Process" ),
		           std::back_inserter( result ), make_wmi_process{} );
		return result;
	}
} // namespace daw
