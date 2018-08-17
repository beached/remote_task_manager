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
		wxString str_value = L"";

		Memory( ) = default;

		explicit Memory( uint64_t v );

		Memory &operator=( uint64_t v );

		int compare( ColumnItem const &rhs ) const override;
		wxString to_string( ) const override;
	};

	struct Date : ColumnItem {
		enum class date_formats { Combined, DateOnly, TimeOnly };

		wxDateTime value = {};
		date_formats date_format = date_formats::Combined;
		wxString str_value = L"";

		Date( ) = default;

		Date( wxDateTime tp, date_formats fmt );

		Date &operator=( wxDateTime v );

		int compare( ColumnItem const &rhs ) const override;
		wxString to_string( ) const override;
	};

	template<typename T>
	struct Integer : ColumnItem {
		T value = 0;
		wxString str_value = L"";

		Integer( ) noexcept = default;

		Integer( uint64_t v ) noexcept
		  : value( v )
		  , str_value( std::to_wstring( value ) ) {}

		Integer &operator=( uint64_t v ) noexcept {
			value = v;
			str_value = std::to_wstring( value );
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
			return str_value;
		}
	};

	struct String : ColumnItem {
		wxString value = L"";

		String( ) = default;
		explicit String( wxString && str );
        explicit String( wxString const & str );
		explicit String( std::wstring_view str );
		String &operator=( std::wstring_view str );
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
