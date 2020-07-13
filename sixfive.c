/*
 * sixfive.c: an assembler for the 6502 microprocessor
 *
 * Usage: sixfive [in.S] [out.bin]
 */

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>

/*****************************/
/* PREPROCESSOR              */
/*****************************/

#define MAX_LINE_LENGTH 256
#define MAX_OUTPUT_LENGTH 256
#define MAX_OPERAND_LENGTH 256
#define MAX_LABELS_COUNT 256

#define LABEL_MAGIC_START 0xfeff
#define ADDRESS_UNKNOWN 0xffff

#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define BLUE    "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN    "\x1b[36m"
#define RESET   "\x1b[0m"

#define LENGTH(x) (sizeof(x)/sizeof(x[0]))

/*****************************/
/* UTILITY FUNCTIONS         */
/*****************************/

/*
 * Dan Bernstein's hash algorithm,
 * http://www.cse.yorku.ca/~oz/hash.html
 */
unsigned long djb2hash(unsigned char *str){
  unsigned long hash = 5381;
  int c;

  while((c = *str++)){
    hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
  }

  return hash;
}

/*
 * For supporting lowercase mnemonics
 */
char *str_uppercase(char *str){
  char *c = str;
  int ind = 0;
  do {
    str[ind++] = toupper(*c);
  } while(*c++);
  return str;
}

/*
 * strdup() is not part of ANSI C
 * https://github.com/OSGeo/PROJ/issues/609
 */
char *pj_strdup(char *str){
  size_t len = strlen(str) + 1;
  char *dup = calloc(sizeof(char), len);
  memcpy(dup, str, len);
  return dup;
}

/*****************************/
/* ENUMS AND TYPEDEFS        */
/*****************************/

/* 
 * 100*strlen(operand)+operand[0]
 *
 * See util/operand_enum.c
 */
enum {
  sixfive_operand_accumulator=165, /* A */
  sixfive_operand_x=188,           /* X */
  sixfive_operand_x2=288,          /* ...,X) */
  sixfive_operand_y=189,           /* Y */
  sixfive_operand_immediate=435,   /* #$ff */
  sixfive_operand_absolute=536,    /* $ffff */
  sixfive_operand_zeropage=336,    /* $ff */
  sixfive_operand_indirect=740,    /* ($ffff) */
  sixfive_operand_indirect_zeropage=540, /* ($ff) */
  sixfive_operand_indirect_zeropage2=440 /* ($ff,X) */
/* sixfive_operand_relative is the same as zeropage */
};

/* 
 * djb2hash(instruction);
 *
 * See util/instruction_enum.c
 */
enum {
  sixfive_instruction_ADC=193450093,
  sixfive_instruction_AND=193450424,
  sixfive_instruction_ASL=193450597,
  sixfive_instruction_BCC=193451149,
  sixfive_instruction_BCS=193451165,
  sixfive_instruction_BEQ=193451229,
  sixfive_instruction_BIT=193451364,
  sixfive_instruction_BMI=193451485,
  sixfive_instruction_BNE=193451514,
  sixfive_instruction_BPL=193451587,
  sixfive_instruction_BRK=193451652,
  sixfive_instruction_BVC=193451776,
  sixfive_instruction_BVS=193451792,
  sixfive_instruction_CLC=193452535,
  sixfive_instruction_CLD=193452536,
  sixfive_instruction_CLI=193452541,
  sixfive_instruction_CLV=193452554,
  sixfive_instruction_CMP=193452581,
  sixfive_instruction_CPX=193452688,
  sixfive_instruction_CPY=193452689,
  sixfive_instruction_DEC=193453393,
  sixfive_instruction_DEX=193453414,
  sixfive_instruction_DEY=193453415,
  sixfive_instruction_EOR=193454827,
  sixfive_instruction_INC=193459135,
  sixfive_instruction_INX=193459156,
  sixfive_instruction_INY=193459157,
  sixfive_instruction_JMP=193460204,
  sixfive_instruction_JSR=193460404,
  sixfive_instruction_LDA=193462070,
  sixfive_instruction_LDX=193462093,
  sixfive_instruction_LDY=193462094,
  sixfive_instruction_LSR=193462582,
  sixfive_instruction_NOP=193464626,
  sixfive_instruction_ORA=193465799,
  sixfive_instruction_PHA=193466558,
  sixfive_instruction_PHP=193466573,
  sixfive_instruction_PLA=193466690,
  sixfive_instruction_PLP=193466705,
  sixfive_instruction_ROL=193468978,
  sixfive_instruction_ROR=193468984,
  sixfive_instruction_RTI=193469140,
  sixfive_instruction_RTS=193469150,
  sixfive_instruction_SBC=193469629,
  sixfive_instruction_SEC=193469728,
  sixfive_instruction_SED=193469729,
  sixfive_instruction_SEI=193469734,
  sixfive_instruction_STA=193470221,
  sixfive_instruction_STX=193470244,
  sixfive_instruction_STY=193470245,
  sixfive_instruction_TAX=193470706,
  sixfive_instruction_TAY=193470707,
  sixfive_instruction_TSX=193471300,
  sixfive_instruction_TXA=193471442,
  sixfive_instruction_TXS=193471460,
  sixfive_instruction_TYA=193471475
};

