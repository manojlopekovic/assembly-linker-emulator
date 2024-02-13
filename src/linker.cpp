#include "../inc/linker.hpp"


Linker::Linker(string outFile, vector<string> inFiles, vector<string> addr_places) : outFileName(outFile), inFileNames(inFiles), address_places(addr_places) {
  for(int i = 0; i < inFileNames.size(); i++) {
    inputFiles.push_back(ifstream());
    inputFiles[i].open(inFileNames[i], ios::binary);
    if(!inputFiles[i].is_open())
      throw CustomException("*LE : Failed to open input file");
  }
  outputFile.open(outFileName, ios::out | ios::trunc);
  if(!outputFile.is_open())
    throw CustomException("*LE : Failed to open output file");
  
  string hlpFile = outFileName.substr(0, outFileName.length() - 4);
  hlpFile.append(".txt");
  helperFile.open(hlpFile, ios::out | ios::trunc);
  if(!helperFile.is_open())
    throw CustomException("*LE : Failed to open helper file");
}


Linker::~Linker(){
  for(int i = 0; i < inFileNames.size(); i++) {
    inputFiles[i].close();
  }
  outputFile.close();
  helperFile.close();
}
// *********************************************************************************************************************
// Linker functions
void Linker::first_pass() {
  for(int i = 0; i < inputFiles.size(); i++){
    read_file(i);
  }
}


void Linker::second_pass(){
  is_input_correct();
  for(auto& elem : sec_address) {
    sections[sec_name_ndx[elem.first]].sh_addr = static_cast<uint32_t> (elem.second);
    insert_sorted_to_vect(used_addresses, static_cast<uint32_t> (elem.second));
    addr_size.insert(make_pair(static_cast<uint32_t>(elem.second), sections[sec_name_ndx[elem.first]].sh_size));
  }
  for(int i = 0; i < sections.size(); i++) {
    if(sections[i].sh_type == SHT_PROGBITS && sections[i].sh_addr == 0) {
      bool isTherePlace;
      uint32_t start_addr = find_start_addr(i, isTherePlace);
      if(isTherePlace)
        sections[i].sh_addr = start_addr;
      else 
        throw CustomException("*LE : There is no place to store section");
    }
  }
  helperFile << "Sym values:" << endl << setw(5) << setfill(' ') << "Ndx: " << setw(25) << setfill(' ') << "Name: " << setw(25) << setfill(' ') << "Value: " << endl;
  for(int i = 0; i < symTbl.size(); i++) {
    sym_vals.insert(make_pair(i, sections[symTbl[i].sym_ndx].sh_addr + symTbl[i].sym_value));
    helperFile << setw(5) << setfill(' ') << i << setw(25) << setfill(' ') << find_name_by_sym_ndx(i) << setw(25) << setfill(' ') << hex << sym_vals[i] << endl;
  }
  for(int i = 1; i < symTbl.size(); i++) {
    if(symTbl[i].sym_ndx == 0) {
      cout << "Symbol : " << find_name_by_sym_ndx(i) << " is undefined" << endl;
      throw CustomException("*LE : Undefined symbol");
    }
  }
  for(auto& elem : reloc){
    for(int i = 0; i < elem.second.size(); i++) {
      // int secNdx = symTbl[elem.second[i].r_symval].sym_ndx;
      // for(int j = 0; j < elem.second[i].r_size / _HALF_BYTE; j++) {
      //   sec_content.at(elem.first)[elem.second[i].r_offset + j * WORD_SIZE] = ((sym_vals[elem.second[i].r_symval] & (__MASK_0_3 << j * WORD_SIZE)) >> (j * WORD_SIZE));
      // }
      uint32_t temp = sym_vals[elem.second[i].r_symval] + elem.second[i].r_addend;
      uint32_t mask = static_cast<uint32_t>((0xffffffff << elem.second[i].r_size));
      temp = temp & mask;
      sec_content.at(elem.first)[elem.second[i].r_offset + 0] = __GET_BITS_0_7(temp);
      sec_content.at(elem.first)[elem.second[i].r_offset + 1] = __GET_BITS_8_15(temp);
      sec_content.at(elem.first)[elem.second[i].r_offset + 2] = __GET_BITS_16_23(temp);
      sec_content.at(elem.first)[elem.second[i].r_offset + 3] = __GET_BITS_24_31(temp);
    }
  }
  uint32_t addr = 0;
  string line = addr + ": ";
  outputFile << setw(8) << hex << addr << ": ";
  for(int i = 0; i < used_addresses.size(); i++) {
    int secNdx = find_section_start_on_addr(used_addresses[i]);
    addr = sections[secNdx].sh_addr;
    if(sec_content.count(secNdx) > 0) {
      // uint32_t lower = 0;
      // uint32_t higher = 0;
      for(int j = 0; j < sec_content.at(secNdx).size(); j++) {
        if(addr % _BYTE_SIZE == 0 && addr != 0) 
          outputFile << endl << setw(8) << hex << addr << ": ";
        outputFile << charToHexString(sec_content.at(secNdx)[j]) << " ";
        addr++;
        // outputFile << setw(8) << addr << ": ";
        // __SET_BITS_24_31(lower, sec_content.at(secNdx)[j++]);
        // __SET_BITS_16_23(lower, sec_content.at(secNdx)[j++]);
        // __SET_BITS_8_15(lower, sec_content.at(secNdx)[j++]);
        // __SET_BITS_0_7(lower, sec_content.at(secNdx)[j++]);
        // outputFile << hex << lower << " ";
        // __SET_BITS_24_31(higher, sec_content.at(secNdx)[j++]);
        // __SET_BITS_16_23(higher, sec_content.at(secNdx)[j++]);
        // __SET_BITS_8_15(higher, sec_content.at(secNdx)[j++]);
        // __SET_BITS_0_7(higher, sec_content.at(secNdx)[j++]);
        // outputFile << hex << higher << endl;
      }
    }
  }
}

