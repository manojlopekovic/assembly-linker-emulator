#ifndef _STRUCTURES_HPP
#define _STRUCTURES_HPP

#include <cstdint>
#include <string>

#define _BYTE_SIZE 8
#define _HALF_BYTE 4
#define WORD_SIZE 4

#define __MASK_0_3 0xf
#define __MASK_4_7 0xf0
#define __MASK_8_11 0xf00
#define __MASK_12_15 0xf000
#define __MASK_16_19 0xf0000
#define __MASK_20_23 0xf00000
#define __MASK_24_27 0xf000000
#define __MASK_28_31 0xf0000000

#define __GET_BITS_0_3(code32b) ((code32b & __MASK_0_3) >> 0)
#define __GET_BITS_4_7(code32b) ((code32b & __MASK_4_7) >> 4)
#define __GET_BITS_8_11(code32b) ((code32b & __MASK_8_11) >> 8)
#define __GET_BITS_12_15(code32b) ((code32b & __MASK_12_15) >> 12)
#define __GET_BITS_16_19(code32b) ((code32b & __MASK_16_19) >> 16)
#define __GET_BITS_20_23(code32b) ((code32b & __MASK_20_23) >> 20)
#define __GET_BITS_24_27(code32b) ((code32b & __MASK_24_27) >> 24)
#define __GET_BITS_28_31(code32b) ((code32b & __MASK_28_31) >> 28)

#define __SET_BITS_0_3(base, code4b) (base = base | (code4b << 0))
#define __SET_BITS_4_7(base, code4b) (base = base | (code4b << 4))
#define __SET_BITS_8_11(base, code4b) (base = base | (code4b << 8))
#define __SET_BITS_12_15(base, code4b) (base = base | (code4b << 12))
#define __SET_BITS_16_19(base, code4b) (base = base | (code4b << 16))
#define __SET_BITS_20_23(base, code4b) (base = base | (code4b << 20))
#define __SET_BITS_24_27(base, code4b) (base = base | (code4b << 24))
#define __SET_BITS_28_31(base, code4b) (base = base | (code4b << 28))

#define __GET_BITS_0_7(code32b) ((code32b & (__MASK_0_3 | __MASK_4_7)) >> 0)
#define __GET_BITS_8_15(code32b) ((code32b & (__MASK_8_11 | __MASK_12_15)) >> 8)
#define __GET_BITS_16_23(code32b) ((code32b & (__MASK_16_19 | __MASK_20_23)) >> 16)
#define __GET_BITS_24_31(code32b) ((code32b & (__MASK_24_27 | __MASK_28_31)) >> 24)

#define __SET_BITS_0_7(base, code8b) (base = base | (code8b << 0))
#define __SET_BITS_8_15(base,code8b) (base = base | (code8b << 8))
#define __SET_BITS_16_23(base, code8b) (base = base | (code8b << 16))
#define __SET_BITS_24_31(base, code8b) (base = base | (code8b << 24))

#define _IDENT_NUM 16
#define EI_MAG0 0 /* File identification byte 0 index */
#define EI_MAG1 1 /* File identification byte 1 index */
#define EI_MAG2 2 /* File identification byte 2 index */
#define EI_MAG3 3 /* File identification byte 3 index */
#define EI_CLASS 4 /* File class byte index */
#define EI_DATA 5 /* Data encoding byte index */
#define EI_VERSION 6 /* File version byte index */
#define EI_OSABI 7 /* OS ABI identification */
#define EI_ABIVERSION 8 /* ABI version */
#define EI_PAD 9 /* Byte index of padding bytes */

#define ET_NONE 0 /* No file type */
#define ET_REL 1 /* Relocatable file */
#define ET_EXEC 2 /* Executable file */
#define ET_DYN 3 /* Shared object file */
#define ET_CORE 4 /* Core file */
#define ET_LOOS 0xfe00 /* OS-specific range start */
#define ET_HIOS 0xfeff /* OS-specific range end */
#define ET_LOPROC 0xff00 /* Processor-specific range start */
#define ET_HIPROC 0xffff /* Processor-specific range end */

#define EM_NONE 0 /* No machine */
#define EM_386 3 /* Intel 80386 */
#define EM_860 7 /* Intel 80860 */
#define EM_ARM 40 /* ARM */
#define EM_MIPS_X 51 /* Stanford MIPS-X */
#define EM_X86_64 62 /* AMD x86-64 architecture */
#define EM_RISCV 243 /* RISC-V */
#define EM_SS_EXAM 100 /* Machine designated for testing assembler and linker */

#define EV_NONE 0 /* Invalid ELF version */
#define EV_CURRENT 1 /* Current version */

#define ELFCLASSNONE 0 /* Invalid class */
#define ELFCLASS32 1 /* 32-bit objects */
#define ELFCLASS64 2 /* 64-bit objects */

#define ELFDATANONE 0 /* Invalid data encoding */
#define ELFDATA2LSB 1 /* 2's complement, little endian */
#define ELFDATA2MSB 2 /* 2's complement, big endian */


