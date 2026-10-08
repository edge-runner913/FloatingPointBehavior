cmake_minimum_required(VERSION 3.25)
project(LeetCode)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) # 3. 禁用編譯器特定擴充，只使用純標準 C++
add_executable(f16_test
        f16_test.cpp
        ScopeTimer.cpp
)
add_executable(f32_test
        float_test.cpp
        ScopeTimer.cpp
)
add_executable(double_test
        double_test.cpp
        ScopeTimer.cpp
        weird_float.cpp
        fast_math.cpp
        Q4.cpp
)
add_executable(long_double_test_test
        long_double_test.cpp
        ScopeTimer.cpp
)

