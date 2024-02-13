#include "../inc/emulator.hpp"

// *****************************************************************************************************
// Constructors / destructors

Emulator::Emulator(string inFileName) : inFileName(inFileName) {
  inputFile.open(inFileName, ios::in);
  if(!inputFile.is_open())
    throw CustomException("*EE : Input file not open");
  for(int i = 0; i < 16; i++)
    regs.insert(make_pair(i, 0x00000000));
  for(int i = 0; i < 3; i++) 
    control_regs.insert(make_pair(i, 0x00000000));
  pc = &regs.at(_pc);
  sp = &regs.at(_sp);
}

Emulator::~Emulator(){
  inputFile.close();
}
// *****************************************************************************************************
// *****************************************************************************************************
// Flow handling
void Emulator::pass() {
  *pc = pc_start_addr;
  fill_memory();
  int intrpt = 0;
  do {
    // cout << " PC : " << hex << *pc << " | ";
    decode_pc_instruction();
    if(oc == '0')
      do_halt();
    else if(oc == '1') 
      do_int(intrpt);
    else if (oc == '2')
      do_call();
    else if (oc == '3')
      do_jmp();
    else if (oc == '4')
      do_xchng();
    else if (oc == '5')
      do_aritm();
    else if (oc == '6')
      do_logic();
    else if (oc == '7')
      do_shift();
    else if (oc == '8')
      do_store();
    else if (oc == '9')
      do_load(intrpt);
    else 
      throw CustomException("*EE : Unsupported instruction in emulator");
  } while(oc != '0');
}


uint32_t Emulator::push_reg(int regNo, bool ctrl_regs) {
  *sp -= WORD_SIZE;
  if(memory.count(*sp) <= 0) {
    memory.insert(make_pair(*sp, vector<string>()));
    memory.at(*sp) = {"00", "00", "00", "00"};
  }
  if(!ctrl_regs)
    if(regNo >= _r0 && regNo <= _r15)
      set_memory(*sp, regs[regNo]);
    else 
      throw CustomException("*EE : Register index out of bounds");
  else 
    if(regNo >= _status && regNo <= _cause)
      set_memory(*sp, control_regs[regNo]);
    else 
      throw CustomException("*EE : Status register index out of bounds");
  return *sp;
}

void Emulator::set_reg(int regNo, uint32_t val, bool ctrl_regs) {
  if(!ctrl_regs)
    if(regNo == _r0)
      throw CustomException("*EE : Tried to write to r0");
    else if(regNo >= _r1 && regNo <= _r15)
      regs[regNo] = val;
    else 
      throw CustomException("*EE : Register index out of bounds");
  else 
    if(regNo >= _status && regNo <= _cause)
      control_regs[regNo] = val;
    else 
      throw CustomException("*EE : Status register index out of bounds");
}


uint32_t Emulator::set_reg_from_mem(int regNo, uint32_t addr, bool ctrl_regs) {
  if(!ctrl_regs)
    if(regNo == _r0)
      throw CustomException("*EE : Tried to write to r0");
    else if(regNo >= _r1 && regNo <= _r15) {
      if(memory.count(addr) <= 0) {
        // throw CustomException("*EE : Unknown data on address");
        memory.insert(make_pair(addr, vector<string>()));
        memory[addr] = {"00", "00", "00", "00"};
      }
      regs[regNo] = load_val_from_mem(addr);
      return regs[regNo];
    }
    else 
      throw CustomException("*EE : Register index out of bounds");
  else 
    if(regNo >= _status && regNo <= _cause){
      if(memory.count(addr) <= 0) {
        // throw CustomException("*EE : Unknown data on address");
        memory.insert(make_pair(addr, vector<string>()));
        memory[addr] = {"00", "00", "00", "00"};
      }
      control_regs[regNo] = load_val_from_mem(addr);
      return control_regs[regNo];
    }
    else 
      throw CustomException("*EE : Status register index out of bounds");
}


void Emulator::set_mem(uint32_t addr, uint32_t val) {
  if(memory.count(addr) <= 0) {
    memory.insert(make_pair(addr, vector<string>()));
    memory.at(addr) = {"00", "00", "00", "00"};
  }
  memory.at(addr)[0] = uint32_t_to_string(__GET_BITS_0_7(val));
  memory.at(addr)[1] = uint32_t_to_string(__GET_BITS_8_15(val));
  memory.at(addr)[2] = uint32_t_to_string(__GET_BITS_16_23(val));
  memory.at(addr)[3] = uint32_t_to_string(__GET_BITS_24_31(val));
}


