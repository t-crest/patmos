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

add_library(ramulator ALDRAM.cpp Cache.cpp Config.cpp Controller.cpp
                      DDR3.cpp DDR4.cpp DSARP.cpp GDDR5.cpp HBM.cpp
                      LPDDR3.cpp LPDDR4.cpp Main.cpp MemoryFactory.cpp
                      Processor.cpp Refresh.cpp SALP.cpp StatType.cpp TLDRAM.cpp
                      WideIO2.cpp WideIO.cpp)

set_property(TARGET ramulator PROPERTY CXX_STANDARD 11)
set_property(TARGET ramulator PROPERTY CXX_STANDARD_REQUIRED ON)
