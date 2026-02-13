cmake_minimum_required(VERSION 3.17)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(FigmaToJson)
include(FetchContent)

# fetch required modules

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

FetchContent_MakeAvailable(zstd ZLIB rapidjson)



# Set variables to control ZSTD's build options (e.g., static vs shared)
set(ZSTD_BUILD_STATIC ON)
set(ZSTD_BUILD_SHARED OFF)

set(ZLIB_LIBRARY zlib)
set(ZLIB_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SKIP_INSTALL_ALL ON CACHE BOOL "" FORCE)

file(GLOB_RECURSE HEADER_FILES ${CMAKE_SOURCE_DIR}/*.h)
file(GLOB_RECURSE SOURCE_FILES ${CMAKE_SOURCE_DIR}/src/*.cpp)

add_executable(${PROJECT_NAME} 
${SOURCE_FILES}
${HEADER_FILES}
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/kiwi
    ${CMAKE_SOURCE_DIR}/include/json
    ${rapidjson_SOURCE_DIR}/include
    ${zstd_SOURCE_DIR}/lib
    ${zlib_SOURCE_DIR}
    ${zlib_BINARY_DIR}
)

target_link_libraries(${PROJECT_NAME} PUBLIC zlib)
target_link_libraries(${PROJECT_NAME} PUBLIC libzstd_static)

if(TARGET zlibstatic)
    set(ZLIB_TARGET zlibstatic) 
elseif(TARGET zlib)
    set(ZLIB_TARGET zlib)
else()
    message(FATAL_ERROR "zlib not found after add_subdirectory!")
endif()

add_custom_command(
  TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:zlib>
    $<TARGET_FILE_DIR:${PROJECT_NAME}>
  COMMENT "Copying ZLIB DLL to output directory"
)

target_compile_definitions(${PROJECT_NAME} PRIVATE ZSTD_STATIC_LINKING_ONLY)
set_target_properties(${PROJECT_NAME} PROPERTIES 
    VS_DEBUGGER_COMMAND "debug_command"
    VS_DEBUGGER_COMMAND_ARGUMENTS "debug_arguments")