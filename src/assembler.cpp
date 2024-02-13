
#include "../inc/assembler.hpp"

Assembler::Assembler(string outFile, string inFile) : outFile(outFile), inFile(inFile) {
  // opening of provided files
  string tempInFile = inFile;
  inputFile.open(tempInFile, ios::in);
  if(!inputFile.is_open()) {
    tempInFile = "./tests/" + inFile;
    inputFile.open(tempInFile, ios::in);
    if(!inputFile.is_open()) {
      tempInFile = "./hidden_tests/" + inFile;
      inputFile.open(tempInFile, ios::in);
      if(!inputFile.is_open()) throw CustomException("*E : Input file is not open");
    }
  }
  outputFile.open(outFile, ios::binary | ios::trunc);
  if(!outputFile.is_open()) throw CustomException("*E : Output file is not open");

  string hlpFile = outFile.substr(0, outFile.length() - 2);
  hlpFile = hlpFile.append(".txt");
  hlpFile = "./txtFiles/" + hlpFile; 
  helperFile.open(hlpFile, ios::out | ios::trunc);

  // creation of first entries in symbol table and section header table
  secHdrTbl.push_back(s_SecHdr(""));
  symTable.push_back(s_Sym(""));
}

Assembler::~Assembler() {
  inputFile.close();
  // if(inputFile.is_open()) throw CustomException("*E : Input file not closed");
  outputFile.close();
  // if(outputFile.is_open()) throw CustomException("*E : Output file not closed");
  helperFile.close();
  // if(helperFile.is_open()) throw CustomException("*E : Helper file not closed");
}

// **************************************************************************************************************************
// Startup functions
// **************************************************************************************************************************
void Assembler::check_cmd_line(int argc, char const *argv[]){
  // Todo : for debugging, delete after 
  if(argc != CMD_LINE_SIZE + 1)
    if(argc != CMD_LINE_SIZE) throw CustomException("*E : Command line does not have CMD_LINE_SIZE");
  for(int i = 0; i < argc; i++)
    if(!argv[i]) throw CustomException("*E : Not all arguments are set");
}


void Assembler::check_input_params(string options, string outputFile, string inputFile){
  if(options != "-o") throw CustomException("*E : Option -o was not set");
  if(outputFile.length() < 2 || outputFile.substr(outputFile.length() - 2) != ".o") throw CustomException("*E : Output file name not set to .o");
  if(inputFile.length() < 2 || inputFile.substr(inputFile.length() - 2) != ".s") throw CustomException("*E : Input file was not assembly file");
}
// **************************************************************************************************************************

// **************************************************************************************************************************
// Assembly functions
// **************************************************************************************************************************
void Assembler::first_pass() {
  if(inputFile.is_open()) {
    string line;
    int i = 0;
    bool isEnd = false;
    while(getline(inputFile, line) && !isEnd){
      // Deleting all comments
      line = line.substr(0, line.find(COMMENT_DELIMETER));
      int pos = line.find_first_not_of(SPACE_CHAR);
      line = pos > line.length() ? EMPTY_STRING : line.substr(pos, line.length());
      if(!line.empty()) {
        s_LineStruct tempStruct;
        tempStruct.line = line;
        if(line[0] == DIRECTIVE_START_CHAR) {
          tempStruct.type = DIRECTIVE;
          // handle directive
          isEnd = handle_directive_first_pass(tempStruct);
        } else {
          pos = line.find(LABEL_END_CHAR, 0);
          if(pos > line.length()) {
            tempStruct.type = INSTRUCTION;
            // handle instruction
            handle_instruction_first_pass(tempStruct);
          } else {
            pos = line.find(DIRECTIVE_START_CHAR, pos);
            if(pos < line.length())
              tempStruct.type = L_D;
            else {
              pos = line.find_first_not_of(SPACE_CHAR, (line.find(LABEL_END_CHAR) + 1));
              if(pos > line.length())
                tempStruct.type = LABEL;
              else
                tempStruct.type = L_I;
            }
            handle_label_first_pass(tempStruct);
          }
        }
        if(line.find_last_of(DIRECTIVE_START_CHAR) != line.find_first_of(DIRECTIVE_START_CHAR)) throw CustomException("More than one directive started in one line");
        if(line.find_last_of(LABEL_END_CHAR) != line.find_first_of(LABEL_END_CHAR)) throw CustomException("More than one label in one line");
        lines.insert(make_pair(i++, tempStruct));
      }
    }
  } else throw CustomException("*E : Input file not open for first pass");
  // for(int i = 0; i < lines.size(); i++) {
  //   cout << "Line no: " << setw(6) << setfill(SPACE_CHAR) << i << " | " << left << setw(100) << setfill(SPACE_CHAR) << lines[i].line << " | Type : " << setw(15) << setfill(SPACE_CHAR)  
  //       << e_n_LineType[lines[i].type] << " | " << setw(15) << setfill(SPACE_CHAR) << lines[i].info << " | " << setw(2) << setfill(SPACE_CHAR) << lines[i].param_begin 
  //       << setw(5) << setfill(SPACE_CHAR) << " |";
  //   for(int j = 0; j < lines[i].operands.size(); j++)
  //     cout << setw(7) << setfill(SPACE_CHAR) << lines[i].operands[j] << " ";
  //   cout << endl;
  // }
  // print_symbol_table();
  // print_header_table();
  // print_reloc_table();
}

void Assembler::second_pass() {
  bool isEnd = false;
  for(int i = 0; i < lines.size(); i++) {
    switch (lines[i].type){
    case LABEL:
    case L_I : 
    case L_D :
      handle_label_second_pass(lines[i]);
      break;
    case DIRECTIVE :
      isEnd = handle_directive_second_pass(lines[i]);
      break;
    case INSTRUCTION : 
      handle_instruction_second_pass(lines[i]);
      break;
    default:
      throw CustomException("*E : Type for line not correctly specified");
      break;
    }
    if(isEnd)
      break;
  }
  // for(int i = 0; i < lines.size(); i++) {
  //   cout << "Line no: " << setw(6) << setfill(SPACE_CHAR) << i << " | " << left << setw(100) << setfill(SPACE_CHAR) << lines[i].line << " | Type : " << setw(15) << setfill(SPACE_CHAR)  
  //       << e_n_LineType[lines[i].type] << " | " << setw(15) << setfill(SPACE_CHAR) << lines[i].info << " | " << setw(2) << setfill(SPACE_CHAR) << lines[i].param_begin 
  //       << setw(5) << setfill(SPACE_CHAR) << " |";
  //   for(int j = 0; j < lines[i].operands.size(); j++)
  //     cout << setw(7) << setfill(SPACE_CHAR) << lines[i].operands[j] << " ";
  //   cout << endl;
  // }
  // print_symbol_table();
  // print_header_table();
  // print_reloc_table();
  // for(auto& it : sec_literals) {
  //   cout << "Literal pool for section : " << secHdrTbl[it.first].sec_name << endl;
  //   for(int j = 0; j < sec_literals.at(it.first).size(); j++)
  //     cout << "Literal : " << setw(20) << setfill(SPACE_CHAR)  << it.second[j].literal << " | Sym : " << setw(20) << setfill(SPACE_CHAR) << it.second[j].symName << endl;
  // }
  write_to_out_file();
}

void Assembler::handle_label_second_pass(s_LineStruct& line) {
  if(line.type == L_I)
    handle_instruction_second_pass(line);
  if(line.type == L_D)
    handle_directive_second_pass(line);
}

bool Assembler::handle_directive_second_pass(s_LineStruct& line){
  if(line.info == __DIR_GLOBAL || line.info == __DIR_EXTERN) {
    for(int i = 0; i < line.operands.size(); i++)
      if(find_sym_tbl_entry(line.operands[i]) > 0)
        symTable[find_sym_tbl_entry(line.operands[i])].sym_bind = GLOBAL;
      else throw CustomException("*E : Second passage of assembler, symbol not in symTab");
  } else if (line.info == __DIR_SECTION) {
    currentSection = line.info;
    if(locationCounter != secHdrTbl[currSecIndex].sec_size) 
      throw CustomException("*Err : Second passage and first passage section sizes not same");
    // currSecIndex = (sec_literals.count(currSecIndex) > 0) ? currSecIndex + 2 : currSecIndex + 1;
    currSecIndex++;
    // currSecIndex++;
    if(secHdrTbl.size() < currSecIndex || secHdrTbl[currSecIndex].sec_name != line.operands[0]) throw CustomException("*E : No entry for current section in second passage");
    locationCounter = 0;
    if(sec_content.count(currSecIndex) < 0)
      sec_content.insert(make_pair(currSecIndex, vector<uint8_t>()));
  } else if (line.info == __DIR_WORD) {
    for(int i = 0; i < line.operands.size(); i++) {
      int value = string_to_val(line.operands[i]);
      if(sec_content.count(currSecIndex) < 0) throw CustomException ("*E : Current section does not have its map for storing data");
      if(value != -INT32_MAX) {
      // Todo : decide if i "flip" the bits here, or if i pack it like this for now and then after put bits [0,7] first, then [8,15] etc
      // Also : probably here i will alter put them to the object file 
        // sec_content[currSecIndex].push_back(value);
        put_4Byte_section_little_endian(value);
        locationCounter += WORD_SIZE;
      } else {
        if(!check_sym_in_tbl(line.operands[i])){
          symTable.push_back(s_Sym(line.operands[i], GLOBAL, NOTYPE, 0, 0));
        }
        // Todo : Add relocation table entry for place of 0x00000000
        relocTbl.push_back(s_RelocAdd(locationCounter, find_sym_tbl_entry(line.operands[i]), 0, 0, 32));
        if(reloc.count(currSecIndex) <= 0)
          reloc.insert(make_pair(currSecIndex, vector<s_Rela>()));
        reloc.at(currSecIndex).push_back(s_Rela(locationCounter, find_sym_tbl_entry(line.operands[i]), 0, 0, 32));
        // sec_content[currSecIndex].push_back(0x00000000);
        put_4Byte_section_little_endian(0x00000000);
        locationCounter += WORD_SIZE;
      }
    }
  } else if (line.info == __DIR_SKIP) {
    int value = string_to_val(line.operands[0]);
    if(value == -INT32_MAX) throw CustomException("*E : value for skip couldn't be converted");
    // Todo : decide if i "flip" the bits here, or if i pack it like this for now and then after put bits [0,7] first, then [8,15] etc
    // Also : probably here i will alter put them to the object file 
    if(sec_content.count(currSecIndex) < 0) throw CustomException ("*E : Current section does not have its map for storing data");
    int sz = string_to_val(line.operands[0]);
    if(sz >= 0) {
      // Todo : put sz bytes to section
      for(int i = 0; i < sz; i++) {
        put_byte_section(0x00);
        locationCounter++;
      }
    }
    else 
      throw CustomException("*E : value for skip couldnt be converted to int");
  } else if (line.info == __DIR_END) {
    locationCounter = 0;
    currSecIndex = 0;
    currentSection = "";
    return true;
  } else {
    throw CustomException("*E : Unsupported directive");
  }
  return false;
}

