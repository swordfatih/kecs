cmake_minimum_required(VERSION 3.28)

set(KECS_VERSION 0.0.1)
project(kecs VERSION ${KECS_VERSION} LANGUAGES CXX)

include(FetchContent)

FetchContent_Declare(kdb GIT_REPOSITORY https://github.com/KxSystems/kdb.git GIT_TAG master)
FetchContent_MakeAvailable(kdb)

FetchContent_Declare(spdlog GIT_REPOSITORY https://github.com/gabime/spdlog.git GIT_TAG v1.x)
FetchContent_MakeAvailable(spdlog)

FetchContent_Declare(boost_pfr GIT_REPOSITORY https://github.com/boostorg/pfr.git GIT_TAG 2.2.0)
FetchContent_MakeAvailable(boost_pfr)

file(GLOB_RECURSE project_sources "${CMAKE_SOURCE_DIR}/src/*.cpp")
add_executable(kecs ${project_sources})

target_include_directories(kecs PRIVATE ${CMAKE_SOURCE_DIR}/include ${kdb_SOURCE_DIR}/c)
target_compile_features(kecs PRIVATE cxx_std_23)
# target_compile_options(kecs PRIVATE -Wall -Wextra -Wpedantic -Wno-unused-but-set-variable -Wno-unused-parameter)

set(KDB_LIB ${kdb_SOURCE_DIR}/w64/c.lib)
target_link_libraries(kecs PRIVATE ${KDB_LIB} ws2_32 iphlpapi spdlog::spdlog Boost::pfr)