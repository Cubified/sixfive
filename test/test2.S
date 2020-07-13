; test2.S: All forms of operands
LDA #$01
STA $f0
LDA #$cc
STA $f1
STA $0001
LDA ($ff), Y
JMP ($00f0)