string Linker::charToHexString(char ch) {
    std::stringstream stream;
    stream << std::hex << std::setw(4) << std::setfill('0') << static_cast<uint16_t>(ch);
    return stream.str().substr(2);
}

// *********************************************************************************************************************

// *********************************************************************************************************************
// Helper functions
int Linker::find_section_start_on_addr(uint32_t addr){
  for(int i = 1; i < sections.size(); i++)
    if(sections[i].sh_addr == addr)
      return i;
  return -1;
}


void Linker::insert_sorted_to_vect(vector<uint32_t> &vect, uint32_t data) {
  auto it = lower_bound(vect.begin(), vect.end(), data);
  vect.insert(it, data);
}


uint32_t Linker::find_start_addr(int secNdx, bool &isTherePlace) {
  uint32_t base_addr = 0;
  if(used_addresses.size() == 0) {
    insert_sorted_to_vect(used_addresses, base_addr);
    addr_size.insert(make_pair(base_addr, sections[secNdx].sh_size));
    isTherePlace = true;
    return base_addr;
  } else {
    for(int i = 0; i < used_addresses.size(); i++) {
      if(base_addr + sections[secNdx].sh_size < used_addresses[i]) {
        insert_sorted_to_vect(used_addresses, base_addr);
        addr_size.insert(make_pair(base_addr, sections[secNdx].sh_size));
        isTherePlace = true;
        return base_addr;
      }
      base_addr += addr_size[used_addresses[i]];
    }
    if(base_addr + sections[secNdx].sh_size < INT32_MAX) {
      insert_sorted_to_vect(used_addresses, base_addr);
      addr_size.insert(make_pair(base_addr, sections[secNdx].sh_size));
      isTherePlace = true;
      return base_addr;
    }
    isTherePlace = false;
    return -1;
  }
}


