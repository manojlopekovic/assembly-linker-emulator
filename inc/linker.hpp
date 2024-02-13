#ifndef _LINKER_HPP
#define _LINKER_HPP

#include "./exception.hpp"
#include "./structures.hpp"

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

using namespace std;

#define MIN_CMD_ARGS 5

struct s_FileStruct {
  s_ELFHdr                hdr;
  vector<s_SSym>          symTbl;
  vector<s_SHdr>          secHdr;
  vector<unsigned char>   secString;
  vector<unsigned char>   symString;
  map<int, s_Rela>        reloc;
  map<int, uint8_t>       sec_content;
};

class Linker{
private:

  string                  outFileName;
  vector<string>          inFileNames;
  ofstream                outputFile;
  vector<ifstream>        inputFiles;
  ofstream                helperFile;
  vector<string>          address_places;

  // used structures
  vector<s_SSym>                  symTbl;
  map<string, int>                sym_name_ndx;
  map<string, int>                sec_name_ndx;
  map<int, vector<char>>          sec_content;
  vector<s_SHdr>                  sections;
  map<int, vector<s_Rela>>        reloc;
  map<string, int>                sec_address;
  vector<uint32_t>                used_addresses;
  map<uint32_t, uint32_t>         addr_size;
  map<int, uint32_t>              sym_vals;

public:

  // constructors
  Linker(string outFile, vector<string> inFiles, vector<string> addr_places);
  ~Linker();

  // helper functions
  void read_file(int ndx);
  vector<char> read_from_binary(int ndx, int chunk, uint32_t start_addr = 0);
  size_t load_data_from_char_vect(vector<char> vect, int size, size_t start_pos);
  vector<s_SHdr> get_section_headers(vector<char> sec_hdr_tbl, uint16_t ent_size, uint16_t num);
  void load_sections_from_file(vector<s_SHdr> hdrTbl, vector<char> secString, int fileNdx);
  string get_name_from_str_array(int start_ndx, vector<char> secString);
  void load_progbits(s_SHdr secHdr, string name, int fileNdx);
  vector<s_SSym> get_sym_tab(vector<char> sym_tbl_vect, uint16_t ent_size, uint16_t num);
  string find_name_by_sec_ndx(int ndx);
  string find_name_by_sym_ndx(int ndx);
  void load_symbols(vector<s_SHdr>hdrTbl, int i, vector<char> secString, vector<s_SSym> temp_sym_tab, vector<char> symString, int fileNdx);
  vector<s_Rela> get_rela_tab(vector<char> rela_tab_vec, uint16_t ent_size, uint16_t num);

  // Second pass helper instructions
  void is_input_correct();
  uint32_t find_start_addr(int secNdx, bool &isTherePlace);
  void insert_sorted_to_vect(vector<uint32_t> &vect, uint32_t data);
  int find_section_start_on_addr(uint32_t addr);
  string charToHexString(char ch);
  // Linker functions
  void first_pass();
  void second_pass();

  // Printer functions 
  void print_header_table();
  void print_symbol_table();
  void print_reloc_table();

};



#endif