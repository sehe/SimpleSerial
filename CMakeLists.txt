cmake_minimum_required(VERSION 3.5)

SET(BOOST_DIR /home/sehe/custom/boost)
ADD_EXECUTABLE(demo main.cpp)
#SET(CMAKE_CXX_COMPILER clang++-5.0)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} \
    -Wall -Wextra -pedantic \
    -isystem ${BOOST_DIR} \
    -I. \
    -std=c++14 \
    -fsanitize=address,undefined \
    -O2 -g -pthread -march=native")

TARGET_LINK_LIBRARIES(demo pugixml)
