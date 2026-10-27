cmake_minimum_required(VERSION 3.20)
project(pqc_zero_trust_router VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Strict compiler warnings to ensure clean, industry-grade code for evaluation
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -O2)
endif()

include_directories(${PROJECT_SOURCE_DIR}/include)

add_executable(pqc_router
        src/main.cpp
        src/PqcNttEngine.cpp
        src/ZeroTrustRouter.cpp
        src/CepTestSuite.cpp
)