void Linker::is_input_correct() {
  for(int i = 0; i < address_places.size(); i++){
    string section = address_places[i].substr(address_places[i].find("=") + 1, address_places[i].find("@") - address_places[i].find("=") - 1);
    if(sec_name_ndx.count(section) <= 0)
      throw CustomException("*LE : The section for which address was specified was not found");
    string address_str = address_places[i].substr(address_places[i].find("@") + 1);
    size_t pos;
    int address = stoi(address_str, &pos);
    if(pos != address_str.size()) {
      address = stoul(address_str, &pos, 16);
      if(pos != address_str.size())
        throw CustomException("*LE : Address provided for placing the section could not be translated to number");
    }
    sec_address.insert(make_pair(section, address)); 
  }
  for(auto &elem : sec_address) {
    uint32_t next_addr = 0xffffffff;
    string next_sec = "";
    for(auto &it : sec_address) {
      if(it.first != elem.first && it.second > elem.second && it.second < next_addr) {
        next_addr = it.second;
        next_sec = it.first;
      }
    }
    if(elem.second < next_addr && elem.second + sections[sec_name_ndx[elem.first]].sh_size - next_addr < 0) {
      cout << "Linkage command line error, section : " << elem.first << " was designated to start from : " << hex << elem.second << " and its size is : " << dec <<  sections[sec_name_ndx[elem.first]].sh_size
          << " but section : " << next_sec << " is starting from address : " << hex << next_addr << endl;
      throw CustomException("*LE : Bad address alocation from cmd line"); 
    }
  }
}


void Linker::read_file(int ndx) {
  string fileName = inFileNames[ndx];
  s_FileStruct file;
  vector<char> elfHdr = read_from_binary(ndx, sizeof(s_ELFHdr));
  uint64_t secHdrTblOffset = static_cast<uint64_t>(load_data_from_char_vect(elfHdr, sizeof(uint64_t), offsetof(s_ELFHdr, e_shoff)));
  uint16_t secHdrEntrySize = static_cast<uint16_t>(load_data_from_char_vect(elfHdr, sizeof(uint16_t), offsetof(s_ELFHdr, e_shentsize)));
  uint16_t secHdrEntries = static_cast<uint16_t>(load_data_from_char_vect(elfHdr, sizeof(uint16_t), offsetof(s_ELFHdr, e_shnum)));
  uint16_t secStringNdx = static_cast<uint16_t>(load_data_from_char_vect(elfHdr, sizeof(uint16_t), offsetof(s_ELFHdr, e_shstrndx)));
  vector<char> secHdrTbl = read_from_binary(ndx, (secHdrEntrySize * secHdrEntries), secHdrTblOffset);
  vector<s_SHdr> sectionHeader = get_section_headers(secHdrTbl, secHdrEntrySize, secHdrEntries);
  vector<char> secStrings = read_from_binary(ndx, (sectionHeader[secStringNdx].sh_size), sectionHeader[secStringNdx].sh_offset);
  load_sections_from_file(sectionHeader, secStrings, ndx);
}


vector<s_SHdr> Linker::get_section_headers(vector<char> sec_hdr_tbl, uint16_t ent_size, uint16_t num) {
  vector<s_SHdr> temp;
  for(int i = 0; i < num; i++){
    vector<char> entry(ent_size);
    copy(sec_hdr_tbl.begin() + i * ent_size, sec_hdr_tbl.begin() + (i+1) * ent_size, entry.begin());
    temp.push_back(s_SHdr(
      static_cast<uint32_t> (load_data_from_char_vect(entry, sizeof(uint32_t), offsetof(s_SHdr, sh_ndx))),
      static_cast<uint32_t> (load_data_from_char_vect(entry, sizeof(uint32_t), offsetof(s_SHdr, sh_name))),
      static_cast<uint64_t> (load_data_from_char_vect(entry, sizeof(uint64_t), offsetof(s_SHdr, sh_size))),
      static_cast<uint32_t> (load_data_from_char_vect(entry, sizeof(uint32_t), offsetof(s_SHdr, sh_type))),
      static_cast<uint64_t> (load_data_from_char_vect(entry, sizeof(uint64_t), offsetof(s_SHdr, sh_addr))),
      static_cast<uint64_t> (load_data_from_char_vect(entry, sizeof(uint64_t), offsetof(s_SHdr, sh_offset))),
      static_cast<uint32_t> (load_data_from_char_vect(entry, sizeof(uint32_t), offsetof(s_SHdr, sh_flags))),
      static_cast<uint32_t> (load_data_from_char_vect(entry, sizeof(uint32_t), offsetof(s_SHdr, sh_link))),
      static_cast<uint32_t> (load_data_from_char_vect(entry, sizeof(uint32_t), offsetof(s_SHdr, sh_info))),
      static_cast<uint64_t> (load_data_from_char_vect(entry, sizeof(uint64_t), offsetof(s_SHdr, sh_addralign))),
      static_cast<uint64_t> (load_data_from_char_vect(entry, sizeof(uint64_t), offsetof(s_SHdr, sh_entsize)))
    ));
  }
  return temp;
}


