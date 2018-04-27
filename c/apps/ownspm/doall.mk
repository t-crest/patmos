# This Makefile is used from the main Patmos folder to run all measurements
# for one buffer configuration. Buffer configuration is set in ownspm.h

EMUP?=exp

all: manual

manual:
	-rm log.txt
	make app APP=ownspm MAIN=pc COPTS="-D DATA_LEN=4096 -D _MAINMEM"
	$(EMUP)/emu4sspm tmp/ownspm.elf >> log.txt
	make app APP=ownspm MAIN=pc COPTS="-D DATA_LEN=4096 -D _SSPM"
	$(EMUP)/emu4sspm tmp/ownspm.elf >> log.txt
	$(EMUP)/emu8sspm tmp/ownspm.elf >> log.txt
	make app APP=ownspm MAIN=pc COPTS="-D DATA_LEN=4096 -D _OWN"
	$(EMUP)/emu4_8own tmp/ownspm.elf >> log.txt
	make app APP=ownspm MAIN=pc COPTS="-D DATA_LEN=4096 -D _MULTIOWN"
	$(EMUP)/emu4_8multi tmp/ownspm.elf >> log.txt
	grep measure log.txt > results.txt
	cat results.txt

pc:
	-rm log.txt
	for i in 4 8 16 32 64 128 256; \
	do \
		make app APP=ownspm MAIN=pc COPTS="-D DATA_LEN=4096 -D BUFFER_SIZE=$$i -D _MAINMEM"; \
		$(EMUP)/emu4sspm tmp/ownspm.elf >> log.txt; \
		make app APP=ownspm MAIN=pc COPTS="-D DATA_LEN=4096 -D BUFFER_SIZE=$$i -D _SSPM"; \
		$(EMUP)/emu4sspm tmp/ownspm.elf >> log.txt; \
		$(EMUP)/emu8sspm tmp/ownspm.elf >> log.txt; \
		make app APP=ownspm MAIN=pc COPTS="-D DATA_LEN=4096 -D BUFFER_SIZE=$$i -D _OWN"; \
		$(EMUP)/emu4_8own tmp/ownspm.elf >> log.txt; \
		make app APP=ownspm MAIN=pc COPTS="-D DATA_LEN=4096 -D BUFFER_SIZE=$$i -D _MULTIOWN"; \
		$(EMUP)/emu4_8multi tmp/ownspm.elf >> log.txt; \
	done
	grep measure log.txt > results.txt
	cat results.txt

df_single:
	-rm log.txt
#	make app APP=ownspm MAIN=pc_df COPTS="-D DATA_LEN=4096 -D BUFFER_SIZE=$$i -D _MAINMEM_DF -D _SINGLE_RELAY"; \
#	$(EMUP)/emu4sspm tmp/ownspm.elf >> log.txt
#	make app APP=ownspm MAIN=pc_df COPTS="-D DATA_LEN=4096 -D BUFFER_SIZE=$$i -D _SSPM_DF -D _SINGLE_RELAY"; \
#	$(EMUP)/emu4sspm tmp/ownspm.elf >> log.txt; 
#	$(EMUP)/emu8sspm tmp/ownspm.elf >> log.txt; 
#	make app APP=ownspm MAIN=pc_df COPTS="-D DATA_LEN=4096 -D BUFFER_SIZE=$$i -D _OWN_DF -D _SINGLE_RELAY"; \
#	$(EMUP)/emu4_8own tmp/ownspm.elf >> log.txt
#	make app APP=ownspm MAIN=pc_df COPTS="-D DATA_LEN=4096 -D BUFFER_SIZE=$$i -D _MULTIOWN_DF -D _SINGLE_RELAY"; \
#	$(EMUP)/emu4_8multi tmp/ownspm.elf >> log.txt
	grep measure log.txt > results.txt
	cat results.txt

df_multi:
# _MULTIOWN_DF with multiple relay nodes requires SPM number of (CNT-1)*2
	-rm log.txt
#	make app APP=ownspm MAIN=pc_df COPTS="-D DATA_LEN=4096 -D BUFFER_SIZE=$$i -D _MAINMEM_DF -D _MULTI_RELAY"; \
#	$(EMUP)/emu4sspm tmp/ownspm.elf >> log.txt
#	make app APP=ownspm MAIN=pc_df COPTS="-D DATA_LEN=4096 -D BUFFER_SIZE=$$i -D _SSPM_DF -D _MULTI_RELAY"; \
#	$(EMUP)/emu4sspm tmp/ownspm.elf >> log.txt; 
#	$(EMUP)/emu8sspm tmp/ownspm.elf >> log.txt; 
#	make app APP=ownspm MAIN=pc_df COPTS="-D DATA_LEN=4096 -D BUFFER_SIZE=$$i -D _OWN_DF -D _MULTI_RELAY"; \
#	$(EMUP)/emu4_8own tmp/ownspm.elf >> log.txt
#	make app APP=ownspm MAIN=pc_df COPTS="-D DATA_LEN=4096 -D BUFFER_SIZE=$$i -D _MULTIOWN_DF -D _MULTI_RELAY"; \
#	$(EMUP)/emu4_8multi tmp/ownspm.elf >> log.txt
	grep measure log.txt > results.txt
	cat results.txt
