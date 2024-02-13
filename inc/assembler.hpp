#ifndef _ASSEMBLER_HPP
#define _ASSEMBLER_HPP

#include "./structures.hpp"

// Includes
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <iomanip>
#include <sstream>
#include <unordered_set>
#include <algorithm>
#include <cctype>
#include <limits>

#include "./exception.hpp"

using namespace std;

// Define
#define CMD_LINE_SIZE 4
// this means one operand but can be expression
#define CALL_JMP_OPERANDS 10
// this means 3 operands, but last one can be expression
#define C_JMP_OPERANDS 12
// this means two operands, but one can be expression
#define LD_ST_OPERANDS 11

// macro defines used in code
#define COMMENT_DELIMETER "#"
#define DIRECTIVE_START_CHAR '.'
#define LABEL_END_CHAR ':'
#define SPACE_CHAR ' '
#define EMPTY_STRING ""
#define EMPTY_CHAR ""
#define COMMA_CHAR ','

#define _ZERO 0
#define _BIT_00 (1<<0)
#define _BIT_01 (1<<1)
#define _BIT_02 (1<<2)
#define _BIT_03 (1<<3)
#define _BIT_04 (1<<4)
#define _BIT_05 (1<<5)
#define _BIT_06 (1<<6)
#define _BIT_07 (1<<7)
#define _BIT_08 (1<<8)
#define _BIT_09 (1<<9)
#define _BIT_10 (1<<10)
#define _BIT_11 (1<<11)
#define _BIT_12 (1<<12)
#define _BIT_13 (1<<13)
#define _BIT_14 (1<<14)
#define _BIT_15 (1<<15)
#define _BIT_16 (1<<16)
#define _BIT_17 (1<<17)
#define _BIT_18 (1<<18)
#define _BIT_19 (1<<19)
#define _BIT_20 (1<<20)
#define _BIT_21 (1<<21)
#define _BIT_22 (1<<22)
#define _BIT_23 (1<<23)
#define _BIT_24 (1<<24)
#define _BIT_25 (1<<25)
#define _BIT_26 (1<<26)
#define _BIT_27 (1<<27)
#define _BIT_28 (1<<28)
#define _BIT_29 (1<<29)
#define _BIT_30 (1<<30)
#define _BIT_31 (1<<31)

// Enums
enum e_LineType {LABEL = 0, DIRECTIVE, INSTRUCTION, L_D, L_I};
const string e_n_LineType[] =  {"LABEL", "DIRECTIVE", "INSTRUCTION", "L_D", "L_I"};

// *******************************************************************************************************************


// definisanje vrednosti za polja sekcija

enum e_SecType {SEC_TYPE_NULL = 0, SEC_TYPE_PROGBITS, SEC_TYPE_SYMTAB, SEC_TYPE_STRTAB, SEC_TYPE_RELA, SEC_TYPE_DYNAMIC, SEC_TYPE_NOTE, SEC_TYPE_NOBITS, SEC_TYPE_REL};
const string e_n_SecType [] = {"NULL", "PROGBITS", "SYMTAB", "STRTAB", "RELA", "", "DYNAMIC", "NOTE", "NOBITS", "REL", "HDRTAB"};

// Sec flags
#define SEC_FLAGS_WRITE (1 << 0) /* Writable */
#define SEC_FLAGS_ALLOC (1 << 1) /* Occupies memory during execution */
#define SEC_FLAGS_EXECINSTR (1 << 2) /* Executable */
#define SEC_FLAGS_MERGE (1 << 4) /* Might be merged */
#define SEC_FLAGS_STRINGS (1 << 5) /* Contains nul-terminated strings */
#define SEC_FLAGS_INFO_LINK (1 << 6) /* 'sh_info' contains SHT index */
#define SEC_FLAGS_GROUP (1 << 9) /* Section is member of a group. */

// Definisanje vrednosti za polja symbol table

enum e_SymType {NOTYPE = 0, SECTION};
const string e_n_SymType [] = {"NOTYPE", "SECTION"};

enum e_SymBind {LOCAL = 0, GLOBAL};
const string e_n_SymBind [] = {"LOCAL", "GLOBAL"};

enum e_SymVis {DEFAULT = 0};
const string e_n_SymVis [] = {"DEFAULT"};

