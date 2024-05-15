cmake_minimum_required (VERSION 3.18.6)
project(tsim)

################################
# general config
################################

message("-- use c compiler ${CMAKE_C_COMPILER}")
message("-- use c++ compiler ${CMAKE_CXX_COMPILER}")

# set compile parameter
if (${CMAKE_C_COMPILER_ID} STREQUAL GNU)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
elseif (${CMAKE_C_COMPILER_ID} MATCHES Clang)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
elseif (${CMAKE_C_COMPILER_ID} STREQUAL MSVC)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS=1 -D_UNICODE -DUNICODE)
	add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
	add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
endif()

# set standard and print features
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

message("-- c compiler support features: ")
foreach(feature ${CMAKE_C_COMPILE_FEATURES})
	message("support feature: ${feature}")
endforeach()

# set output directory
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# for LSP
if (NOT ${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
	set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

# set use folder in vs
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

################################
# deps & install
################################

include(FetchContent)
set(FETCHCONTENT_BASE_DIR ${CMAKE_BINARY_DIR}/_deps)

message("# fetch mugglec")
FetchContent_Declare(
	mugglec
	GIT_REPOSITORY https://github.com/MuggleWei/mugglec.git
	GIT_TAG v1.8.1
	GIT_SHALLOW TRUE
	)
FetchContent_MakeAvailable(mugglec)

message("# fetch haclog")
FetchContent_Declare(
	haclog
	GIT_REPOSITORY https://github.com/MuggleWei/haclog.git
	GIT_TAG v0.2.0
	GIT_SHALLOW TRUE
	)
FetchContent_MakeAvailable(haclog)

message("# fetch libxml2")
FetchContent_Declare(
	libxml2
	GIT_REPOSITORY https://github.com/GNOME/libxml2.git
	GIT_TAG v2.12.7
	GIT_SHALLOW TRUE
	)
FetchContent_MakeAvailable(libxml2)

# install
include(GNUInstallDirs)
set(CMAKE_INSTALL_LIBDIR lib)

################################
# configure
################################

# version
file(STRINGS "version.txt" version)
string(REPLACE "-" ";" semver_ext ${version})
list(GET semver_ext 0 semver)
string(REPLACE "." ";" semver_list ${semver})

list(GET semver_list 0 TSIM_VERSION_MAJOR)
list(GET semver_list 1 TSIM_VERSION_MINOR)
list(GET semver_list 2 TSIM_VERSION_PATCH)

set(TSIM_VERSION "${version}")
set(TSIM_SOVERSION "${TSIM_VERSION_MAJOR}")

# dll
if (BUILD_SHARED_LIBS)
	set(TSIM_USE_DLL ON)
else()
	set(TSIM_USE_DLL OFF)
endif()

# log
option(USE_ASYNC_LOG "use async log" ON)
if (USE_ASYNC_LOG)
	set(TSIM_USE_ASYNC_LOG ON)
else()
	set(TSIM_USE_ASYNC_LOG OFF)
endif()

set(gen_dir ${CMAKE_BINARY_DIR}/gen)
configure_file(
	"${CMAKE_CURRENT_LIST_DIR}/src/tsim/config.h.in"
	"${gen_dir}/tsim/config.h")

################################
# tsim base
################################

file(GLOB tsim_h ${CMAKE_CURRENT_LIST_DIR}/src/tsim/*.h)
file(GLOB tsim_c ${CMAKE_CURRENT_LIST_DIR}/src/tsim/*.c)
add_library(tsim
	${tsim_h}
	${tsim_c})
target_include_directories(tsim PUBLIC ${CMAKE_CURRENT_LIST_DIR}/src)
target_link_libraries(tsim mugglec haclog LibXml2::LibXml2)
add_dependencies(tsim mugglec haclog LibXml2::LibXml2)
target_compile_definitions(tsim PRIVATE TSIM_EXPORTS)
set_target_properties(tsim PROPERTIES 
	LINKER_LANGUAGE C
	VERSION ${TSIM_VERSION}
	SOVERSION ${TSIM_SOVERSION}
	DEBUG_POSTFIX d
)

if (APPLE)
	set_target_properties(tsim
		PROPERTIES
		INSTALL_RPATH "@executable_path/../lib"
	)
else ()
	set_target_properties(tsim
		PROPERTIES
		INSTALL_RPATH "\$ORIGIN/../lib"
	)
endif()
install(TARGETS tsim
	RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
	ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
	LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")

################################
# function
################################

function(add_module name folder)
	file(GLOB tmp_h ${folder}/*.h)
	file(GLOB tmp_c ${folder}/*.c)

	if (WIN32)
		add_library(${name} STATIC ${tmp_h} ${tmp_c})
	else()
		add_library(${name} STATIC ${tmp_c})
	endif(WIN32)
	add_dependencies(${name} tsim)
	target_link_libraries(${name} PUBLIC tsim)
	target_include_directories(${name} PUBLIC ${folder})

	if (APPLE)
		set_target_properties(${name}
			PROPERTIES
			INSTALL_RPATH "@executable_path/../lib"
		)
	else ()
		set_target_properties(${name}
			PROPERTIES
			INSTALL_RPATH "\$ORIGIN/../lib"
		)
	endif()
endfunction()

function(add_app name folder)
	file(GLOB tmp_h ${folder}/*.h)
	file(GLOB tmp_c ${folder}/*.c)

	if (WIN32)
		add_executable(${name} ${tmp_h} ${tmp_c})
		set_target_properties(${name}
			PROPERTIES
			FOLDER "app"
			VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)"
		)
	else()
		add_executable(${name} ${tmp_c})
	endif(WIN32)

	install(TARGETS ${name}
		RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
		ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
		LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")
	if (APPLE)
		set_target_properties(${name}
			PROPERTIES
			INSTALL_RPATH "@executable_path/../lib"
		)
	else ()
		set_target_properties(${name}
			PROPERTIES
			INSTALL_RPATH "\$ORIGIN/../lib"
		)
	endif()
endfunction()

function(add_app_module name folder)
	add_module(tsim_module_${name} ${folder}/module)
	add_app(tsim_${name} ${folder}/exe)
	add_dependencies(tsim_${name} tsim_module_${name})
	target_link_libraries(tsim_${name} PUBLIC tsim_module_${name})
endfunction()

################################
# exe & module
################################

set(src_dir "${CMAKE_CURRENT_LIST_DIR}/src")

add_app_module(tgw "${src_dir}/tgw")
