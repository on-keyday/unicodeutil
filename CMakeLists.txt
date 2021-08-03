cmake_minimum_required(VERSION 3.16)
project(unicode)

include_directories("src/commonlib")

set(CMAKE_CXX_STANDARD 20)

add_executable(unicode "src/main.cpp")

add_library(unicoderuntime SHARED "src/runtime.cpp" "src/search.cpp" "src/random.cpp" "src/makebin.cpp" "src/common.cpp" "src/utf.cpp")

add_library(unicodedata SHARED "src/unicodeload.cpp")

target_link_libraries(unicode unicoderuntime)

target_link_libraries(unicoderuntime unicodedata)

if(MSVC)
target_compile_options(unicodedata PUBLIC /EHsc /source-charset:utf-8 /Zc:__cplusplus)
target_compile_options(unicode PUBLIC /EHsc /source-charset:utf-8 /Zc:__cplusplus)
endif()