enum e_SymNdx {ABS = -1, UND = 0};
const string e_n_SymNdx [] = {"ABS", "UND"};

// definisanje vrednosti za polja rela table

enum e_RelaType {R_X86_64_PC32 = 2, R_X86_64_PLT32 = 4, R_X86_64_32S = 11};
map<int, string> e_n_RelaType = {{R_X86_64_PC32,"R_X86_64_PC32"}, 
{R_X86_64_PLT32, "R_X86_64_PLT32"}, {R_X86_64_32S, "R_X86_64_32S"}};

// *******************************************************************************************************************

// Defined structures

struct s_InstructionStruct {
  int               no_operands;
  uint32_t          opcode;

  s_InstructionStruct(int no_operands = 0, uint32_t code = 0) : no_operands(no_operands), opcode(code) { }
};

struct s_LineStruct {
  string            line;
  e_LineType        type;
  string            info;
  int               param_begin;
  vector<string>    operands;

  // Constructor(
  s_LineStruct (string line = "", e_LineType type = LABEL, string info = "", int param_begin = 0, vector<string> operands = vector<string>()) : line(line), type(type), info(info), param_begin(param_begin), operands(operands) { }
};

// This represents one entry in symbol table
struct s_Sym {
  uint64_t          sym_value;
  uint64_t          sym_size;
  // weather its label, symbol
  int               sym_type;
  // binding type - local/global
  int               sym_bind;
  // index to the section of the symbol
  int               sym_ndx;
  // Will translate to offset in symbol string table
  string            sym_name;

  // Constructor
  s_Sym (string name = "", int bind = 0, int type = NOTYPE, uint64_t value = 0, int ndx = 0, uint64_t size = 0) 
  : sym_name(name), sym_bind(bind), sym_type(type), sym_value(value), sym_ndx(ndx), sym_size(size) { }
};

// This represents header of one section
struct s_SecHdr {
  // offset inside section name string tabele
  // Will translate to section header string table
  string            sec_name;
  int               sec_type;
  uint64_t          sec_flags;
  // Virtual address of first byte of the section, if it needs to be inside mem during execution
  uint64_t          sec_addr;
  // Offset from the start of ELF file do first byte of said section in no. B
  uint64_t          sec_offset;
  // Size of the section in B
  uint64_t          sec_size;
  // Index of some other entrie that is connected somehow
  uint32_t          sec_link;
  uint32_t          sec_info;
  // Alignment - 0/1 means no alignment, other values must be power of 2
  uint64_t          sec_addralign;
  // Entrt size of one entry if section contains entries of the same size
  uint64_t          sec_entsize;

  // Constructor
  s_SecHdr (string name = "", uint64_t size = 0, int type = SEC_TYPE_NULL, uint64_t addr = 0, uint64_t offset = 0, uint64_t flags = 0, uint64_t link = 0, uint64_t info = 0, uint64_t align = 0, uint64_t entsize = 0) 
  : sec_name(name), sec_size(size), sec_type(type), sec_addr(addr), sec_offset(offset), sec_flags(flags), sec_link(link), sec_info(info), sec_addralign(align), sec_entsize(entsize) { }
};

// One relocation entry
struct s_RelocAdd {
  // Offset to location where correction needs to be done
  // Contains virtual address of location where correction needs to be done, or offset from start of its section if its declared via section(for local)
  uint64_t          rel_offset;
  // Type of offset, specific to processor
  int               rel_type;
  // Symbol value via its index in symtab
  uint64_t          rel_symval;
  // immediate value that will be added 
  uint64_t          rel_addend;
  // how many bits
  double               rel_size;

  s_RelocAdd(uint64_t offset = 0, uint64_t symval = 0, int type = 0, uint64_t addend = 0, double size = 4) : rel_offset(offset), rel_type(type), rel_symval(symval), rel_addend(addend), rel_size(size) { }
};


// Direktive


#define __HALT _ZERO
#define __INT _BIT_28
#define __UNDEF_OPCODE -1

#define __CALL _BIT_29
#define __CALL_REG_ADD 0x0
#define __CALL_MEM32_ADD 0x1

