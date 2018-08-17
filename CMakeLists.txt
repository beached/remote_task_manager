cmake_minimum_required(VERSION 3.1)
project(remote_task_management CXX)

find_package( Threads )

set(wxWidgets_CONFIGURATION mswu)
find_package(wxWidgets REQUIRED adv core base)
include( ${wxWidgets_USE_FILE} )

set( CMAKE_CXX_STANDARD 17 CACHE STRING "The C++ standard whose features are requested.")

add_compile_definitions( UNICODE _UNICODE WINVER=0x0601 _WIN32_WINNT=0x0601 )

include( ExternalProject )

externalproject_add(
	header_libraries_prj
	GIT_REPOSITORY "https://github.com/beached/header_libraries.git"
	SOURCE_DIR "${CMAKE_BINARY_DIR}/dependencies/header_libraries"
	GIT_TAG "master"
	INSTALL_DIR "${CMAKE_BINARY_DIR}/install"
	CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/install
)

include_directories( SYSTEM "${CMAKE_BINARY_DIR}/install/include" )
#link_directories( "${CMAKE_BINARY_DIR}"/install/lib )

set( HEADER_FOLDER "include" )
set( SOURCE_FOLDER "src" )

set( HEADER_FILES
	${HEADER_FOLDER}/daw/column_items.h
	${HEADER_FOLDER}/daw/remote_task_management.h
	${HEADER_FOLDER}/daw/remote_task_management_frame.h
	${HEADER_FOLDER}/daw/wmi_exec.h
	${HEADER_FOLDER}/daw/wmi_impl.h
	${HEADER_FOLDER}/daw/wmi_process.h
	${HEADER_FOLDER}/daw/wmi_process_table.h
	${HEADER_FOLDER}/daw/variant_visit.h
)

set( SOURCE_FILES 
	${SOURCE_FOLDER}/column_items.cpp
	${SOURCE_FOLDER}/remote_task_management.cpp
	${SOURCE_FOLDER}/remote_task_management_frame.cpp
	${SOURCE_FOLDER}/wmi_exec.cpp
	${SOURCE_FOLDER}/wmi_impl.cpp
	${SOURCE_FOLDER}/wmi_process.cpp
	${SOURCE_FOLDER}/wmi_process_table.cpp
)

include_directories( SYSTEM "${CMAKE_BINARY_DIR}/install/include" )

include_directories( ${HEADER_FOLDER} )

add_executable( remote_task_management_bin WIN32 ${HEADER_FILES} ${SOURCE_FILES} )
add_dependencies( remote_task_management_bin header_libraries_prj )
target_link_libraries( remote_task_management_bin ${wxWidgets_LIBRARIES} )
