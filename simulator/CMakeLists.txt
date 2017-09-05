# 
#  Copyright 2012 Technical University of Denmark, DTU Compute.
#  All rights reserved.
#
#  This file is part of the Patmos Simulator.
#
#   Redistribution and use in source and binary forms, with or without
#   modification, are permitted provided that the following conditions are met:
#
#      1. Redistributions of source code must retain the above copyright notice,
#         this list of conditions and the following disclaimer.
#
#      2. Redistributions in binary form must reproduce the above copyright
#         notice, this list of conditions and the following disclaimer in the
#         documentation and/or other materials provided with the distribution.
#
#   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER ``AS IS'' AND ANY EXPRESS
#   OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
#   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
#   NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
#   DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
#   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
#   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
#   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
#   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
#   THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
#   The views and conclusions contained in the software and documentation are
#   those of the authors and should not be interpreted as representing official
#   policies, either expressed or implied, of the copyright holder.


project(patmos)

cmake_minimum_required(VERSION 2.8)

option(USE_RAMULATOR "USE_RAMULATOR" OFF)

# Build release with debug by default
if (NOT CMAKE_BUILD_TYPE)
  message(STATUS "No build type selected, defaulting to RelWithDebInfo")
  set(CMAKE_BUILD_TYPE "RelWithDebInfo")
else()
  message(STATUS "Build type set to: ${CMAKE_BUILD_TYPE}")
endif()

# Find boost library
include(FindBoost)
find_package( Boost 1.46.0 COMPONENTS program_options REQUIRED)

# Find libelf
find_library(ELF elf)
if (NOT ELF)
  message(FATAL_ERROR "libelf library not found.")
endif()

find_path(ELF_INCLUDE_DIRS gelf.h PATH_SUFFIXES libelf)
if (NOT ELF_INCLUDE_DIRS)
  message(FATAL_ERROR "libelf headers not found.")
endif()

include_directories(include ./ ${Boost_INCLUDE_DIRS} ${ELF_INCLUDE_DIRS})

# make sure the boost templates compile
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftemplate-depth-1024")

# enable testing using ctest
enable_testing()

# add sub-directories
if(USE_RAMULATOR)
  add_definitions("-DRAMULATOR")
  add_subdirectory(ramulator)
endif()
add_subdirectory(src)
add_subdirectory(tests)

