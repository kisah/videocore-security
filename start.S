.extern _secure_stack_top
.extern _stack_top
.extern c_init
.extern c_start
.extern leave_secure_mode

.section .crypto
.global _start
_start:
    version r0
    btest r0, 0x10
    bne .loop

    lea r0, _secure_stack_top
    mov sp, r0
    lea r0, _stack_top
    mov r28, r0

    bl c_init
    bl leave_secure_mode
    bl c_start

.loop:
    b .loop