vector<s_SSym> Linker::get_sym_tab(vector<char> sym_tbl_vect, uint16_t ent_size, uint16_t num){
  vector<s_SSym> temp;
  for(int i = 0; i < num; i++) {
    vector<char> entry(ent_size);
    copy(sym_tbl_vect.begin() + i * ent_size, sym_tbl_vect.begin() + (i+1) * ent_size, entry.begin());
    temp.push_back(s_SSym(
      static_cast<uint32_t> (load_data_from_char_vect(entry, sizeof(uint32_t), offsetof(s_SSym, sym_name))),
      static_cast<unsigned char> (load_data_from_char_vect(entry, sizeof(unsigned char), offsetof(s_SSym, sym_bind))),
      static_cast<unsigned char> (load_data_from_char_vect(entry, sizeof(unsigned char), offsetof(s_SSym, sym_type))),
      static_cast<uint64_t> (load_data_from_char_vect(entry, sizeof(uint64_t), offsetof(s_SSym, sym_value))),
      static_cast<uint16_t> (load_data_from_char_vect(entry, sizeof(uint16_t), offsetof(s_SSym, sym_ndx))),
      static_cast<uint64_t> (load_data_from_char_vect(entry, sizeof(uint64_t), offsetof(s_SSym, sym_size)))
    ));
  }

  return temp;
}


void Linker::load_sections_from_file(vector<s_SHdr> hdrTbl, vector<char> secString, int fileNdx){
  vector<char> symString;
  vector<s_SSym> temp_sym_tab;
  for(int i = 0; i < hdrTbl.size(); i++) {
    string name = get_name_from_str_array(hdrTbl[i].sh_name, secString);
    if(hdrTbl[i].sh_type == SHT_PROGBITS || hdrTbl[i].sh_type == SHT_NULL) {
      load_progbits(hdrTbl[i], name, fileNdx);
    } else if (hdrTbl[i].sh_type == SHT_RELA) {
      vector<char> rela_tab_vect = read_from_binary(fileNdx, hdrTbl[i].sh_size, hdrTbl[i].sh_offset);
      vector<s_Rela> rela_tbl = get_rela_tab(rela_tab_vect, hdrTbl[i].sh_entsize, (hdrTbl[i].sh_size / hdrTbl[i].sh_entsize));
      string nameSec = get_name_from_str_array(hdrTbl[hdrTbl[i].sh_link].sh_name, secString);
      for(int j = 0; j < rela_tbl.size(); j++) {
        if(reloc.count(sec_name_ndx[nameSec]) <= 0)
          reloc.insert(make_pair(sec_name_ndx[nameSec], vector<s_Rela>()));
        string symName = get_name_from_str_array(temp_sym_tab[rela_tbl[j].r_symval].sym_name, symString);
        reloc.at(sec_name_ndx[nameSec]).push_back(s_Rela(
          sections[sec_name_ndx[nameSec]].sh_size - hdrTbl[hdrTbl[i].sh_link].sh_size + rela_tbl[j].r_offset,
          sym_name_ndx[symName],
          rela_tbl[j].r_type,
          0,
          rela_tbl[j].r_size
        ));
        if(symTbl[sym_name_ndx[symName]].sym_type == STT_SECTION && rela_tbl[j].r_addend != 0) {
          reloc.at(sec_name_ndx[nameSec])[reloc.at(sec_name_ndx[nameSec]).size() - 1].r_addend = 
            sections[sec_name_ndx[symName]].sh_size - hdrTbl[temp_sym_tab[rela_tbl[j].r_symval].sym_ndx].sh_size + rela_tbl[j].r_addend;
        }
      }
    } else if (hdrTbl[i].sh_type == SHT_SYMTAB) {
        symString = read_from_binary(fileNdx, hdrTbl[hdrTbl[i].sh_link].sh_size, hdrTbl[hdrTbl[i].sh_link].sh_offset);
        vector<char> symtab_vect = read_from_binary(fileNdx, hdrTbl[i].sh_size, hdrTbl[i].sh_offset);
        temp_sym_tab = get_sym_tab(symtab_vect, hdrTbl[i].sh_entsize, (hdrTbl[i].sh_size / hdrTbl[i].sh_entsize));
        load_symbols(hdrTbl, i, secString, temp_sym_tab, symString, fileNdx);
    }
  }
}


