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

add_library(patmos-simulator assembler.cc simulation-core.cc instruction.cc
                             binary-formats.cc decoder.cc command-line.cc
                             symbol.cc profiling.cc excunit.cc memory-map.cc
                             dbgstack.cc loader.cc memory.cc method-cache.cc
                             stack-cache.cc data-cache.cc instr-cache.cc
                             instr-spm.cc)

add_executable(pasim pasim.cc)

if(USE_RAMULATOR)
  target_link_libraries(pasim patmos-simulator ramulator ${Boost_LIBRARIES} ${ELF})
else()
  target_link_libraries(pasim patmos-simulator ${Boost_LIBRARIES} ${ELF})
endif()

add_executable(paasm paasm.cc)

target_link_libraries(paasm patmos-simulator ${Boost_LIBRARIES})

add_executable(padasm padasm.cc)

target_link_libraries(padasm patmos-simulator ${Boost_LIBRARIES} ${ELF})

add_executable(pacheck pacheck.cc)

target_link_libraries(pacheck patmos-simulator ${Boost_LIBRARIES} ${ELF})

install(TARGETS pasim paasm padasm pacheck RUNTIME DESTINATION bin)
