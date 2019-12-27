cmake_minimum_required(VERSION 3.1.0)

project(scaper)
set (CMAKE_CXX_STANDARD 11)
# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed
set(CMAKE_AUTOMOC ON)
# Create code from a list of Qt designer ui files
#set(CMAKE_AUTOUIC ON)

# SERVER

#set include directory
set(HDRS inc/)
set(SRCS src/)
include_directories(${HDRS})
# Populate a CMake variable with the sources
set(SRCS
        ${SRCS}/main.c
        #${SRCS}/server.cpp
	    #${HDRS}/scaper.h)
	    )
add_executable(glaas_srv WIN32 ${SRCS})


unset (SRCS)
# CLIENT

#set include directory
set(HDRS inc/)
set(SRCS src/)
include_directories(${HDRS})
# Populate a CMake variable with the sources
set(SRCS
        ${SRCS}/client.c
        #${SRCS}/server.cpp
	    #${HDRS}/scaper.h)
	    )
add_executable(glaas_clnt WIN32 ${SRCS})