vector<s_Rela> Linker::get_rela_tab(vector<char> rela_tab_vec, uint16_t ent_size, uint16_t num) {
  vector<s_Rela> temp;
  for(int i = 0; i < num; i++) {
    vector<char> entry(ent_size);
    copy(rela_tab_vec.begin() + i * ent_size, rela_tab_vec.begin() + (i+1) * ent_size, entry.begin());
    temp.push_back(s_Rela(
      static_cast<uint64_t> (load_data_from_char_vect(entry, sizeof(uint64_t), offsetof(s_Rela, r_offset))),
      static_cast<uint64_t> (load_data_from_char_vect(entry, sizeof(uint64_t), offsetof(s_Rela, r_symval))),
      static_cast<uint32_t> (load_data_from_char_vect(entry, sizeof(uint32_t), offsetof(s_Rela, r_type))),
      static_cast<uint64_t> (load_data_from_char_vect(entry, sizeof(uint64_t), offsetof(s_Rela, r_addend))),
      static_cast<uint32_t> (load_data_from_char_vect(entry, sizeof(uint32_t), offsetof(s_Rela, r_size)))
    ));
  }

  return temp;

}


void Linker::load_symbols(vector<s_SHdr>hdrTbl, int i, vector<char> secString, vector<s_SSym> temp_sym_tab, vector<char> symString, int fileNdx) {
    for(int j = 0; j < temp_sym_tab.size(); j++) {
      string name_sym = get_name_from_str_array(temp_sym_tab[j].sym_name, symString);
      if(sym_name_ndx.count(name_sym) <= 0) {
        sym_name_ndx.insert(make_pair(name_sym, symTbl.size()));
        // Value and ndx will be different
        // Ndx will be ndx in the newly created section table, if its not UNDEF(0)
        // Value will be offset in said section(that is possibly concatenated of multiple sections from different files)
        symTbl.push_back(s_SSym(temp_sym_tab[j].sym_name, temp_sym_tab[j].sym_bind, temp_sym_tab[j].sym_type, 
                              (sections[sec_name_ndx[get_name_from_str_array(static_cast<int>(hdrTbl[temp_sym_tab[j].sym_ndx].sh_name), secString)]].sh_size - hdrTbl[temp_sym_tab[j].sym_ndx].sh_size + temp_sym_tab[j].sym_value), 
                              sec_name_ndx[get_name_from_str_array(static_cast<int>(hdrTbl[temp_sym_tab[j].sym_ndx].sh_name), secString)], temp_sym_tab[j].sym_size));
      } else {
        if(temp_sym_tab[j].sym_type != STT_SECTION) {
          if(symTbl[sym_name_ndx[name_sym]].sym_bind == STB_GLOBAL) {
            if(symTbl[sym_name_ndx[name_sym]].sym_ndx == 0) {
              symTbl[sym_name_ndx[name_sym]].sym_ndx = sec_name_ndx[get_name_from_str_array(static_cast<int>(hdrTbl[temp_sym_tab[j].sym_ndx].sh_name), secString)];
              symTbl[sym_name_ndx[name_sym]].sym_value = sections[sec_name_ndx[get_name_from_str_array(static_cast<int>(hdrTbl[temp_sym_tab[j].sym_ndx].sh_name), secString)]].sh_size - hdrTbl[temp_sym_tab[j].sym_ndx].sh_size + temp_sym_tab[j].sym_value;
            } else {
              // Todo : Change report of this
              if(temp_sym_tab[j].sym_ndx != 0) {
                cout << "Symbol : " << name_sym << " is already defined. Current file definition : " << inFileNames[fileNdx] << endl;
                throw CustomException("*LE : Symbol defined multiple times");
              }
            }
          }
        }
      }
    }

}


string Linker::get_name_from_str_array(int start_ndx, vector<char> secString) {
  string temp = "";
  for(int i = start_ndx; i < secString.size(); i++) {
    if(secString[i] != 0)
      temp.push_back(secString[i]);
    else 
      return temp;
  }
  return temp;
}


