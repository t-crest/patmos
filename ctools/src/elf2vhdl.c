#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>

#include <gelf.h>
#include <libelf.h>

// TODO assert...

static void readelf(const char *name)
{
  // check libelf version
  elf_version(EV_CURRENT);

  // open elf binary
  int fd = open(name, O_RDONLY, 0);
  // assert(fd > 0);

  Elf *elf = elf_begin(fd, ELF_C_READ, NULL);
  // assert(elf);

  // check file kind
  Elf_Kind ek = elf_kind(elf);
  // assert(ek == ELF_K_ELF);

  // check class
  int ec = gelf_getclass(elf);
  // assert(ec == ELFCLASS32);

  // get elf header
  GElf_Ehdr hdr;
  GElf_Ehdr *tmphdr = gelf_getehdr(elf, &hdr);
  // assert(tmphdr);

  // get program headers
  size_t n, i;
  int ntmp = elf_getphdrnum (elf, &n);
  // assert(ntmp == 0);

  for(i = 0; i < n; i++)
  {
    // get program header
    GElf_Phdr phdr;
    GElf_Phdr *phdrtmp = gelf_getphdr(elf, i, &phdr);
    // assert(phdrtmp);

    if (phdr.p_type == PT_LOAD)
    {
      // some assertions
      // assert(phdr.p_vaddr == phdr.p_paddr);
      // assert(phdr.p_filesz <= phdr.p_memsz);

      // allocate buffer
      char *buf = malloc(phdr.p_filesz);
      // assert(buf);

      printf("Size is %d\n", phdr.p_filesz);

      // copy from the buffer into the main memory
      lseek(fd, phdr.p_offset, SEEK_SET);
      read(fd, buf, phdr.p_filesz);

      // TODO: generate code from buffer
      unsigned int this_is_the_start_offset = phdr.p_vaddr;
      unsigned int this_is_the_size_in_the_file = phdr.p_filesz;
      unsigned int this_is_the_total_size = phdr.p_memsz;

      free(buf);
    }
  }

  // TODO: emit code to branch to entry here
  unsigned int this_is_the_entry = hdr.e_entry;

  elf_end(elf);
}

int main(int argc, char* argv[]) {

	if (argc!=2) {
		printf("Argument missing\n");
		exit(-1);
	}

	readelf(argv[1]);
	
	return 0;
}
