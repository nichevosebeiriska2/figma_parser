cmake_minimum_required(VERSION 3.10)

# Force static linking of MSVC runtime (/MT, /MTd) to avoid vcruntime140.dll dependency
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(FigmaToJson)

include(FetchContent)

# --- Global dependency settings (must be set before FetchContent_MakeAvailable) ---

# Force static builds for all dependencies
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build static libraries" FORCE)

# Zstd build flags
set(ZSTD_BUILD_STATIC ON CACHE BOOL "Build static zstd" FORCE)
set(ZSTD_BUILD_SHARED OFF CACHE BOOL "Do not build shared zstd" FORCE)
set(ZSTD_BUILD_PROGRAMS OFF CACHE BOOL "Skip zstd CLI tools" FORCE)

# Zlib build flags
set(ZLIB_BUILD_EXAMPLES OFF CACHE BOOL "Skip zlib examples" FORCE)
set(SKIP_INSTALL_ALL ON CACHE BOOL "Skip installation" FORCE)

# --- Fetch dependencies ---

FetchContent_Declare(
  zstd
  URL "https://github.com/facebook/zstd/releases/download/v1.5.5/zstd-1.5.5.tar.gz"
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
  SOURCE_SUBDIR build/cmake
)

FetchContent_Declare(
  ZLIB
  GIT_REPOSITORY https://github.com/madler/zlib.git
  GIT_TAG        v1.3.1
  GIT_SHALLOW    TRUE
)

FetchContent_Declare(
  rapidjson
  GIT_REPOSITORY https://github.com/Tencent/rapidjson.git
  GIT_TAG        v1.1.0  
)

FetchContent_Populate(rapidjson)
FetchContent_MakeAvailable(zstd ZLIB)

# --- Main executable ---

file(GLOB_RECURSE HEADER_FILES ${CMAKE_SOURCE_DIR}/include/*.h)
file(GLOB_RECURSE SOURCE_FILES ${CMAKE_SOURCE_DIR}/src/*.cpp)

add_executable(${PROJECT_NAME} 
    ${SOURCE_FILES}
    ${HEADER_FILES}
)

# Resolve zlib target name (usually zlibstatic for static builds)
if(TARGET zlibstatic)
    set(ZLIB_TARGET zlibstatic) 
elseif(TARGET zlib)
    set(ZLIB_TARGET zlib)
else()
    message(FATAL_ERROR "zlib target not found!")
endif()

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/kiwi
    ${CMAKE_SOURCE_DIR}/include/json
    ${rapidjson_SOURCE_DIR}/include
    ${zlib_SOURCE_DIR}      # zlib.h
    ${zlib_BINARY_DIR}      # zconf.h (generated)
    ${zstd_SOURCE_DIR}/lib  # zstd.h
)

target_link_libraries(${PROJECT_NAME} PRIVATE 
    ${ZLIB_TARGET}
    libzstd_static
)

# Required for some static zstd API calls
target_compile_definitions(${PROJECT_NAME} PRIVATE ZSTD_STATIC_LINKING_ONLY)

# --- Visual Studio debug settings ---

set_target_properties(${PROJECT_NAME} PROPERTIES 
    VS_DEBUGGER_COMMAND "$<TARGET_FILE:${PROJECT_NAME}>"
    # VS_DEBUGGER_COMMAND_ARGUMENTS "--input test.fig --output out.json"
)