
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

#include "memory.h"

#include <stdio.h>

#define MEMORY_NUM_ENTRIES_SMALL_PT 1024
#define MEMORY_NUM_ENTRIES_COARSE_PT 256
#define MEMORY_NUM_ENTRIES_TRANSLATION_T 4096

/* A small page table entry.
 *
 * Reference: http://infocenter.arm.com/help/topic/com.arm.doc.ddi0198e/DDI0198E_arm926ejs_r0p5_trm.pdf#page=78
 */
struct memory_small_pte_t
{
  /* These bits form the corresponding bits of the physical address. */
  uint32_t base_address: 20;
  
  /* Access permission bits. How to interpret these bits can be found here:
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
  uint8_t c: 1;
  uint8_t b: 1;

  /* These bits indicate the page size and validity and are interpreted as
   * follows:
   * 0b00: Invalid. Generates a page translation fault.
   * 0b01: Large page. Indicates that this is a 64KB page.
   * 0b10: Small page. Indicates that this is a 4KB page.
   * 0b11: Tiny page. Indicates that this is a 1KB page.
   */
  uint8_t descriptor: 2;
};
typedef struct memory_small_pte_t memory_small_pte_t;

/* A small page table.
 *
 * Reference: http://infocenter.arm.com/help/topic/com.arm.doc.ddi0198e/DDI0198E_arm926ejs_r0p5_trm.pdf#page=71
 */
struct memory_small_pt_t {
  memory_small_pte_t entries[MEMORY_NUM_ENTRIES_SMALL_PT]; // 4 KiB / 32 bits
};
typedef struct memory_small_pt_t memory_small_pt_t;


/* A coarse page table entry.
 *
 * Reference: http://infocenter.arm.com/help/topic/com.arm.doc.ddi0198e/DDI0198E_arm926ejs_r0p5_trm.pdf#page=72
 */
struct memory_coarse_pte_t
{
  /* These bits form the base for referencing the second-level descriptor (the
   * coarse page table index for the entry is derived from the MVA).
   */
  uint32_t base_address: 22;
  uint8_t : 1;

  /* These bits specify one of the 16 possible domains, held in the domain
   * access control registers, that contain the primary access controls.
   *
   * The domain determines if:
   * - access permissions are used to qualify the access
   * - the access is unconditionally allowed to proceed
   * - the access is unconditionally aborted.
   * 
   * Reference: http://infocenter.arm.com/help/topic/com.arm.doc.ddi0198e/DDI0198E_arm926ejs_r0p5_trm.pdf#page=47
   */
  uint8_t domain: 4;
  
  /* Must always be 0b1. */
  uint8_t const_1: 1;
  uint8_t : 2;

  /* Must always be 0b01 to indicate a coarse page table descriptor. */
  uint8_t descriptor_type: 2;
};
typedef struct memory_coarse_pte_t memory_coarse_pte_t;

struct memory_coarse_pt_t {
  // See http://infocenter.arm.com/help/topic/com.arm.doc.ddi0198e/DDI0198E_arm926ejs_r0p5_trm.pdf#page=71
  memory_coarse_pte_t entries[MEMORY_NUM_ENTRIES_COARSE_PT];
};
typedef struct memory_coarse_pt_t memory_coarse_pt_t;

/* The root translation table.
 *
 * Reference: http://infocenter.arm.com/help/topic/com.arm.doc.ddi0198e/DDI0198E_arm926ejs_r0p5_trm.pdf#page=72
 */
struct memory_translation_table_entry_t {

  uint32_t base_address: 22;

  uint8_t const_0: 1;

  uint8_t domain: 4;

  uint8_t const_1: 1;

  uint8_t const_00: 2;

  /* Indicates page size and validity.
   *
   * 00 Invalid
   * 01 Coarse Pagetable
   * 10 Section
   * 11 Fine Page Table
   */
  uint8_t validity: 2;

  /*Uint32_t translation_base: 18;

  uint16_t table_index: 12;

  Must always be 0b00. 
  uint8_t descriptors: 2; */
};
typedef struct memory_translation_table_entry_t memory_translation_table_entry_t;

struct memory_translation_table_t {
  memory_translation_table_entry_t entries[MEMORY_NUM_ENTRIES_TRANSLATION_T];
};
typedef struct memory_translation_table_t memory_translation_table_t;

