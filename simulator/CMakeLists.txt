# 
#  This file is part of the Patmos Simulator.
#  The Patmos Simulator is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
# 
#  The Patmos Simulator is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
# 
#  You should have received a copy of the GNU General Public License
#  along with the Patmos Simulator. If not, see <http://www.gnu.org/licenses/>.
#

project(patmos)

cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_FLAGS "-O2 -g -ftemplate-depth-1024")

# Find boost library
include(FindBoost)
find_package( Boost 1.46.0 COMPONENTS program_options unit_test_framework 
  system REQUIRED)

# Find libelf
find_library(ELF elf)
if (NOT ELF)
  message(FATAL_ERROR "libelf library not found.")
endif()

find_path(ELF_INCLUDE_DIRS gelf.h PATH_SUFFIXES libelf)
if (NOT ELF_INCLUDE_DIRS)
  message(FATAL_ERROR "libelf headers not found.")
endif()

include_directories(include ${Boost_INCLUDE_DIRS} ${ELF_INCLUDE_DIRS})

# enable testing using ctest
enable_testing()

# add sub-directories
add_subdirectory(src)
add_subdirectory(src/debug)
add_subdirectory(tests)
add_subdirectory(tests/debug)
