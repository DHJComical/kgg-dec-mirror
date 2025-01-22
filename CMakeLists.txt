cmake_minimum_required(VERSION 3.10)

project(kgg-dec VERSION 0.3 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(kgg-dec
        src/main.cpp
        src/common/base64.cpp
        src/infra/infra.cpp
        src/tc_tea/tc_tea.cpp
        src/qmc2/ekey.cpp
        src/qmc2/qmc2_factory.cpp
        src/qmc2/qmc2_map.cpp
        src/qmc2/qmc2_rc4.cpp
)

target_include_directories(kgg-dec
        PRIVATE
        src/common
        src/tc_tea
)

target_link_libraries(kgg-dec PRIVATE shell32 ole32)
target_compile_definitions(kgg-dec PRIVATE NOMINMAX)
target_compile_definitions(kgg-dec PRIVATE KGGDEC_PROJECT_VERSION="${PROJECT_VERSION}")
