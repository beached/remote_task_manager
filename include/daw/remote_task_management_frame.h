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

#include <vector>
#include <wx/event.h>
#include <wx/frame.h>
#include <wx/notebook.h>
#include <wx/string.h>
#include <wx/timer.h>

namespace daw {
	class remote_task_management_frame : public wxFrame {
		std::unique_ptr<wxTimer> m_tmr = nullptr;
		wxNotebook *m_notebook = nullptr; // unowned

		void add_page( wxString const &host );
		void setup_handlers( );
		void setup_menus( );
		void setup_notebook( );

	public:
		explicit remote_task_management_frame(
		  std::vector<wxString> const &connect_to, wxString const &title,
		  wxPoint const &pos = wxDefaultPosition,
		  wxSize const &size = wxDefaultSize );
	};
} // namespace daw