void Emulator::do_halt() {
    // print all registers
    // cout << " | HALT" << endl;
    cout << "------------------------------------------------------------------" << endl;
    cout << "Emulated processor executed halt instruction" << endl;
    cout << "Emulated processor state:" << endl;
    for(int i = 0; i < regs.size(); i++) {
      if(i%4==0 && i > 0)
        cout << endl;
      cout << setw(6) << setfill(' ') << "r" << dec << i << "=0x" << setw(8) << setfill('0') << hex << regs[i];
      cout << "\t";
    }
    cout << endl;
}


void Emulator::do_int(int &intrpt){
  intrpt++;
  // push status;
  push_reg(_status, true);
  // push pc;
  push_reg(_pc);
  // cause = 4;
  set_reg(_cause, 4, true);
  // status = status &(~0x1);
  control_regs.at(_status) &= ~0x1;
  // pc = handle;
  set_reg(_pc, control_regs.at(_handle));
  // cout << " | INT" << endl;
}


void Emulator::do_call(){
  // push pc;
  push_reg(_pc);
  if(mod == '0') {
    // pc = regA + regB + D;
    // cout << " | CALL &" << (regs.at(regA) + regs.at(regB) + disp) << endl;
    set_reg(_pc, (regs.at(regA) + regs.at(regB) + disp));
  } else if (mod == '1') {
    // pc = mem[regA + regB + D];
    // cout << " | CALL mem[" << (regs.at(regA) + regs.at(regB) + disp) << "]" << endl;
    set_reg_from_mem(_pc, regs.at(regA) + regs.at(regB) + disp);
  } else 
    throw CustomException("*EE : Wrong modificator for call");
}


void Emulator::do_jmp(){
  switch (mod){
  case '0': {
    // pc = gprA + D;
    // cout << " | JMP &" << (regs.at(regA) + disp) << endl;
    set_reg(_pc, (regs.at(regA) + disp));
    break;
  }
  case '1': {
    // beq : if(gprB == gprC) pc = gprA + D;
    // cout << " | BEQ &" << (regs.at(regA) + disp) << endl;
    if(regs.at(regB) == regs.at(regC))
      set_reg(_pc, (regs.at(regA) + disp));
    break;
  }
  case '2': {
    // bne : if(gprB != gprC) pc = gprA + D;
    // cout << " | BNE &" << (regs.at(regA) + disp) << endl;
    if(regs.at(regB) != regs.at(regC))
      set_reg(_pc, (regs.at(regA) + disp));
    break;
  }
  case '3': {
    // bgt : if(gprB signed> gprC) pc = gprA + D;
    // cout << " | BGT &" << (regs.at(regA) + disp) << endl;
    if(regs.at(regB) > regs.at(regC))
      set_reg(_pc, (regs.at(regA) + disp));
    break;
  }
  case '8': {
    // pc = mem32[gprA + D];
    // cout << " | JMP [" << (regs.at(regA) + disp) << "]" << endl;
    set_reg_from_mem(_pc, (regs.at(regA) + disp));
    break;
  }
  case '9': {
    // beq : if(gprB == gprC) pc = mem32[gprA + D];
    // cout << " | BEQ [" << (regs.at(regA) + disp) << "]" << endl;
    if(regs.at(regB) == regs.at(regC))
      set_reg_from_mem(_pc, (regs.at(regA) + disp));
    break;
  }
  case 'a': {
    // bne : if(gprB != gprC) pc = mem32[gprA + D];
    // cout << " | BNE [" << (regs.at(regA) + disp) << "]" << endl;
    if(regs.at(regB) != regs.at(regC))
      set_reg_from_mem(_pc, (regs.at(regA) + disp));
    break;
  }
  case 'b': {
    // bgt : if(gprB signed> gprC) pc = mem32[gprA + D];
    // cout << " | BGT [" << (regs.at(regA) + disp) << "]" << endl;
    if(regs.at(regB) > regs.at(regC))
      set_reg_from_mem(_pc, (regs.at(regA) + disp));
    break;
  }
  default:
    throw CustomException("*EE : Wrong modificator for jump");
    break;
  }
}


