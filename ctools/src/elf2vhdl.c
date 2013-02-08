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

// Hoping that int is 32 bits on the machines we use...
static int *progr = NULL;
static int start;
static int words;
static int entry;

static void readelf(const char *name)
{
  // check libelf version
  elf_version(EV_CURRENT);

  // open elf binary
  int fd = open(name, O_RDONLY, 0);
  assert(fd > 0);

  Elf *elf = elf_begin(fd, ELF_C_READ, NULL);
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


      // copy from the buffer into the main memory
      lseek(fd, phdr.p_offset, SEEK_SET);
      read(fd, buf, phdr.p_filesz);

      // TODO: generate code from buffer
      unsigned int start_offset = phdr.p_vaddr;
      unsigned int size = phdr.p_filesz;
      unsigned int total_size = phdr.p_memsz;
      // MS: What is the difference between size_in_the_file and total_size?

      printf("Start %d\nFile size %d\nTotal size %d\n",
	start_offset, size, total_size);

	// We now only look at the text segment, assuming it starts somewhere at a low address
	if (phdr.p_flags & PF_X) {
		progr = (int *) buf;
		start = start_offset/4;
		words = size/4;
printf("X section, %d\n", start);
		break;  // Edgar: Overwise we'll free the program buffer if the text section is not the la
	} else {
		free(buf);
	}
    }
  }

  entry = hdr.e_entry/4;
  printf("Entry: %d\n", entry);

  elf_end(elf);
}


int main(int argc, char* argv[]) {

	if (argc!=3) {
		printf("Argument missing\n");
		exit(-1);
	}

	readelf(argv[1]);

	// To dumb to create and open a file in C :-(((
	int fd = open(argv[2], O_CREAT | O_WRONLY | O_TRUNC, 0644);
	if (fd == -1)
          perror("Error:");

	int val = 0;
	int i;
//	for (i=0; i<start; ++i) {
//		printf("%08x\n", 0);
//		// Branch at offset 1 to the program start
//		// but we also could execute the NOPs
//		if (i==1) {
//			// val = 0x06400000 + start;
//			// Why do we have tools and then fight with byte order..
//			val = htonl(0x06400000 + start);
//			val = htonl(0x06400000 + entry);
//		} else {
//			val = 0;
//		}
//		write(fd, &val, 4);
//	}
	assert(entry-start >= 0);
	for (i=entry-start; i<words; ++i) {
		printf("%08x %08x\n", progr[i], htonl(progr[i]));
		// val = htonl(progr[i]);
		// The Java tool does the byte order....
		val = progr[i];
		write(fd, &val, 4);
	}
	close(fd);


	return 0;
}
