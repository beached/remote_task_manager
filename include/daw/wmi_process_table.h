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

#include <wx/grid.h>
#include <wx/string.h>

#include <daw/daw_validated.h>

#include "wmi_process.h"

namespace daw {
	struct wmi_process_table : public wxGridTableBase {
		using table_data_t = std::vector<wmi_process>;
		enum class SortOrder : uint_fast8_t { Next, Ascending, Descending };

	private:
		wxString m_remote_host;
		std::shared_ptr<table_data_t> m_data;

		struct sorted_t {
			int column = -1;
			SortOrder sort_order = SortOrder::Descending;
		} sorted;

	public:
		explicit wmi_process_table( wxString remote_host = L"." );
		explicit wmi_process_table( std::shared_ptr<table_data_t> data );
		explicit wmi_process_table( table_data_t const &data );
		explicit wmi_process_table( table_data_t &&data );

		int GetNumberRows( ) override;
		int GetNumberCols( ) override;

		wxString GetValue( int row, int col ) override;
		wxString GetColLabelValue( int col ) override;

		void sort_column( int col, SortOrder sort_order = SortOrder::Next );

		inline void sort_column( wmi_process::column_number col,
		                         SortOrder sort_order = SortOrder::Next ) {

			sort_column( static_cast<int>( col ), sort_order );
		}

		void update_data( );
		void change_host( wxString const &remote_host = L"." );

		inline bool IsEmptyCell( int, int ) override {
			return false;
		}

		inline void SetValue( int, int, wxString const & ) override {
		} // Read only table
	};
} // namespace daw
