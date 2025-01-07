.global calculate_sum

calculate_sum:
    MOV W1, #0    //sum
    MOV W2, #1    // i = 1

loop:
    CMP W2, W0 
    B.GT end

    ADD W1, W1, W2  
    ADD W2, W2, #1  

    B loop        

end:
    MOV X0, W1
    RET