void Linker::load_progbits(s_SHdr secHdr, string name, int fileNdx){
  bool new_sec = false;
  if(sec_name_ndx.count(name) <= 0 ) {
    sec_name_ndx.insert(make_pair(name, sections.size()));
    sec_content.insert(make_pair(sections.size(), vector<char>()));
    // Offset is zero because sections will be defined differently now
    sections.push_back(s_SHdr(sections.size(), secHdr.sh_name, secHdr.sh_size, secHdr.sh_type, secHdr.sh_addr, 0, secHdr.sh_flags, 
                            secHdr.sh_link, secHdr.sh_info, secHdr.sh_addralign, secHdr.sh_entsize));
    new_sec = true;
  }
  vector<char> sec_data = read_from_binary(fileNdx, secHdr.sh_size, secHdr.sh_offset);
  if(sec_data.size() > 0)
    sec_content[sec_name_ndx[name]].insert(sec_content[sec_name_ndx[name]].end(), sec_data.begin(), sec_data.end());
  if(!new_sec)
    sections[sec_name_ndx[name]].sh_size += secHdr.sh_size;
}


size_t Linker::load_data_from_char_vect(vector<char> vect, int size, size_t start_pos){
  size_t temp;
  if(start_pos + size <= vect.size())
    memcpy(&temp, &vect[start_pos], size);
  else 
    throw CustomException("*LI : Tried to copy over vector boundary");
  return temp;
}


vector<char> Linker::read_from_binary(int ndx, int chunk, uint32_t start_addr){
  vector<char> temp(chunk);
  inputFiles[ndx].seekg(start_addr);
  inputFiles[ndx].read(temp.data(), chunk);
  return temp;
}
// *********************************************************************************************************************

int main(int argc, char const *argv[]) {
  string outFile;

  try {
    int num_arg = argc;
    vector<string> arr_arg;
    for(int i = 0; i < argc; i++)
      arr_arg.push_back(argv[i]);

    if(num_arg < MIN_CMD_ARGS + 1)
      if(num_arg < MIN_CMD_ARGS)
        throw CustomException("*LE : Not enough arguments provided");
    
    // For executing via debugger
    int hex = -1;
    int opt = -1;
    vector<int> place = vector<int> ();
    vector<string> places = vector<string>();
    outFile = "";
    vector<string> inFiles = vector<string> ();
    int i = 2; // For debug execution
    i = 1; // For normal execution
    for(i; i < arr_arg.size(); i++){
      if(arr_arg[i] == "-hex")
        hex = i;
      else if(arr_arg[i].find("-place") == 0) {
        place.push_back(i);
        places.push_back(arr_arg[i]);
      }else if (arr_arg[i] == "-o"){
        opt = i++;
        outFile = arr_arg[i];
      } else {
        if(arr_arg[i].substr(arr_arg[i].length() - 2) != ".o") throw CustomException("*LE : You must pass only object files (.o)");
        inFiles.push_back(arr_arg[i]);
      }
    }

    if(hex < 0) throw CustomException("*LE : Hex option not specified");
    if(opt < 0) throw CustomException("*LE : -o option not specified");
    if(inFiles.size() == 0) throw CustomException("*LE : No input files specified");
    if(outFile.substr(outFile.length() - 4) != ".hex") throw CustomException("*LE : Output file doesn't have hex suffix");

    // For normal execution
    // for(int i = 1; i < num_arg.size(); i++){

    // }

    Linker ld(outFile, inFiles, places);

    ld.first_pass();
    // ld.print_header_table();
    // ld.print_symbol_table();
    // ld.print_reloc_table();
    ld.second_pass();
    ld.print_header_table();
    ld.print_symbol_table();
    ld.print_reloc_table();

    cout << "Linking to " << outFile << " done" << endl;

  } catch(const std::exception &e){
    // char filename[outFile.length()] = outFile;
    // remove(filename);
    cerr << e.what() << endl;
  }


  return 0;
}


