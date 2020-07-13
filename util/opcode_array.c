#!/usr/bin/tcc -run

#include <stdio.h>

enum {
  acc = 165,
  x = 188,
  x2 = 288,
  y = 189,
  imm = 435,
  ind = 740,
  ind_zp = 540,
  ind_zp2 = 440,
  zp = 336,
  rel = 336,
  abs = 536
};

unsigned long djb(unsigned char *str){
  unsigned long hash = 5381;
  int c;

  while((c = *str++)){
    hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
  }

  return hash;
}

int main(){
  int i, j;
  unsigned long hash;

  /* This was written entirely by hand.
   *
   * Instruction set used:
   * https://www.masswerk.at/6502/6502_instruction_set.html
   */
  printf("int sixfive_instruction_opcodes[] = {\n");
  printf("  %u, %u, 0x00, 0x00, 0x00, %u, %u, 0x00, %u, %u, %u, 0x00, 0x00, %u, %u, 0x00,\n", djb("BRK"), djb("ORA")+ind_zp2+x2, djb("ORA")+zp, djb("ASL")+zp, djb("PHP"), djb("ORA")+imm, djb("ASL")+acc, djb("ORA")+abs, djb("ASL")+abs);
  printf("  %u, %u, 0x00, 0x00, 0x00, %u, %u, 0x00, %u, %u, 0x00, 0x00, 0x00, %u, %u, 0x00,\n", djb("BPL")+rel, djb("ORA")+ind_zp+y, djb("ORA")+zp+x, djb("ASL")+zp+x, djb("CLC"), djb("ORA")+abs+y, djb("ORA")+abs+x, djb("ASL")+abs+x);
  printf("  %u, %u, 0x00, 0x00, %u, %u, %u, 0x00, %u, %u, %u, 0x00, %u, %u, %u, 0x00,\n", djb("JSR")+abs, djb("AND")+ind_zp2+x2, djb("BIT")+zp, djb("AND")+zp, djb("ROL")+zp, djb("PLP"), djb("AND")+imm, djb("ROL")+acc, djb("BIT")+abs, djb("AND")+abs, djb("ROL")+abs);
  printf("  %u, %u, 0x00, 0x00, 0x00, %u, %u, 0x00, %u, %u, 0x00, 0x00, 0x00, %u, %u, 0x00,\n", djb("BMI")+rel, djb("AND")+ind_zp+y, djb("AND")+zp+x, djb("ROL")+zp+x, djb("SEC"), djb("AND")+abs+y, djb("AND")+abs+x, djb("ROL")+abs+x);
  printf("  %u, %u, 0x00, 0x00, 0x00, %u, %u, 0x00, %u, %u, %u, 0x00, %u, %u, %u, 0x00,\n", djb("RTI"), djb("EOR")+ind_zp2+x2, djb("EOR")+zp, djb("LSR")+zp, djb("PHA"), djb("EOR")+imm, djb("LSR")+acc, djb("JMP")+abs, djb("EOR")+abs, djb("LSR")+abs);
  printf("  %u, %u, 0x00, 0x00, 0x00, %u, %u, 0x00, %u, %u, 0x00, 0x00, 0x00, %u, %u, 0x00,\n", djb("BVC")+rel, djb("EOR")+ind_zp+y, djb("EOR")+zp+x, djb("LSR")+zp+x, djb("CLI"), djb("EOR")+abs+y, djb("EOR")+abs+x, djb("LSR")+abs+x);
  printf("  %u, %u, 0x00, 0x00, 0x00, %u, %u, 0x00, %u, %u, %u, 0x00, %u, %u, %u, 0x00,\n", djb("RTS"), djb("ADC")+ind_zp2+x2, djb("ADC")+zp, djb("ROR")+zp, djb("PLA"), djb("ADC")+imm, djb("ROR")+acc, djb("JMP")+ind, djb("ADC")+abs, djb("ROR")+abs);
  printf("  %u, %u, 0x00, 0x00, 0x00, %u, %u, 0x00, %u, %u, 0x00, 0x00, 0x00, %u, %u, 0x00,\n", djb("BVS")+rel, djb("ADC")+ind_zp+y, djb("ADC")+zp+x, djb("ROR")+zp+x, djb("SEI"), djb("ADC")+abs+y, djb("ADC")+abs+x, djb("ROR")+abs+x);
  printf("  0x00, %u, 0x00, 0x00, %u, %u, %u, 0x00, %u, 0x00, %u, 0x00, %u, %u, %u, 0x00,\n", djb("STA")+ind_zp2+x2, djb("STY")+zp, djb("STA")+zp, djb("STX")+zp, djb("DEY"), djb("TXA"), djb("STY")+abs, djb("STA")+abs, djb("STX")+abs);
  printf("  %u, %u, 0x00, 0x00, %u, %u, %u, 0x00, %u, %u, %u, 0x00, 0x00, %u, 0x00, 0x00,\n", djb("BCC")+rel, djb("STA")+ind_zp+y, djb("STY")+zp+x, djb("STA")+zp+x, djb("STX")+zp+y, djb("TYA"), djb("STA")+abs+y, djb("TXS"), djb("STA")+abs+x);
  printf("  %u, %u, %u, 0x00, %u, %u, %u, 0x00, %u, %u, %u, 0x00, %u, %u, %u, 0x00,\n", djb("LDY")+imm, djb("LDA")+ind_zp2+x2, djb("LDX")+imm, djb("LDY")+zp, djb("LDA")+zp, djb("LDX")+zp, djb("TAY"), djb("LDA")+imm, djb("TAX"), djb("LDY")+abs, djb("LDA")+abs, djb("LDX")+abs);
  printf("  %u, %u, 0x00, 0x00, %u, %u, %u, 0x00, %u, %u, %u, 0x00, %u, %u, %u, 0x00,\n", djb("BCS")+rel, djb("LDA")+ind_zp+y, djb("LDY")+zp+x, djb("LDA")+zp+x, djb("LDX")+zp+y, djb("CLV"), djb("LDA")+abs+y, djb("TSX"), djb("LDY")+abs+x, djb("LDA")+abs+x, djb("LDX")+abs+y);
  printf("  %u, %u, 0x00, 0x00, %u, %u, %u, 0x00, %u, %u, %u, 0x00, %u, %u, %u, 0x00,\n", djb("CPY")+imm, djb("CMP")+ind_zp2+x2, djb("CPY")+zp, djb("CMP")+zp, djb("DEC")+zp, djb("INY"), djb("CMP")+imm, djb("DEX"), djb("CPY")+abs, djb("CMP")+abs, djb("DEC")+abs);
  printf("  %u, %u, 0x00, 0x00, 0x00, %u, %u, 0x00, %u, %u, 0x00, 0x00, 0x00, %u, %u, 0x00,\n", djb("BNE")+rel, djb("CMP")+ind_zp+y, djb("CMP")+zp+x, djb("DEC")+zp+x, djb("CLD"), djb("CMP")+abs+y, djb("CMP")+abs+x, djb("DEC")+abs+x);
  printf("  %u, %u, 0x00, 0x00, %u, %u, %u, 0x00, %u, %u, %u, 0x00, %u, %u, %u, 0x00,\n", djb("CPX")+imm, djb("SBC")+ind_zp2+x2, djb("CPX")+zp, djb("SBC")+zp, djb("INC")+zp, djb("INX"), djb("SBC")+imm, djb("NOP"), djb("CPX")+abs, djb("SBC")+abs, djb("INC")+abs);
  printf("  %u, %u, 0x00, 0x00, 0x00, %u, %u, 0x00, %u, %u, 0x00, 0x00, 0x00, %u, %u, 0x00\n", djb("BEQ")+rel, djb("SBC")+ind_zp+y, djb("SBC")+zp+x, djb("INC")+zp+x, djb("SED"), djb("SBC")+abs+y, djb("SBC")+abs+x, djb("INC")+abs+x);
  printf("};\n");

  return 0;
}
