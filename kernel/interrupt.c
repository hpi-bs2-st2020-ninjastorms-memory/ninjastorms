
/******************************************************************************
 *       ninjastorms - shuriken operating system                              *
 *                                                                            *
 *    Copyright (C) 2013 - 2016  Andreas Grapentin et al.                     *
 *                                                                            *
 *    This program is free software: you can redistribute it and/or modify    *
 *    it under the terms of the GNU General Public License as published by    *
 *    the Free Software Foundation, either version 3 of the License, or       *
 *    (at your option) any later version.                                     *
 *                                                                            *
 *    This program is distributed in the hope that it will be useful,         *
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *    GNU General Public License for more details.                            *
 *                                                                            *
 *    You should have received a copy of the GNU General Public License       *
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.   *
 ******************************************************************************/

#include "interrupt.h"
#include "syscall_handler.h"

#include "kernel/hal.h"
#include "kernel/interrupt_handler.h"
#include "kernel/memory.h"

#include <stdio.h>

#if BOARD_EV3
#  define IVT_OFFSET (unsigned int) 0xFFFF0000
#endif

#if BOARD_VERSATILEPB
#  define IVT_OFFSET (unsigned int) 0x0
#endif

// builds the interrupt vector table
/* 
	.def _data
	.ref C_data

	.text

	.align 4
	.armfunc _data
	.arm ; Use ARM mode and UAL Syntax

_data .asmfunc
	SUB LR, LR, #4                   ; construct the return address for the following instruction
	;SUB LR, LR, #8                  ; construct the return address for the causing instruction
	SRSDB #0x17!                     ; Save LR_abt and SPSR_abt to Abort Mode stack
	PUSH {R0-R3, R12}                ; Store AAPCS registers

	; 64-bit Stack alignement neccessary as described in: http://infocenter.arm.com/help/topic/com.arm.doc.ihi0042d/IHI0042D_aapcs.pdf "5.2.1.2 Stack constraints at a public interface"
	AND R0, SP, #4                   ; Calculate Stack adjustment to 64bit boundary
    SUB SP, SP, R0                   ; Adjust System Stack
    PUSH {R0, R1}                    ; Put Stack adjustment and System Mode LR on Stack

	SUB  R0, LR, #4                  ; Copy LR to first C argument register and adjust address to causing address
	BL C_data                        ; Call second level C Abort handler

	; Undo stack alignment
    POP {R0, R1}                     ; Get Stack adjustment and System Mode LR from Stack
    ADD SP, SP, R0                   ; Undo System Stack adjustment

	POP {R0-R3, R12}                 ; Restore AAPCS registers
	RFEIA SP!                        ; Return using RFE from Abort mode stack

	.endasmfunc

	.end
   */

