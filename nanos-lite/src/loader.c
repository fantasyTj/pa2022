#include <proc.h>
#include <elf.h>

// uintptr_t program_break;

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

int fs_open(const char *pathname, int flags, int mode);
size_t fs_read(int fd, void *buf, size_t count);
size_t fs_write(int fd, const void *buf, size_t count);
size_t fs_lseek(int fd, size_t offset, int whence);
#include <fs.h>

static uintptr_t loader(PCB *pcb, const char *filename) {
  assert(filename);
  int fd = fs_open(filename, 0, 0);
  Elf_Ehdr ehdr;
  fs_lseek(fd, 0, SEEK_SET);
  fs_read(fd, &ehdr, sizeof(Elf_Ehdr));
  // ramdisk_read(&ehdr, 0, sizeof(Elf_Ehdr));
  assert(*(uint32_t *)ehdr.e_ident == 0x464c457f); // check magic number for elf
  assert(ehdr.e_machine == EXPECT_TYPE);

  Elf_Off e_phoff = ehdr.e_phoff;
  uint16_t e_phnum = ehdr.e_phnum;
  assert(e_phoff);
  
  // find the entry of .text section
  uintptr_t entry = ehdr.e_entry;

  Elf_Phdr phdr[e_phnum];
  fs_lseek(fd, e_phoff, SEEK_SET);
  fs_read(fd, phdr, e_phnum*sizeof(Elf_Phdr));
  // ramdisk_read(phdr, e_phoff, e_phnum*sizeof(Elf_Phdr));
  Elf_Off p_offset;
  Elf_Filesz p_filesz, p_memsz;
  Elf_Addr p_vaddr;
  // Load read-exec segment and read-write segment
  for(int i = 0; i < e_phnum; i++){
    if(phdr[i].p_type == PT_LOAD){
      p_offset = phdr[i].p_offset;
      p_filesz = phdr[i].p_filesz;
      p_vaddr = phdr[i].p_vaddr;
      p_memsz = phdr[i].p_memsz;
      printf("vaddr is %p, end is %p\n", p_vaddr, p_vaddr + p_memsz);
      // printf("offset is %u\n", p_offset);
      fs_lseek(fd, p_offset, SEEK_SET);
      void *start_page = (void *)ROUNDDOWN(p_vaddr, PGSIZE);
      void *end_page = (void *)ROUNDDOWN(p_vaddr + p_memsz, PGSIZE);
      void *va = start_page;
      for( ; va <= end_page; va += PGSIZE) {
        void *pa = new_page(1);
        printf("va is %p, pa is %p\n", va, pa);
        map(&pcb->as, va, pa, 0);
      }
      printf("p_vaddr is %p, p_filesz is %p\n", p_vaddr, p_filesz);
      fs_read(fd, (void *)p_vaddr, p_filesz);
      printf("finish read\n");
      // ramdisk_read((void *)p_vaddr, p_offset, p_filesz);
      memset((void *)(p_vaddr+p_filesz), 0, p_memsz-p_filesz);

      // if(p_memsz > p_filesz){
      //   program_break = p_vaddr + p_memsz;
      // }
      // if(phdr[i].p_flags == 5) entry = p_vaddr;
    }
  }
  // printf("entry is %u\n", entry);
  return entry;
}

uintptr_t load_getentry(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  return entry;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", (void *)entry);
  ((void(*)())entry) ();
}

