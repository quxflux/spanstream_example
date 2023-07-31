project (csv_parsing)

set(BENCHMARK_ENABLE_GTEST_TESTS False)
set(BENCHMARK_ENABLE_TESTING False)
CPMAddPackage("gh:google/benchmark#v1.8.2")

add_library(base_project INTERFACE)
target_include_directories(base_project INTERFACE "inc")

add_executable(csv_parsing)
target_sources(csv_parsing PRIVATE "src/csv_parsing.cpp")
target_link_libraries(csv_parsing base_project benchmark::benchmark)
set_target_properties(csv_parsing PROPERTIES CXX_STANDARD 23
                                             CXX_STANDARD_REQUIRED ON
                                             CXX_EXTENSIONS OFF)

add_executable(data_serialization)
target_sources(data_serialization PRIVATE "src/data_serialization.cpp")
target_link_libraries(data_serialization base_project benchmark::benchmark)
set_target_properties(data_serialization PROPERTIES CXX_STANDARD 23
                                                    CXX_STANDARD_REQUIRED ON
                                                    CXX_EXTENSIONS OFF)