void setup_ivt (void) {
  printf("data abort handler: 0x%x\n", &interrupt_handler_data_abort);
  printf("mem data abort handler: 0x%x\n", &mem_interrupt_handler_data_abort);
  // printf("mem_test_data_abort: 0x%x\n", &mem_test_data_abort);

  // https://www.scss.tcd.ie/~waldroj/3d1/arm_arm.pdf#page=54
  // http://www.ti.com/lit/ug/sprugm9b/sprugm9b.pdf?ts=1590218446020#page=26
  *(unsigned int*) (IVT_OFFSET + 0x00) = 0;           //TODO: reset
  *(unsigned int*) (IVT_OFFSET + 0x04) = 0xe59ff014;  //ldr pc, [pc, #20] ; 0x20 undefined instruction
  *(unsigned int*) (IVT_OFFSET + 0x08) = 0xe59ff014;  //ldr pc, [pc, #20] ; 0x24 software interrupt
  // TODO: handle page fault (pre-fetch abort)
  // https://stackoverflow.com/questions/6292620/why-are-the-return-addresses-of-prefetch-abort-and-data-abort-different-in-arm-e
  *(unsigned int*) (IVT_OFFSET + 0x0c) = 0xe59ff014;  //ldr pc, [pc, #20] ; 0x28 prefetch abort
  *(unsigned int*) (IVT_OFFSET + 0x10) = 0xe59ff014;  //ldr pc, [pc, #20] ; 0x2c data abort
  *(unsigned int*) (IVT_OFFSET + 0x14) = 0xe59ff014;  //ldr pc, [pc, #20] ; 0x30 reserved
  *(unsigned int*) (IVT_OFFSET + 0x18) = 0xe59ff014;  //ldr pc, [pc, #20] ; 0x34 IRQ
  *(unsigned int*) (IVT_OFFSET + 0x1c) = 0xe59ff014;  //ldr pc, [pc, #20] ; 0x38 FIQ

  *(unsigned int*) (IVT_OFFSET + 0x20) = (unsigned int) 0;
  //ATTENTION: don't use software interrupts in supervisor mode
  *(unsigned int*) (IVT_OFFSET + 0x24) = (unsigned int) &syscall_handler;
  *(unsigned int*) (IVT_OFFSET + 0x28) = (unsigned int) &interrupt_handler_data_abort;
  *(unsigned int*) (IVT_OFFSET + 0x2c) = (unsigned int) &interrupt_handler_data_abort;
  *(unsigned int*) (IVT_OFFSET + 0x30) = (unsigned int) 0;
  *(unsigned int*) (IVT_OFFSET + 0x34) = (unsigned int) &irq_handler_timer;
  *(unsigned int*) (IVT_OFFSET + 0x38) = (unsigned int) 0;

#if BOARD_EV3
  // set V bit in c1 register in cp15 to
  // locate interrupt vector table to 0xFFFF0000
  asm (
    "mrc  p15, 0, r0, c1, c0, 0\n"
    "orr  r0, #0x2000\n"
    "mcr  p15, 0, r0, c1, c0, 0\n"
  );
#endif
}

void setup_stack(uint8_t mode, uint32_t stack_base_address) {
  asm volatile (
    "mrs  r0, cpsr \n"
    "bic  r0, #0x1f \n"   // Clear mode bits
    "orr  r0, %[mode] \n" // Select correct mode
    "msr  cpsr, r0 \n"    // Enter correct mode
    "mov  sp, %[stack_address] \n"    // set stack pointer
    "bic  r0, #0x1f \n"   // Clear mode bits
    "orr  r0, #0x13 \n"   // Select SVC mode
    "msr  cpsr, r0 \n"    // Enter SVC mode
    : : [mode] "r" (mode),
      [stack_address] "r" (stack_base_address)
  );
}

void setup_irq_stack(void) {
  setup_stack(0x12, IRQ_STACK_ADDRESS);
}

void setup_abt_stack(void) {
  setup_stack(0x17, ABT_STACK_BASE_ADDRESS);
}


void init_timer_interrupt(void) {
    #if BOARD_VERSATILEPB
    *PIC_INTENABLE |= TIMER1_INTBIT;  // unmask interrupt bit for timer1  
    #endif

    #if BOARD_EV3
    *AINTC_ESR1 |= T64P0_TINT34; // enable timer interrupt
    *AINTC_CMR5 |= 2 << (2*8);   // set channel of timer interrupt to 2
    #endif
}

void
init_interrupt_controller (void)
{
#if BOARD_VERSATILEPB
  *PIC_INTENABLE |= 2; // unmask interrupt bit for software interrupt
#endif

#if BOARD_EV3
  *AINTC_SECR1 = 0xFFFFFFFF;   // clear current interrupts
  *AINTC_GER   = GER_ENABLE;   // enable global interrupts
  *AINTC_HIER |= HIER_IRQ;     // enable IRQ interrupt line
  // 0-1 are FIQ channels, 2-31 are IRQ channels, lower channels have higher priority
#endif
 init_timer_interrupt();
}

void
enable_irq (void)
{
  asm(
    "mrs  r1, cpsr\n"
    "bic  r1, r1, #0x80\n"
    "msr  cpsr_c, r1\n"
  );
}

void
init_interrupt_handling (void)
{
  setup_ivt();
  setup_irq_stack();
  setup_abt_stack();
  init_interrupt_controller();
  enable_irq();
}
