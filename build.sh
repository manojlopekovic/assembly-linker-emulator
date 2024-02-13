ASSEMBLER=./as
LINKER=./ld
EMULATOR=./emu

# g++ -g -o as ./assembler/cpp/*.cpp
# g++ -g -o ld ./linker/cpp/*.cpp
# g++ -g -o emu ./emulator/cpp/*.cpp

# g++ -g -o as ./src/assembler.cpp
# g++ -g -o ld ./src/linker.cpp
# g++ -g -o emu ./src/emulator.cpp

${ASSEMBLER} -o main.o main.s
${ASSEMBLER} -o math.o math.s
${ASSEMBLER} -o handler.o handler.s
${ASSEMBLER} -o isr_timer.o isr_timer.s
${ASSEMBLER} -o isr_terminal.o isr_terminal.s
${ASSEMBLER} -o isr_software.o isr_software.s
${LINKER} -hex -place=my_code@0x40000000 -place=math@0xF0000000 -o program.hex main.o math.o handler.o isr_terminal.o isr_timer.o isr_software.o
${EMULATOR} program.hex