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
#include "daw/wmi_impl.h"
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
		template<size_t n>
		std::wstring to_wstring( wchar_t const ( &str )[n] ) {
			return str;
		}

		template<size_t n>
		std::wstring to_wstring( wchar_t ( &str )[n] ) {
			return str;
		}

		constexpr wchar_t to_wstring( wchar_t c ) noexcept {
			return c;
		}

		template<typename... Args>
		std::wstring fmtw( Args &&... args ) {
			using impl::to_wstring;
			using std::to_wstring;
			return ( to_wstring( std::forward<Args>( args ) ) + ... );
		}

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
				return from_char_range<UnsignedInteger>( first,
				                                         first + SysStringLen( str ) );
			}
		};

		struct from_variant_date {
			wxDateTime operator( )( DATE var_dte ) const {
				SYSTEMTIME st{};
				auto const ret_val = VariantTimeToSystemTime( var_dte, &st );
				if( ret_val == FALSE ) {
					throw std::runtime_error( "Error converting DATE to wxDateTime" );
				}
				std::tm tm{};
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
			static_assert(
			  std::is_invocable_v<Function, CComPtr<IWbemClassObject>>,
			  "Function must be callable with CComPtr<IWbemClassObject>" );

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

		template<typename Enumerator, typename Function>
		void for_each( Enumerator &&enumerator, Function &&func ) {
			static_assert(
			  std::is_invocable_v<Function, CComPtr<IWbemClassObject>>,
			  "Function must be callable with CComPtr<IWbemClassObject>" );

			while( enumerator ) {
				CComPtr<IWbemClassObject> current_record;
				unsigned long record_count = 0;
				auto const hr =
				  enumerator->Next( WBEM_INFINITE, 1, &current_record, &record_count );
				if( record_count == 0 || FAILED( hr ) ) {
					// We have an error or no more records left
					break;
				}
				func( current_record );
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

	void terminate_process_by_where( std::wstring const &machine,
	                                 std::wstring const &where_clause ) {
		wmi_state_t wmi_state( COINIT_APARTMENTTHREADED );
		wmi_state.connect( L"ROOT\\CIMV2", machine );

		CComPtr<IWbemClassObject> class_object;
		auto result = wmi_state.service->GetObjectW(
		  CComBSTR( L"Win32_Process" ), 0, nullptr, &class_object, nullptr );
		if( FAILED( result ) ) {
			// TODO
			return;
		}
		CComPtr<IWbemClassObject> in_param_def;
		CComPtr<IWbemClassObject> out_method;
		result =
		  class_object->GetMethod( L"Terminate", 0, &in_param_def, &out_method );

		if( FAILED( result ) ) {
			// TODO: could not get method
			return;
		}
		CComPtr<IWbemClassObject> class_instance;
		result = in_param_def->SpawnInstance( 0, &class_instance );
		if( FAILED( result ) ) {
			// TODO:
		}

		auto var_pid = CComVariant( static_cast<unsigned long>( 1 ) );
		result = class_instance->Put( L"Reason", 0, &var_pid, 0 );
		if( FAILED( result ) ) {
			// TODO:
		}

		result = wmi_state.service->ExecMethod(
		  CComBSTR( where_clause.size( ), where_clause.c_str( ) ),
		  CComBSTR( L"Terminate" ), 0, nullptr, class_instance, nullptr, nullptr );
		if( FAILED( result ) ) {
			// TODO:
		}
	}

	void terminate_process_by_pid( std::wstring const &machine, uint32_t pid ) {
		    auto const where_clause = std::wstring( L"Win32_Process.Handle=\"" ) +
		                       std::to_wstring( pid ) + L'"';
		    terminate_process_by_where( machine, where_clause );
	}
} // namespace daw