void Assembler::handle_instruction_second_pass(s_LineStruct& line) {
  if(sec_content.count(currSecIndex) < 0) throw CustomException ("*E : Current section does not have its map for storing data");
  if(line.info == __INST_HALT || line.info == __INST_INT) {
          locationCounter += WORD_SIZE;
    uint32_t opcode = instruction_map.at(line.info).opcode;

    // outputFile << "HALT|INST | [0][0] : " << hex << __GET_BITS_0_7(opcode) << " [0][0] : " << hex << __GET_BITS_8_15(opcode) << " [0][0] : " << hex << __GET_BITS_16_23(opcode) << " CMD : " << hex << __GET_BITS_24_31(opcode) << endl;
    // outputFile << hex << __GET_BITS_0_7(opcode) << " " << hex << __GET_BITS_8_15(opcode) << " " << hex << __GET_BITS_16_23(opcode) << " " << hex << __GET_BITS_24_31(opcode) << endl;

    // sec_content[currSecIndex].push_back(opcode);
    put_4Byte_section_little_endian(opcode);
    // locationCounter += WORD_SIZE;
  } else if (line.info == __INST_RET){
    pop_reg(__PC);
  } else if (line.info == __INST_IRET) {
    pop_reg(__PC);
    pop_status_reg(__REG_STATUS);
  } else if (line.info == __INST_CALL) {
    jmp_call(line.operands[0], __INST_CALL);
  } else if (line.info == __INST_JMP) {
    jmp_call(line.operands[0], __INST_JMP);
  } else if (line.info == __INST_BEQ || line.info == __INST_BNE || line.info == __INST_BGT) {
          locationCounter += WORD_SIZE;
    uint32_t opcode = _ZERO | __JMP;

    uint8_t reg1 = get_reg(line.operands[0]);
    uint8_t reg2 = get_reg(line.operands[1]);

    if(reg1 >= 0 && reg1 <= 15 && reg1 >= 0 && reg1 <= 15){
      string op_type = get_op_type(line.operands[2]);
      int index_of_l = (op_type == "ML") ? get_index_of_literal_for_section(string_to_val(line.operands[2])) : 
                      (op_type == "MS") ? get_index_of_literal_for_section(line.operands[2]) : -2;
      if(index_of_l == -1)
        throw CustomException("*E : No literal in the pool for second passage");
      else if (index_of_l == -2)
        throw CustomException("*I : Illegal operand type for call/jmp");
      uint32_t displacement = index_of_l * WORD_SIZE + (secHdrTbl[currSecIndex].sec_size - locationCounter);
      // 0010 - mmmm - aaaa - bbbb - cccc - dddd - dddd - dddd
      // MMMM==0b1001: if (gpr[B] == gpr[C]) pc<=mem32[gpr[A]+D];
      // MMMM==0b1010: if (gpr[B] != gpr[C]) pc<=mem32[gpr[A]+D];  
      // MMMM==0b1011: if (gpr[B] signed> gpr[C]) pc<=mem32[gpr[A]+D];
      opcode = (line.info == __INST_BEQ) ? __SET_BITS_24_27(opcode, __BEQ_MEM) : 
              ((line.info == __INST_BNE) ? __SET_BITS_24_27(opcode, __BNE_MEM) :
                                            __SET_BITS_24_27(opcode, __BGT_MEM));
      __SET_BITS_20_23(opcode, __PC); // aaaa
      __SET_BITS_16_19(opcode, reg1); // bbbb
      __SET_BITS_12_15(opcode, reg2); // cccc
      __SET_BITS_8_11(opcode, __GET_BITS_8_11(displacement)); // dddd - 8 - 11
      __SET_BITS_0_7(opcode ,__GET_BITS_0_7(displacement)); // dddd - dddd 

      // if(line.info == __INST_BEQ)
      //   outputFile << "BEQ | [d][d] : " << hex << __GET_BITS_0_7(opcode) << " [c][d] : " << hex << __GET_BITS_8_15(opcode) << " [a][b] : " << hex << __GET_BITS_16_23(opcode) << " CMD : " << hex << __GET_BITS_24_31(opcode) << endl;
      // else if(line.info == __INST_BNE)
      //   outputFile << "BNE | [d][d] : " << hex << __GET_BITS_0_7(opcode) << " [c][d] : " << hex << __GET_BITS_8_15(opcode) << " [a][b] : " << hex << __GET_BITS_16_23(opcode) << " CMD : " << hex << __GET_BITS_24_31(opcode) << endl;
      // else 
      //   outputFile << "BGT | [d][d] : " << hex << __GET_BITS_0_7(opcode) << " [c][d] : " << hex << __GET_BITS_8_15(opcode) << " [a][b] : " << hex << __GET_BITS_16_23(opcode) << " CMD : " << hex << __GET_BITS_24_31(opcode) << endl;
      // outputFile << hex << __GET_BITS_0_7(opcode) << " " << hex << __GET_BITS_8_15(opcode) << " " << hex << __GET_BITS_16_23(opcode) << " " << hex << __GET_BITS_24_31(opcode) << endl;
      

      // sec_content[currSecIndex].push_back(opcode);
      put_4Byte_section_little_endian(opcode);
      // locationCounter += WORD_SIZE;
    }
    else 
      throw CustomException("*E : Wrong conversion of register value in push");                                       
  } else if (line.info == __INST_PUSH) {
    uint8_t reg = get_reg(line.operands[0]);
    if(reg >= 0 && reg <= 15)
      push_reg(reg);
    else if (line.operands[0].substr(1) == "pc")
      push_reg(__PC);
    else if (line.operands[0].substr(1) == "sp")
      push_reg(__SP);
    else 
      throw CustomException("*E : Wrong conversion of register value in push");
  } else if (line.info == __INST_POP) {
    // get register from operands
    uint8_t reg = get_reg(line.operands[0]);
    if(reg >= 0 && reg <= 15 && reg != 14)
      pop_reg(reg);
    else if (line.operands[0] == "pc")
      pop_reg(__PC);
    else if (line.operands[0] == "sp" || reg == 14)
      throw CustomException("*I : Tried to pop to sp");
    else 
      throw CustomException("*E : Wrong conversion of register value in pop");
  } else if (line.info == __INST_XCHG) {
          locationCounter += WORD_SIZE;
    uint32_t opcode = _ZERO | __XCHG;
    
    //0100 - 0000 - 0000 - bbbb - cccc - 0000 - 0000 - 0000
    // temp<=gpr[B]; gpr[B]<=gpr[C]; gpr[C]<=temp; 

    if(line.operands[0] == "sp") throw CustomException("*I : Tried to alter SP via xchg operation");
    if(line.operands[0] == "pc") throw CustomException("*I : Tried to alter PC via xchg operation");
    uint8_t gprS = get_reg(line.operands[0]);
    if(line.operands[1] == "sp") throw CustomException("*I : Tried to alter SP via xchg operation");
    if(line.operands[1] == "pc") throw CustomException("*I : Tried to alter PC via xchg operation");
    uint8_t gprD = get_reg(line.operands[1]);

    __SET_BITS_12_15(opcode, gprS); // cccc
    __SET_BITS_16_19(opcode, gprD); // bbbb

    // outputFile << "XCHG | [0][0] : " << hex << __GET_BITS_0_7(opcode) << " [src][0] : " << hex << __GET_BITS_8_15(opcode) << " [0][dest] : " << hex << __GET_BITS_16_23(opcode) << " CMD : " << hex << __GET_BITS_24_31(opcode) << endl;
    // outputFile << hex << __GET_BITS_0_7(opcode) << " " << hex << __GET_BITS_8_15(opcode) << " " << hex << __GET_BITS_16_23(opcode) << " " << hex << __GET_BITS_24_31(opcode) << endl;


    // sec_content[currSecIndex].push_back(opcode);
    put_4Byte_section_little_endian(opcode);
    // locationCounter += WORD_SIZE;
  } else if (line.info == __INST_ADD || line.info == __INST_SUB || line.info == __INST_MUL || line.info == __INST_DIV){
    // get registers from operands of line
    uint8_t gprS = line.operands[0] == "pc" ? __PC : (line.operands[0] == "sp" ? __SP : get_reg(line.operands[0]));
    if(line.operands[1] == "sp") throw CustomException("*I : Tried to alter SP via aritmetic operation");
    if(line.operands[1] == "pc") throw CustomException("*I : Tried to alter PC via aritmetic operation");
    uint8_t gprD = get_reg(line.operands[1]);
    if(gprS >= 0 && gprD >= 0)
      arithmetic_op(gprS, gprD, line.info);
    else 
      throw CustomException("*E : Wrong operands for arithmetic operation");
  } else if (line.info == __INST_NOT || line.info == __INST_AND || line.info == __INST_OR || line.info == __INST_XOR){
    if(line.info != __INST_NOT) {
      uint8_t gprS = line.operands[0] == "pc" ? __PC : (line.operands[0] == "sp" ? __SP : get_reg(line.operands[0]));
      if(line.operands[1] == "sp") throw CustomException("*I : Tried to alter SP via aritmetic operation");
      if(line.operands[1] == "pc") throw CustomException("*I : Tried to alter PC via aritmetic operation");
      uint8_t gprD = get_reg(line.operands[1]);
      if(gprS >= 0 && gprD >= 0 && gprS <= 15 && gprD <= 15)
        logic_op(line.line, gprD, gprS);
      else 
        throw CustomException("*E : Wrong operands for logic operation");
    } else {
      if(line.operands[0] == "sp") throw CustomException("*I : Tried to alter SP via aritmetic operation");
      if(line.operands[0] == "pc") throw CustomException("*I : Tried to alter PC via aritmetic operation");
      uint8_t gpr = get_reg(line.operands[0]);
      if(gpr >= 0 && gpr <= 15)
        logic_op(line.line, gpr);
      else 
        throw CustomException("*E : Wrong operands for logic operation");
    }
  } else if (line.info == __INST_SHL || line.info == __INST_SHR){
    uint8_t gprS = line.operands[0] == "pc" ? __PC : (line.operands[0] == "sp" ? __SP : get_reg(line.operands[0]));
    if(line.operands[1] == "sp") throw CustomException("*I : Tried to alter SP via shift operation");
    if(line.operands[1] == "pc") throw CustomException("*I : Tried to alter PC via shift operation");
    uint8_t gprD = get_reg(line.operands[1]);
    if(gprS >= 0 && gprD >= 0 && gprS <= 15 && gprD <= 15)
      shift_op(gprS, gprD, line.info);
    else 
      throw CustomException("*E : Wrong operands for shift operation");
  } else if (line.info == __INST_LD) {
    load_instr(line.operands[0], line.operands[1]);
  } else if (line.info == __INST_ST) {
    store_instr(line.operands[0], line.operands[1]);
  } else if (line.info == __INST_CSRRD || line.info == __INST_CSRWR){
    uint8_t gpr = line.info == __INST_CSRWR ? (line.operands[0] == "pc" ? __PC : (line.operands[0] == "sp" ? __SP : get_reg(line.operands[0]))) : get_reg(line.operands[1]);
    uint8_t csr;
    if(line.info == __INST_CSRRD) {
      if(line.operands[1] == "sp") throw CustomException("*I : Tried to alter SP via csr operation");
      if(line.operands[1] == "pc") throw CustomException("*I : Tried to alter PC via csr operation");
      if(line.operands[0][0] != __REG_INDICATOR) throw CustomException("*E : No register start indicator '%' for csr register");
      string csr_reg = line.operands[0].substr(1);
      if(csr_reg == "handler") csr = __REG_HANDLER;
      else if (csr_reg == "status") csr = __REG_STATUS;
      else if (csr_reg == "cause") csr = __REG_CAUSE;
      else throw CustomException("*E : Bad name for csr register");
    } else {
      if(line.operands[1][0] != __REG_INDICATOR) throw CustomException("*E : No register start indicator '%' for csr register");
      string csr_reg = line.operands[1].substr(1);
      if(csr_reg == "handler") csr = __REG_HANDLER;
      else if (csr_reg == "status") csr = __REG_STATUS;
      else if (csr_reg == "cause") csr = __REG_CAUSE;
      else throw CustomException("*E : Bad name for csr register");
    }
    uint8_t gprD = get_reg(line.operands[1]);
    if(gpr >= 0 && gpr <= 15 && csr >= 0 && csr <= 2)
      gpr_csr(gpr, csr, line.info);
    else 
      throw CustomException("*E : Wrong operands for csr operation");
  } else
    throw CustomException ("*E : Not supported instruction in second passage");
}

