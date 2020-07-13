; test4.S: Labels
start:
  LDA #$01
  STY $1234
  JMP start ; Label defined before jump
  JMP loop  ; Label defined after jump
loop:
  LDA #$02
  STY $abcd
