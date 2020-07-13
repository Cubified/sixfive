## sixfive

An assembler for the 6502 microprocessor, written in one weekend for fun and learning.

Not entirely functional, but assembles simple programs into their raw opcodes.

### Building and Running

`sixfive` has no dependencies other than the C standard library:

     $ make

Will produce the `sixfive` binary, which can be run using:

     $ sixfive [in.S] [out.bin]

Additionally:

     $ make debug

Will produce a binary which outputs additional (and colorful) parser information.

To test the assembler, a number of example programs are included in the `test/` folder.

### Syntax

`sixfive` is relatively lenient in its syntax/formatting requirements, meaning the following will assemble:

```asm
; Sample comment
LDA #$01               ; Uppercase mnemonic, no indentation
lda #$01               ; Lowercase mnemonic, no indentation
section1:              ; Section label on own line
  STY $0001            ; Mnemonic with indentation
section2: JMP section1 ; Section label and code on same line
```

### Utility Scripts

In the `util/` folder are three scripts, used to generate the enums and arrays which encode information about the processor's instructions, opcodes, and operands.  These are not necessary to run `sixfive`, but are included to increase clarity for portions of the source which may lack it.

To run the scripts, install [`tcc`](https://bellard.org/tcc) then run them directly.  They are written in C, and compiled at runtime using tcc's unique "C script" functionality.

- `instruction_enum.c`: Generates the enum containing each instruction's hash.  To avoid wasting memory and adding unnecessary code, instruction opcodes are stored in a hashmap.  The hash function used is [Dan Bernstein's simple hash function](http://www.cse.yorku.ca/~oz/hash.html).
- `operand_enum.c`: Generates the enum containing each operand type's hash.  Similarly to instructions, operands are hashed, but for categorization (i.e. determining into what general class or category a string's contents fall) rather than identification (i.e. creating a unique value which describes the exact contents of a string).  Specifically, this function is `100*strlen(operand)+operand[0]`, although this was chosen arbitrarily due to it avoiding collisions for the limited number of possible operand types.
- `opcode_array.c`: Generates the hashmap array which connects a given set of instructions and operands to its respective opcode.  The advantages of such a method are that memory usage is minimal and searches are O(n) (as, to find an opcode, the assembler must only iterate over a single hashmap rather than an array containing every instruction as well as an array containing every instruction's possible opcodes), however the hashmap does appear to be somewhat of a black box within the source code.

### To-Do

- Directive support (e.g. `.org $8000`)
- Variable support (e.g. `var = $0400`)
- More complete immediate operand evaluation (e.g. binary, decimal, octal, etc.)
- Ability to create executable binaries (rather than binaries containing raw opcodes)
- More robust error checking/more informative error messages
- Implement more robust label(/variable) system
- Fix sscanf format code to be strictly ANSI C compliant
