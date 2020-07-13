#!/usr/bin/tcc -run

#include <stdio.h>

char *instructions[] = {
  "ADC",
  "AND",
  "ASL",
  "BCC",
  "BCS",
  "BEQ",
  "BIT",
  "BMI",
  "BNE",
  "BPL",
  "BRK",
  "BVC",
  "BVS",
  "CLC",
  "CLD",
  "CLI",
  "CLV",
  "CMP",
  "CPX",
  "CPY",
  "DEC",
  "DEX",
  "DEY",
  "EOR",
  "INC",
  "INX",
  "INY",
  "JMP",
  "JSR",
  "LDA",
  "LDX",
  "LDY",
  "LSR",
  "NOP",
  "ORA",
  "PHA",
  "PHP",
  "PLA",
  "PLP",
  "ROL",
  "ROR",
  "RTI",
  "RTS",
  "SBC",
  "SEC",
  "SED",
  "SEI",
  "STA",
  "STX",
  "STY",
  "TAX",
  "TAY",
  "TSX",
  "TXA",
  "TXS",
  "TYA"
};

unsigned long djb2hash(unsigned char *str){
  unsigned long hash = 5381;
  int c;

  while((c = *str++)){
    hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
  }

  return hash;
}

int main(){
  int i;
  printf("enum {\n");
  for(i=0;i<(sizeof(instructions)/sizeof(instructions[0]));i++){
    printf("  sixfive_instruction_%s=%i,\n", instructions[i], djb2hash(instructions[i]));
  }
  printf("};\n");
  return 0;
}