/* Used to describe the parser's state */
enum {
  sixfive_state_unknown,
  sixfive_state_comment,
  sixfive_state_instruction,
  sixfive_state_operand,
  sixfive_state_label,
  sixfive_state_directive
};

/* Used for clarity in return statements */
enum {
  sixfive_output_error=-1,
  sixfive_output_none=0,
  sixfive_output_success=1
};

/* 
 * Used to store identifying information
 * about a label, used on the parser's
 * second pass
 */
typedef struct sixfive_label {
  char *string;
  uint16_t magic;
  uint16_t address;
} sixfive_label;

/*****************************/
/* LOGGING UTILITIES         */
/*****************************/

void sixfive_print_info(int depth, char *str, ...){
  va_list args;

  switch(depth){
    case 0:
      printf(CYAN "> ");
      break;
    case 1:
      printf(GREEN "=> ");
      break;
    case 2:
      printf(YELLOW "==> ");
      break;
    case 3:
      printf(BLUE "===> ");
      break;
    case 4:
      printf(MAGENTA "====> ");
      break;
  }

  va_start(args, str);
  vfprintf(stdout, str, args);
  printf("\n" RESET);
  va_end(args);
}

void sixfive_print_error(char *str, ...){
  va_list args;

  va_start(args, str);
  fprintf(stderr, RED);
  vfprintf(stderr, str, args);
  fprintf(stderr, "\n" RESET);
  va_end(args);
}

/*****************************/
/* OPERANDS                  */
/*****************************/

/*
 * Returns the type of a given operand,
 * using the hash function
 * 100*strlen(operand)+operand[0], or
 * 0 if its type is not recognized
 */
int sixfive_operand_type(char *op){
  int out;

  if(op == NULL){
    return 0;
  }

  out = 100*strlen(op)+op[0];

  switch(out){
    case sixfive_operand_accumulator: /* Fall through */
    case sixfive_operand_x:
    case sixfive_operand_x2:
    case sixfive_operand_y:
    case sixfive_operand_immediate:
    case sixfive_operand_absolute:
    case sixfive_operand_zeropage:
    case sixfive_operand_indirect:
    case sixfive_operand_indirect_zeropage:
    case sixfive_operand_indirect_zeropage2:
      return out;
  }

  return 0;
}

/*
 * Given a byte in text, returns its
 * corresponding value as a char
 */
char sixfive_operand_to_byte(char *operand, int take_upper){
  char out = 0;
  int offset = 0;

  if(operand == NULL){
    return 0;
  }

  switch(sixfive_operand_type(operand)){
    case sixfive_operand_immediate: /* Fall through */
    case sixfive_operand_indirect_zeropage:
    case sixfive_operand_indirect_zeropage2:
      offset = 2;
      break;
    case sixfive_operand_absolute:
      offset = 1+(2*take_upper);
      break;
    case sixfive_operand_zeropage:
      offset = 1;
      break;
    case sixfive_operand_indirect:
      offset = 2+(2*take_upper);
      break;
  }

  sscanf(operand+offset, "%2hhx", (unsigned char*)&out);

  return out;
}

/*****************************/
/* LABELS                    */
/*****************************/

int label_index;
sixfive_label labels[MAX_LABELS_COUNT];

/*
 * Finds a label in the array of known labels,
 * adding one if it does not already exist
 */