void Assembler::handle_label_first_pass(s_LineStruct& line) {
  string lab_name = line.line.substr(0, line.line.find_first_of(LABEL_END_CHAR));
  if(check_sym_in_tbl(lab_name)) throw CustomException("*E : Label already in symbol table");
  symTable.push_back(s_Sym(lab_name, LOCAL, NOTYPE, locationCounter, currSecIndex));
  if(line.type == L_I) {
    s_LineStruct tempStruct = line;
    tempStruct.line = tempStruct.line.substr(tempStruct.line.find_first_of(LABEL_END_CHAR) + 1);
    handle_instruction_first_pass(tempStruct);
    line.info = tempStruct.info;
    line.param_begin = line.line.find_first_of(line.info, line.line.find_first_of(LABEL_END_CHAR)) + line.info.length() + 1;
  } else if (line.type == L_D) {
    s_LineStruct tempStruct = line;
    tempStruct.line = tempStruct.line.substr(tempStruct.line.find_first_not_of(SPACE_CHAR,tempStruct.line.find_first_of(LABEL_END_CHAR) + 1));
    handle_directive_first_pass(tempStruct);
    line.info = tempStruct.info;
    line.param_begin = line.line.find_first_of(line.info, line.line.find_first_of(LABEL_END_CHAR)) + line.info.length() + 1;
  }
}

// Todo : before entry, must check if already in the table
bool Assembler::handle_directive_first_pass(s_LineStruct& line) {
  int first_space_pos = line.line.find_first_of(SPACE_CHAR, line.line.find_first_of(DIRECTIVE_START_CHAR));
  string dir = line.line.substr(line.line.find_first_of(DIRECTIVE_START_CHAR), first_space_pos);
  line.info = dir;
  line.param_begin = line.line.find_first_of(SPACE_CHAR, line.line.find_first_of(line.info)) + 1;
  if(!try_to_map_dir(dir)) throw CustomException("Directive not supported");
  if(dir != __DIR_END) line.operands = divide_line(line.line.substr(line.param_begin), COMMA_CHAR);
  if(dir == __DIR_GLOBAL) {
  } else if (dir == __DIR_EXTERN) {
    vector<string> tokens = divide_line(line.line.substr(first_space_pos + 1), COMMA_CHAR);
    for(int i = 0; i < tokens.size(); i++) {
      // just the name of the symbol, with spaces removed
      if(!check_sym_in_tbl(tokens[i]))
        symTable.push_back(s_Sym(remove_space_back_front(tokens[i])));
      else 
        throw CustomException("*E : Extern tries to put already declared symbol in symTable");
    }
  } else if (dir == __DIR_SECTION) {
    string sec_name = remove_space_back_front(line.line.substr(line.line.find_first_not_of(SPACE_CHAR, first_space_pos)));
    if(currSecIndex != 0) {
      secHdrTbl[currSecIndex].sec_size = locationCounter;
      // if(sec_literals.count(currSecIndex) >= 0) {
      //   secHdrTbl.push_back(s_SecHdr(("litpool." + secHdrTbl[currSecIndex].sec_name), (sec_literals.at(currSecIndex).size() * WORD_SIZE), SEC_TYPE_NULL, 0, 0, 0, currSecIndex));
      // }
      // currSecIndex++;
    }
    if(!check_sym_in_tbl(sec_name)) symTable.push_back(s_Sym(sec_name, LOCAL, SECTION, 0, secHdrTbl.size()));
    secHdrTbl.push_back(s_SecHdr(sec_name));
    currentSection = sec_name;
    currSecIndex++;
    locationCounter = 0;
  } else if (dir == __DIR_WORD) {
    locationCounter += WORD_SIZE * line.operands.size();
  } else if (dir == __DIR_SKIP) {
    // .skip size
    int sz = string_to_val(line.operands[0]);
    if(sz >= 0)
      locationCounter += sz;
    else 
      throw CustomException("*E : value for skip couldnt be converted to int");
  } else if (dir == __DIR_END) {
    if(currSecIndex != 0) {
      secHdrTbl[currSecIndex].sec_size = locationCounter;
      // if(sec_literals.find(currSecIndex) != sec_literals.end()) {
      //   secHdrTbl.push_back(s_SecHdr(("litpool." + secHdrTbl[currSecIndex].sec_name), (sec_literals.at(currSecIndex).size() * WORD_SIZE)));
      // }
    }
    currentSection = "";
    currSecIndex = 0;
    locationCounter = 0;
    return true;
  } else {
    throw CustomException("*E : Unsupported directive");
  }
  return false;
}


void Assembler::handle_instruction_first_pass(s_LineStruct& line) {
  line.info = remove_space_back_front(line.line);
  if(instruction_map.count(line.info) <= 0) throw CustomException("*E : Instruction not supported");
  line.param_begin = line.line.find_first_of(SPACE_CHAR, line.line.find_first_of(line.info)) + 1;
  if(line.param_begin == 0)
    line.param_begin = line.line.find_last_not_of(SPACE_CHAR) + 1;
  if(instruction_map.at(line.info).no_operands < CALL_JMP_OPERANDS) {
    vector<string> tokens = divide_line(line.line.substr(line.param_begin), COMMA_CHAR);
    if(tokens.size() != instruction_map.at(line.info).no_operands) 
      throw CustomException("*E : Wrong number of operands");
  }
  string operands = line.line.substr(line.param_begin);
  operands = remove_chars_in_string(operands, SPACE_CHAR);
  switch(instruction_map.at(line.info).no_operands){
    case 0 : {
      if(!remove_space_back_front(operands).empty()) throw CustomException("*E : Instruction that should have no operands had them");
      break;
    }
    case 1 :
    case 2 :
    case C_JMP_OPERANDS :
    case CALL_JMP_OPERANDS :
    case LD_ST_OPERANDS : {
      line.operands = divide_line(operands, COMMA_CHAR);
      break;
    }
    default : {
      throw CustomException("*E : Usupported number of operands");
      break;
    }
  }
  for(int i = 0; i <line.operands.size(); i++){
    string op_type = get_op_type(line.operands[i]);
    if(op_type == "L") {
      // $literal
      add_literal_to_pool(string_to_val(line.operands[i].substr(1)));
    } else if (op_type == "S") {
      // $symbol
      add_symbol_to_pool(line.operands[i].substr(1));
    } else if (op_type == "ML") {
      add_literal_to_pool(string_to_val(line.operands[i]));
    } else if (op_type == "MS") {
      add_symbol_to_pool(line.operands[i]);
    } else if (op_type == "RL") {
      // vector<string> ops = divide_line(remove_space_back_front(line.operands[i]).substr(1), ']');
      // ops[0] = remove_chars_in_string(ops[0], SPACE_CHAR);
      // ops = divide_line(ops[0], '+');
      vector<string> ops = get_mem_ops(line.operands[i]);
      if(string_to_val(ops[1]) >= -2048 && string_to_val(ops[1]) < 2048)
        add_literal_to_pool(string_to_val(ops[1]));
      else throw CustomException("*Err : Literal value greaten than signed 2^12");
    } else if (op_type == "RS") {
      // vector<string> ops = divide_line(remove_space_back_front(line.operands[i]).substr(1), ']');
      // ops[0] = remove_chars_in_string(ops[0], SPACE_CHAR);
      // ops = divide_line(ops[0], '+');
      vector<string> ops = get_mem_ops(line.operands[i]);
      add_symbol_to_pool(ops[1]);
    } else if (op_type == "I" || op_type == "R" || op_type == "MR") {

    } else 
      throw CustomException("*E : Bad operand for instruction");
  }
  locationCounter = (line.info == __INST_IRET || (line.info == __INST_LD && (get_op_type(line.operands[0]) == "ML" || get_op_type(line.operands[0]) == "MS"))) ?  
                    (locationCounter + 2 * WORD_SIZE) : (locationCounter + WORD_SIZE);
}
// **************************************************************************************************************************

