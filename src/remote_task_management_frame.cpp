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
#include <vector>
#include <wx/menu.h>
#include <wx/string.h>
#include <wx/wx.h>

#include "daw/remote_task_management_frame.h"
#include "daw/wmi_process.h"
#include "daw/wmi_process_table.h"

namespace daw {
	namespace remote_task_management_frame_event_ids {
		enum event_ids { id_open_remote = 1 };
	}

	remote_task_management_frame::remote_task_management_frame(
	  std::vector<wxString> const &connect_to, wxString const &title,
	  wxPoint const &pos, wxSize const &size )
	  : wxFrame( nullptr, wxID_ANY, title, pos, size ) {

		Bind( wxEVT_COMMAND_MENU_SELECTED, [&]( wxCommandEvent & ) { Close( ); },
		      wxID_EXIT );

		Bind( wxEVT_COMMAND_MENU_SELECTED,
		      [&]( wxCommandEvent & ) {
			      wxMessageBox(
			        L"Remote Task Management\nby Darrell Wright\nRemotely manage "
			        L"windows processes",
			        L"About Remote Task Management", wxOK | wxICON_INFORMATION );
		      },
		      wxID_ABOUT );

		Bind( wxEVT_COMMAND_MENU_SELECTED,
		      [&]( wxCommandEvent & ) {
			      auto dlg = new wxTextEntryDialog( this, L"Enter remote system name",
			                                        L"Open remote system", L"." );
			      if( dlg->ShowModal( ) ) {
				      m_tbl->change_host( dlg->GetValue( ) );
				      SetStatusText( L"Connected to " + dlg->GetValue( ) );
				      m_data_grid->ForceRefresh( );
			      }
		      },
		      remote_task_management_frame_event_ids::id_open_remote );

		auto menu_file = new wxMenu( );
		menu_file->Append( remote_task_management_frame_event_ids::id_open_remote,
		                   L"&Open Remote\tCtrl-O", L"Open task on remote system" );
		menu_file->AppendSeparator( );
		menu_file->Append( wxID_EXIT );

		auto menu_help = new wxMenu( );
		menu_help->Append( wxID_ABOUT );

		auto menu_bar = new wxMenuBar( );
		menu_bar->Append( menu_file, "&File" );
		menu_bar->Append( menu_help, "&Help" );

		m_data_grid = new wxGrid( this, wxID_ANY );
		if( !m_data_grid ) {
			throw std::runtime_error( "Could not create data grid" );
		}

		if( connect_to.empty( ) ) {
			m_tbl = new wmi_process_table( );
		} else {
			try {
				// TODO Add multiple hosts
				m_tbl = new wmi_process_table( connect_to.front( ) );
			} catch( ... ) {
				// Could not connect ask to try local host
				auto dlg = new wxMessageDialog(
				  this, L"Could not connect to remote system, try local?",
				  L"Error openning remote system", wxYES_NO );
				if( dlg->ShowModal( ) == wxID_YES ) {
					m_tbl = new wmi_process_table( );
				} else {
					Close( );
				}
			}
		}
		if( m_tbl ) {
			m_tbl->sort_column( wmi_process::column_number::CreationDate );
			if( m_tbl ) {
				m_data_grid->SetTable( m_tbl, true );
			}
			m_data_grid->HideRowLabels( );
			m_data_grid->AutoSizeColumns( );
			m_data_grid->Bind( wxEVT_GRID_COL_SORT, [&]( wxGridEvent &event ) {
				if( !m_tbl ) {
					return;
				}
				m_tbl->sort_column( event.GetCol( ) );
			} );
			m_data_grid->ForceRefresh( );
		}
		m_tmr = std::make_unique<wxTimer>( );
		m_tmr->Bind( wxEVT_TIMER, [&]( wxTimerEvent & ) {
			m_tbl->update_data( );
			m_data_grid->ForceRefresh( );
		} );
		m_tmr->Start( 1500 );

		wxFrameBase::SetMenuBar( menu_bar );

		wxFrameBase::CreateStatusBar( );
		if( connect_to.empty( ) ) {
			wxFrameBase::SetStatusText( L"Connected to local machine" );
		} else {
			wxFrameBase::SetStatusText( L"Connected to local " +
			                            connect_to.front( ) );
		}
	}
} // namespace daw
