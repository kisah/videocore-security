#pragma once

#ifndef __ASSEMBLER__

#include <stdint.h>

#define SECURE_FUNCTION __attribute__((section(".crypto")))

#define SECURE_DATA __attribute__((section(".secure_data")))

#define SECURE_API(ret, name, ...) \
    ret name(__VA_ARGS__); \
    __attribute__((section(".secfns." #name))) void* _ptr_##name = name; \
    __attribute__((section(".crypto"))) ret name(__VA_ARGS__)

typedef uint32_t secure_func_handle_t;

int secure_function_register(void* fn, secure_func_handle_t* handle);
uint32_t secure_function_call(uint32_t sfn_handle, uint32_t r0, uint32_t r1, uint32_t r2, uint32_t r3, uint32_t r4);
void leave_secure_mode(void);

#else

.macro secure_function name
.section .crypto
.global \name
.endm

.macro secure_api name
.section .secfns.\name
._ptr_\name:
    .long \name
.section .crypto
.global \name
.endm

#endif