// **************************************************************************************************************************
// helper instructions
void Assembler::load_instr(string op0, string op1){
    uint32_t opcode = _ZERO | __LD;

    string op_type = get_op_type(op0);
    uint8_t reg = get_reg(op1);

    if(reg >= 0 && reg <= 15 ) {
      if(op_type == "L" || op_type == "S"){
        // $literal
        int index_of_l = (op_type == "L") ? get_index_of_literal_for_section(string_to_val(op0.substr(1))) : get_index_of_literal_for_section(op0.substr(1));
        if(index_of_l == -1)
          throw CustomException("*E : No literal in the pool for second passage");
        if(op_type == "S" && !check_sym_in_tbl(op0.substr(1)))
          symTable.push_back(s_Sym(op0.substr(1), GLOBAL, NOTYPE, 0, 0));
        uint32_t displacement = (index_of_l-1) * WORD_SIZE + (secHdrTbl[currSecIndex].sec_size - locationCounter);
        ld_mem_reg(reg, __PC, displacement);
      } else if (op_type == "ML" || op_type == "MS") {
        int index_of_l = (op_type == "ML") ? get_index_of_literal_for_section(string_to_val(op0)) : get_index_of_literal_for_section(op0);
        if(index_of_l == -1)
          throw CustomException("*E : No literal in the pool for second passage");
        if(op_type == "MS" && !check_sym_in_tbl(op0))
          symTable.push_back(s_Sym(op0, GLOBAL, NOTYPE, 0, 0));
        uint32_t displacement = (index_of_l - 1) * WORD_SIZE + (secHdrTbl[currSecIndex].sec_size - locationCounter);
        ld_mem_reg(reg, __PC, displacement);
        ld_mem_reg(reg, reg, 0);
      } else if (op_type == "R") {
          locationCounter += WORD_SIZE;
        uint8_t reg1 = get_reg(op0);
        if(reg1 >= 0 && reg1 <15){
          __SET_BITS_24_27(opcode, __LD_GPR_GPR_ADD);

          __SET_BITS_20_23(opcode, reg); // aaaa
          __SET_BITS_16_19(opcode, reg1); // bbbb

          outputFile << "LD R | [d][d] : " << hex << __GET_BITS_0_7(opcode) << " [c][d] : " << hex << __GET_BITS_8_15(opcode) << " [a][b] : " << hex << __GET_BITS_16_23(opcode) << " CMD : " << hex << __GET_BITS_24_31(opcode) << endl;
          outputFile << hex << __GET_BITS_0_7(opcode) << " " << hex << __GET_BITS_8_15(opcode) << " " << hex << __GET_BITS_16_23(opcode) << " " << hex << __GET_BITS_24_31(opcode) << endl;
          
          sec_content[currSecIndex].push_back(opcode);
          // locationCounter += WORD_SIZE;
        } else 
          throw CustomException("*Err : Bad reg conversion for load");
      } else if (op_type == "MR") {
        vector<string> ops = get_mem_ops(op0);
        uint8_t reg1 = get_reg(ops[0]);
        if(reg1 >= 0 && reg1 <= 15){
          ld_mem_reg(reg, reg1, 0);
        } else 
          throw CustomException("*Err : Bad reg conversion for load");
      } else if (op_type == "RL") {
        vector<string> ops = get_mem_ops(op0);
        uint8_t reg1 = get_reg(ops[0]);
        if(reg1 >= 0 && reg1 <= 15){
          int displacement = string_to_val(ops[1]);
          if(displacement != -INT32_MAX)
            ld_mem_reg(reg, reg1, displacement);
          else 
            throw CustomException("*Err : Bad string conversion");
        } else 
          throw CustomException("*Err : Bad reg conversion for load");
      } else if (op_type == "RS") {
        vector<string> ops = get_mem_ops(op0);
        uint8_t reg1 = get_reg(ops[0]);
        if(reg1 >= 0 && reg1 <= 15){
          if(!check_sym_in_tbl(ops[1]))
            // symTable.push_back(s_Sym(ops[1], GLOBAL, NOTYPE, 0, 0));
            throw CustomException("*E : Symbol must be known for indexed load with symbol");
          ld_mem_reg(reg, reg1, 0);
          relocTbl.push_back(s_RelocAdd(locationCounter - WORD_SIZE, find_sym_tbl_entry(ops[1]), 0, 0, 12));
          if(reloc.count(currSecIndex) <= 0)
            reloc.insert(make_pair(currSecIndex, vector<s_Rela>()));
          reloc.at(currSecIndex).push_back(s_Rela(locationCounter - WORD_SIZE, find_sym_tbl_entry(ops[1]), 0, 0, 12));
        } else 
          throw CustomException("*Err : Bad reg conversion for load");
      } else 
        throw CustomException("*Err : Bad conversion of operands for load");
    } else if (reg == __PC)
      throw CustomException("*I : Tried to load to PC");
    else if (reg == __SP)
      throw CustomException("*I : Tried to load to SP");
    else 
      throw CustomException("*Err : Bad register conversion for load");
}


void Assembler::store_instr(string op0, string op1){
          locationCounter += WORD_SIZE;
    uint32_t opcode = _ZERO | __ST;

    string op_type = get_op_type(op1);
    uint8_t reg = get_reg(op0);

    // 1000 - mmmm - aaaa - bbbb - cccc - dddd - dddd - dddd
    // MMMM==0b0000: mem32[gpr[A]+gpr[B]+D]<=gpr[C];
    // MMMM==0b0010: mem32[mem32[gpr[A]+gpr[B]+D]]<=gpr[C];
    // MMMM==0b0001: gpr[A]<=gpr[A]+D; mem32[gpr[A]]<=gpr[C];
    if(reg >= 0 && reg <= 15) {
      if(op_type == "L" || op_type == "S") {
        int index_of_l = (op_type == "L") ? get_index_of_literal_for_section(string_to_val(op1.substr(1))) : get_index_of_literal_for_section(op1.substr(1));
        if(index_of_l == -1)
          throw CustomException("*E : No literal in the pool for second passage");
        if(op_type == "S" && !check_sym_in_tbl(op1.substr(1)))
          symTable.push_back(s_Sym(op1.substr(1), GLOBAL, NOTYPE, 0, 0));
        uint32_t displacement = index_of_l * WORD_SIZE + (secHdrTbl[currSecIndex].sec_size - locationCounter);

        opcode = store(reg, __PC, displacement, __ST_MEM);
      } else if (op_type == "R") {
        uint8_t reg1 = get_reg(op1);
        if(reg1 >=0 && reg1 <= 15)
          opcode = store(reg, reg1, 0, __ST_MEM);
        else 
          throw CustomException("*Err : Wrong register conversion for store");
      } else if (op_type == "MR") {
        vector<string> ops = get_mem_ops(op1);
        uint8_t reg1 = get_reg(ops[0]);
        if(reg1 >=0 && reg1 <= 15)
          opcode = store(reg, reg1, 0, __ST_MEM_MEM);
        else 
          throw CustomException("*Err : Wrong register conversion for store");
      } else if (op_type == "MS" || op_type == "ML") {
        int index_of_l = (op_type == "ML") ? get_index_of_literal_for_section(string_to_val(op1)) : get_index_of_literal_for_section(op1);
        if(index_of_l == -1)
          throw CustomException("*E : No literal in the pool for second passage");
        if(op_type == "MS" && !check_sym_in_tbl(op1))
          symTable.push_back(s_Sym(op1, GLOBAL, NOTYPE, 0, 0));
        uint32_t displacement = index_of_l * WORD_SIZE + (secHdrTbl[currSecIndex].sec_size - locationCounter);

        opcode = store(reg, __PC, displacement, __ST_MEM_MEM);
      } else if (op_type == "RL") {
        vector<string> ops = get_mem_ops(op1);
        uint8_t reg1 = get_reg(ops[0]);
        if(reg1 >= 0 && reg1 <= 15){
          int displacement = string_to_val(ops[1]);
          if(displacement != -INT32_MAX)
            opcode = store(reg, reg1, displacement, __ST_MEM_MEM);
          else 
            throw CustomException("*Err : Bad string conversion");
        } else 
          throw CustomException("*Err : Bad reg conversion for load");
      } else if (op_type == "RS") {
        vector<string> ops = get_mem_ops(op1);
        uint8_t reg1 = get_reg(ops[0]);
        if(reg1 >= 0 && reg1 <= 15){
          if(!check_sym_in_tbl(ops[1]))
            // symTable.push_back(s_Sym(ops[1], GLOBAL, NOTYPE, 0, 0));
            throw CustomException("*E : Symbol must be known for indexed store with symbol");
          opcode = store(reg, reg1, 0, __ST_MEM_MEM);
          relocTbl.push_back(s_RelocAdd(locationCounter, find_sym_tbl_entry(ops[1]), 0, 0, 1.5));
          if(reloc.count(currSecIndex) <= 0)
            reloc.insert(make_pair(currSecIndex, vector<s_Rela>()));
          reloc.at(currSecIndex).push_back(s_Rela(locationCounter, find_sym_tbl_entry(ops[1]), 0, 0, 12));
        } else 
          throw CustomException("*Err : Bad reg conversion for load");
      } else 
        throw CustomException("*Err : Bad operand for store");
    } else 
      throw CustomException("*Err : Wrong register conversion for store");
    
    // outputFile << "ST | [d][d] : " << hex << __GET_BITS_0_7(opcode) << " [c][d] : " << hex << __GET_BITS_8_15(opcode) << " [a][b] : " << hex << __GET_BITS_16_23(opcode) << " CMD : " << hex << __GET_BITS_24_31(opcode) << endl;
    // outputFile << hex << __GET_BITS_0_7(opcode) << " " << hex << __GET_BITS_8_15(opcode) << " " << hex << __GET_BITS_16_23(opcode) << " " << hex << __GET_BITS_24_31(opcode) << endl;
  
    // sec_content[currSecIndex].push_back(opcode);
    put_4Byte_section_little_endian(opcode);
    // locationCounter += WORD_SIZE;

}


uint32_t Assembler::store(uint8_t stReg, uint8_t reg, int displacement, uint8_t op){
  // 1000 - mmmm - aaaa - bbbb - cccc - dddd - dddd - dddd
  // MMMM==0b0000: mem32[gpr[A]+gpr[B]+D]<=gpr[C];
  // MMMM==0b0010: mem32[mem32[gpr[A]+gpr[B]+D]]<=gpr[C];
  // MMMM==0b0001: gpr[A]<=gpr[A]+D; mem32[gpr[A]]<=gpr[C];

  uint32_t opcode = _ZERO | __ST; 
  __SET_BITS_24_27(opcode, op);

  __SET_BITS_20_23(opcode, reg); //aaaa
  __SET_BITS_12_15(opcode, stReg); //cccc

  __SET_BITS_8_11(opcode, __GET_BITS_8_11(displacement));
  __SET_BITS_0_7(opcode, __GET_BITS_0_7(displacement));

  return opcode;
}


void Assembler::ld_mem_reg(uint8_t ldReg, uint8_t reg, int displacement){
          locationCounter += WORD_SIZE;
  uint32_t opcode = _ZERO | __LD;

  __SET_BITS_24_27(opcode, __LD_GPR_MEM_GPR_D);

  __SET_BITS_20_23(opcode, ldReg); // aaaa
  __SET_BITS_16_19(opcode, reg); // bbbb

  __SET_BITS_8_11(opcode, __GET_BITS_8_11(displacement)); // dddd - 8 - 11
  __SET_BITS_0_7(opcode ,__GET_BITS_0_7(displacement)); // dddd - dddd 

  // outputFile << "LD | [d][d] : " << hex << __GET_BITS_0_7(opcode) << " [c][d] : " << hex << __GET_BITS_8_15(opcode) << " [a][b] : " << hex << __GET_BITS_16_23(opcode) << " CMD : " << hex << __GET_BITS_24_31(opcode) << endl;
  // outputFile << hex << __GET_BITS_0_7(opcode) << " " << hex << __GET_BITS_8_15(opcode) << " " << hex << __GET_BITS_16_23(opcode) << " " << hex << __GET_BITS_24_31(opcode) << endl;
  
  // sec_content[currSecIndex].push_back(opcode);
  put_4Byte_section_little_endian(opcode);
  // locationCounter += WORD_SIZE;
}


