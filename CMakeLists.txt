cmake_minimum_required(VERSION 3.5)

# Set extension name here
set(TARGET_NAME netquack)
set(EXTENSION_NAME ${TARGET_NAME}_extension)
set(LOADABLE_EXTENSION_NAME ${TARGET_NAME}_loadable_extension)

project(${TARGET_NAME})

# Force C++17 standard for string_view support
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(EXTENSION_SOURCES ${EXTENSION_SOURCES})
include_directories(src/include)
file(GLOB_RECURSE EXTENSION_SOURCES src/*.cpp)

build_static_extension(${TARGET_NAME} ${EXTENSION_SOURCES})
build_loadable_extension(${TARGET_NAME} " " ${EXTENSION_SOURCES})

# Link Curl in both the static library as the loadable extension
find_package(CURL REQUIRED)
target_link_libraries(${EXTENSION_NAME} CURL::libcurl)
target_link_libraries(${LOADABLE_EXTENSION_NAME} CURL::libcurl)

install(
  TARGETS ${EXTENSION_NAME}
  EXPORT "${DUCKDB_EXPORT_SET}"
  LIBRARY DESTINATION "${INSTALL_LIB_DIR}"
  ARCHIVE DESTINATION "${INSTALL_LIB_DIR}")
