cmake_minimum_required(VERSION 3.9)
project(myshell)

find_package(Boost COMPONENTS program_options filesystem REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

set(CMAKE_CXX_STANDARD 14)

add_executable(myshell myshell.cpp)
add_executable(mycat mycat.cpp)
add_executable(myls myls.cpp)

add_executable(mymkdir mymkdir.cpp)
target_link_libraries(mymkdir ${Boost_LIBRARIES})

add_executable(mycp mycp.cpp)
target_link_libraries(mycp ${Boost_LIBRARIES})

add_executable(mymv mymv.cpp)
target_link_libraries(mymv ${Boost_LIBRARIES})

add_executable(myrm myrm.cpp)
target_link_libraries(myrm ${Boost_LIBRARIES})
