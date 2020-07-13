#!/usr/bin/tcc -run

#include <stdio.h>

int main(){
  printf("enum {\n");
  printf("  sixfive_operand_accumulator=%i,\n", 100+'A');
  printf("  sixfive_operand_x=%i,\n", 100+'X');
  printf("  sixfive_operand_x2=%i,\n", 200+'X');
  printf("  sixfive_operand_y=%i,\n", 100+'Y');
  printf("  sixfive_operand_immediate=%i,\n", 400+'#');
  printf("  sixfive_operand_absolute=%i\n", 500+'$');
  printf("  sixfive_operand_zeropage=%i,\n", 300+'$');
  printf("  sixfive_operand_indirect=%i,\n", 700+'(');
  printf("  sixfive_operand_indirect_zeropage=%i,\n", 500+'(');
  printf("  sixfive_operand_indirect_zeropage2=%i,\n", 400+'(');
  printf("};\n");

  return 0;
}
