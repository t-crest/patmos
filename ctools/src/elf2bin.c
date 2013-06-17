#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <stddef.h>
#include <sys/types.h>
// #include <net/hton.h>

#include <gelf.h>
#include <libelf.h>

static void elf2bin(int infd, int outfd)
{
  // check libelf version
  elf_version(EV_CURRENT);

  Elf *elf = elf_begin(infd, ELF_C_READ, NULL);
  assert(elf);

  // check file kind
  Elf_Kind ek = elf_kind(elf);
  assert(ek == ELF_K_ELF);

  // check class
  int ec = gelf_getclass(elf);
  assert(ec == ELFCLASS32);

  // get elf header
  GElf_Ehdr hdr;
  GElf_Ehdr *tmphdr = gelf_getehdr(elf, &hdr);
  assert(tmphdr);

  // get program headers
  size_t n, i;
  int ntmp = elf_getphdrnum (elf, &n);
  assert(ntmp == 0);

  for(i = 0; i < n; i++)
  {
    // get program header
    GElf_Phdr phdr;
    GElf_Phdr *phdrtmp = gelf_getphdr(elf, i, &phdr);
    assert(phdrtmp);

    if (phdr.p_type == PT_LOAD)
    {
      // some assertions
      assert(phdr.p_vaddr == phdr.p_paddr);
      assert(phdr.p_filesz <= phdr.p_memsz);

      // allocate buffer
      char *buf = malloc(phdr.p_filesz);
      assert(buf);

      // copy from the file into the main memory
	  lseek(infd, phdr.p_offset, SEEK_SET);
      read(infd, buf, phdr.p_filesz);

	  // reposition and write to output file
	  lseek(outfd, phdr.p_paddr, SEEK_SET);
	  write(outfd, buf, phdr.p_filesz);

	  // pad to correct size
	  size_t k;
	  for (k = phdr.p_filesz; k < phdr.p_memsz; k++) {
		const int b = 0;
		write(outfd, &b, 1);
	  }

	  free(buf);
    }
  }

  elf_end(elf);
}


int main(int argc, char* argv[]) {

	if (argc != 3) {
	  fprintf(stderr, "Usage: %s infile outfile\n", argv[0]);
		exit(-1);
	}

	int infd = open(argv[1], O_RDONLY, 0);
	if (infd == -1) {
	  perror("Cannot open input file:");
	}

	int outfd = open(argv[2], O_CREAT | O_WRONLY | O_TRUNC, 0644);
	if (outfd == -1) {
	  perror("Cannot open output file:");
	}

	elf2bin(infd, outfd);

	close(infd);
	close(outfd);

	return 0;
}
