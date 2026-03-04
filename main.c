#include "lib/hardware.h"
#include "lib/runtime.h"
#include "lib/xprintf.h"

#include "uart.h"
#include "security.h"

void exception_ignore(void);
void secure_software_interrupt(void);
void timer_isr(void);

typedef void (*irqType)(void);

__attribute__((section(".isr_vectors"))) irqType isr_vectors[128];

extern char _secure_ram_start[];
extern char _secure_ram_end[];

SECURE_DATA int test_secure_var;

SECURE_API(void, secure_test, uint32_t arg) {
    printf("Secure test:\n");
    printf("arg = 0x%x\n", arg);
    uint32_t sr, sp;
    asm volatile("mov %0, sr" : "=r"(sr));
    asm volatile("mov %0, sp" : "=r"(sp));
    printf("sr = 0x%x, sp = 0x%x\n", sr, sp);
}

SECURE_API(void, secure_write, uint32_t val) {
    test_secure_var = val;
}

SECURE_API(uint32_t, secure_read, void) {
    return test_secure_var;
}

SECURE_FUNCTION void set_interrupt(int intno, bool enable, int core) {
  uint32_t base = (core == 0) ? IC0_BASE : IC1_BASE;

  int offset = 0x10 + ((intno >> 3) << 2);
  uint32_t slot = 0xF << ((intno & 7) << 2);

  uint32_t v = *(uint32_t*)(base + offset) & ~slot;
  *(uint32_t*)(base + offset) = enable ? v | slot : v;
}

SECURE_FUNCTION void c_init(void) {
    // Exception vectors
    for(int i = 0; i < 32; i++)
        isr_vectors[i] = exception_ignore;

    // Software interrupt 0 with secure bit set
    isr_vectors[32] = (irqType)((uint32_t)secure_software_interrupt | 1);

    // Hadware interrupt 0 (timer 0)
    isr_vectors[64] = timer_isr;

    // Configure the interrupt controller
    IC0_VADDR = (uint32_t)isr_vectors;
    IC1_VADDR = (uint32_t)isr_vectors;

    set_interrupt(0, true, 0);

    // Enable a secure memory region
    SDSECEND0 = ((uint32_t)_secure_ram_end & 0x3FFFFFFF) - 1;
    SDSECSRT0 = ((uint32_t)_secure_ram_start & 0x3FFFFFFF) | 1;
}

void start_timer(void) {
    ST_C0 = ST_CLO + 1000000;
}

void timer_isr_c(void) {
    uint32_t cs = ST_CS;
    ST_CS = cs;
    printf("Timer go brrr:\n");
    uint32_t sr, sp;
    asm volatile("mov %0, sr" : "=r"(sr));
    asm volatile("mov %0, sp" : "=r"(sp));
    printf("sr = 0x%x, sp = 0x%x\n", sr, sp);
    ST_C0 = ST_CLO + 1000000;
}

void c_start(void) {
    uart_init();

    printf("Hello from VideoCore!!!\n");

    asm volatile("ei");

    printf("Current state:\n");
    uint32_t sr, sp;
    asm volatile("mov %0, sr" : "=r"(sr));
    asm volatile("mov %0, sp" : "=r"(sp));
    printf("sr = 0x%x, sp = 0x%x\n", sr, sp);

    secure_func_handle_t secure_test_handle;
    secure_func_handle_t secure_write_handle;
    secure_func_handle_t secure_read_handle;
    if(secure_function_register(secure_test, &secure_test_handle) < 0 || secure_function_register(secure_write, &secure_write_handle) < 0 || secure_function_register(secure_read, &secure_read_handle) < 0) {
        printf("register failed\n");
        while(1);
    }

    printf("secure_test_handle: %d, secure_write_handle: %d, secure_read_handle: %d\n", secure_test_handle, secure_write_handle, secure_read_handle);

    start_timer();

    secure_function_call(secure_write_handle, 0xDEADBEEF, 0, 0, 0, 0);
    printf("1 test_secure_var from sup = 0x%x\n", test_secure_var);
    printf("1 test_secure_var from sec = 0x%x\n", secure_function_call(secure_read_handle, 0, 0, 0, 0, 0));
    test_secure_var = 0xABCDEF12;
    udelay(1000);
    printf("2 test_secure_var from sec = 0x%x\n", secure_function_call(secure_read_handle, 0, 0, 0, 0, 0));
    printf("2 test_secure_var from sup = 0x%x\n", test_secure_var);

    while(1) {
        secure_function_call(secure_test_handle, 0x1234, 0, 0, 0, 0);

        printf("Secure returned:\n");
        asm volatile("mov %0, sr" : "=r"(sr));
        asm volatile("mov %0, sp" : "=r"(sp));
        printf("sr = 0x%x, sp = 0x%x\n", sr, sp);

        udelay(3000000);
    }
}