void Assembler::jmp_call(string operand, string op){
          locationCounter += WORD_SIZE;
  uint32_t opcode = (op == __INST_CALL) ? (_ZERO | __CALL) : 
                    ((op == __INST_JMP) ? (_ZERO | __JMP) : throw CustomException("*ERR : Bad call for function jmp call"));
  string op_type = get_op_type(operand);
  int index_of_l = (op_type == "ML") ? get_index_of_literal_for_section(string_to_val(operand)) : 
                  (op_type == "MS") ? get_index_of_literal_for_section(operand) : -2;
  if(index_of_l == -1)
    throw CustomException("*E : No literal in the pool for second passage");
  else if (index_of_l == -2)
    throw CustomException("*I : Illegal operand type for call/jmp");
  uint32_t displacement = index_of_l * WORD_SIZE + (secHdrTbl[currSecIndex].sec_size - locationCounter);

  // 0010 - mmmm - aaaa - bbbb - 0000 - dddd - dddd - dddd
  // CALL : push pc; pc<=mem32[gpr[A]+gpr[B]+D];
  // JMP : pc<=mem32[gpr[A]+D];
  opcode = (op == __INST_CALL) ? __SET_BITS_24_27(opcode, __CALL_MEM32_ADD) : __SET_BITS_24_27(opcode, __JMP_MEM);
  

  __SET_BITS_20_23(opcode, __PC); // aaaa
  __SET_BITS_8_11(opcode, __GET_BITS_8_11(displacement)); // dddd - 8 - 11
  __SET_BITS_0_7(opcode ,__GET_BITS_0_7(displacement)); // dddd - dddd 

  // if(op == __INST_CALL)
  //   outputFile << "CALL | [d][d] : " << hex << __GET_BITS_0_7(opcode) << " [0][d] : " << hex << __GET_BITS_8_15(opcode) << " [a][b] : " << hex << __GET_BITS_16_23(opcode) << " CMD : " << hex << __GET_BITS_24_31(opcode) << endl;
  // else 
  //   outputFile << "JMP | [d][d] : " << hex << __GET_BITS_0_7(opcode) << " [0][d] : " << hex << __GET_BITS_8_15(opcode) << " [a][b] : " << hex << __GET_BITS_16_23(opcode) << " CMD : " << hex << __GET_BITS_24_31(opcode) << endl;
  // outputFile << hex << __GET_BITS_0_7(opcode) << " " << hex << __GET_BITS_8_15(opcode) << " " << hex << __GET_BITS_16_23(opcode) << " " << hex << __GET_BITS_24_31(opcode) << endl;
  
  // sec_content[currSecIndex].push_back(opcode);
  put_4Byte_section_little_endian(opcode);
  // locationCounter += WORD_SIZE;
}


void Assembler::shift_op(uint8_t gprS, uint8_t gprD, string op){
          locationCounter += WORD_SIZE;
  uint32_t opcode = _ZERO | __SHIFT;
  opcode = (op == __INST_SHL) ? (__SET_BITS_24_27(opcode, __SHIFT_LEFT)) : (__SET_BITS_24_27(opcode, __SHIFT_RIGHT));

  // 0111 - mmmm - aaaa - bbbb - cccc - 0000 - 0000 - 0000

  __SET_BITS_20_23(opcode, gprD); // aaaa
  __SET_BITS_16_19(opcode, gprD); // bbbb
  __SET_BITS_12_15(opcode, gprS); // cccc

  // outputFile << "SHIFT | [0][0] : " << hex << __GET_BITS_0_7(opcode) << " [src][0] : " << hex << __GET_BITS_8_15(opcode) << " [dest][dest] : " << hex << __GET_BITS_16_23(opcode) << " CMD : " << hex << __GET_BITS_24_31(opcode) << endl;
  // outputFile << hex << __GET_BITS_0_7(opcode) << " " << hex << __GET_BITS_8_15(opcode) << " " << hex << __GET_BITS_16_23(opcode) << " " << hex << __GET_BITS_24_31(opcode) << endl;

  // sec_content[currSecIndex].push_back(opcode);
  put_4Byte_section_little_endian(opcode);
  // locationCounter += WORD_SIZE;
}


void Assembler::gpr_csr(uint8_t gpr, uint8_t csr, string op){
          locationCounter += WORD_SIZE;
  uint32_t opcode = _ZERO | __LD;

  opcode = (op == __INST_CSRRD) ? (__SET_BITS_24_27(opcode, __LD_GPR_CSR)) : (__SET_BITS_24_27(opcode, __LD_CSR_GPR));

  // 1001 - mmmm - aaaa - bbbb - cccc - dddd - dddd - dddd

  if(op == __INST_CSRRD) {
    //  gpr[A]<=csr[B]
    __SET_BITS_20_23(opcode, gpr); // aaaa
    __SET_BITS_16_19(opcode, csr); // bbbb
  } else {
    //  csr[A]<=gpr[B];
    __SET_BITS_20_23(opcode, csr); // aaaa
    __SET_BITS_16_19(opcode, gpr); // bbbb
  }

  // outputFile << "CSR | [d][d] : " << hex << __GET_BITS_0_7(opcode) << " [c][d] : " << hex << __GET_BITS_8_15(opcode) << " [a][b] : " << hex << __GET_BITS_16_23(opcode) << " CMD : " << hex << __GET_BITS_24_31(opcode) << endl;
  // outputFile << hex << __GET_BITS_0_7(opcode) << " " << hex << __GET_BITS_8_15(opcode) << " " << hex << __GET_BITS_16_23(opcode) << " " << hex << __GET_BITS_24_31(opcode) << endl;


  // sec_content[currSecIndex].push_back(opcode);
  put_4Byte_section_little_endian(opcode);
  // locationCounter += WORD_SIZE;
}


void Assembler::logic_op(string op, uint8_t gprD, uint8_t gprS) {
          locationCounter += WORD_SIZE;
  // get registers from operands of line
  uint32_t opcode = _ZERO | __LOGIC;

  opcode = (op == __INST_NOT) ? (__SET_BITS_24_27(opcode, __LOGIC_NOT_OP)) : 
          ((op == __INST_AND) ? (__SET_BITS_24_27(opcode, __LOGIC_AND_OP)) : 
          ((op == __INST_OR) ? (__SET_BITS_24_27(opcode, __LOGIC_OR_OP)) : (__SET_BITS_24_27(opcode, __LOGIC_XOR_OP))));

  // 0110 - mmmm - aaaa - bbbb - cccc - 0000 - 0000 - 0000
  //  gpr[A]<=~gpr[B];
  //  gpr[A]<=gpr[B] op gpr[C];

  __SET_BITS_20_23(opcode, gprD); // aaaa
  __SET_BITS_16_19(opcode, gprD); // bbbb
  __SET_BITS_12_15(opcode, gprS); // cccc
  
  // outputFile << "LOG | [0][0] : " << hex << __GET_BITS_0_7(opcode) << " [SRC][0] : " << hex << __GET_BITS_8_15(opcode) << " [DEST][DEST] : " << hex << __GET_BITS_16_23(opcode) << " CMD : " << hex << __GET_BITS_24_31(opcode) << endl;
  // outputFile << hex << __GET_BITS_0_7(opcode) << " " << hex << __GET_BITS_8_15(opcode) << " " << hex << __GET_BITS_16_23(opcode) << " " << hex << __GET_BITS_24_31(opcode) << endl;


  // sec_content[currSecIndex].push_back(opcode);
  put_4Byte_section_little_endian(opcode);
  // locationCounter += WORD_SIZE;
}


void Assembler::arithmetic_op(uint8_t gprS, uint8_t gprD, string op){
          locationCounter += WORD_SIZE;
  uint32_t opcode = _ZERO | __ARITHMETIC;
  opcode = (op == __INST_ADD) ? (__SET_BITS_24_27(opcode, __ARITHMETIC_ADD_OP)) : 
          ((op == __INST_SUB) ? (__SET_BITS_24_27(opcode, __ARITHMETIC_SUB_OP)) : 
          ((op == __INST_MUL) ? (__SET_BITS_24_27(opcode, __ARITHMETIC_MUL_OP)) : (__SET_BITS_24_27(opcode, __ARITHMETIC_DIV_OP))));
  
  // 0101 - mmmm - aaaa - bbbb - cccc - 0000 - 0000 - 0000
  //  gpr[A]<=gpr[B] op gpr[C];

  __SET_BITS_20_23(opcode, gprD); //aaaa
  __SET_BITS_16_19(opcode, gprD); //bbbb
  __SET_BITS_12_15(opcode, gprS); //cccc

  // outputFile << "ART | [0][0] : " << hex << __GET_BITS_0_7(opcode) << " [SRC][0] : " << hex << __GET_BITS_8_15(opcode) << " [DEST][DEST] : " << hex << __GET_BITS_16_23(opcode) << " CMD : " << hex << __GET_BITS_24_31(opcode) << endl;
  // outputFile << hex << __GET_BITS_0_7(opcode) << " " << hex << __GET_BITS_8_15(opcode) << " " << hex << __GET_BITS_16_23(opcode) << " " << hex << __GET_BITS_24_31(opcode) << endl;

  // sec_content[currSecIndex].push_back(opcode);
  put_4Byte_section_little_endian(opcode);
  // locationCounter += WORD_SIZE;
}


void Assembler::pop_reg(uint8_t reg) {
          locationCounter += WORD_SIZE;
  // __LD + MOD + AAAA + BBBB + CCCC + DDDD + DDDD + DDDD
  //  gpr[A]<=mem32[gpr[B]]; gpr[B]<=gpr[B]+D;

  uint32_t opcode = _ZERO | __LD;
  __SET_BITS_24_27(opcode, __LD_GPR_MEM_GPR_GPR_D);

  __SET_BITS_20_23(opcode, reg); // AAAA
  __SET_BITS_16_19(opcode, __SP); // BBBB

  __SET_BITS_0_3(opcode, __WORD_OFFSET_POS); // DDDD

  // outputFile << "POP | DISP : " << hex << __GET_BITS_0_7(opcode) << " [B][DISP] : " << hex << __GET_BITS_8_15(opcode) << " [REG][SP] : " << hex << __GET_BITS_16_23(opcode) << " CMD : " << hex << __GET_BITS_24_31(opcode) << endl;
  // outputFile << hex << __GET_BITS_0_7(opcode) << " " << hex << __GET_BITS_8_15(opcode) << " " << hex << __GET_BITS_16_23(opcode) << " " << hex << __GET_BITS_24_31(opcode) << endl;

  // sec_content[currSecIndex].push_back(opcode);
  put_4Byte_section_little_endian(opcode);
  // locationCounter += WORD_SIZE;
}


void Assembler::pop_status_reg(uint8_t reg) {
          locationCounter += WORD_SIZE;
  // __LD + MOD + AAAA + BBBB + CCCC + DDDD + DDDD + DDDD
  //  gpr[A]<=mem32[gpr[B]]; gpr[B]<=gpr[B]+D;

  uint32_t opcode = _ZERO | __LD;
  __SET_BITS_24_27(opcode, __LD_CSR_MEM_GPR_GPR_D);

  __SET_BITS_20_23(opcode, reg); // AAAA
  __SET_BITS_16_19(opcode, __SP); // BBBB

  __SET_BITS_0_3(opcode, __WORD_OFFSET_POS); // DDDD

  // outputFile << "POP | DISP : " << hex << __GET_BITS_0_7(opcode) << " [C][DISP] : " << hex << __GET_BITS_8_15(opcode) << " [CSR][SP] : " << hex << __GET_BITS_16_23(opcode) << " CMD : " << hex << __GET_BITS_24_31(opcode) << endl;
  // outputFile << hex << __GET_BITS_0_7(opcode) << " " << hex << __GET_BITS_8_15(opcode) << " " << hex << __GET_BITS_16_23(opcode) << " " << hex << __GET_BITS_24_31(opcode) << endl;

  // sec_content[currSecIndex].push_back(opcode);
  put_4Byte_section_little_endian(opcode);
  // locationCounter += WORD_SIZE;
}