int sixfive_label_find(char *str, uint16_t adr){
  int i;
  
/*  printf("%s %x\n", str, adr); */

  for(i=0;i<label_index;i++){
    if(strcmp(str, labels[i].string) == 0){
      if(adr != ADDRESS_UNKNOWN){
        labels[i].address = adr;
      }
      return i;
    }
  }

  if(sixfive_operand_type(str) == 0){
    labels[label_index].string = pj_strdup(str);
    labels[label_index].magic = LABEL_MAGIC_START+label_index;
    labels[label_index++].address = adr;
    return label_index-1;
  }
  return sixfive_output_error;
}

/*****************************/
/* INSTRUCTIONS              */
/*****************************/

/*
 * Array from 0x00 to 0xff of (instruction hash)+(type of first arg)+(type of second arg)
 *
 * This may seem to be both a bit of a black box as well as an unnecessarily complex solution
 * to the problem of storing opcodes, however it is advantageous in that:
 *   1. This array can be generated (relatively) automatically, see util/opcode_array.c
 *   2. Memory usage remains relatively low, as there exists one array for every opcode
 *        rather than one array for every instruction
 *   3. Lookup is O(n) rather than O(n^2) (i.e. iterating through single array versus
 *        iterating through an array of instructions and iterating through an array
 *        of an instruction's opcodes
 */
int sixfive_instruction_opcodes[] = {
  193451652, 193466527, 0x00,      0x00, 0x00,      193466135, 193450933, 0x00, 193466573, 193466234, 193450762, 0x00, 0x00,      193466335, 193451133, 0x00,
  193451923, 193466528, 0x00,      0x00, 0x00,      193466323, 193451121, 0x00, 193452535, 193466524, 0x00,      0x00, 0x00,      193466523, 193451321, 0x00,
  193460940, 193451152, 0x00,      0x00, 193451700, 193450760, 193469314, 0x00, 193466705, 193450859, 193469143, 0x00, 193451900, 193450960, 193469514, 0x00,
  193451821, 193451153, 0x00,      0x00, 0x00,      193450948, 193469502, 0x00, 193469728, 193451149, 0x00,      0x00, 0x00,      193451148, 193469702, 0x00,
  193469140, 193455555, 0x00,      0x00, 0x00,      193455163, 193462918, 0x00, 193466558, 193455262, 193462747, 0x00, 193460740, 193455363, 193463118, 0x00,
  193452112, 193455556, 0x00,      0x00, 0x00,      193455351, 193463106, 0x00, 193452541, 193455552, 0x00,      0x00, 0x00,      193455551, 193463306, 0x00,
  193469150, 193450821, 0x00,      0x00, 0x00,      193450429, 193469320, 0x00, 193466690, 193450528, 193469149, 0x00, 193460944, 193450629, 193469520, 0x00,
  193452128, 193450822, 0x00,      0x00, 0x00,      193450617, 193469508, 0x00, 193469734, 193450818, 0x00,      0x00, 0x00,      193450817, 193469708, 0x00,
  0x00,      193470949, 0x00,      0x00, 193470581, 193470557, 193470580, 0x00, 193453415, 0x00,      193471442, 0x00, 193470781, 193470757, 193470780, 0x00,
  193451485, 193470950, 0x00,      0x00, 193470769, 193470745, 193470769, 0x00, 193471475, 193470946, 193471460, 0x00, 0x00,      193470945, 0x00,      0x00,
  193462529, 193462798, 193462528, 0x00, 193462430, 193462406, 193462429, 0x00, 193470707, 193462505, 193470706, 0x00, 193462630, 193462606, 193462629, 0x00,
  193451501, 193462799, 0x00,      0x00, 193462618, 193462594, 193462618, 0x00, 193452554, 193462795, 193471300, 0x00, 193462818, 193462794, 193462818, 0x00,
  193453124, 193453309, 0x00,      0x00, 193453025, 193452917, 193453729, 0x00, 193459157, 193453016, 193453414, 0x00, 193453225, 193453117, 193453929, 0x00,
  193451850, 193453310, 0x00,      0x00, 0x00,      193453105, 193453917, 0x00, 193452536, 193453306, 0x00,      0x00, 0x00,      193453305, 193454117, 0x00,
  193453123, 193470357, 0x00,      0x00, 193453024, 193469965, 193459471, 0x00, 193459156, 193470064, 193464626, 0x00, 193453224, 193470165, 193459671, 0x00,
  193451565, 193470358, 0x00,      0x00, 0x00,      193470153, 193459659, 0x00, 193469729, 193470354, 0x00,      0x00, 0x00,      193470353, 193459859, 0x00
};