struct s_ELFHdr {
  unsigned char         e_ident[_IDENT_NUM];
  uint16_t              e_type;
  uint16_t              e_machine;
  uint32_t              e_version;
  // Defines entry point of program, virtual address where system wil start executing
  uint64_t              e_entry;
  // Defines program header table offset in number of bytes from the start of ELF file
  uint64_t              e_phoff;
  // Defines section header table offset in number of bytes from the start of ELF file
  uint64_t              e_shoff;
  // Defines machine specific flags 
  uint32_t              e_flags;
  // Number of header defined in number of bytes
  uint16_t              e_ehsize;
  // Entry size in program header table in number of bytes
  uint16_t              e_phentsize;
  // Number of entries in program header table
  uint16_t              e_phnum;
  // Entry size in section header table in number of bytes
  uint16_t              e_shentsize;
  // Number of entries in section header table
  uint16_t              e_shnum;
  // Specifies index of entry in section header table that specifies section with strings for names of sections
  uint16_t              e_shstrndx;
};


#define SHT_NULL 0 /* Section header table entry unused */
#define SHT_PROGBITS 1 /* Program data */
#define SHT_SYMTAB 2 /* Symbol table */
#define SHT_STRTAB 3 /* String table */
#define SHT_RELA 4 /* Relocation entries with addends */
#define SHT_DYNAMIC 6 /* Dynamic linking information */
#define SHT_NOTE 7 /* Notes */
#define SHT_NOBITS 8 /* Program space with no data (bss) */
#define SHT_REL 9 /* Relocation entries, no addends */
#define SHT_HDRTAB 10 /* SEction header table */

const std::string e_SHT [] {"SHT_NULL", "SHT_PROGBITS", "SHT_SYMTAB", "SHT_STRTAB", "SHT_RELA", "SHT_DYNAMIC", "SHT_NOTE", "SHT_NOBITS", "SHT_REL", "SHT_HDRTAB"} ;

struct s_SHdr {
  // Number of the section
  uint32_t              sh_ndx;
  // Offset in section name string table from where the name will be retrieved. Name end with 00
  uint32_t              sh_name;
  // Type of the section
  uint32_t              sh_type;
  // Mask for flags
  uint32_t              sh_flags;
  // Virtual address of first byte
  uint64_t              sh_addr;
  // Offset from start of ELF to first byte
  uint64_t              sh_offset;
  // Size in bytes
  uint64_t              sh_size;
  // To which section is linked, depends on the section
  uint32_t              sh_link;
  // Additional info
  uint32_t              sh_info;
  uint64_t              sh_addralign;
  uint64_t              sh_entsize;

  s_SHdr (uint32_t ndx, uint32_t name, uint64_t size, uint32_t type, uint64_t addr, uint64_t offset, uint32_t flags, uint32_t link, uint32_t info, uint64_t align, uint64_t entsize) 
  : sh_ndx(ndx), sh_name(name), sh_size(size), sh_type(type), sh_addr(addr), sh_offset(offset), sh_flags(flags), sh_link(link), sh_info(info), sh_addralign(align), sh_entsize(entsize) { }
};


#define ELF64_ST_INFO(bind, type) (((bind) << 4) + ((type) & 0xf))
#define ELF64_ST_BIND(val) (((unsigned char) (val)) >> 4)
#define STB_LOCAL 0 /* Local symbol */
#define STB_GLOBAL 1 /* Global symbol */
#define STB_WEAK 2 /* Weak symbol */

const std::string e_STB [] = {"LOCAL", "GLOBAL", "WEAK"};

#define ELF64_ST_INFO(bind, type) (((bind) << 4) + ((type) & 0xf))
#define ELF32_ST_TYPE(val) ((val) & 0xf)
#define STT_NOTYPE 0 /* Symbol type is unspecified */
#define STT_SECTION 1 /* Symbol is a data object */

const std::string e_STT [] = {"NOTYPE", "SECTION"};

// This represents one entry in symbol table
struct s_SSym {
  // Offset inside symbol string table
  uint32_t              sym_name;
  // weather its section, symbol
  unsigned char         sym_type;
  // binding type - local/global
  unsigned char         sym_bind;
  // index to the section of the symbol
  uint16_t              sym_ndx;
  // Offset in section
  uint64_t              sym_value;
  // Assigned size
  uint64_t              sym_size;

  // Constructor
  s_SSym (uint32_t name, unsigned char bind, unsigned char type, uint64_t value, uint16_t ndx, uint64_t size) 
  : sym_name(name), sym_bind(bind), sym_type(type), sym_value(value), sym_ndx(ndx), sym_size(size) { }
};


// One relocation entry
struct s_Rela {
  // Offset to location where correction needs to be done
  // Contains virtual address of location where correction needs to be done, or offset from start of its section if its declared via section(for local)
  uint64_t              r_offset;
  // Type of offset, specific to processor
  uint32_t              r_type;
  // how many bits
  uint32_t              r_size;
  // Symbol value via its index in symtab
  uint64_t              r_symval;
  // immediate value that will be added 
  uint64_t              r_addend;

  s_Rela(uint64_t offset, uint64_t symval, uint32_t type, uint64_t addend, uint32_t size) : r_offset(offset), r_type(type), r_symval(symval), r_addend(addend), r_size(size) { }
};
#endif