// Validity

#define MEMORY_SMALL_DESCRIPTOR_INVALID 0b00
#define MEMORY_SMALL_DESCRIPTOR_LARGE_PAGE 0b01
#define MEMORY_SMALL_DESCRIPTOR_SMALL_PAGE 0b10
#define MEMORY_SMALL_DESCRIPTOR_TINY_PAGE 0b11

#define MEMORY_TRANSLATION_VALIDITY_INVALID 0b00
#define MEMORY_TRANSLATION_VALIDITY_COARSE_PAGE_TABLE 0b01
#define MEMORY_TRANSLATION_VALIDITY_SECTION 0b10
#define MEMORY_TRANSLATION_VALIDITY_FINE_PAGE_TABLE 0b11

// ## Domains

#define MEMORY_DOMAIN_SYSTEM 0b0000

// Domain control values

/* Any access generates a domain fault. */
#define MEMORY_DOMAIN_CONTROL_NO_ACCESS 0b00

/* Accesses are checked against the access permission bits in the section or
 * page descriptor.
 */
#define MEMORY_DOMAIN_CONTROL_CLIENT 0b01

// 0b10 is reserved.

/* Accesses are not checked against the access permission bits so a permission
 * fault cannot be generated.
 */
#define MEMORY_DOMAIN_CONTROL_MANAGER 0b11

// Clearing of entries.

void
clear_small_pte(memory_small_pte_t* entry)
{
  // TODO: Think about what to do with the ap and c/b bits.
  entry->base_address = 0x0;
  entry->ap3 = 0;
  entry->ap2 = 0;
  entry->ap1 = 0;
  entry->ap0 = 0;
  entry->c = 0;
  entry->b = 0;
  entry->descriptor = MEMORY_SMALL_DESCRIPTOR_INVALID;
}

void
clear_coarse_pte(memory_coarse_pte_t* entry)
{
  entry->base_address = 0x0;
  entry->domain = MEMORY_DOMAIN_CONTROL_NO_ACCESS;
  entry->const_1 = 1;
}

void
clear_translation_table_entry(memory_translation_table_entry_t* entry)
{
  entry->base_address = 0x0;
  entry->const_0 = 0;
  entry->domain = MEMORY_DOMAIN_CONTROL_NO_ACCESS;
  entry->const_1 = 1;
  entry->const_00 = 0;
  entry->validity = MEMORY_TRANSLATION_VALIDITY_INVALID;
}

// Clearing of whole tables.

void
clear_small_table(memory_small_pt_t* small_table)
{
  for (int i = 0; i < MEMORY_NUM_ENTRIES_SMALL_PT; i++) {
    clear_small_table_entry(&small_table->entries[i]);
  }
}

void
clear_coarse_table(memory_coarse_pt_t* coarse_table)
{
  for (int i = 0; i < MEMORY_NUM_ENTRIES_COARSE_PT; i++) {
   clear_coarse_table_entry(&coarse_table->entries[i]);
  }
}

void
clear_translation_table(memory_translation_table_t* translation_table)
{
  for (int i = 0; i < MEMORY_NUM_ENTRIES_TRANSLATION_T; i++) {
    clear_translation_table_entry(&translation_table->entries[i]);
  }
}

// Creation of entries.

memory_small_pte_t create_small_pte() {
  memory_small_pte_t entry;
  clear_small_pte(entry);
  return entry;
}

memory_coarse_pte_t create_coarse_pte() {
  memory_coarse_pte_t entry;
  clear_coarse_pte(entry);
  return entry;
}

memory_translation_entry_t create_translation_entry() {
  memory_translation_entry_t entry;
  clear_translation_entry(entry);
  return entry;
}

// Adding of entries.

bool add_translation_entry_to_translation_table(memory_translation_table_t* translation_table, memory_translation_entry_t entry)
{
  // Find an empty slot (with invalid descriptor bits).
  for (int i = 0; i < MEMORY_NUM_ENTRIES_TRANSLATION_T; i++) {
    memory_translation_entry_t table_entry = translation_table[i];

    if (table_entry.validity == MEMORY_TRANSLATION_VALIDITY_INVALID) {
      // Insert the new table here.
      *table_entry = *entry;
      return true;
    }
  }
  return false;
}

