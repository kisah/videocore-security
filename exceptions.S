.extern timer_isr_c

.section .text
.global exception_ignore
exception_ignore:
    rti

.global timer_isr
timer_isr:
    stm r0-r5, lr, (--sp)
    bl timer_isr_c
    ldm r0-r5, (sp++)
    ld lr, (sp++)
    rti