void Assembler::push_reg(uint8_t reg){
          locationCounter += WORD_SIZE;

  uint32_t opcode = _ZERO | __ST;
  //  gpr[A]<=gpr[A]+D; mem32[gpr[A]]<=gpr[C] 

  // 1000 + MMMM + AAAA + BBBB + CCCC + DDDD + DDDD + DDDD
  __SET_BITS_24_27(opcode, __ST_MEM_REG);

  __SET_BITS_20_23(opcode, __SP); // sp - AAAA 
  __SET_BITS_12_15(opcode, reg); //CCCC

  __SET_BITS_8_11(opcode, __GET_BITS_8_11(__WORD_OFFSET_NEG)); // DDDD 8 - 11
  __SET_BITS_0_7(opcode, __GET_BITS_0_7(__WORD_OFFSET_NEG)); // DDDD DDDD - 0 - 7

  // outputFile << "PUSH | DISP : " << hex << __GET_BITS_0_7(opcode) << " [REG][DISP] :" << hex << __GET_BITS_8_15(opcode) << " [SP][B] : " << hex << __GET_BITS_16_23(opcode) << " CMD : " << hex << __GET_BITS_24_31(opcode) << endl;
  // outputFile << hex << __GET_BITS_0_7(opcode) << " " << hex << __GET_BITS_8_15(opcode) << " " << hex << __GET_BITS_16_23(opcode) << " " << hex << __GET_BITS_24_31(opcode) << endl;

  // sec_content[currSecIndex].push_back(opcode);
  put_4Byte_section_little_endian(opcode);
  // locationCounter += WORD_SIZE;
}


uint8_t Assembler::get_reg(string op) {
  if(op.substr(1) == "pc")
    return __PC;
  if(op.substr(1) == "sp")
    return __SP;
  if(op.substr(1) == "handler")
    return __REG_HANDLER;
  if(op.substr(1) == "status")
    return __REG_STATUS;
  if(op.substr(1) == "cause")
    return __REG_CAUSE;
  if(op[0] != __REG_INDICATOR) throw CustomException("*E : Operand not correct for translation to reg, missing '%' at the start");
  if(op[1] != 'r') 
    throw CustomException("*E : Operand not correct for translation to reg, missing r as indicator");
  // instruction will be like pop %r[val], so we need to skip % and r, and the operand we recieve is %r[val]
  return (uint8_t)string_to_val(op.substr(2));
}
// **************************************************************************************************************************

// **************************************************************************************************************************
// helper functions
void Assembler::put_byte_section(uint8_t _byte){
  sec_content[currSecIndex].push_back(_byte);
}


void Assembler::put_4Byte_section_little_endian(uint32_t _4byte){
  sec_content[currSecIndex].push_back(__GET_BITS_0_7(_4byte));
  sec_content[currSecIndex].push_back(__GET_BITS_8_15(_4byte));
  sec_content[currSecIndex].push_back(__GET_BITS_16_23(_4byte));
  sec_content[currSecIndex].push_back(__GET_BITS_24_31(_4byte));
}


bool Assembler::try_to_map_dir(string dir) {
  return directive_map.count(dir) > 0;
}


int Assembler::find_sym_tbl_entry(string sym_name){
  for(int i = 0; i < symTable.size(); i++)
    if(symTable[i].sym_name == sym_name)
      return i;
  return -1;
}


bool Assembler::check_sym_in_tbl(string sym_name) {
  for(int i = 0; i < symTable.size(); i++)
    if(symTable[i].sym_name == sym_name)
      return true;
  return false;
}


vector<string> Assembler::divide_line(string line, char divider) {
    vector<string> tokens = vector<string>();
    string token;
    istringstream iss(line);
    while(getline(iss, token, divider)) {
      token = remove_space_back_front(token);
      tokens.push_back(token);
    }
    return tokens;
}


int Assembler::get_index_of_literal_for_section(int lit){
  for(int i = 0; i < sec_literals.at(currSecIndex).size(); i++){
    if(sec_literals.at(currSecIndex)[i] == s_LitSym(lit))
      return i;
  }
  return -1;
}


int Assembler::get_index_of_literal_for_section(string sym){
  for(int i = 0; i < sec_literals.at(currSecIndex).size(); i++){
    if(sec_literals.at(currSecIndex)[i] == s_LitSym(-INT32_MAX, sym))
      return i;
  }
  return -1;
}


vector<string> Assembler::get_mem_ops(string op) {
  vector<string> ops = divide_line(remove_space_back_front(op).substr(op.find_first_of("[") + 1), ']');
  ops[0] = remove_chars_in_string(ops[0], SPACE_CHAR);
  return divide_line(ops[0], '+');
}


bool Assembler::is_number(const string& str) {
    if (str.empty()) {
        return false;
    }

    // Check if the string is a valid decimal or hexadecimal number
    if ((str[0] == '-' && std::all_of(str.begin() + 1, str.end(), ::isdigit)) ||
        (str[0] != '-' && std::all_of(str.begin(), str.end(), ::isdigit)) ||
        (str.substr(0, 2) == "0x" && std::all_of(str.begin() + 2, str.end(), ::isxdigit))) {
        return true;
    }

    return false;
}


int Assembler::string_to_val(string str){
  size_t pos;
  if(is_number(str)) {
    if (str.substr(0, 2) == "0x") {
        unsigned long ulValue = std::stoul(str, &pos, 16);

        // Check if the parsed value exceeds the maximum positive int value
        if (ulValue > static_cast<unsigned long>(std::numeric_limits<int>::max()) + 1) {
            // Convert to negative value if necessary
            return -static_cast<int>(std::numeric_limits<unsigned int>::max() - ulValue + 1);
        } else {
            // Otherwise, convert to positive int
            return static_cast<int>(ulValue);
        }
    } else {
        // Handle decimal values
        return std::stoi(str);
    }
  } 
  return -INT32_MAX;
}


string Assembler::remove_space_back_front(string line){
  if(line.find_first_not_of(SPACE_CHAR) > line.length())
      return "";
  return line.substr(line.find_first_not_of(SPACE_CHAR), line.find_first_of(SPACE_CHAR, line.find_first_not_of(SPACE_CHAR) + 1));
}


string Assembler::remove_chars_in_string(string line, char removalChar){
  line.erase(std::remove_if(line.begin(), line.end(), [removalChar](char c) { return c == removalChar; }), line.end());
  return line;
}


void Assembler::add_literal_to_pool(int lit) {
  if(literals.count(s_LitSym(lit)) <= 0)
    literals.insert(make_pair(s_LitSym(lit), vector<s_LiteralPool>()));
  literals.at(s_LitSym(lit)).push_back(s_LiteralPool(currSecIndex, locationCounter));
  if(sec_literals.count(currSecIndex) <= 0)
    sec_literals.insert(make_pair(currSecIndex, vector<s_LitSym>()));
  if(std::find(sec_literals.at(currSecIndex).begin(), sec_literals.at(currSecIndex).end(), s_LitSym(lit)) == sec_literals.at(currSecIndex).end())
    sec_literals.at(currSecIndex).push_back(s_LitSym(lit));
}


void Assembler::add_symbol_to_pool(string sym){
  // if(!check_sym_in_tbl(sym))
  //   symTable.push_back(s_Sym(sym, LOCAL, NOTYPE, 0, currSecIndex));
  if(literals.count(s_LitSym(-INT32_MAX, sym)) <= 0)
    literals.insert(make_pair(s_LitSym(-INT32_MAX, sym), vector<s_LiteralPool>()));
  literals.at(s_LitSym(-INT32_MAX, sym)).push_back(s_LiteralPool(currSecIndex, locationCounter));
  if(sec_literals.count(currSecIndex) <= 0)
    sec_literals.insert(make_pair(currSecIndex, vector<s_LitSym>()));
  if(std::find(sec_literals.at(currSecIndex).begin(), sec_literals.at(currSecIndex).end(), s_LitSym(-INT32_MAX, sym)) == sec_literals.at(currSecIndex).end())
    sec_literals.at(currSecIndex).push_back(s_LitSym(-INT32_MAX, sym));
}


string Assembler::get_op_type(string op) {
  if(op[0] == '$') {
    int val = string_to_val(op.substr(1));
    if(val != -INT32_MAX)
      return "L";
    else 
      return "S";
  } else if (op[0] == __REG_INDICATOR){
    return "R";
  } else if (op[0] == '[') {
    if(op[1] != __REG_INDICATOR)
      throw CustomException("*E : Memory indexed operator does not have register indicator");
    if(op.find_first_of("+") < op.find_first_of("]")) {
      if(string_to_val(op.substr((op.find_first_of("+") + 1), op.find_first_of("]") - (op.find_first_of("+") + 1))) != -INT32_MAX)
        return "RL";
      else 
        return "RS";
    } else 
      return "MR";
  } else {
    if(string_to_val(op) != -INT32_MAX)
      return "ML";
    else 
      return "MS";
  }
}
// **************************************************************************************************************************


// **************************************************************************************************************************
// print functions
void Assembler::print_header_table() {
  helperFile << "Section Header Table" << endl;
  helperFile << setw(4) << setfill(SPACE_CHAR) << left << "[NR] " <<
    setw(17) << setfill(SPACE_CHAR) << left << "Index " << 
    setw(17) << setfill(SPACE_CHAR) << left << "Name " << 
    setw(17) << setfill(SPACE_CHAR) << left << "Type " << 
    setw(17) << setfill(SPACE_CHAR) << left << "Address " << 
    setw(17) << setfill(SPACE_CHAR) << left << "Offset " << 
    setw(17) << setfill(SPACE_CHAR) << left << "Size " << 
    setw(17) << setfill(SPACE_CHAR) << left << "EntSize " << 
    setw(17) << setfill(SPACE_CHAR) << left << "Flags " <<
    setw(9) << setfill(SPACE_CHAR) << left << "Link " << 
    setw(9) << setfill(SPACE_CHAR) << left << "Info " <<
    setw(17) << setfill(SPACE_CHAR) << left << "Align" << endl;
  for(int i = 0; i < sections.size(); i++) {
    helperFile << setw(4) << setfill(SPACE_CHAR) << left << i << " " <<
      setw(17) << setfill(SPACE_CHAR) << left << sections[i].sh_ndx << 
      setw(17) << setfill(SPACE_CHAR) << left << get_name_sec(sections[i].sh_name) << 
      setw(17) << setfill(SPACE_CHAR) << left << e_n_SecType[sections[i].sh_type] << 
      setw(17) << setfill(SPACE_CHAR) << left << sections[i].sh_addr << 
      setw(17) << setfill(SPACE_CHAR) << left << sections[i].sh_offset  << 
      setw(17) << setfill(SPACE_CHAR) << left << sections[i].sh_size << 
      setw(17) << setfill(SPACE_CHAR) << left << sections[i].sh_entsize << 
      setw(17) << setfill(SPACE_CHAR) << left << sections[i].sh_flags <<
      setw(9) << setfill(SPACE_CHAR) << left << sections[i].sh_link << 
      setw(9) << setfill(SPACE_CHAR) << left << sections[i].sh_info <<
      setw(17) << setfill(SPACE_CHAR) << left << sections[i].sh_addralign << endl;
  }
}