// **************************************************************************************************************************
// print functions
void Linker::print_header_table() {
  helperFile << "Section Header Table" << endl;
  helperFile << setw(4) << setfill(' ') << left << "[NR] " <<
    setw(17) << setfill(' ') << left << "Index " << 
    setw(17) << setfill(' ') << left << "Name " << 
    setw(17) << setfill(' ') << left << "Type " << 
    setw(17) << setfill(' ') << left << "Address " << 
    setw(17) << setfill(' ') << left << "Offset " << 
    setw(17) << setfill(' ') << left << "Size " << 
    setw(17) << setfill(' ') << left << "EntSize " << 
    setw(17) << setfill(' ') << left << "Flags " <<
    setw(9) << setfill(' ') << left << "Link " << 
    setw(9) << setfill(' ') << left << "Info " <<
    setw(17) << setfill(' ') << left << "Align" << endl;
  for(int i = 0; i < sections.size(); i++) {
    helperFile << setw(4) << setfill(' ') << left << i << " " <<
      setw(17) << setfill(' ') << left << sections[i].sh_ndx << 
      // setw(17) << setfill(' ') << left << sections[i].sh_name << 
      setw(17) << setfill(' ') << left << find_name_by_sec_ndx(i) << 
      setw(17) << setfill(' ') << left << e_SHT[sections[i].sh_type] << 
      setw(17) << setfill(' ') << left << hex << sections[i].sh_addr << 
      setw(17) << setfill(' ') << left << dec << sections[i].sh_offset  << 
      setw(17) << setfill(' ') << left << sections[i].sh_size << 
      setw(17) << setfill(' ') << left << sections[i].sh_entsize << 
      setw(17) << setfill(' ') << left << sections[i].sh_flags <<
      setw(9) << setfill(' ') << left << sections[i].sh_link << 
      setw(9) << setfill(' ') << left << sections[i].sh_info <<
      setw(17) << setfill(' ') << left << sections[i].sh_addralign << endl;
  }
}


void Linker::print_symbol_table() {
  helperFile << "Symbol Table" << endl;
  helperFile << setw(5) << setfill(' ') << left << "Num " <<
    setw(18) << setfill(' ') << left << "Value " << 
    setw(8) << setfill(' ') << left << "Size " << 
    setw(10) << setfill(' ') << left << "Type " <<
    setw(10) << setfill(' ') << left << "Bind " <<
    setw(20) << setfill(' ') << left << "Ndx " << 
    setw(15) << setfill(' ') << left << "Name" << endl;
  for(int i = 0; i < symTbl.size(); i++) {
    helperFile << setw(5) << setfill(' ') << left << dec << i << 
      setw(18) << setfill(' ') << left << symTbl[i].sym_value << 
      setw(8) << setfill(' ') << left << symTbl[i].sym_size << 
      setw(10) << setfill(' ') << left << e_STT[symTbl[i].sym_type] << 
      setw(10) << setfill(' ') << left << e_STB[symTbl[i].sym_bind] << 
      setw(20) << setfill(' ') << left << find_name_by_sec_ndx(symTbl[i].sym_ndx) << 
      // setw(15) << setfill(' ') << left << symTbl[i].sym_name << endl;
      setw(15) << setfill(' ') << left << find_name_by_sym_ndx(i) << endl;
  }
}


void Linker::print_reloc_table(){
  for(auto& elem : reloc) {
    helperFile << ".rela." << find_name_by_sec_ndx(elem.first) << endl;
    helperFile << setw(20) << setfill(' ') << left << "Offset " << 
      setw(20) << setfill(' ') << left << "Type " <<
      setw(20) << setfill(' ') << left << "Sym. Val. " << 
      setw(20) << setfill(' ') << left << "Addend" << 
      setw(20) << setfill(' ') << left << "Size " << endl;
    for(int i = 0; i < elem.second.size(); i++) {
    helperFile << setw(20) << setfill(' ') << left << elem.second[i].r_offset << 
      setw(20) << setfill(' ') << left << elem.second[i].r_type <<
      setw(20) << setfill(' ') << left << find_name_by_sym_ndx(elem.second[i].r_symval) << 
      setw(20) << setfill(' ') << left << elem.second[i].r_addend << 
      setw(20) << setfill(' ') << left << elem.second[i].r_size << endl;
    }
  }
}

string Linker::find_name_by_sec_ndx(int ndx){
  for(auto& elem : sec_name_ndx)
    if(elem.second == ndx)
      return elem.first;
  return "";
}


string Linker::find_name_by_sym_ndx(int ndx){
  for(auto& elem : sym_name_ndx)
    if(elem.second == ndx)
      return elem.first;
  return "";
  }
// **************************************************************************************************************************