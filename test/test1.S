; test1.S: Syntax error checking
LDA #$01
STA $0200
LDA #$050 ; Syntax error
STA $0201
LDA #$08
STA $0202
