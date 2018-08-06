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

#include <cstdint>
#include <wx/datetime.h>
#include <wx/string.h>

namespace daw {
	struct ColumnItem {
		ColumnItem( ) noexcept = default;
		ColumnItem( ColumnItem const & ) noexcept = default;
		ColumnItem( ColumnItem && ) noexcept = default;
		ColumnItem &operator=( ColumnItem const & ) noexcept = default;
		ColumnItem &operator=( ColumnItem && ) noexcept = default;

		virtual ~ColumnItem( ) = default;
		virtual int compare( ColumnItem const &rhs ) const = 0;
		virtual wxString to_string( ) const = 0;
	};

	struct Memory : ColumnItem {
		uint64_t value = 0;

		Memory( ) noexcept = default;

		Memory( uint64_t v ) noexcept
		  : value( v ) {}

		Memory &operator=( uint64_t v ) noexcept {
			value = v;
			return *this;
		}

		int compare( ColumnItem const &rhs ) const override;
		wxString to_string( ) const override;
	};

	struct Date : ColumnItem {
		wxDateTime value;
		enum class date_formats { Combined, DateOnly, TimeOnly };
		date_formats date_format = date_formats::Combined;

		Date( ) = default;

		Date( wxDateTime tp, date_formats fmt )
		  : value( tp )
		  , date_format( fmt ) {}

		Date &operator=( wxDateTime v ) {
			value = v;
			return *this;
		}

		int compare( ColumnItem const &rhs ) const override;
		wxString to_string( ) const override;
	};

	template<typename T>
	struct Integer : ColumnItem {
		T value = 0;

		Integer( ) noexcept = default;

		Integer( uint64_t v ) noexcept
		  : value( v ) {}

		Integer &operator=( uint64_t v ) noexcept {
			value = v;
			return *this;
		}

		int compare( ColumnItem const &rhs ) const override {
			auto const &val = dynamic_cast<Integer const &>( rhs );
			if( value < val.value ) {
				return -1;
			}
			if( value > val.value ) {
				return 1;
			}
			return 0;
		}

		wxString to_string( ) const override {
			using std::to_wstring;
			using std::to_wstring;
			return to_wstring( value );
		}
	};

	struct String : ColumnItem {
		wxString value = L"";

		String( ) = default;
		String( wxString const &str )
		  : value( str ) {}
		String( std::wstring const &str )
		  : value( str ) {}

		String &operator=( wxString const &str ) {
			value = str;
			return *this;
		}

		String &operator=( std::wstring const &str ) {
			value = str;
			return *this;
		}

		int compare( ColumnItem const &rhs ) const override;

		inline wxString to_string( ) const override {
			return value;
		}
	};

	inline bool operator<( ColumnItem const &lhs, ColumnItem const &rhs ) {
		return lhs.compare( rhs ) < 0;
	}

	inline bool operator<=( ColumnItem const &lhs, ColumnItem const &rhs ) {
		return lhs.compare( rhs ) <= 0;
	}

	inline bool operator==( ColumnItem const &lhs, ColumnItem const &rhs ) {
		return lhs.compare( rhs ) == 0;
	}

	inline bool operator>( ColumnItem const &lhs, ColumnItem const &rhs ) {
		return lhs.compare( rhs ) > 0;
	}

	inline bool operator>=( ColumnItem const &lhs, ColumnItem const &rhs ) {
		return lhs.compare( rhs ) >= 0;
	}

	wxString to_wstring( Memory value );
} // namespace daw
