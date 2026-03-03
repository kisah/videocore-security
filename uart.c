#include "lib/hardware.h"
#include "lib/runtime.h"

#define UART_IBRD   (UART_BASE+0x24)
#define UART_FBRD   (UART_BASE+0x28)
#define UART_LCRH   (UART_BASE+0x2C)
#define UART_CR     (UART_BASE+0x30)
#define UART_ICR    (UART_BASE+0x44)

void uart_init(void) {
    unsigned int ra = GP_FSEL1;
    ra &= ~(7 << 12);
    ra |= 4 << 12;
    ra &= ~(7 << 15);
    ra |= 4 << 15;
    GP_FSEL1 = ra;

    mmio_write32(UART_CR, 0);

    GP_PUD = 0;
    udelay(150);
    GP_PUDCLK0 = (1 << 14) | (1 << 15);
    udelay(150);
    GP_PUDCLK0 = 0;

    CM_UARTDIV = CM_PASSWORD | 0x6666;
    CM_UARTCTL = CM_PASSWORD | CM_SRC_OSC | CM_UARTCTL_FRAC_SET | CM_UARTCTL_ENAB_SET;

    mmio_write32(UART_ICR, 0x7FF);
    mmio_write32(UART_IBRD, 1);
    mmio_write32(UART_FBRD, 40);
    mmio_write32(UART_LCRH, 0x70);
    mmio_write32(UART_CR, 0x301);
}

void uart_putc(unsigned int ch) {
    while(UART_MSR & 0x20);
    UART_RBRTHRDLL = ch;
}
