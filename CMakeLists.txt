cmake_minimum_required(VERSION 3.10)

project(TradingGame CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(Threads REQUIRED)

add_executable(TradingEngine 
    cpp_engine/main.cpp 
    cpp_engine/order_book.cpp
    cpp_engine/traders.cpp
)

target_link_libraries(TradingEngine Threads::Threads)