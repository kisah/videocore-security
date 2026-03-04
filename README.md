# VideoCore IV VPU Security
## Some context

There appear to be three modes of operation:
- Secure mode (bit 29 of sr is set)
- Supervisor mode (bit 29 of sr is clear)
- User mode (bit 31 of sr is set)

Upon startup the VPU is in secure mode. Official RPi firmware (start*.elf and rpi5 eeprom bootmain) however quickly drops down to supervisor mode after setting things up.

The only way to return to secure mode from supervisor mode (or user mode) is via an exception/software interrupt/hardware interrupt. The interrupt's vector table entry must have its least significant bit set for this to happen. If the least significant bit of the entry is not set, the interrupt/exception handler is always executed in supervisor mode (even if the VPU was previously in secure mode - kicking it out of secure state). 

The official firmware configures software interrupt 0 handler as secure, everything else is non-secure. This software interrupt is used by the firmware as a dispatcher for a set of callable secure functions. Original source code for some of this functionality is public [5].

There is memory security in the hardware, that supports up to 4 secure regions of memory - those are only accessible by secure mode VPU. The regions are configured via SDSECSRTn/SDSECENDn MMIO registers.

Some peripherals are secure-only as well (e.g. VPU's interrupt controller, OTP, some of ARM configuration registers).

## The project

Code in this repo is a WIP playground demonstrating some of VC4's security capabilities by replicating what the official firmware does. The playground configures a secure software interrupt and a secure memory region, drops to supervisor mode. It contains an implementation of callable secure API functions and a few sanity checks for the hw.

Compiles to a `start.elf`. Tested on rpi3, but should work on rpi0-2 as well.

## Credits

1. [original VPU docs by Herman Hermitage](https://github.com/hermanhermitage/videocoreiv/wiki/VideoCore-IV-Programmers-Manual)
2. [VC4 toolchain by itszor](https://github.com/itszor/vc4-toolchain)
3. [rpi-open-firmware by christinaa](https://github.com/christinaa/rpi-open-firmware/)
4. [lk-overlay by clever](https://github.com/librerpi/lk-overlay)
5. [Broadcom's public source code](https://github.com/bieltura/brcm_android_ICS_graphics_stack/blob/master/brcm_usrlib/dag/vmcsx/vcfw/rtos/common/rtos_common_secureasm.s)
6. RPi firmware binaries :D