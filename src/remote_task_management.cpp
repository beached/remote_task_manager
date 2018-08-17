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

#include <wx/app.h>
#include <wx/cmdline.h>
#include <wx/wx.h>

#include <daw/daw_array.h>

#include "daw/remote_task_management.h"
#include "daw/remote_task_management_frame.h"

namespace daw {
	bool remote_task_management_app::OnInit( ) {
		if( !wxApp::OnInit( ) ) {
			return false;
		}
		auto frame = new remote_task_management_frame( m_remote_hosts,
		                                               L"Remote Task Management" );
		frame->Show( true );
		return true;
	}

	void remote_task_management_app::OnInitCmdLine( wxCmdLineParser &parser ) {
		using T = wxCmdLineEntryDesc;
		static auto const cmd_line_desc = daw::make_array<T>(
		  T{wxCMD_LINE_SWITCH, "h", "help",
		    "displays help on the command line parameters\n", wxCMD_LINE_VAL_NONE,
		    wxCMD_LINE_OPTION_HELP},

		  T{wxCMD_LINE_PARAM, nullptr, nullptr,
		    "host(s) (. can be used for local machine)\n", wxCMD_LINE_VAL_STRING,
		    wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_PARAM_MULTIPLE},

		  T{wxCMD_LINE_NONE} );

		parser.SetDesc( cmd_line_desc.data( ) );
		parser.SetSwitchChars( "-" );
	}

	bool
	daw::remote_task_management_app::OnCmdLineParsed( wxCmdLineParser &parser ) {
		for( size_t n = 0; n < parser.GetParamCount( ); ++n ) {
			m_remote_hosts.push_back( parser.GetParam( n ) );
		}
		return true;
	}
} // namespace daw

wxIMPLEMENT_APP( daw::remote_task_management_app );