/*
 * Given the hash of the current instruction
 * and the arguments passed, writes the
 * appropriate bytes to the output file
 *
 * TODO: Clean argument storage/freeing,
 * functional but overcomplicated as is
 */
int sixfive_instruction_eval(int instruc, int argc, char **argv, FILE *fp_out, int num, char *buf){
  unsigned char opcode;
  unsigned char output[3];

  int type_arg1 = sixfive_operand_type(argv[0]);
  int type_arg2 = sixfive_operand_type(argv[1]);

  for(opcode=0;opcode<0xff;opcode++){
    if(sixfive_instruction_opcodes[opcode] == instruc+type_arg1+type_arg2){
      output[0] = opcode;

      switch(type_arg1){
        case sixfive_operand_absolute:
        case sixfive_operand_indirect:
          output[1] = sixfive_operand_to_byte(argv[0], 1);
          output[2] = sixfive_operand_to_byte(argv[0], 0);
          fwrite(output, 1, 3, fp_out);
          break;
        case sixfive_operand_immediate:
        case sixfive_operand_zeropage:
        case sixfive_operand_indirect_zeropage:
        case sixfive_operand_indirect_zeropage2:
          output[1] = sixfive_operand_to_byte(argv[0], 0);
          fwrite(output, 1, 2, fp_out);
          break;
        default:
          fwrite(output, 1, 1, fp_out);
          break;
      }

      if(argc >= 1){
        free(argv[0]);
      }
      if(argc == 2){
        free(argv[1]);
      }
      free(argv);
      return sixfive_output_success;
    }
  }

  if(argc >= 1){
    free(argv[0]);
  }
  if(argc == 2){
    free(argv[1]);
  }
  free(argv);
  return sixfive_output_error;
}

/*
 * Returns the hash of the given instruction
 * mnemonic, used to access the above array
 * and determine an instruction's opcode
 */
#define sixfive_instruction_type(buf) (djb2hash(buf))

/*****************************/
/* PARSER                    */
/*****************************/

/*
 * Parses a single line of input,
 * using spaces, commas, and EOFs
 * to evaluate tokens
 *
 * TODO: Labels, directives, and
 * variables
 */
int sixfive_parse_line(char *line, int num, FILE *fp_out){
  int current_state = sixfive_state_unknown;
  int current_instruction = -1;
  int current_argument = 0;
  int label_ind = 0;
  int buf_ind = 0;
  char *c = line;
  char buf[MAX_LINE_LENGTH];
  char **args = calloc(sizeof(char*)*MAX_OPERAND_LENGTH, 2);
#ifdef DEBUG_BUILD
  sixfive_print_info(1, "Start parsing line.");
#endif
  do {
    switch(*c){
      case ';':
#ifdef DEBUG_BUILD
        sixfive_print_info(2, "Comment");
#endif
        current_state = sixfive_state_comment;
        goto finish_parsing_line;
      case ':':
#ifdef DEBUG_BUILD
        sixfive_print_info(2, "Label: %s", buf);
#endif
        current_state = sixfive_state_label;
        buf[buf_ind] = '\0';
        sixfive_label_find(buf, ftell(fp_out));
        break;
      case '.':
#ifdef DEBUG_BUILD
        sixfive_print_info(2, "Directive");
#endif
        current_state = sixfive_state_directive;
        break;
      case ' ': /* Fall through */
      case ',':
      case '\0':
        if(buf_ind == 0){
          break;
        }
        buf[buf_ind] = '\0';
        buf_ind = 0;
        switch(current_state){
          case sixfive_state_unknown:
          case sixfive_state_instruction:
#ifdef DEBUG_BUILD
              sixfive_print_info(3, "Instruction: %s", buf);
#endif
            current_state = sixfive_state_operand;
            current_instruction = sixfive_instruction_type((unsigned char*)str_uppercase(buf));
            break;
          case sixfive_state_operand:
#ifdef DEBUG_BUILD
            sixfive_print_info(4, "Operand: %s", buf);
#endif
            if((label_ind = sixfive_label_find(buf, ADDRESS_UNKNOWN)) != sixfive_output_error){
              sprintf(buf, "$%.4x", labels[label_ind].magic);
            }
            args[current_argument++] = pj_strdup(buf);
            break;
          case sixfive_state_label:
            current_state = sixfive_state_instruction;
            break;
          case sixfive_state_directive:
            break;
        }
        break;
      default:
        buf[buf_ind++] = *c;
        break;
    }
  } while(*c++);

finish_parsing_line:;
#ifdef DEBUG_BUILD
  sixfive_print_info(1, "End parsing line.");
#endif

  if(current_instruction > -1){
    return sixfive_instruction_eval(current_instruction, current_argument, args, fp_out, num, buf);
  }

  free(args);
  return sixfive_output_none;
}