#define __JMP (_BIT_29 | _BIT_28)
//  pc<=gpr[A]+D;
#define __JMP_REG 0x0
//  if (gpr[B] == gpr[C]) pc<=gpr[A]+D;
#define __BEQ_REG 0x1
//  if (gpr[B] != gpr[C]) pc<=gpr[A]+D;
#define __BNE_REG 0x2
//  if (gpr[B] signed> gpr[C]) pc<=gpr[A]+D;
#define __BGT_REG 0x3
// pc<=mem32[gpr[A]+D];
#define __JMP_MEM 0x8
// if (gpr[B] == gpr[C]) pc<=mem32[gpr[A]+D];
#define __BEQ_MEM 0x9
// if (gpr[B] != gpr[C]) pc<=mem32[gpr[A]+D];
#define __BNE_MEM 0xa
//  if (gpr[B] signed> gpr[C]) pc<=mem32[gpr[A]+D];
#define __BGT_MEM 0xb

#define __XCHG _BIT_30

#define __ARITHMETIC (_BIT_30 | _BIT_28)
#define __ARITHMETIC_ADD_OP 0x0
#define __ARITHMETIC_SUB_OP 0x1
#define __ARITHMETIC_MUL_OP 0x2
#define __ARITHMETIC_DIV_OP 0x3

#define __LOGIC (_BIT_30 | _BIT_29)
#define __LOGIC_NOT_OP 0x0
#define __LOGIC_AND_OP 0x1
#define __LOGIC_OR_OP 0x2
#define __LOGIC_XOR_OP 0x3

#define __SHIFT (_BIT_30 | _BIT_29 | _BIT_28)
//  gpr[A]<=gpr[B] << gpr[C];
#define __SHIFT_LEFT 0x0
//  gpr[A]<=gpr[B] >> gpr[C];
#define __SHIFT_RIGHT 0x1

#define __LD (_BIT_31 | _BIT_28)
// gpr[A]<=csr[B];
#define __LD_GPR_CSR 0x0
//  gpr[A]<=gpr[B]+D;
#define __LD_GPR_GPR_ADD 0x1
// gpr[A]<=mem32[gpr[B]+gpr[C]+D];
#define __LD_GPR_MEM_GPR_D 0x2
// gpr[A]<=mem32[gpr[B]]; gpr[B]<=gpr[B]+D;
#define __LD_GPR_MEM_GPR_GPR_D 0x3
// csr[A]<=gpr[B];
#define __LD_CSR_GPR 0x4
// csr[A]<=csr[B]|D;
#define __LD_CSR_CSR_D 0x5
// csr[A]<=mem32[gpr[B]+gpr[C]+D];
#define __LD_CSR_MEM_D 0x6
// csr[A]<=mem32[gpr[B]]; gpr[B]<=gpr[B]+D;
#define __LD_CSR_MEM_GPR_GPR_D 0x7

#define __ST _BIT_31
// mem32[gpr[A]+gpr[B]+D]<=gpr[C];
#define __ST_MEM 0x0
// mem32[mem32[gpr[A]+gpr[B]+D]]<=gpr[C];
#define __ST_MEM_MEM 0x2
// gpr[A]<=gpr[A]+D; mem32[gpr[A]]<=gpr[C];
#define __ST_MEM_REG 0x1

#define __DIR_GLOBAL ".global"
#define __DIR_EXTERN ".extern"
#define __DIR_SECTION ".section"
#define __DIR_WORD ".word"
#define __DIR_SKIP ".skip"
#define __DIR_END ".end"

#define __INST_HALT "halt"
#define __INST_INT "int"
#define __INST_IRET "iret"
#define __INST_RET "ret"
#define __INST_CALL "call"
#define __INST_JMP "jmp"
#define __INST_BEQ "beq"
#define __INST_BNE "bne"
#define __INST_BGT "bgt"
#define __INST_PUSH "push"
#define __INST_POP "pop"
#define __INST_XCHG "xchg"
#define __INST_ADD "add"
#define __INST_SUB "sub"
#define __INST_MUL "mul"
#define __INST_DIV "div"
#define __INST_NOT "not"
#define __INST_AND "and"
#define __INST_OR "or"
#define __INST_XOR "xor"
#define __INST_SHL "shl"
#define __INST_SHR "shr"
#define __INST_LD "ld"
#define __INST_ST "st"
#define __INST_CSRRD "csrrd"
#define __INST_CSRWR "csrwr"

#define __REG_INDICATOR '%'

