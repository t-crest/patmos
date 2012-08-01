#
#

add_executable(elf2vhdl elf2vhdl.c)

target_link_libraries(elf2vhdl ${ELF})

install(TARGETS elf2vhdl RUNTIME DESTINATION bin)
