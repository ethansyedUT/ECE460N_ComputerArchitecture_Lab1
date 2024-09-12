.ORIG x3000
AND R7, R7, #0
LEA R1, NUM1
LDW R1, R1, #0

LEA R2, NUM2
LDW R2, R2, #0

LEA R4, RESULT
LDW R4, R4, #0

LEA R5, OVERFLOW
LDW R5, R5, #0

LEA R6, EIGHT
LDW R6, R6, #0

;AND R6, R6, #0 ; R6 contains 0
LDB R1, R1, #0 ; R1 contains num1
LDB R2, R2, #0 ; R2 contains num2
ADD R3, R2, R1 ; R3 contains num1 + num2
STB R3, R4, #0 ; x3102 contains num1 + num2

AND R3, R3, R6
BRZ CHECK ;if result is negative check if both ops are neg

AND R1, R1, R6
BRP DONE

AND R2, R2, R6
BRZ NGOOD

BR DONE

CHECK AND R1, R1, R6
BRZ DONE

AND R2, R2, R6
BRZ DONE

NGOOD ADD R7, R7, #1
 

DONE STB R7, R5, #0
TRAP x25

EIGHT   .FILL x80
NUM1    .FILL x3100   ; Address of first number 
NUM2    .FILL x3101   ; Address of second number
RESULT  .FILL x3102   ; Address to store result
OVERFLOW .FILL x3103 ; Address for overflow flag

.END