#define __REG_00 0x0
#define __REG_01 0x1
#define __REG_02 0x2
#define __REG_03 0x3
#define __REG_04 0x4
#define __REG_05 0x5
#define __REG_06 0x6
#define __REG_07 0x7
#define __REG_08 0x8
#define __REG_09 0x9
#define __REG_10 0xa
#define __REG_11 0xb
#define __REG_12 0xc
#define __REG_13 0xd
#define __REG_14 0xe
#define __REG_15 0xf
// Vrednost poslednje zauzete, raste ka nizim adresama
#define __SP 0xe
#define __PC 0xf

#define __WORD_OFFSET_POS 0x4
#define __WORD_OFFSET_NEG 0xfffffffffffc


#define __SHIFT_TYPE_BITS(opcode) ((opcode & __MASK_28_31) >> 28)
#define __SHIFT_OP_BITS(opcode) ((opcode & __MASK_24_27) >> 24)


#define __REG_STATUS 0x0
#define __REG_HANDLER 0x1
#define __REG_CAUSE 0x2

const map<string, int> directive_map = {{".global", -1}, {".extern", -1}, {".section", 1}, {".word", -1}, {".skip", 1}, {".end", 0}};

// Instrukcije

const map <string, s_InstructionStruct> instruction_map = {{__INST_HALT, s_InstructionStruct(0, __HALT)}, {__INST_INT, s_InstructionStruct(0, __INT)}, {__INST_IRET, s_InstructionStruct(0, __UNDEF_OPCODE)},
  {__INST_RET, s_InstructionStruct(0, __UNDEF_OPCODE)}, {__INST_CALL, s_InstructionStruct(CALL_JMP_OPERANDS, __CALL)}, {__INST_JMP, s_InstructionStruct(CALL_JMP_OPERANDS, __JMP)}, 
  {__INST_BEQ, s_InstructionStruct(C_JMP_OPERANDS, __JMP)}, {__INST_BNE, s_InstructionStruct(C_JMP_OPERANDS, __JMP)}, {__INST_BGT, s_InstructionStruct(C_JMP_OPERANDS, __JMP)},
  {__INST_PUSH, s_InstructionStruct(1, __UNDEF_OPCODE)}, {__INST_POP, s_InstructionStruct(1, __UNDEF_OPCODE)}, {__INST_XCHG, s_InstructionStruct(2, __XCHG)}, {__INST_ADD, s_InstructionStruct(2, __ARITHMETIC)}, 
  {__INST_SUB, s_InstructionStruct(2, __ARITHMETIC)}, {__INST_MUL, s_InstructionStruct(2, __ARITHMETIC)}, {__INST_DIV, s_InstructionStruct(2, __ARITHMETIC)}, 
  {__INST_NOT, s_InstructionStruct(1, __LOGIC)}, {__INST_AND, s_InstructionStruct(2, __LOGIC)}, {__INST_OR, s_InstructionStruct(2, __LOGIC)}, {__INST_XOR, s_InstructionStruct(2, __LOGIC)},
  {__INST_SHL, s_InstructionStruct(2, __SHIFT)}, {__INST_SHR, s_InstructionStruct(2, __SHIFT)}, {__INST_LD, s_InstructionStruct(LD_ST_OPERANDS, __LD)}, {__INST_ST, s_InstructionStruct(LD_ST_OPERANDS, __ST)},
  {__INST_CSRRD, s_InstructionStruct(2, __LD)}, {__INST_CSRWR, s_InstructionStruct(2, __LD)}}; 

struct s_LitSym {
  int               literal;
  string            symName;

  bool operator<(const s_LitSym& other) const {
      // Compare based on literal and then symName
      if (literal != other.literal) {
          return literal < other.literal;
      }
      return symName < other.symName;
  }
  bool operator==(const s_LitSym& other) const {
      // Compare based on literal and then symName
      if (literal == other.literal && symName == other.symName) {
          return true;
      }
      return false;
  }

  s_LitSym() = default;
  s_LitSym(int literal = -1, string symName = "") : literal(literal), symName(symName) { }
};

struct s_LiteralPool {
  int               secId;
  int               offset;

  s_LiteralPool(int secId = 0, int offset = 0) : secId(secId), offset(offset) { }
};

class Assembler {
private:
  
  // files
  string            outFile;
  string            inFile;
  ofstream          outputFile;
  ifstream          inputFile;
  ofstream          helperFile;

