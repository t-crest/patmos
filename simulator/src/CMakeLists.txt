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

add_library(patmos-simulator assembler.cc simulation-core.cc instruction.cc
                             binary-formats.cc decoder.cc command-line.cc
                             symbol.cc profiling.cc excunit.cc memory-map.cc
                             dbgstack.cc loader.cc memory.cc method-cache.cc
			     stack-cache.cc data-cache.cc instr-cache.cc)

add_executable(pasim pasim.cc)

target_link_libraries(pasim patmos-simulator ${Boost_LIBRARIES} ${ELF})

add_executable(paasm paasm.cc)

target_link_libraries(paasm patmos-simulator ${Boost_LIBRARIES})

add_executable(padasm padasm.cc)

target_link_libraries(padasm patmos-simulator ${Boost_LIBRARIES} ${ELF})

add_executable(pacheck pacheck.cc)

target_link_libraries(pacheck patmos-simulator ${Boost_LIBRARIES} ${ELF})

install(TARGETS pasim paasm padasm pacheck RUNTIME DESTINATION bin)
