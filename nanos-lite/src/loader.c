#include <proc.h>
#include <elf.h>


#ifdef __LP64__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
# define Elf_Off Elf64_Off
# define Elf_Filesz uint64_t
# define Elf_Addr Elf64_Addr
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
# define Elf_Off Elf32_Off
# define Elf_Filesz uint32_t
# define Elf_Addr Elf32_Addr
#endif

#if defined(__ISA_AM_NATIVE__)
# define EXPECT_TYPE EM_X86_64
#elif defined(__ISA_RISCV32__)
# define EXPECT_TYPE EM_RISCV
#else
# error Unsupported ISA
#endif

size_t ramdisk_read(void *buf, size_t offset, size_t len);
size_t ramdisk_write(const void *buf, size_t offset, size_t len);
size_t get_ramdisk_size(void);

// static uintptr_t loader(PCB *pcb, const char *filename) {
//   if(filename == NULL) filename = "../build/ramdisk.img";
//   FILE *elf = fopen(filename, "r");
//   assert(elf);

//   size_t fread_buf;
//   Elf_Ehdr ehdr;
//   fread_buf = fread(&ehdr, sizeof(Elf_Ehdr), 1, elf);
//   assert(fread_buf);
//   assert(*(uint32_t *)ehdr.e_ident == 0x464c457f); // check magic number for elf
//   assert(ehdr.e_machine == EXPECT_TYPE);

//   Elf_Off e_phoff = ehdr.e_phoff;
//   uint16_t e_phnum = ehdr.e_phnum;
//   assert(e_phoff);
//   fseek(elf, e_phoff, SEEK_SET);
//   Elf_Phdr phdr[e_phnum];
//   fread_buf = fread(phdr, sizeof(Elf_Phdr), e_phnum, elf);
//   assert(fread_buf);
//   uintptr_t entry = 0;
//   Elf_Off p_offset;
//   Elf_Filesz p_filesz, p_memsz;
//   Elf_Addr p_vaddr;
//   // Load read-exec segment and read-write segment
//   for(int i = 0; i < e_phnum; i++){
//     if(phdr[i].p_type == PT_LOAD){
//       p_offset = phdr[i].p_offset;
//       p_filesz = phdr[i].p_filesz;
//       p_vaddr = phdr[i].p_vaddr;
//       printf("vaddr is %x\n", p_vaddr);
//       ramdisk_read((void *)p_vaddr, p_offset, p_filesz);
//       p_memsz = phdr[i].p_memsz;
//       memset((void *)(p_vaddr+p_filesz), 0, p_memsz-p_memsz);

//       if(phdr[i].p_flags == 5) entry = p_vaddr;
//     }
//   }
//   printf("entry is %x\n", entry);
//   return entry;
// }

static uintptr_t loader(PCB *pcb, const char *filename) {
  Elf_Ehdr ehdr;
  ramdisk_read(&ehdr, 0, sizeof(Elf_Ehdr));
  assert(*(uint32_t *)ehdr.e_ident == 0x464c457f); // check magic number for elf
  assert(ehdr.e_machine == EXPECT_TYPE);

  Elf_Off e_phoff = ehdr.e_phoff;
  uint16_t e_phnum = ehdr.e_phnum;
  assert(e_phoff);
  
  // find the entry of .text section
  uintptr_t entry = ehdr.e_entry;

  Elf_Phdr phdr[e_phnum];
  ramdisk_read(phdr, e_phoff, e_phnum*sizeof(Elf_Phdr));
  Elf_Off p_offset;
  Elf_Filesz p_filesz, p_memsz;
  Elf_Addr p_vaddr;
  // Load read-exec segment and read-write segment
  for(int i = 0; i < e_phnum; i++){
    if(phdr[i].p_type == PT_LOAD){
      p_offset = phdr[i].p_offset;
      p_filesz = phdr[i].p_filesz;
      p_vaddr = phdr[i].p_vaddr;
      printf("vaddr is %u\n", p_vaddr);
      printf("offset is %u\n", p_offset);
      ramdisk_read((void *)p_vaddr, p_offset, p_filesz);
      p_memsz = phdr[i].p_memsz;
      memset((void *)(p_vaddr+p_filesz), 0, p_memsz-p_filesz);

      // if(phdr[i].p_flags == 5) entry = p_vaddr;
    }
  }
  // printf("entry is %u\n", entry);
  return entry;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", (void *)entry);
  ((void(*)())entry) ();
}