  // helper structures
  vector<s_Sym>                           symTable;
  vector<s_SecHdr>                        secHdrTbl;
  vector<s_RelocAdd>                      relocTbl;
  map<int, vector<s_Rela>>                reloc;
  map<int, vector<uint8_t>>               sec_content;
  map<s_LitSym, vector<s_LiteralPool>>    literals;
  map<int, vector<s_LitSym>>              sec_literals;

  // assembler work
  long long                               locationCounter = 0;
  string                                  currentSection = "";
  int                                     currSecIndex = 0;
  map<int, s_LineStruct>                  lines;
  vector<unsigned char>                   secStringTbl;
  vector<unsigned char>                   symStringTbl;
  vector<s_SHdr>                          sections;
  vector<s_SSym>                          sTab;
  s_ELFHdr                                hdr;

public:

  // constructors
  Assembler(string outFile, string inFile);
  // Destructor
  ~Assembler();

  // Startup functions
  // Check weather okay number of assembly parameters are set, using CMD_LINE_SIZE define, so it can be used to add options if neccessary
  static void check_cmd_line(int argc, char const *argv[]);
  // Check if all files set are appropriate
  static void check_input_params(string options, string outputFile, string inputFile);

  // functions used in assembly
  void first_pass();
  void second_pass();
  void handle_label_first_pass(s_LineStruct& line);
  bool handle_directive_first_pass(s_LineStruct& line);
  void handle_instruction_first_pass(s_LineStruct& line);
  void handle_label_second_pass(s_LineStruct& line);
  bool handle_directive_second_pass(s_LineStruct& line);
  void handle_instruction_second_pass(s_LineStruct& line);

  // helper functions
  bool try_to_map_dir(string dir);
  vector<string> divide_line(string line, char divider);
  string remove_space_back_front(string line);
  bool check_sym_in_tbl(string sym_name);
  string remove_chars_in_string(string line, char removalChar);
  int find_sym_tbl_entry(string sym_name);
  int string_to_val(string str);
  string get_op_type(string op);
  void add_literal_to_pool(int lit);
  void add_symbol_to_pool(string sym);
  bool is_number(const string& str);
  vector<string> get_mem_ops(string op);
  int get_index_of_literal_for_section(int lit);
  int get_index_of_literal_for_section(string sym);

  // Helper instructions for instructions
  void pop_reg(uint8_t reg);
  void pop_status_reg(uint8_t reg);
  void push_reg(uint8_t reg);
  uint8_t get_reg(string op);
  void arithmetic_op(uint8_t gprS, uint8_t gprD, string op);
  void logic_op(string op, uint8_t gprD, uint8_t gprS = 0);
  void shift_op(uint8_t gprS, uint8_t gprD, string op);
  void gpr_csr(uint8_t gpr, uint8_t csr, string op);
  void jmp_call(string operand, string op);
  void load_instr(string op0, string op1);
  void ld_mem_reg(uint8_t ldReg, uint8_t reg, int displacement);
  void store_instr(string op0, string op1);
  uint32_t store(uint8_t stReg, uint8_t reg, int displacement, uint8_t op);
  void put_byte_section(uint8_t _byte);
  void put_4Byte_section_little_endian(uint32_t _4byte);
  int calc_offset_for_index(vector<s_SHdr> hdr, int ndx);
  void remove_index_symTbl_update_rela(int ndx);
  void outputData();

  // print functions
  void print_header_table();
  void print_symbol_table();
  void print_reloc_table();
  void write_to_out_file();
  string get_name_sec(int start_ndx);
  string get_name_sym(int start_ndx);
};

/*

Za rad sa podacima - vrednost podatka
• $<literal> - vrednost <literal>
• $<simbol> - vrednost <simbol>
• <literal> - vrednost iz memorije na adresi <literal>
• <simbol> - vrednost iz memorije na adresi <simbol>
• %<reg> - vrednost u registru <reg>
• [%<reg>] - vrednost iz memorije na adresi <reg>
• [%<reg> + <literal>] - vrednost iz memorije na adresi <reg> + <literal>1
• [%<reg> + <simbol>] - vrednost iz memorije na adresi <reg> + <simbol>2

Za skokove - vrednost odredisne adrese skoka
• <literal> - vrednost <literal>
• <simbol> - vrednost <simbol>

*/

#endif