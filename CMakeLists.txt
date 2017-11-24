cmake_minimum_required(VERSION 3.5)

SET(BOOST_DIR /home/sehe/custom/boost)
ADD_EXECUTABLE(demo main.cpp)
#SET(CMAKE_CXX_COMPILER clang++-5.0)

execute_process(COMMAND pkg-config --cflags libxml++-2.6 OUTPUT_VARIABLE XMLPPFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE) 
execute_process(COMMAND pkg-config --libs libxml++-2.6 OUTPUT_VARIABLE XMLPPLIBS OUTPUT_STRIP_TRAILING_WHITESPACE)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} \
    -Wall -Wextra -pedantic -Wno-deprecated-declarations \
    -isystem ${BOOST_DIR} \
    ${XMLPPFLAGS} \
    -I. \
    -std=c++11 \
    -fsanitize=address,undefined \
    -O0 -ggdb -pthread -march=native")

TARGET_LINK_LIBRARIES(demo pugixml ${XMLPPLIBS})
