# 
#

project(patmos)

cmake_minimum_required(VERSION 2.8)

# Find boost library
include(FindBoost)
find_package( Boost 1.40.0 COMPONENTS program_options REQUIRED)

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

# add sub-directories
add_subdirectory(src)