bool add_coarse_pte_to_coarse_pt(memory_coarse_pt_t* coarse_table, memory_coarse_pte_t entry)
{
  // Find an empty slot (with invalid descriptor bits).
  for (int i = 0; i < MEMORY_NUM_ENTRIES_COARSE_PT; i++) {
    memory_coarse_pte_t table_entry = coarse_table[i];

    if (table_entry.descriptor_type == MEMORY_COARSE_VALIDITY_INVALID) {
      // Insert the new table here.
      *table_entry = *entry;
      return true;
    }
  }
  return false;
}

bool add_coarse_pte_to_translation_table(memory_translation_table_t* translation_table, memory_coarse_pte_t entry)
{
  bool isDone;
  
  // Find an empty slot (with invalid descriptor bits).
  for (int i = 0; i < MEMORY_NUM_ENTRIES_TRANSLATION_T && !isDone; i++) {
    memory_translation_table_t table_entry = translation_table[i];

    if (table_entry.descriptor_type == MEMORY_COARSE_VALIDITY_INVALID) {
      // Insert the new table here.
      *table_entry = *entry;
      return true;
    }
  }
  return false;
}

bool add_small_pte_to_translation_table(memory_translation_table_t* translation_table, memory_small_pte_t entry)
{
  // Try adding the entry to an existing small pt.
  for (int i = 0; i < MEMORY_NUM_ENTRIES_TRANSLATION_T; i++) {
    memory_coarse_pte_t table_entry = translation_table[i];
    if (table_entry.validity == MEMORY_TRANSLATION_VALIDITY_INVALID) {
      continue;
    }

    bool result = add_small_pte_to_coarse_pt((memory_coarse_pt_t*)(table_entry.base_address << 10), entry);
    if (result) {
      return true;
    }
  }

  // Otherwise, try to create a new translation table entry.
  memory_translation_table_t *translation_entry;
  if (!add_translation_entry_to_translation_table(translation_entry)) {
    return false;
  }
  
  memory_coarse_pt_t coarse_table;
  translation_entry->base_address = (uint32_t)(&coarse_table >> 10);

  memory_small_pt_t small_table;
  coarse_entry->base_address = (uint32_t)(&small_table >> 10);
  return true;
}



/* Adds a new entry to the given small page table.
 * Returns if the the new entry was successfully inserted.
 */
bool add_small_pte_to_small_pt(memory_small_pt_t* small_table, memory_small_pte_t entry)
{
  // Find an empty slot (with invalid descriptor bits).
  for (int i = 0; i < MEMORY_NUM_ENTRIES_SMALL_PT; i++) {
    memory_small_pte_t table_entry = small_table[i];

    if (table_entry.descriptor == MEMORY_SMALL_VALIDITY_INVALID) {
      // Insert the new table here.
      *table_entry = *entry;
      return true;
    }
  }
  return false;
}

bool add_small_pte_to_coarse_pt(memory_coarse_pt_t* coarse_table, memory_small_pte_t entry)
{
  // Try adding the entry to an existing small pt.
  for (int i = 0; i < MEMORY_NUM_ENTRIES_COARSE_PT; i++) {
    memory_coarse_pte_t table_entry = coarse_table[i];
    if (table_entry.descriptor_type == MEMORY_COARSE_VALIDITY_INVALID) {
      continue;
    }

    bool result = add_small_pte_to_small_pt((memory_small_pt_t*)(table_entry.base_address << 10), entry);
    if (result) {
      return true;
    }
  }

  // Otherwise, try to create a new coarse pte.
  memory_coarse_pte_t *coarse_entry = create_coarse_pte();
  if (!add_coarse_pte_to_coarse_pt(coarse_entry)) {
    return false;
  }
  
  memory_small_pt_t small_table;
  coarse_entry->base_address = (uint32_t)(&small_table >> 10);
  return true;
}


// void add_translation_entry(translation_table_t* translation_table)
// {
//   translation_table_entry_t* translation_entry;

//   memory_coarse_pt_t *coarse_table = (memory_coarse_pt_t *) 0x12345678;

