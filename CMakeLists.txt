cmake_minimum_required(VERSION 3.24)
project(My_Raft_Practice)


set(CMAKE_CXX_STANDARD 20)
find_package(Boost 1.74.0 REQUIRED COMPONENTS system)
include_directories(${Boost_INCLUDE_DIRS})
if(Boost_FOUND)
add_executable(My_Raft_Practice main.cpp
        src/Node.h
        src/states/Follower.h
        src/states/Leader.h
        src/states/Candidate.h
        src/Raft_server.h
        src/NetworkContext.h
        src/Contexts.h
        src/states/Follower.cpp
        src/states/Candidate.cpp
        src/states/Leader.cpp)

target_link_libraries(My_Raft_Practice PRIVATE Boost::system)
else ()
    message(FATAL_ERROR "Boost not found")
endif ()