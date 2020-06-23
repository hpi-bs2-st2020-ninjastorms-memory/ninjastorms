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

#pragma once

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <sys/types.h>


/* An entry for the root translation table.
 *
 * Reference: http://infocenter.arm.com/help/topic/com.arm.doc.ddi0198e/DDI0198E_arm926ejs_r0p5_trm.pdf#page=72
 */
typedef struct mem_lvl1_entry_t {
  uint32_t base_address: 22;
  uint8_t const_0: 1;
  uint8_t domain: 4;
  uint8_t const_1: 1;
  uint8_t const_00: 2;

  /* Indicates page size and validity. See MEM_LVL1_DESCRIPTOR_*. */
  uint8_t descriptor: 2;

  /*Uint32_t translation_base: 18;

  uint16_t table_index: 12;

  Must always be 0b00. 
  uint8_t descriptors: 2; */
} mem_lvl1_entry_t;


#define MEM_LVL1_DESCRIPTOR_INVALID 0b00
#define MEM_LVL1_DESCRIPTOR_COARSE_PAGE_TABLE 0b01
// #define MEM_LVL1_DESCRIPTOR_SECTION 0b10
// #define MEM_LVL1_DESCRIPTOR_FINE_PAGE_TABLE 0b11

#define MEM_NUM_ENTRIES_TRANSLATION_TABLE 4096

typedef struct mem_translation_table_t {
  mem_lvl1_entry_t entries[MEM_NUM_ENTRIES_TRANSLATION_TABLE];
} mem_translation_table_t;


/* An entry for the second level, i.e. a small page.
 *
 * Reference: http://infocenter.arm.com/help/topic/com.arm.doc.ddi0198e/DDI0198E_arm926ejs_r0p5_trm.pdf#page=78
 */
typedef struct mem_lvl2_entry_t
{
  /* These bits form the corresponding bits of the physical address. */
  uint32_t base_address: 20;
  
  /* Access permission bits. How to interpret these bits can be found here:
   * - MEM_DOMAIN_CONTROL_*
   * - Domain access control: http://infocenter.arm.com/help/topic/com.arm.doc.ddi0198e/DDI0198E_arm926ejs_r0p5_trm.pdf#page=87
   * - Fault checking squence: http://infocenter.arm.com/help/topic/com.arm.doc.ddi0198e/DDI0198E_arm926ejs_r0p5_trm.pdf#page=89
   */
  uint8_t ap3: 2;
  uint8_t ap2: 2;
  uint8_t ap1: 2;
  uint8_t ap0: 2;

  /* These bits, C and B, indicate whether the area of memory mapped by this
   * page is treated as write-back cacheable, write-through cacheable, noncached
   * buffered, or noncached nonbuffered.
   * 
   * http://infocenter.arm.com/help/topic/com.arm.doc.ddi0198e/DDI0198E_arm926ejs_r0p5_trm.pdf#page=100
   */
  uint8_t cache_behavior;

  /* Indicates page size and validity. See MEM_LVL2_DESCRIPTOR_*. */
  uint8_t descriptor: 2;
} mem_lvl2_entry_t;


/* DCache disabled. Read from external memory. Write as a nonbuffered store(s)
 * to external memory. DCache is not updated. */
#define MEM_LVL2_CACHE_BEHAVIOR_NONCACHEABLE_NONBUFFERABLE 0b00
/* DCache disabled. Read from external memory. Write as a buffered store(s) to
 * external memory. DCache is not updated. */
#define MEM_LVL2_CACHE_BEHAVIOR_NONCACHEABLE_BUFFERABLE 0b01
/* DCache enabled:
 * Read hit    Read from DCache
 * Read miss   Linefill
 * Write hit   Write to the DCache, and buffered store to external memory
 * Write miss  Buffered store to external memory */
#define MEM_LVL2_CACHE_BEHAVIOR_CACHEABLE_WRITETHROUGH 0b10
/* DCache enabled:
 * Read hit    Read from DCache
 * Read miss   Linefill
 * Write hit   Write to the DCache only
 * Write miss  Buffered store to external memory. */
#define MEM_LVL2_CACHE_BEHAVIOR_CACHEABLE_WRITEBACK 0b11