//   translation_entry.base_address = coarse_table;
//   translation_entry.const_0 = 0;
//   translation_entry.domain = 0; //TODO: proper Domain here!
//   translation_entry.const_1 = 1; //0b1
//   translation_entry.const_00 = 0; //0b00
//   translation_entry.validity = 1; //0b01

  
//   // TODO: add entry to table
// }

// /* Creates a new Small-Page-Table on Demand */
// void add_coarse_pte(memory_coarse_pt_t* coarse_table)
// {
//   memory_coarse_pte_t* coarse_entry;

//   memory_small_pt_t *page_table = ()

//   coarse_entry.base_address = page_table.
//   coarse_entry.domain =
//   coarse_entry.const_1 = 1; // 0b1
//   coarse_entry.descriptor = 1; //0b01

//   // TODO: add entry to table
// }


void
memory_init (void)
{
  printf("Initializing memory…");

  // Get the current address of the stack.
  int a = 42;
  int

  // Create a new translation table.
  memory_translation_table_t translation_table;
  clear_translation_table(&translation_table);

  memory_small_pt_t small_table;
  clear_small_table(&small_table);

  memory_small_pte_t small_table_entry;
  small_table_entry.ap3 = 0;
  small_table_entry.ap2 = 0;
  small_table_entry.ap1 = 0;
  small_table_entry.ap0 = 0;
  small_table_entry.base_address = 0x40;
  small_table_entry.c = 0;
  small_table_entry.b = 0;
  small_table_entry.descriptor = MEMORY_SMALL_DESCRIPTOR_SMALL_PAGE;
  small_table.entries[0] = small_table_entry;

  memory_coarse_pt_t coarse_table;
  clear_coarse_table(&coarse_table);
  
  memory_coarse_pte_t coarse_table_entry;
  coarse_table_entry.base_address = ((unsigned int) &small_table) >> 10;
  coarse_table_entry.domain = 0;
  coarse_table_entry.const_1 = 1; // 0b1
  coarse_table_entry.descriptor_type = 1; //0b01
  coarse_table.entries[0] = coarse_table_entry;

  translation_table.entries[0].base_address = (unsigned int) &coarse_table;
  translation_table.entries[0].domain = 0;
  translation_table.entries[0].const_1 = 1; // 0b1
  translation_table.entries[0].validity = MEMORY_TRANSLATION_VALIDITY_COARSE_PAGE_TABLE;

  int *position = (int*) 0x400042;
  *position = 42;

  // Write the address of the translation table to 0xc2. The register c2
  // (Control Register 2) is the Translation Table Base Register (TTBR), for the
  // base address of the first-level translation table.
  // http://infocenter.arm.com/help/topic/com.arm.doc.ddi0198e/DDI0198E_arm926ejs_r0p5_trm.pdf#page=70
  // http://infocenter.arm.com/help/topic/com.arm.doc.ddi0198e/DDI0198E_arm926ejs_r0p5_trm.pdf#page=92
  // int volatile * const c2 = (int *) 0xc2;
  // *c2 = (int) &translation_table;
  asm (
    "MCR p15, 0, %0, c2, c0, 0\n" // read TTBR"
    : : "r" (&translation_table)
  );

  // Configure the Domain Access Control Register (c3).
  //
  // - D15 – D1 (used by programs): initially set to 0b00 for "No access"
  // - D00 (used by the kernel): set to 0b11 for "Manager"
  asm (
    "MCR p15, 0, %0, c3, c0, 0\n"
    : : "r" (MEMORY_DOMAIN_CONTROL_MANAGER)
  );

  // TODO: Initialize Pages for Kernel
  printf("Enabling MMU…");

  // Actually enable the MMU.
  asm (
    "MRC p15, 0, R1, c1, C0, 0\n" // Read control register
    "ORR R1, #0x1\n"              // Sets M bit from read Control Register to 1
    "MCR p15, 0,R1,C1, C0,0\n"    // Write control register and enables MMU
    "NOP\n"
    "NOP\n"
    "NOP\n"
    "NOP\n"
    "NOP\n"
    "NOP\n"
    "NOP\n"
    "NOP\n"
    "NOP\n"
    "NOP\n"
  );

  printf("Enabled MMU");

  int *answer = (int*) 0x000042;
  printf("The translated answer is %d.", answer);

  printf("Memory Setup Done.");
}
