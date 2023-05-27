cmake_minimum_required(VERSION 3.1)

project(WHR)

set(CMAKE_CXX_STANDARD 11)

add_library(whr
 src/CWHR.cpp
 src/userflag.cpp
 src/CGameCollection.cpp
 src/CEGF.cpp
 src/CElo.cpp
 src/CGlicko.cpp
 src/CTrueSkill.cpp
 src/CIncrementalWHR.cpp
 src/CDecayedHistory.cpp
)