void Emulator::do_xchng(){
  // temp = gprB; gprB = gprC; gprC = temp;
  // cout << " | XCHNG " << regs.at(regB) << " and " << regs.at(regC) << endl;
  uint32_t temp = regs.at(regB);
  set_reg(regB, regs.at(regC));
  set_reg(regC, temp);
}


void Emulator::do_aritm(){
  switch (mod){
  case '0': {
    // gprA = gprB + gprC;
    // cout << " | reg" << regA << " = " << regs.at(regB) << "+" << regs.at(regC) << endl;
    set_reg(regA, (regs.at(regB) + regs.at(regC)));
    break;
  }
  case '1': {
    // gprA = gprB - gprC;
    // cout << " | reg" << regA << " = " << regs.at(regB) << "-" << regs.at(regC) << endl;
    set_reg(regA, (regs.at(regB) - regs.at(regC)));
    break;
  }
  case '2': {
    // gprA = gprB * gprC;
    // cout << " | reg" << regA << " = " << regs.at(regB) << "*" << regs.at(regC) << endl;
    set_reg(regA, (regs.at(regB) * regs.at(regC)));
    break;
  }
  case '3': {
    // gprA = gprB / gprC;
    // cout << " | reg" << regA << " = " << regs.at(regB) << "/" << regs.at(regC) << endl;
    set_reg(regA, (regs.at(regB) / regs.at(regC)));
    break;
  }
  default:
    throw CustomException("*EE : Wrong modificator for aritmetic");
    break;
  }
}


void Emulator::do_logic(){
  switch (mod){
  case '0': {
    // gprA = ~gprB;
    // cout << " | reg" << regA << " = " << "~" << regs.at(regB) << endl;
    set_reg(regA, ~regs.at(regB));
    break;
  }
  case '1': {
    // gprA = gprB & gprC;
    // cout << " | reg" << regA << " = " << regs.at(regB) << "&" << regs.at(regC) << endl;
    set_reg(regA, (regs.at(regB) & regs.at(regC)));
    break;
  }
  case '2': {
    //  gprA = gprB | gprC;
    // cout << " | reg" << regA << " = " << regs.at(regB) << "|" << regs.at(regC) << endl;
    set_reg(regA, (regs.at(regB) | regs.at(regC)));
    break;
  }
  case '3': {
    //  gprA = gprB ^ gprC;
    // cout << " | reg" << regA << " = " << regs.at(regB) << "^" << regs.at(regC) << endl;
    set_reg(regA, (regs.at(regB) ^ regs.at(regC)));
    break;
  }
  default:
    throw CustomException("*EE : Wrong modificator for logic");
    break;
  }
}


void Emulator::do_shift(){
  switch (mod){
  case '0': {
    // gprA = gprB << gprC
    // cout << " | reg" << regA << " = " << regs.at(regB) << ">>" << regs.at(regC) << endl;
    set_reg(regA, (regs.at(regB) << regs.at(regC)));
    break;
  }
  case '1': {
    // gprA = gprB >> gprC
    // cout << " | reg" << regA << " = " << regs.at(regB) << "<<" << regs.at(regC) << endl;
    set_reg(regA, (regs.at(regB) >> regs.at(regC)));
    break;
  }
  default:
    throw CustomException("*EE : Wrong modificator for logic");
    break;
  }
}


void Emulator::do_store(){
  switch (mod){
  case '0': {
    // mem32[gprA + gprB + D] = gprC;
    // cout << " | st mem[" << (regs.at(regA) + regs.at(regB) + disp) << "] = " << regs.at(regC) << endl;
    set_mem((regs.at(regA) + regs.at(regB) + disp), regs.at(regC));
    break;
  }
  case '2': {
    // mem32[mem32[gprA + gprB + D]] = gprC;
    // cout << " | st mem[" << load_val_from_mem(regs.at(regA) + regs.at(regB) + disp) << "] = " << regs.at(regC) << endl;
    set_mem(load_val_from_mem(regs.at(regA) + regs.at(regB) + disp), regs.at(regC));
    break;
  }
  case '1': {
    // gprA = gprA + D; mem[gprA] = gprC;
    // cout << " | reg" << regA << " = " << (regs.at(regA) + disp) << endl;
    set_reg(regA, (regs.at(regA) + disp));
    // cout << " | st mem[" << regs.at(regA) << "] = " << regs.at(regC) << endl;
    set_mem(regs.at(regA), regs.at(regC));
    break;
  }
  default:
    throw CustomException("*EE : Wrong modificator for store");
    break;
  }

}


