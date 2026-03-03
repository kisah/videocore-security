/*
 * Test stub (256MB SDRAM at 0xcec00000) link script for Broadcom
 * Videocore 4.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
OUTPUT_FORMAT("elf32-vc4", "elf32-vc4",
              "elf32-vc4")
OUTPUT_ARCH(vc4)
ENTRY(_start)
MEMORY
{
  ram (rwx) : ORIGIN = 0xcec00000, LENGTH = 0x10000000
}
SECTIONS
{
  .text : ALIGN(32)
  {
    *(.text .text.*)
  } >ram =0

  .rodata :
  {
    *(.rodata .rodata.* .gnu.linkonce.r.*)
    *(.rodata1)
  } >ram

  .data : ALIGN(32)
  {
    *(.data .data.* .gnu.linkonce.d.*)
    *(.data1)
    *(.sdata .sdata.* .gnu.linkonce.s.*)
  } >ram

  .bss :
  {
    . = ALIGN(4);
    *(.dynsbss)
    *(.sbss .sbss.* .gnu.linkonce.sb.*)
    *(.scommon)
    *(.dynbss)
    *(.bss .bss.* .gnu.linkonce.b.*)
    *(COMMON)
    . = ALIGN(4);
  } >ram

  .stack : ALIGN(16)
  {
    . = . + 0x4000;
    _stack_top = .;
  } >ram

  .secure_stack : ALIGN(0x2000)
  {
    _secure_ram_start = .;
    . = . + 0x200;
    _secure_stack_top = .;
  } >ram

  .isr_vectors : ALIGN(0x200)
  {
    *(.isr_vectors)
  } >ram

  .secfns : ALIGN(32)
  {
    _secfns_start = .;
    KEEP(*(.secfns.secure_function_lookup))
    KEEP(*(.secfns .secfns.*))
    _secfns_end = .;
    . = ALIGN(4);
  } >ram

  .secure_data : ALIGN(32)
  {
    *(.secure_data)
  } >ram

  .crypto : ALIGN(32)
  {
    *(.crypto)
    . = ALIGN(0x2000);
    _secure_ram_end = .;
  } >ram
}
