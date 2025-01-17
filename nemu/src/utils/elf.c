// elf parsing completed by Tj
#include <common.h>
#ifdef CONFIG_FTRACE
#include <elf.h>

FILE *elf_fp = NULL;

Elf32_Ehdr ehdr[1];
Elf32_Shdr shdr[24];

Elf32_Addr shoff;
uint16_t shnum;

uint32_t symtab_size;
uint32_t symtab_off;
Elf32_Sym symtab[128];

uint32_t strtab_size;
uint32_t strtab_off;
char strtab[512];

size_t fread_buf;

void load_ehdr();
void load_shdr();
void load_symtab();
void load_strtab();

void init_elf(const char *elf_file){
  if (elf_file != NULL) {
    FILE *e_fp = fopen(elf_file, "r");
    Assert(e_fp, "Can not open '%s'", elf_file);
    elf_fp = e_fp;
    load_ehdr();
    load_shdr();
    load_symtab();
    load_strtab();
  }
}

void load_ehdr(){
  fseek(elf_fp, 0, SEEK_SET);
  fread_buf = fread(ehdr, sizeof(Elf32_Ehdr), 1, elf_fp);
  assert(fread_buf > 0);
}

void load_shdr(){
  shoff = ehdr->e_shoff;
  shnum = ehdr->e_shnum;

  fseek(elf_fp, shoff, SEEK_SET);
  fread_buf = fread(shdr, sizeof(Elf32_Shdr), shnum, elf_fp);
  assert(fread_buf > 0);
}

void load_symtab(){
  for(uint16_t i = 0; i < shnum; i++){
    if(shdr[i].sh_type == SHT_SYMTAB){
      symtab_off = shdr[i].sh_offset;
      symtab_size = shdr[i].sh_size;
      break;
    }
  }

  fseek(elf_fp, symtab_off, SEEK_SET);
  fread_buf = fread(symtab, sizeof(Elf32_Sym), symtab_size/sizeof(Elf32_Sym), elf_fp);
  assert(fread_buf > 0);
}

void load_strtab(){
  for(uint16_t i = 0; i < shnum; i++){
    if(shdr[i].sh_type == SHT_STRTAB){
      strtab_off = shdr[i].sh_offset;
      strtab_size = shdr[i].sh_size;
      break;
    }
  }

  fseek(elf_fp, strtab_off, SEEK_SET);
  fread_buf = fread(strtab, 1, strtab_size, elf_fp);
  assert(fread_buf > 0);
}
#endif