/*
 * Replaces all instances of a label's
 * "magic" with that label's address
 *
 * TODO: Support magic longer than 16
 * bits
 */
int sixfive_parse_labels(FILE *fp_out, int num){
  int i, ind = 0;
  int len_max = ftell(fp_out);
  uint16_t tmp = 0;

#ifdef DEBUG_BUILD
  sixfive_print_info(1, "Start replacing labels.");
#endif

  fseek(fp_out, 0, SEEK_SET);
  while(ind++ < len_max){
    tmp <<= 8;
    tmp |= fgetc(fp_out);
    for(i=0;i<label_index;i++){
      if(((tmp >> 8) | ((tmp&0xff) << 8)) == labels[i].magic){
        if(labels[i].address == ADDRESS_UNKNOWN){
          sixfive_print_error("Syntax error on line %i: unrecognized operand/label \"%s\".", num, labels[i].string);
          return sixfive_output_error;
        } else {

#ifdef DEBUG_BUILD
          sixfive_print_info(2, "Label \"%s\" becomes \"%.4x\"", labels[i].string, labels[i].address);
#endif

          fseek(fp_out, -2, SEEK_CUR);
          fwrite(&labels[i].address, 1, 2, fp_out);
        }
      }
    }
  }

#ifdef DEBUG_BUILD
  sixfive_print_info(1, "End replacing labels.");
#endif

  return sixfive_output_success;
}

/*
 * Parses an entire string of
 * input, tokenizing by line
 */
int sixfive_parse_string(char *str, FILE *fp_out){
  int num = 1;
  char *line = strtok(str, "\n");
  label_index = 0;

#ifdef DEBUG_BUILD
  sixfive_print_info(0, "Start parsing file.");
#endif

  while(line != NULL){
    if(sixfive_parse_line(line, num++, fp_out) == sixfive_output_error){
      sixfive_print_error("Syntax error on line %i: invalid instruction/operand combination: \"%s\"", num-1, line);
      return sixfive_output_error;
    }
    line = strtok(NULL, "\n");
  }
  
#ifdef DEBUG_BUILD
  sixfive_print_info(0, "End parsing file.");
#endif

  return sixfive_parse_labels(fp_out, num);
}

/*****************************/
/* MAIN                      */
/*****************************/

int main(int argc, char **argv){
  FILE *fp_in, *fp_out;
  int file_len;
  char *file_buf;

  if(argc < 3){
    sixfive_print_info(-1, CYAN "sixfive: a small 6502 assembler.\n" YELLOW "Usage: sixfive [file.S] [out.bin]" RESET);
    return 0;
  }

  fp_in = fopen(argv[1], "r");
  if(fp_in == NULL){
    sixfive_print_error("Error: unable to open file \"%s\" for reading.", argv[1]);
    return 1;
  }
  fseek(fp_in, 0, SEEK_END);
  file_len = ftell(fp_in);
  fseek(fp_in, 0, SEEK_SET);

  file_buf = malloc(file_len);
  fread(file_buf, file_len, 1, fp_in);

  fp_out = fopen(argv[2], "w+b");
  if(fp_out == NULL){
    sixfive_print_error("Error: unable to open file \"%s\" for writing.", argv[2]);
    return 1;
  }

  if(sixfive_parse_string(file_buf, fp_out) == sixfive_output_error){
    remove(argv[2]);
  } else {
    sixfive_print_info(-1, GREEN "Successfully assembled \"%s\" into \"%s\".", argv[1], argv[2]);
  }
  fclose(fp_out);

  free(file_buf);
  fclose(fp_in);

  return 0;
}