void Assembler::print_symbol_table() {
  helperFile << "Symbol Table" << endl;
  helperFile << setw(5) << setfill(SPACE_CHAR) << left << "Num " <<
    setw(18) << setfill(SPACE_CHAR) << left << "Value " << 
    setw(8) << setfill(SPACE_CHAR) << left << "Size " << 
    setw(10) << setfill(SPACE_CHAR) << left << "Type " <<
    setw(10) << setfill(SPACE_CHAR) << left << "Bind " <<
    setw(15) << setfill(SPACE_CHAR) << left << "Ndx " << 
    setw(15) << setfill(SPACE_CHAR) << left << "Name" << endl;
  for(int i = 0; i < sTab.size(); i++) {
    helperFile << setw(5) << setfill(SPACE_CHAR) << left << i << 
      setw(18) << setfill(SPACE_CHAR) << left << sTab[i].sym_value << 
      setw(8) << setfill(SPACE_CHAR) << left << sTab[i].sym_size << 
      setw(10) << setfill(SPACE_CHAR) << left << e_n_SymType[sTab[i].sym_type] << 
      setw(10) << setfill(SPACE_CHAR) << left << e_n_SymBind[sTab[i].sym_bind] << 
      setw(15) << setfill(SPACE_CHAR) << left << get_name_sec(sections[sTab[i].sym_ndx].sh_name) << 
      setw(15) << setfill(SPACE_CHAR) << left << get_name_sym(sTab[i].sym_name) << endl;
  }
}


void Assembler::print_reloc_table(){
  for(auto& elem : reloc) {
    helperFile << ".rela." << get_name_sec(sections[elem.first].sh_name) << endl;
    helperFile << setw(20) << setfill(SPACE_CHAR) << left << "Offset " << 
      setw(20) << setfill(SPACE_CHAR) << left << "Type " <<
      setw(20) << setfill(SPACE_CHAR) << left << "Sym. Val. " << 
      setw(20) << setfill(SPACE_CHAR) << left << "Addend" << 
      setw(20) << setfill(SPACE_CHAR) << left << "Size " << endl;
    for(int i = 0; i < elem.second.size(); i++) {
    helperFile << setw(20) << setfill(SPACE_CHAR) << left << elem.second[i].r_offset << 
      setw(20) << setfill(SPACE_CHAR) << left << elem.second[i].r_type;
      // setw(20) << setfill(SPACE_CHAR) << left << get_name_sym(sTab[elem.second[i].r_symval].sym_name) << 
    // if(elem.second[i].r_addend == 0)
    //   helperFile << setw(20) << setfill(SPACE_CHAR) << left << get_name_sym(sTab[elem.second[i].r_symval].sym_name);
    // else 
    //   helperFile << setw(20) << setfill(SPACE_CHAR) << left << get_name_sec(sections[elem.first].sh_name);
    helperFile << setw(20) << setfill(SPACE_CHAR) << left << get_name_sym(sTab[elem.second[i].r_symval].sym_name);
    helperFile << setw(20) << setfill(SPACE_CHAR) << left << elem.second[i].r_addend << 
      setw(20) << setfill(SPACE_CHAR) << left << elem.second[i].r_size << endl;
    }
  }
}


void Assembler::write_to_out_file(){
  sections.push_back(s_SHdr(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0));
  secStringTbl.push_back(0x00);
  sTab.push_back(s_SSym(0, 0, 0, 0, 0, 0));
  symStringTbl.push_back(0x00);
  for(int i = 1; i < secHdrTbl.size(); i++) {
    sections.push_back(s_SHdr(i, secStringTbl.size(), secHdrTbl[i].sec_size, SHT_PROGBITS, secHdrTbl[i].sec_addr, calc_offset_for_index(sections, i), 
                      secHdrTbl[i].sec_flags, secHdrTbl[i].sec_link, secHdrTbl[i].sec_info, 0, 0));
    for(int j = 0; j < secHdrTbl[i].sec_name.length(); j++){
      secStringTbl.push_back(secHdrTbl[i].sec_name[j]);
    }
    secStringTbl.push_back(0x00);
    if(sec_literals.count(i) > 0) {
      for(int j = 0; j < sec_literals.at(i).size(); j++) {
        if(sec_literals.at(i)[j].literal != -INT32_MAX) {
          sec_content.at(i).push_back(__GET_BITS_0_7(sec_literals.at(i)[j].literal));
          sec_content.at(i).push_back(__GET_BITS_8_15(sec_literals.at(i)[j].literal));
          sec_content.at(i).push_back(__GET_BITS_16_23(sec_literals.at(i)[j].literal));
          sec_content.at(i).push_back(__GET_BITS_24_31(sec_literals.at(i)[j].literal));
        } else {
          if(!check_sym_in_tbl(sec_literals.at(i)[j].symName))
            symTable.push_back(s_Sym(sec_literals.at(i)[j].symName, GLOBAL));
          sec_content.at(i).push_back(0x00);
          sec_content.at(i).push_back(0x00);
          sec_content.at(i).push_back(0x00);
          sec_content.at(i).push_back(0x00);
          if(reloc.count(i) <= 0)
            reloc.insert(make_pair(i, vector<s_Rela>()));
          if(symTable[find_sym_tbl_entry(sec_literals.at(i)[j].symName)].sym_bind == LOCAL && symTable[find_sym_tbl_entry(sec_literals.at(i)[j].symName)].sym_type != SECTION)
            reloc.at(i).push_back(s_Rela(sections[i].sh_size, find_sym_tbl_entry(get_name_sec(sections[symTable[find_sym_tbl_entry(sec_literals.at(i)[j].symName)].sym_ndx].sh_name)), 0, symTable[find_sym_tbl_entry(sec_literals.at(i)[j].symName)].sym_value, 32));
          else 
            reloc.at(i).push_back(s_Rela(sections[i].sh_size, find_sym_tbl_entry(sec_literals.at(i)[j].symName), 0, 0, 32));
        }
        sections[i].sh_size += WORD_SIZE;
      }
    }
  }
  for(int i = symTable.size() - 1; i > 0; i--)
    if(symTable[i].sym_bind == LOCAL && symTable[i].sym_type != SECTION)
      remove_index_symTbl_update_rela(i);
  for(int i = 1; i < symTable.size(); i++) {
    sTab.push_back(s_SSym(symStringTbl.size(), symTable[i].sym_bind, symTable[i].sym_type, symTable[i].sym_value, symTable[i].sym_ndx, symTable[i].sym_size));
    for(int j = 0; j < symTable[i].sym_name.length(); j++){
      symStringTbl.push_back(symTable[i].sym_name[j]);
    }
    symStringTbl.push_back(0x00);
  }
  // linke to symString which will be next
  sections.push_back(s_SHdr(sections.size(), secStringTbl.size(), (sTab.size() * sizeof(s_SSym)), SHT_SYMTAB, 0, calc_offset_for_index(sections, sections.size()), 
                    0, (sections.size() + 1), 0, 0, sizeof(s_SSym)));
  secStringTbl.push_back('s'); secStringTbl.push_back('y'); secStringTbl.push_back('m'); secStringTbl.push_back('T'); secStringTbl.push_back('b'); secStringTbl.push_back('l'); secStringTbl.push_back(0x00);
  sections.push_back(s_SHdr(sections.size(), secStringTbl.size(), symStringTbl.size(), SHT_STRTAB, 0, calc_offset_for_index(sections, sections.size()),
                    0, (sections.size() - 1), 0, 0, 0));
  secStringTbl.push_back('s'); secStringTbl.push_back('y'); secStringTbl.push_back('m'); secStringTbl.push_back('S'); secStringTbl.push_back('t'); secStringTbl.push_back('r'); secStringTbl.push_back(0x00);
  for(auto& elem : reloc) {
    sections.push_back(s_SHdr(sections.size(), secStringTbl.size(), (elem.second.size() * sizeof(s_Rela)), SHT_RELA, 0, calc_offset_for_index(sections, sections.size()), 0, elem.first, 0, 0, sizeof(s_Rela)));
    secStringTbl.push_back('r'); secStringTbl.push_back('e'); secStringTbl.push_back('l'); secStringTbl.push_back('a'); secStringTbl.push_back('.');
    for(int i = 0; i < secHdrTbl[elem.first].sec_name.length(); i++)
      secStringTbl.push_back(secHdrTbl[elem.first].sec_name[i]);
    secStringTbl.push_back(0x00);
  }     
  int hdrStrBeginNdx = secStringTbl.size();
  int hdrStrNdx = sections.size();
  secStringTbl.push_back('h'); secStringTbl.push_back('d'); secStringTbl.push_back('r'); secStringTbl.push_back('S'); secStringTbl.push_back('t'); secStringTbl.push_back('r'); secStringTbl.push_back(0x00);
  int hdrTblBeginNdx = secStringTbl.size(); 
  secStringTbl.push_back('h'); secStringTbl.push_back('d'); secStringTbl.push_back('r'); secStringTbl.push_back('T'); secStringTbl.push_back('b'); secStringTbl.push_back('l'); secStringTbl.push_back(0x00);
  sections.push_back(s_SHdr(sections.size(), hdrStrBeginNdx, secStringTbl.size(), SHT_STRTAB, 0, calc_offset_for_index(sections, sections.size()),
                    0, (sections.size() + 1), 0, 0, 0));
  int secSize = sections.size();
  int offset_to_sec_hdr_tbl = calc_offset_for_index(sections, sections.size());
  sections.push_back(s_SHdr(sections.size(), hdrTblBeginNdx, (secSize * sizeof(s_SHdr) + sizeof(s_SHdr)), SHT_HDRTAB, 0, offset_to_sec_hdr_tbl,
                    0, (sections.size() - 1), 0, 0, sizeof(s_SHdr)));

  
  // Assigning values to ELF header
  // Magic numbers in e_ident
  hdr.e_ident[EI_MAG0] = 0x7f;
  hdr.e_ident[EI_MAG1] = 'E';
  hdr.e_ident[EI_MAG2] = 'L';
  hdr.e_ident[EI_MAG3] = 'F';
  // 32bit arch
  hdr.e_ident[EI_CLASS] = ELFCLASS32;
  // Endianess
  hdr.e_ident[EI_DATA] = ELFDATA2LSB;
  hdr.e_ident[EI_VERSION] = EV_CURRENT;
  hdr.e_ident[EI_OSABI] = 0;
  hdr.e_ident[EI_ABIVERSION] = 0;
  hdr.e_ident[EI_PAD] = 0;

  hdr.e_type = ET_REL;
  hdr.e_machine = EM_SS_EXAM;
  hdr.e_version = EV_CURRENT;
  hdr.e_entry = 0;
  hdr.e_phoff = 0;
  // Must be redefined after
  hdr.e_shoff = offset_to_sec_hdr_tbl;
  hdr.e_ehsize = sizeof(s_ELFHdr);
  hdr.e_phentsize = 0;
  hdr.e_phnum = 0;
  hdr.e_shentsize = sizeof(s_SHdr);
  // Must be redefined after
  hdr.e_shnum = sections.size();
  // Must be redefined after
  hdr.e_shstrndx = hdrStrNdx;

  outputData();           
}