void Emulator::do_load(int &intrpt){
  switch (mod){
  case '0': {
    // gprA = csrB;
    // Todo : check if valid values
    // cout << " | reg" << regA << " = " << control_regs.at(regB) << endl;
    regs.at(regA) = control_regs.at(regB);
    break;
  }
  case '1': {
    // gprA = gprB + D;
    // cout << " | reg" << regA << " = " << (regs.at(regB) + disp) << endl;
    set_reg(regA, (regs.at(regB) + disp));
    break;
  }
  case '2': {
    // gprA = mem32[gprB + gprC + D];
    // cout << " | reg" << regA << " = mem[" << (regs.at(regB) + regs.at(regC) + disp) << "]" << endl;
    set_reg_from_mem(regA, (regs.at(regB) + regs.at(regC) + disp));
    break;
  }
  case '3': {
    // gprA = mem32[gprB]; gprB = gprB + D;
    bool iret = memory.at(*pc)[3][0] == '9' && memory.at(*pc)[3][1] == '7';
    // cout << " | reg" << regA << " = mem[" << regs.at(regB) << "]" << endl;
    set_reg_from_mem(regA, regs.at(regB));
    // cout << " | reg" << regA << " = " << (regs.at(regB) + disp) << endl;
    set_reg(regB, (regs.at(regB) + disp));
    if(intrpt > 0 && iret) {
      set_reg_from_mem(_status, regs.at(regB), true);
      set_reg(regB, (regs.at(regB) + disp));
      intrpt--;
    }
    break;
  }
  case '4': { 
    // csrA = gprB
    // Todo : check if valid values
    // cout << " | ctrl reg" << regA << " = " << regs.at(regB) << endl;
    control_regs.at(regA) = regs.at(regB);
    break;
  }
  case '5': {
    // csrA = csrB | D;
    // Todo : check if valid values
    // cout << " | ctrl reg" << regA << " = " << (control_regs.at(regB) | disp) << endl;
    control_regs.at(regA) = control_regs.at(regB) | disp;
    break;
  }
  case '6': {
    // csrA = mem32[gprB + gprC + D];
    // cout << " | ctrl reg" << regA << " = mem[" << (regs.at(regB) + regs.at(regC) + disp) << "]" << endl;
    set_reg(regA, (regs.at(regB) + regs.at(regC) + disp), true);
    break;
  }
  case '7': {
    // csrA = mem[gprB]; gprB = gprB + D;
    // cout << " | ctrl reg" << regA << " = mem[" << regs.at(regB) << "]" << endl;
    set_reg_from_mem(regA, regs.at(regB), true);
    // cout << " | reg" << regA << " = " << (regs.at(regB) + disp) << endl;
    set_reg(regB, (regs.at(regB) + disp));
    break;
  }
  default:
    throw CustomException("*EE : Wrong modificator for logic");
    break;
  }
}
// *****************************************************************************************************

// *****************************************************************************************************
// Passage instructions
void Emulator::decode_pc_instruction(){
  if(memory.count(*pc) <= 0)
    throw CustomException("*EE : There is no instruction at current pc");
  // mem is ass follows : addr : 7_0 15_8 23_16 31_24 : little endian
  // addr : str0[0][1] str1[0][1] str2[0][1] str3[0][1]
  // Instruction is op - mod - rega - regb - regc - disp - disp - disp => all 4b
  oc = memory.at(*pc)[3][0];
  mod = memory.at(*pc)[3][1];
  regA_ch = memory.at(*pc)[2][0];
  regB_ch = memory.at(*pc)[2][1];
  regC_ch = memory.at(*pc)[1][0];
  dh = memory.at(*pc)[1][1];
  dm = memory.at(*pc)[0][0];
  dl = memory.at(*pc)[0][1];

  regA = char_to_val(regA_ch);
  regB = char_to_val(regB_ch);
  regC = char_to_val(regC_ch);

  stringstream ss;
  ss << dh << dm << dl;  
  unsigned int hexValue = std::stoi(ss.str(), nullptr, 16);
  // Check the most significant bit to determine the sign
  bool isNegative = (hexValue & 0x800) != 0;
  // Convert to a signed integer
  disp = isNegative ? static_cast<int>(hexValue | 0xFFFFF000) : static_cast<int>(hexValue);
  
  // cout << oc << mod << regA_ch << regB_ch  << regC_ch << dh << dm << dl;
  // cout << " | regA = " << regA << " regB = " << regB << " regC = " << regC << " disp = " << disp;
  // cout << endl;

  *pc += WORD_SIZE;
}
// *****************************************************************************************************

