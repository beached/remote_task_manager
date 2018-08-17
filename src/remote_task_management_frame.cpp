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
#include <future>
#include <vector>
#include <wx/menu.h>
#include <wx/string.h>
#include <wx/wx.h>

#include "daw/remote_task_management_frame.h"
#include "daw/wmi_process.h"
#include "daw/wmi_process_table.h"

namespace daw {
	namespace remote_task_management_frame_event_ids {
		enum event_ids { id_open_remote = 1, id_close_by_pid, id_close_by_name };
	}

	namespace {
		template<typename R>
		bool is_ready( std::future<R> const &f ) noexcept {
			try {
				return f.wait_for( std::chrono::seconds( 0 ) ) ==
				       std::future_status::ready;
			} catch( ... ) {
				// Return true so that we stop waiting and any exceptions would be
				// propegated
				return true;
			}
		}

		auto grid_update_timer( wxTimer *tmr, wmi_process_table *tbl,
		                        wxGrid *dg ) noexcept {
			return [ tmr, tbl, dg, has_data = std::shared_ptr<std::future<void>>( ) ](
			  wxTimerEvent & ) mutable {
				if( !has_data ) {
					// Timer has expired and we haven't tried to reteive any data yet
					has_data = std::make_shared<std::future<void>>(
					  std::async( [tbl = tbl]( ) { tbl->update_data( ); } ) );
					tmr->StartOnce( 500 );
				} else if( !is_ready( *has_data ) ) {
					// We have requested data but it has not shown up yet
					tmr->StartOnce( 250 );
				} else {
					// We have data, process it
					try {
						// Force any exceptions to throw by running get( ) on future
						has_data->get( );
						dg->ForceRefresh( );
						tmr->StartOnce( 1700 );
					} catch( ... ) {
						// TODO put error message
						dg->SetTable( nullptr );
						tmr->Stop( );
					}
					has_data.reset( );
				}
			};
		}
		uint32_t to_uint32( wxString const &str ) {
			unsigned long result = 0xDEADBEEF;
			(void)str.ToULong( &result );
			return static_cast<uint32_t>( result );
		}
	} // namespace

	void remote_task_management_frame::add_page( wxString const &host ) {
		try {
			auto tbl = new wmi_process_table( host );
			if( tbl ) {
				tbl->sort_column( wmi_process::column_number::CreationDate );

				auto dg = new wxGrid( m_notebook, wxID_ANY );
				if( !dg ) {
					throw std::runtime_error( "Could not create data grid" );
				}
				dg->SetTable( tbl, true );
				dg->HideRowLabels( );
				dg->EnableEditing( false );	
				dg->AutoSizeColumns( );
				dg->Bind( wxEVT_GRID_COL_SORT, [tbl]( wxGridEvent &event ) {
					tbl->sort_column( event.GetCol( ) );
				} );

				struct popup_data_t {
					uint32_t pid;
					wxString name;
				};
				dg->Bind( wxEVT_GRID_CELL_RIGHT_CLICK, [tbl, dg]( wxGridEvent &event ) {
					constexpr auto const pid_col =
					  static_cast<int>( wmi_process::column_number::ProcessId );
					constexpr auto const name_col =
					  static_cast<int>( wmi_process::column_number::Name );

					auto const pid = tbl->GetValue( event.GetRow( ), pid_col );
					auto const name = tbl->GetValue( event.GetRow( ), name_col );
					
					auto const data =
					  static_cast<void *>( new popup_data_t{to_uint32( pid ), name} );

					auto menu = new wxMenu( );
					menu->SetClientData( data );
					auto mnu1 = menu->Append(
					  remote_task_management_frame_event_ids::id_close_by_pid,
					  L"Close pid " + pid );
					mnu1->SetId( 0 );
					auto mnu2 = menu->Append(
					  remote_task_management_frame_event_ids::id_close_by_name,
					  L"Close all " + name );
					mnu2->SetId( 1 );

					dg->PopupMenu( menu, event.GetPosition( ) );
				} );

				dg->Bind(
				  wxEVT_COMMAND_MENU_SELECTED,
				  [host]( wxCommandEvent const &event ) {
					  auto const source_menu =
					    dynamic_cast<wxMenu *>( event.GetEventObject( ) );
					  auto const data = std::unique_ptr<popup_data_t>(
					    static_cast<popup_data_t *>( source_menu->GetClientData( ) ) );
					  wxString const msg = L"Right click from " + std::to_wstring( data->pid ) +
					                 L' ' + data->name; 
						terminate_process_by_pid( host.ToStdWstring( ), data->pid );
				  },
				  remote_task_management_frame_event_ids::id_close_by_pid );

				auto tmr = new wxTimer( this );
				this->Bind( wxEVT_TIMER, grid_update_timer( tmr, tbl, dg ),
				            tmr->GetId( ) );
				tmr->StartOnce( 2500 );
				if( host == L"." ) {
					m_notebook->AddPage( dg, L"local machine", true );
				} else {
					m_notebook->AddPage( dg, host, true );
				}
			}
		} catch( ... ) {
			wxMessageBox( L"Error connecting to " + host, L"Connection error" );
		}
	}

	void remote_task_management_frame::setup_handlers( ) {
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
				      add_page( dlg->GetValue( ) );
			      }
		      },
		      remote_task_management_frame_event_ids::id_open_remote );
	}

	void remote_task_management_frame::setup_menus( ) {
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
		wxFrameBase::SetMenuBar( menu_bar );
	}

	void remote_task_management_frame::setup_notebook( ) {
		auto pnl = new wxPanel( this );

		m_notebook = new wxNotebook( pnl, wxID_ANY );
		if( !m_notebook ) {
			throw std::runtime_error( "Could not create notebook" );
		}

		auto pnl_sz = new wxBoxSizer( wxHORIZONTAL );
		pnl_sz->Add( m_notebook, 1, wxEXPAND );
		pnl->SetSizer( pnl_sz );

		auto frm_sz = new wxBoxSizer( wxHORIZONTAL );
		frm_sz->Add( pnl, 1, wxEXPAND );
		frm_sz->SetMinSize( 800, 600 );
		SetSizerAndFit( frm_sz );
	}

	remote_task_management_frame::remote_task_management_frame(
	  std::vector<wxString> const &connect_to, wxString const &title,
	  wxPoint const &pos, wxSize const &size )
	  : wxFrame( nullptr, wxID_ANY, title, pos, size ) {

		setup_handlers( );
		setup_menus( );
		setup_notebook( );

		// Add hosts
		if( connect_to.empty( ) ) {
			add_page( L"." );
		}
		for( auto const &host : connect_to ) {
			add_page( host );
		}
	}
} // namespace daw
