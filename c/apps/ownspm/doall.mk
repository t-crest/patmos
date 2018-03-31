# This Makefile is used from the main Patmos folder to run all measurements
# for one buffer configuration. Buffer configuration is set in ownspm.h

EMUP?=exp

all:
	-rm log.txt
	make app APP=ownspm MAIN=pc_main_mem
	$(EMUP)/emu4sspm tmp/ownspm.elf >> log.txt
	make app APP=ownspm MAIN=pc_sspm
	$(EMUP)/emu8sspm tmp/ownspm.elf >> log.txt
	make app APP=ownspm MAIN=pc_sspm
	$(EMUP)/emu4sspm tmp/ownspm.elf >> log.txt
	make app APP=ownspm MAIN=pc_pool
	$(EMUP)/emu4_8multi tmp/ownspm.elf >> log.txt
	make app APP=ownspm MAIN=pc_own
	$(EMUP)/emu4_8own tmp/ownspm.elf >> log.txt
	grep measure log.txt > results.txt
	cat results.txt
