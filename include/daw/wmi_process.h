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

#include <array>
#include <cstdint>
#include <iomanip>
#include <string>
#include <variant>
#include <vector>
#include <wx/string.h>

#include "column_items.h"

namespace daw {
	struct wmi_error_t {
		std::wstring message;
		long code;
	};

	namespace impl {
		inline std::wstring &&to_wstring( std::wstring &&str ) noexcept {
			return std::move( str );
		}

		inline std::wstring const &to_wstring( std::wstring const &str ) noexcept {
			return str;
		}
	} // namespace impl

	struct wmi_process {
		static std::array<wxString, 14> const column_names;
		enum class column_number : int {
			Name,
			ProcessId,
			ParentProcessId,
			SessionId,
			CreationDate,
			ThreadCount,
			PageFaults,
			WorkingSetSize,
			PeakWorkingSetSize,
			PageFileUsage,
			PeakPageFileUsage,
			ReadTransferCount,
			WriteTransferCount,
			CommandLine
		};
		String name;
		String command_line;
		Integer<uint32_t> process_id;
		Integer<uint32_t> parent_process_id;
		Integer<uint32_t> session_id;
		Date creation_date;
		// Resources
		Integer<uint32_t> thread_count;

		Integer<uint32_t> page_faults;
		Memory page_file_usage;
		Memory peak_page_file_usage;
		Memory working_set_size;
		Memory peak_working_set_size;

		// IO
		Memory read_transfer_count;
		Memory write_transfer_count;

		ColumnItem const &operator[]( size_t n ) const {
			switch( static_cast<column_number>( n ) ) {
			case column_number::Name:
				return name;
			case column_number::ProcessId:
				return process_id;
			case column_number::ParentProcessId:
				return parent_process_id;
			case column_number::SessionId:
				return session_id;
			case column_number::CreationDate:
				return creation_date;
			case column_number::ThreadCount:
				return thread_count;
			case column_number::PageFaults:
				return page_faults;
			case column_number::PageFileUsage:
				return page_file_usage;
			case column_number::PeakPageFileUsage:
				return peak_page_file_usage;
			case column_number::WorkingSetSize:
				return working_set_size;
			case column_number::PeakWorkingSetSize:
				return peak_working_set_size;
			case column_number::ReadTransferCount:
				return read_transfer_count;
			case column_number::WriteTransferCount:
				return write_transfer_count;
			case column_number::CommandLine:
				return command_line;
			}
			std::terminate( );
		}
	};

	std::vector<wmi_process> get_wmi_win32_process( std::wstring const &machine = L"" );
} // namespace daw
