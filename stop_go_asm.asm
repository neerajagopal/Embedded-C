; Stop Go C/ASM Mix Example
; Jason Losh

;-----------------------------------------------------------------------------
; Hardware Target
;-----------------------------------------------------------------------------

; Target Platform: EK-TM4C123GXL Evaluation Board
; Target uC:       TM4C123GH6PM
; System Clock:    40 MHz

; Hardware configuration:
; Red LED:
;   PF1 drives an NPN transistor that powers the red LED
; Green LED:
;   PF3 drives an NPN transistor that powers the green LED
; Pushbutton:
;   SW1 pulls pin PF4 low (internal pull-up is used)

;-----------------------------------------------------------------------------
; Device includes, defines, and assembler directives
;-----------------------------------------------------------------------------

   .def waitPbPress
;-----------------------------------------------------------------------------
; Register values and large immediate values
;-----------------------------------------------------------------------------

.thumb
.text
GPIO_PORTF_DATA_R       .field   0x400253FC

;-----------------------------------------------------------------------------
; Subroutines
;-----------------------------------------------------------------------------

; Blocking function that returns only when SW1 is pressed
waitPbPress:
               LDR    R0, GPIO_PORTF_DATA_R  ; get pointer to port F
               LDR    R0, [R0]               ; read port F
               AND    R0, #0x10              ; mask off all but bit 4
               CBNZ   R0, retry              ; 0 if bit set test (note: only support 0-126 branches)
               BX     R14                    ; return from subroutine
retry:         B      waitPbPress

.endm
