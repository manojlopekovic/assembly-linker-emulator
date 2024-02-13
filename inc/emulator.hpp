#ifndef _EMULATOR_HPP
#define _EMULATOR_HPP

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <cstddef>
#include <cstring>
#include <algorithm>
#include <iomanip>
#include <set>
#include <limits>

#include "./exception.hpp"
#include "./structures.hpp"

using namespace std;

#define SPACE_CHAR ' '
#define WORD_SIZE 4

#define _r0 0
#define _r1 1
#define _r2 2
#define _r3 3
#define _r4 4
#define _r5 5
#define _r6 6
#define _r7 7 
#define _r8 8
#define _r9 9
#define _r10 10
#define _r11 11
#define _r12 12
#define _r13 13
#define _r14 14
#define _r15 15
#define _sp 14
#define _pc 15

#define _status 0
#define _handle 1
#define _cause 2

#define pc_start_addr 0x40000000


class Emulator{
private:

  // structures used
  map<uint32_t, vector<string>>       memory;
  ifstream                            inputFile;
  string                              inFileName;
  uint32_t                            *pc;
  uint32_t                            *sp;

  map<int, uint32_t>                  regs;
  map<int, uint32_t>                  control_regs;

  char                                oc;
  char                                mod;
  char                                regA_ch;
  char                                regB_ch;
  char                                regC_ch;
  char                                dh;
  char                                dm;
  char                                dl;

  int                                 regA;
  int                                 regB;
  int                                 regC;
  int                                 disp_from_inst;
  int                                 disp;

public:
  // Constructors
  Emulator(string inFileName);
  ~Emulator();

  // Helper functions
  string remove_space_back_front(string line);
  vector<string> divide_line(string line, char divider);
  int string_to_val(string str);
  bool is_number(const string& str);
  void fill_memory();
  int char_to_val(char ch);
  void set_memory(uint32_t addr, uint32_t val);
  uint32_t load_val_from_mem(uint32_t addr);
  string uint32_t_to_string(uint32_t val);

  // Passage instructions
  void decode_pc_instruction();
  uint32_t push_reg(int regNo, bool ctrl_regs = false);
  void set_reg(int regNo, uint32_t val, bool ctrl_regs = false);
  uint32_t set_reg_from_mem(int regNo, uint32_t addr, bool ctrl_regs = false);
  void set_mem(uint32_t addr, uint32_t val);
  void do_halt();
  void do_int(int &intrpt);
  void do_call();
  void do_jmp();
  void do_xchng();
  void do_aritm();
  void do_logic();
  void do_shift();
  void do_store();
  void do_load(int &intrpt);

  // Functions
  void pass();
};



#endif