void Assembler::outputData() {
  print_header_table();
  print_symbol_table();
  print_reloc_table();

  uint8_t zero = 0;
  outputFile << hdr.e_ident[EI_MAG0] << hdr.e_ident[EI_MAG1] << hdr.e_ident[EI_MAG2] << hdr.e_ident[EI_MAG3] << hdr.e_ident[EI_CLASS] <<
            hdr.e_ident[EI_DATA] << hdr.e_ident[EI_VERSION] << hdr.e_ident[EI_OSABI] << hdr.e_ident[EI_ABIVERSION] << hdr.e_ident[EI_PAD];
  for(int i = 1; i < _IDENT_NUM - EI_PAD; i++)
    outputFile.write(reinterpret_cast<const char*>(&zero), sizeof(zero));
  outputFile.write(reinterpret_cast<const char*>(&hdr.e_type), sizeof(hdr.e_type));
  outputFile.write(reinterpret_cast<const char*>(&hdr.e_machine), sizeof(hdr.e_machine));
  outputFile.write(reinterpret_cast<const char*>(&hdr.e_version), sizeof(hdr.e_version));
  outputFile.write(reinterpret_cast<const char*>(&hdr.e_entry), sizeof(hdr.e_entry));
  outputFile.write(reinterpret_cast<const char*>(&hdr.e_phoff), sizeof(hdr.e_phoff));
  outputFile.write(reinterpret_cast<const char*>(&hdr.e_shoff), sizeof(hdr.e_shoff));
  outputFile.write(reinterpret_cast<const char*>(&hdr.e_flags), sizeof(hdr.e_flags));
  outputFile.write(reinterpret_cast<const char*>(&hdr.e_ehsize), sizeof(hdr.e_ehsize));
  outputFile.write(reinterpret_cast<const char*>(&hdr.e_phentsize), sizeof(hdr.e_phentsize));
  outputFile.write(reinterpret_cast<const char*>(&hdr.e_phnum), sizeof(hdr.e_phnum));
  outputFile.write(reinterpret_cast<const char*>(&hdr.e_shentsize), sizeof(hdr.e_shentsize));
  outputFile.write(reinterpret_cast<const char*>(&hdr.e_shnum), sizeof(hdr.e_shnum));
  outputFile.write(reinterpret_cast<const char*>(&hdr.e_shstrndx), sizeof(hdr.e_shstrndx));

  int sz = sizeof(s_ELFHdr);
  for(int i = 0; i < sections.size(); i++){
    if((sections[i].sh_type == SHT_PROGBITS || sections[i].sh_type == SHT_NULL) && sec_content.count(i) > 0 ){
      for(int j = 0; j < sec_content.at(i).size(); j++) {
        outputFile.write(reinterpret_cast<const char*>(&sec_content.at(i)[j]), sizeof(sec_content.at(i)[j]));
        sz += sizeof(sec_content.at(i)[j]);
      }
    } else if(sections[i].sh_type == SHT_SYMTAB) {
      for(int j = 0; j < sTab.size(); j++) {
        outputFile.write(reinterpret_cast<const char*>(&sTab[j].sym_name), sizeof(sTab[j].sym_name));
        sz += sizeof(sTab[j].sym_name);
        outputFile.write(reinterpret_cast<const char*>(&sTab[j].sym_type), sizeof(sTab[j].sym_type));
        sz += sizeof(sTab[j].sym_type);
        outputFile.write(reinterpret_cast<const char*>(&sTab[j].sym_bind), sizeof(sTab[j].sym_bind));
        sz += sizeof(sTab[j].sym_bind);
        outputFile.write(reinterpret_cast<const char*>(&sTab[j].sym_ndx), sizeof(sTab[j].sym_ndx));
        sz += sizeof(sTab[j].sym_ndx);
        outputFile.write(reinterpret_cast<const char*>(&sTab[j].sym_value), sizeof(sTab[j].sym_value));
        sz += sizeof(sTab[j].sym_value);
        outputFile.write(reinterpret_cast<const char*>(&sTab[j].sym_size), sizeof(sTab[j].sym_size));
        sz += sizeof(sTab[j].sym_size);
      }
    } else if(sections[i].sh_type == SHT_STRTAB) {
      if(sections[i-1].sh_type == SHT_SYMTAB) 
        for(int j = 0; j < symStringTbl.size(); j++) {
          outputFile.write(reinterpret_cast<const char*>(&symStringTbl[j]), sizeof(symStringTbl[j]));
          sz += sizeof(symStringTbl[j]);
        }
      else
        for(int j = 0; j < secStringTbl.size(); j++){
          outputFile.write(reinterpret_cast<const char*>(&secStringTbl[j]), sizeof(secStringTbl[j]));
          sz += sizeof(secStringTbl[j]);
        }
    } else if (sections[i].sh_type == SHT_RELA) {
      for(int j = 0; j < reloc.at(sections[i].sh_link).size(); j++) {
        outputFile.write(reinterpret_cast<const char*>(&reloc.at(sections[i].sh_link)[j].r_offset), sizeof(reloc.at(sections[i].sh_link)[j].r_offset));
          sz += sizeof(reloc.at(sections[i].sh_link)[j].r_offset);
        outputFile.write(reinterpret_cast<const char*>(&reloc.at(sections[i].sh_link)[j].r_type), sizeof(reloc.at(sections[i].sh_link)[j].r_type));
          sz += sizeof(reloc.at(sections[i].sh_link)[j].r_type);
        outputFile.write(reinterpret_cast<const char*>(&reloc.at(sections[i].sh_link)[j].r_size), sizeof(reloc.at(sections[i].sh_link)[j].r_size));
          sz += sizeof(reloc.at(sections[i].sh_link)[j].r_size);
        outputFile.write(reinterpret_cast<const char*>(&reloc.at(sections[i].sh_link)[j].r_symval), sizeof(reloc.at(sections[i].sh_link)[j].r_symval));
          sz += sizeof(reloc.at(sections[i].sh_link)[j].r_symval);
        outputFile.write(reinterpret_cast<const char*>(&reloc.at(sections[i].sh_link)[j].r_addend), sizeof(reloc.at(sections[i].sh_link)[j].r_addend));
          sz += sizeof(reloc.at(sections[i].sh_link)[j].r_addend);
      }
    } else if (sections[i].sh_type == SHT_HDRTAB) {
      for(int j = 0; j < sections.size(); j++) {
        outputFile.write(reinterpret_cast<const char*>(&sections[j].sh_ndx), sizeof(sections[j].sh_ndx));
          sz += sizeof(reloc.at(sections[j].sh_ndx));
        outputFile.write(reinterpret_cast<const char*>(&sections[j].sh_name), sizeof(sections[j].sh_name));
          sz += sizeof(reloc.at(sections[j].sh_name));
        outputFile.write(reinterpret_cast<const char*>(&sections[j].sh_type), sizeof(sections[j].sh_type));
          sz += sizeof(reloc.at(sections[j].sh_type));
        outputFile.write(reinterpret_cast<const char*>(&sections[j].sh_flags), sizeof(sections[j].sh_flags));
          sz += sizeof(reloc.at(sections[j].sh_flags));
        outputFile.write(reinterpret_cast<const char*>(&sections[j].sh_addr), sizeof(sections[j].sh_addr));
          sz += sizeof(reloc.at(sections[j].sh_addr));
        outputFile.write(reinterpret_cast<const char*>(&sections[j].sh_offset), sizeof(sections[j].sh_offset));
          sz += sizeof(reloc.at(sections[j].sh_offset));
        outputFile.write(reinterpret_cast<const char*>(&sections[j].sh_size), sizeof(sections[j].sh_size));
          sz += sizeof(reloc.at(sections[j].sh_size));
        outputFile.write(reinterpret_cast<const char*>(&sections[j].sh_link), sizeof(sections[j].sh_link));
          sz += sizeof(reloc.at(sections[j].sh_link));
        outputFile.write(reinterpret_cast<const char*>(&sections[j].sh_info), sizeof(sections[j].sh_info));
          sz += sizeof(reloc.at(sections[j].sh_info));
        outputFile.write(reinterpret_cast<const char*>(&sections[j].sh_addralign), sizeof(sections[j].sh_addralign));
          sz += sizeof(reloc.at(sections[j].sh_addralign));
        outputFile.write(reinterpret_cast<const char*>(&sections[j].sh_entsize), sizeof(sections[j].sh_entsize));
          sz += sizeof(reloc.at(sections[j].sh_entsize));
      }
    }
  }
}


void Assembler::remove_index_symTbl_update_rela(int ndx) {
  for(int i = ndx; i < symTable.size(); i++) {
    for(int j = 0; j < reloc.at(symTable[ndx].sym_ndx).size(); j++) {
      if(reloc.at(symTable[ndx].sym_ndx)[j].r_addend == 0)
        if(reloc.at(symTable[ndx].sym_ndx)[j].r_symval > ndx)
          reloc.at(symTable[ndx].sym_ndx)[j].r_symval--;
    }
  }
  symTable.erase(symTable.begin() + ndx);
}

int Assembler::calc_offset_for_index(vector<s_SHdr> hdr, int ndx){
  int offset = 0;
  for(int i = 0; i < ndx; i++){
    offset += hdr[i].sh_size;
  }
  return (offset + sizeof(s_ELFHdr));
}
// **************************************************************************************************************************

int main(int argc, char const *argv[]){
  string oF;

  try {
    int num_arg = argc;
    char const* arr_arg [argc];
    for(int i = 0; i < argc; i++)
      arr_arg[i] = argv[i];
    Assembler::check_cmd_line(num_arg, arr_arg);
      
    // Todo : For debugging from 2, when it all works, go from 1, delete after
    // string opt = argv[2];
    // string oF = argv[3];
    // string iF = argv[4];
    string opt = argv[1];
    oF = argv[2];
    string iF = argv[3];

    Assembler::check_input_params(opt, oF, iF);

    // cout << "Read input parameters succesfully" << endl;

    Assembler as(oF, iF);

    as.first_pass();
    as.second_pass();

    cout << "Assembling of " << oF << " done" << endl;

  } catch(const std::exception &e){
    // char filename[oF.length()] = oF;
    // remove(filename);
    cerr << e.what() << endl;
  }

  return 0;
}


string Assembler::get_name_sym(int start_ndx) {
  string temp = "";
  for(int i = start_ndx; i < symStringTbl.size(); i++) {
    if(symStringTbl[i] != 0)
      temp.push_back(symStringTbl[i]);
    else 
      return temp;
  }
  return temp;
}


string Assembler::get_name_sec(int start_ndx) {
  string temp = "";
  for(int i = start_ndx; i < secStringTbl.size(); i++) {
    if(secStringTbl[i] != 0)
      temp.push_back(secStringTbl[i]);
    else 
      return temp;
  }
  return temp;
}