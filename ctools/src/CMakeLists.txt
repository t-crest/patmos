#
#

add_executable(elf2bin elf2bin.c)

target_link_libraries(elf2bin ${ELF})

install(TARGETS elf2bin RUNTIME DESTINATION bin)
