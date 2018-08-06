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
#include "remote_task_management.h"
#include "remote_task_management_frame.h"

#ifdef _DEBUG
#pragma comment( lib, "wxbase31ud.lib" )
#pragma comment( lib, "wxmsw31ud_adv.lib" )
#pragma comment( lib, "wxmsw31ud_core.lib" )
#pragma comment( lib, "wxmsw31ud_gl.lib" )
#else
#pragma comment( lib, "wxbase31u.lib" )
#pragma comment( lib, "wxmsw31u_adv.lib" )
#pragma comment( lib, "wxmsw31u_core.lib" )
#pragma comment( lib, "wxmsw31u_gl.lib" )
#endif

namespace daw {
	bool remote_task_management_app::OnInit( ) {
		auto frame = new remote_task_management_frame( L"Remote Task Management" );

		frame->Show( true );
		return true;
	}

	wxIMPLEMENT_APP( remote_task_management_app );
} // namespace daw
