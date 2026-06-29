# VideoCore IV VPU Security
## Security hardware

There appear to be three modes of operation (original Broadcom names):
- Secure mode (bit 29 of sr is set, NOTE: community manual refers to this mode as supervisor mode)
- Supervisor mode (bit 29 of sr is clear, NOTE: community manual refers to this mode as exception mode)
- User mode (bit 31 of sr is set)

Upon startup the VPU is in secure mode.

The only way to return to secure mode from supervisor mode (or user mode) is via an exception/software interrupt/hardware interrupt. The interrupt's vector table entry must have its least significant bit set for this to happen. If the least significant bit of the entry is not set, the interrupt/exception handler is always executed in supervisor mode (even if the VPU was previously in secure mode - kicking it out of secure state). 

There is memory security in the hardware, that supports up to 4 secure regions of memory - those are only accessible by secure mode VPU. The regions are configured via SDSECSRTn/SDSECENDn MMIO registers.

Some peripherals are secure-only as well (e.g. VPU's interrupt controller, OTP, some of ARM configuration registers).

Apparently only the VPU can be a secure master on the bus, no other SoC component can touch secure memory.

## Secure mode in official firmware

Note: Everything in this section applies to `start*.elf` and RPi5 eeprom `bootmain`.

RPi firmware starts in secure mode but quickly drops down to supervisor mode after setting things up. It configures software interrupt 0 handler as secure, every other interrupt/exception handler is non-secure. This software interrupt is used by the firmware as a dispatcher for a set of callable secure functions. Original source code for some of this functionality is public [5].

Most secure mode code is located in the `.crypto` section of the ELF file. An array of pointers to runtime callable secure functions is placed in a dedicated `.secfns` section, which is a good starting point for RE.

Internally the firmware provides two APIs for interacting with secure functions [5], [6]:
```c
//Function to register secure functions
//NOTE - these functions should be encrypted
extern int32_t rtos_secure_function_register( RTOS_SECURE_FUNC_T secure_func,
                                              RTOS_SECURE_FUNC_HANDLE_T *secure_func_handle );

//Routine to call a secure function
extern int32_t rtos_secure_function_call( const RTOS_SECURE_FUNC_HANDLE_T secure_func_handle,
                                          const uint32_t r0,
                                          const uint32_t r1,
                                          const uint32_t r2,
                                          const uint32_t r3,
                                          const uint32_t r4 );
```

Usage pattern for these is as follows:
```c
extern int do_action_secure(int arg); // secure function reference

RTOS_SECURE_FUNC_HANDLE_T do_action_secure_handle;

void init_routine()
{
    // ...
    rtos_secure_function_register(do_action_secure, &do_action_secure_handle);
    // ...
}

int do_action(int arg)
{
    return rtos_secure_function_call(do_action_secure_handle, arg, 0, 0, 0, 0);
}
```

Easiest way to find `rtos_secure_function_call` in the ELF is cross-referencing any function from `.secfns` list except the first one (it is the internal "registration" function and not referenced directly). The first function call after a `lea` reference should be `rtos_secure_function_register`, which in turn calls `rtos_secure_function_call` with 0 as the first arg. `rtos_secure_function_call` contains the `swi 0` software interrupt instruction which switches to secure mode.

SDSECSRT0/SDSECEND0 regs are configured early in its boot process to enable memory protection. `start*.elf`s however DISABLE it later during boot, whereas RPi5 `bootmain` KEEPS IT ENABLED.

Secure functions are used for ARM bringup, OTP access, VPU interrupt controller and SDRAM configuration.

## The project

Code in this repo is a WIP playground demonstrating some of VC4's security capabilities by replicating what the official firmware does. The playground configures a secure software interrupt and a secure memory region, drops to supervisor mode. It contains an implementation of callable secure API functions and a few sanity checks for the hw.

Compiles to a `start.elf`. Tested on RPi3, but should work on RPi0-2 as well.

## Credits/references

1. [original VPU docs by Herman Hermitage](https://github.com/hermanhermitage/videocoreiv/wiki/VideoCore-IV-Programmers-Manual)
2. [VC4 toolchain by itszor](https://github.com/itszor/vc4-toolchain)
3. [rpi-open-firmware by christinaa](https://github.com/christinaa/rpi-open-firmware/)
4. [lk-overlay by clever](https://github.com/librerpi/lk-overlay)
5. [Broadcom's public source code](https://github.com/bieltura/brcm_android_ICS_graphics_stack/blob/master/brcm_usrlib/dag/vmcsx/vcfw/rtos/common/rtos_common_secureasm.s)
6. [rtos.h](https://github.com/raspberrypi/userland/blob/master/vcfw/rtos/rtos.h)
7. RPi firmware binaries :D