// *****************************************************************************************************
// Helper instructions
string Emulator::uint32_t_to_string(uint32_t val) {
  stringstream ss;
  uint32_t lowestBits = __GET_BITS_0_7(val);
  ss << hex << setw(2) << setfill('0') << lowestBits;
  return ss.str();
}


void Emulator::set_memory(uint32_t addr, uint32_t val) {
  string str = uint32_t_to_string(__GET_BITS_0_7(val));
  // memory.at(addr)[0] = "" + __GET_BITS_0_7(val);
  memory.at(addr)[0] = uint32_t_to_string(__GET_BITS_0_7(val));
  memory.at(addr)[1] = uint32_t_to_string(__GET_BITS_8_15(val));
  memory.at(addr)[2] = uint32_t_to_string(__GET_BITS_16_23(val));
  memory.at(addr)[3] = uint32_t_to_string(__GET_BITS_24_31(val));
}


int Emulator::char_to_val(char ch){
  stringstream ss;
  ss << "0x" << ch;
  return string_to_val(ss.str());
}


uint32_t Emulator::load_val_from_mem(uint32_t addr) {
  uint32_t temp = 0;
  __SET_BITS_0_7(temp, string_to_val("0x" + memory.at(addr)[0]));
  __SET_BITS_8_15(temp, string_to_val("0x" + memory.at(addr)[1]));
  __SET_BITS_16_23(temp, string_to_val("0x" + memory.at(addr)[2]));
  __SET_BITS_24_31(temp, string_to_val("0x" + memory.at(addr)[3]));
  return temp;
}

void Emulator::fill_memory() {
  string line;
  while(getline(inputFile, line)) {
    string address_str = "0x" + remove_space_back_front(line.substr(0, line.find(":")));
    uint32_t addr = string_to_val(address_str);
    if(memory.count(addr) <= 0) {
      memory.insert(make_pair(addr, vector<string>()));
      memory.insert(make_pair(addr + WORD_SIZE, vector<string>()));
    }
    vector<string> instructions = divide_line(line.substr(line.find_first_not_of(SPACE_CHAR, line.find_first_of(":") + 1)), SPACE_CHAR);
    auto middleIterator = (instructions.size() == 8) ? (instructions.begin() + instructions.size() / 2) :
                          (instructions.size() == 4) ? instructions.begin() + instructions.size() : instructions.begin();
    if(middleIterator == instructions.begin())
      throw CustomException("*EE : Bad instruction size");
    // Insert the first half into the first empty vector
    memory.at(addr).insert(memory.at(addr).end(), instructions.begin(), middleIterator);
    // Insert the second half into the second empty vector
    memory.at(addr + WORD_SIZE).insert(memory.at(addr + WORD_SIZE).end(), middleIterator, instructions.end());
  }
}

vector<string> Emulator::divide_line(string line, char divider) {
    vector<string> tokens = vector<string>();
    string token;
    istringstream iss(line);
    while(getline(iss, token, divider)) {
      token = remove_space_back_front(token);
      tokens.push_back(token);
    }
    return tokens;
}


bool Emulator::is_number(const string& str) {
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


int Emulator::string_to_val(string str){
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


string Emulator::remove_space_back_front(string line){
  if(line.find_first_not_of(SPACE_CHAR) > line.length())
      return "";
  return line.substr(line.find_first_not_of(SPACE_CHAR), line.find_first_of(SPACE_CHAR, line.find_first_not_of(SPACE_CHAR) + 1));
}

// *****************************************************************************************************

int main(int argc, const char *argv[]){

  try {
    // string inFile = argv[2]; // for debug
    string inFile = argv[1]; // for use

    Emulator emu(inFile);

    emu.pass();
    
  } catch(const std::exception &e){
    cerr << e.what() << endl;
  }


  return 0;
}