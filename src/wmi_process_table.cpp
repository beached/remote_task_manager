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
#include <algorithm>
#include <array>
#include <wx/string.h>

#include "daw/wmi_process.h"
#include "daw/wmi_process_table.h"

namespace daw {
	wmi_process_table::wmi_process_table( wxString remote_host )
	  : m_remote_host( std::move( remote_host ) )
	  , m_data( std::make_shared<table_data_t>(
	      get_wmi_win32_process( m_remote_host.ToStdWstring( ) ) ) ) {}

	wmi_process_table::wmi_process_table( std::shared_ptr<table_data_t> data )
	  : m_data( std::move( data ) ) {}

	wmi_process_table::wmi_process_table( table_data_t const &data )
	  : m_data( std::make_shared<table_data_t>( data ) ) {}

	wmi_process_table::wmi_process_table( table_data_t &&data )
	  : m_data( std::make_shared<table_data_t>( std::move( data ) ) ) {}

	int wmi_process_table::GetNumberRows( ) {
		auto const tmp_data = m_data;
		if( !tmp_data ) {
			return 0;
		}
		return tmp_data->size( );
	}

	int wmi_process_table::GetNumberCols( ) {
		auto const tmp_data = m_data;
		if( !tmp_data ) {
			return 0;
		}
		using tbl_t = std::decay_t<decltype( tmp_data->front( ) )>;
		return tbl_t::column_names.size( );
	}

	wxString wmi_process_table::GetValue( int row, int col ) {
		auto const tmp_data = m_data;
		if( row < tmp_data->size( ) ) {
			auto const &record = ( *tmp_data )[row];
			return record[col].to_string( );
		}
		return wxString{};
	}

	wxString wmi_process_table::GetColLabelValue( int col ) {
		using tbl_t = std::decay_t<decltype( m_data->front( ) )>;
		return tbl_t::column_names[col];
	}

	namespace {
		void sort_table_on_column( wmi_process_table::table_data_t &tbl, int col,
		                           wmi_process_table::SortOrder sort_order ) {
			if( sort_order == wmi_process_table::SortOrder::Ascending ) {
				std::stable_sort(
				  tbl.begin( ), tbl.end( ),
				  [col]( auto &&lhs, auto &&rhs ) { return lhs[col] < rhs[col]; } );
			} else { // Descending
				std::stable_sort(
				  tbl.begin( ), tbl.end( ),
				  [col]( auto &&lhs, auto &&rhs ) { return lhs[col] > rhs[col]; } );
			}
		}
	} // namespace

	void wmi_process_table::sort_column( int col, SortOrder sort_order ) {
		auto const tmp_data = m_data;
		if( !tmp_data ) {
			return;
		}
		if( sort_order == wmi_process_table::SortOrder::Next ) {
			switch( sorted.sort_order ) {
			case wmi_process_table::SortOrder::Ascending:
				sort_order = wmi_process_table::SortOrder::Descending;
				break;
			default:
				sort_order = wmi_process_table::SortOrder::Ascending;
				break;
			}
			sort_table_on_column( *tmp_data, col, sort_order );
			sorted.column = col;
			sorted.sort_order = sort_order;
		}
	}

	void wmi_process_table::update_data( ) {
		auto ptr = std::make_shared<table_data_t>(
		  get_wmi_win32_process( m_remote_host.ToStdWstring( ) ) );
		sort_table_on_column( *ptr, sorted.column, sorted.sort_order );
		m_data = std::move( ptr );
	}

	void wmi_process_table::change_host( wxString const &remote_host ) {
		m_remote_host = remote_host;
		update_data( );
	}
} // namespace daw
