#include "security.h"

.extern _secfns_start
.extern _secfns_end

.section .text
.global secure_function_call
secure_function_call:
    st r6, (--sp)
    mov r6, sr
    di
    swi 0
    mov sr, r6
    ld r6, (sp++)
    rts

secure_function leave_secure_mode
leave_secure_mode:
    mov r0, 0
    st lr, (--sp)
    st r0, (--sp)
    rti

secure_function secure_software_interrupt
secure_software_interrupt:
    stm r6, lr, (--sp)

    lea r6, _secfns_start
    shl r0, 2
    add r6, r0
    cmp r6, _secfns_end
    bge 1f
    cmp r6, _secfns_start
    blt 1f

    ld r6, (r6)

    mov r0, r1 
    mov r1, r2 
    mov r2, r3 
    mov r3, r4 
    mov r4, r5

    bl r6

    b 2f
    
1:
    mov r0, -1
    
2:
    ld r6, (sp++)
    ld lr, (sp++)
    rti

secure_api secure_function_lookup
secure_function_lookup:
    lea r1, _secfns_start

    lea r2, _secfns_end
    mov r3, 0

.find_func:
    ld r4, (r1++)
    addcmpbeq r4, 0, r0, .found_func
    add r3, 1
    addcmpbne r1, 0, r2, .find_func

    mov r3, -1

.found_func:
    mov r0, r3
    rts
