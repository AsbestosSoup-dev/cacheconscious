cmake_minimum_required(VERSION 3.20)
project(cacheconscious CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Wipe any default flags CMake would inject for Release/Debug.
# We control flags entirely via target_compile_options below.
set(CMAKE_CXX_FLAGS "" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS_DEBUG "" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS_RELEASE "" CACHE STRING "" FORCE)

set(COMMON_SOURCES
    benchmark/oop/world_aos.cpp
    benchmark/ecs/world_soa.cpp
    benchmark/configs/config_1.cpp
    benchmark/configs/config_2.cpp
    benchmark/configs/config_3.cpp
    benchmark/configs/config_4.cpp
    benchmark/configs/config_5.cpp
    benchmark/configs/config_6.cpp
    benchmark/configs/config_7.cpp
    benchmark/configs/config_8.cpp
    main.cpp
)

# ---- O0 binary ----
add_executable(cache_conscious_O0 ${COMMON_SOURCES})
target_include_directories(cache_conscious_O0 PRIVATE ${CMAKE_SOURCE_DIR})
target_compile_options(cache_conscious_O0 PRIVATE
    -O0
    -Wall
    -Wextra
)
target_compile_definitions(cache_conscious_O0 PRIVATE
    BUILD_FLAGS_STR="-O0 -std=c++17"
    BUILD_TAG="O0"
)

# ---- O2 binary ----
add_executable(cache_conscious_O2 ${COMMON_SOURCES})
target_include_directories(cache_conscious_O2 PRIVATE ${CMAKE_SOURCE_DIR})
target_compile_options(cache_conscious_O2 PRIVATE
    -O2
    -Wall
    -Wextra
)
target_compile_definitions(cache_conscious_O2 PRIVATE
    BUILD_FLAGS_STR="-O2 -std=c++17"
    BUILD_TAG="O2"
)

# ---- O3 binary ----
add_executable(cache_conscious_O3 ${COMMON_SOURCES})
target_include_directories(cache_conscious_O3 PRIVATE ${CMAKE_SOURCE_DIR})
target_compile_options(cache_conscious_O3 PRIVATE
    -O3
    -Wall
    -Wextra
)
target_compile_definitions(cache_conscious_O3 PRIVATE
    BUILD_FLAGS_STR="-O3 -std=c++17"
    BUILD_TAG="O3"
)
