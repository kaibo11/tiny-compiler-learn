.global fibonacciSeq

fibonacciSeq:
    MOV W2, #0    // a
    MOV W3, #1    // b
    MOV W4, #1    // i    NEXT = W7
loop:
    CMP W4, W1 
    B.GT end

    CMP W4, #1 
    B.EQ 
    B loop
    STR #0, [x0]

    CMP W4, #2
    B.EQ
    B loop
    SUB W5, W4, #1
    LSL W5, W5, #2
    ADD x6, X0, W5
    STR #0, [x6]

    ADD W7, W2, W3
    MOV W2, W3
    WOV W3, W7
    B loop
    
end:
    RET
