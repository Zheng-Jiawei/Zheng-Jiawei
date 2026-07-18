; Minimal Cortex-M4 startup for the HRC instruction-set simulation target.

Stack_Size      EQU     0x2000
Heap_Size       EQU     0x4000

Stack_Mem       EQU     0x2001E000
__initial_sp    EQU     0x20020000
Heap_Mem        EQU     0x20018000
__heap_base     EQU     Heap_Mem
__heap_limit    EQU     (Heap_Mem + Heap_Size)

                PRESERVE8
                THUMB

                AREA    RESET, DATA, READONLY
                EXPORT  __Vectors
                EXPORT  __Vectors_End
                EXPORT  __Vectors_Size

__Vectors       DCD     __initial_sp
                DCD     Reset_Handler
                DCD     Default_Handler
                DCD     Default_Handler
                DCD     Default_Handler
                DCD     Default_Handler
                DCD     Default_Handler
                DCD     0
                DCD     0
                DCD     0
                DCD     0
                DCD     Default_Handler
                DCD     Default_Handler
                DCD     0
                DCD     Default_Handler
                DCD     Default_Handler
__Vectors_End
__Vectors_Size  EQU     __Vectors_End - __Vectors

                AREA    |.text|, CODE, READONLY

Reset_Handler   PROC
                EXPORT  Reset_Handler [WEAK]
                IMPORT  SystemInit
                IMPORT  __main
                LDR     R0, =SystemInit
                BLX     R0
                LDR     R0, =__main
                BX      R0
                ENDP

Default_Handler PROC
                EXPORT  Default_Handler [WEAK]
                B       .
                ENDP

                ALIGN

                IF      :DEF:__MICROLIB
                EXPORT  __initial_sp
                EXPORT  __heap_base
                EXPORT  __heap_limit
                ELSE
                IMPORT  __use_two_region_memory
                EXPORT  __user_initial_stackheap

__user_initial_stackheap
                LDR     R0, =Heap_Mem
                LDR     R1, =(Stack_Mem + Stack_Size)
                LDR     R2, =(Heap_Mem + Heap_Size)
                LDR     R3, =Stack_Mem
                BX      LR
                ALIGN
                ENDIF

                END