/* Generates a page translation fault. */
#define MEM_LVL2_DESCRIPTOR_INVALID 0b00
/* 64 KB */
// #define MEM_LVL2_DESCRIPTOR_LARGE_PAGE 0b01
/* 4 KB */
#define MEM_LVL2_DESCRIPTOR_SMALL_PAGE 0b10
/* 1 KB */
// #define MEM_LVL2_DESCRIPTOR_TINY_PAGE 0b11


#define MEM_NUM_ENTRIES_COARSE_TABLE 256

/* A coarse page table.
 *
 * Reference: http://infocenter.arm.com/help/topic/com.arm.doc.ddi0198e/DDI0198E_arm926ejs_r0p5_trm.pdf#page=71
 */
typedef struct mem_coarse_table_t {
  mem_lvl2_entry_t entries[MEM_NUM_ENTRIES_COARSE_TABLE]; // 4 KiB / 32 bits
} mem_coarse_table_t;


// /* A coarse page table entry.
//  *
//  * Reference: http://infocenter.arm.com/help/topic/com.arm.doc.ddi0198e/DDI0198E_arm926ejs_r0p5_trm.pdf#page=72
//  */
// typedef struct mem_coarse_pte_t
// {
//   /* These bits form the base for referencing the second-level descriptor (the
//    * coarse page table index for the entry is derived from the MVA).
//    */
//   uint32_t base_address: 22;
//   uint8_t : 1;

//   /* These bits specify one of the 16 possible domains, held in the domain
//    * access control registers, that contain the primary access controls.
//    *
//    * The domain determines if:
//    * - access permissions are used to qualify the access
//    * - the access is unconditionally allowed to proceed
//    * - the access is unconditionally aborted.
//    * 
//    * Reference: http://infocenter.arm.com/help/topic/com.arm.doc.ddi0198e/DDI0198E_arm926ejs_r0p5_trm.pdf#page=47
//    */
//   uint8_t domain: 4;
  
//   /* Must always be 0b1. */
//   uint8_t const_1: 1;
//   uint8_t : 2;

//   /* Must be 0b01 to indicate a coarse page table descriptor or 0x00 to indicate
//    * a fault. */
//   uint8_t descriptor: 2;
// } mem_coarse_pte_t;

// typedef struct mem_coarse_pt_t {
//   // See http://infocenter.arm.com/help/topic/com.arm.doc.ddi0198e/DDI0198E_arm926ejs_r0p5_trm.pdf#page=71
//   mem_coarse_pte_t entries[MEM_NUM_ENTRIES_COARSE_TABLE];
// } mem_coarse_pt_t;






// ## Descriptors.

// #define MEM_COARSE_DESCRIPTOR_INVALID 0b00
// #define MEM_COARSE_DESCRIPTOR_COARSE_TABLE 0b01

// ## Access Permissions
// http://infocenter.arm.com/help/topic/com.arm.doc.ddi0198e/DDI0198E_arm926ejs_r0p5_trm.pdf#page=84&zoom=100,41,652

#define MEM_AP_ALL_NO_ACCESS 0b00
#define MEM_AP_PRIVILEDGED READ_ONLY_USER_NO_ACCESS 0b10
#define MEM_AP_ALL_READ_ONLY 0b01
#define MEM_AP_ALL_UNPREDICTABLE 0b11

// TODO: Actually, the Access Permission Bits should be used:
// 0 0 No access No access
// 0 0 1 0 Read-only No access
// 0 0 0 1 Read-only Read-only
// 0 0 1 1 Unpredictable Unpredictable

// ## Domains



#define MEM_DOMAIN_SYSTEM 0b0000

// ### Domain control values

/* Any access generates a domain fault. */
#define MEM_DOMAIN_CONTROL_NO_ACCESS 0b00

/* Accesses are checked against the access permission bits in the section or
 * page descriptor.
 */
#define MEM_DOMAIN_CONTROL_CLIENT 0b01

// 0b10 is reserved.

/* Accesses are not checked against the access permission bits so a permission
 * fault cannot be generated.
 */
#define MEM_DOMAIN_CONTROL_MANAGER 0b11

void mem_init (void);
