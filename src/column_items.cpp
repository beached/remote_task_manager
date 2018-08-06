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
#include <iomanip>
#include <sstream>
#include <wx/string.h>

#include "daw/column_items.h"

namespace daw {
	namespace {
		std::wstring to_2digit_dec( double value ) {
			std::wstringstream ss;
			ss << std::fixed << std::setprecision( 2 ) << value;
			auto result = ss.str( );
			if( result.find_first_of( L'.' ) != std::wstring::npos ) {
				while( result.back( ) == L'0' ) {
					result.resize( result.size( ) - 1 );
				}
			}
			if( result.back( ) == L'.' ) {
				result.resize( result.size( ) - 1 );
			}
			return result;
		}
	} // namespace

	int Memory::compare( ColumnItem const &rhs ) const {
		auto const &val = dynamic_cast<Memory const &>( rhs );
		if( value < val.value ) {
			return -1;
		}
		if( value > val.value ) {
			return 1;
		}
		return 0;
	}

	int String::compare( ColumnItem const &rhs ) const {
		auto const &val = dynamic_cast<String const &>( rhs );
		auto const rlen = std::min( value.size( ), val.value.size( ) );
		auto const result = _wcsnicmp( value.c_str( ), val.value.c_str( ), rlen );

		if( result == 0 ) {
			if( value.size( ) < rlen ) {
				return -1;
			}
			if( value.size( ) > rlen ) {
				return 1;
			}
		}
		return result;
	}

	wxString to_wstring( Memory value ) {
		if( value.value < 1024ULL ) {
			return std::to_wstring( value.value ) + L"B";
		}
		// Waiting until here accounts for when value.value > 2^53 as by
		auto val = static_cast<double>( value.value ) / 1024.0;
		if( val < 1024.0 ) {
			return std::to_wstring( lround( val ) ) + L"KB";
		}
		val /= 1024.0;
		if( val < 1024.0 ) {
			return to_2digit_dec( val ) + L"MB";
		}
		val /= 1024.0;
		if( val < 1024.0 ) {
			return to_2digit_dec( val ) + L"GB";
		}
		val /= 1024.0;
		if( val < 1024.0 ) {
			return to_2digit_dec( val ) + L"TB";
		}
		val /= 1024.0;
		if( val < 1024.0 ) {
			return to_2digit_dec( val ) + L"PB";
		}
		val /= 1024.0;
		return to_2digit_dec( val ) + L"EB";
	}

	wxString Memory::to_string( ) const {
		return to_wstring( *this );
	}

	int Date::compare( ColumnItem const &rhs ) const {
		auto const &val = dynamic_cast<Date const &>( rhs );
		if( value < val.value ) {
			return -1;
		}
		if( value > val.value ) {
			return 1;
		}
		return 0;
	}

	wxString Date::to_string( ) const {
		switch( date_format ) {
		case date_formats::DateOnly:
			return value.FormatISODate( );
		case date_formats::TimeOnly:
			return value.FormatTime( );
		case date_formats::Combined:
		default:
			return value.Format( L"%Y-%m-%d %H:%M" );
		}
	}

} // namespace daw
