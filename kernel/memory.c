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


#define ENTRY_GET(entry, offset, length) \
    (BITFIELD_GET(UINT32_ENDIANNESS_SWAP(entry), offset, length))
#define ENTRY_SET(entry, value, offset, length) \
    ((entry) = UINT32_ENDIANNESS_SWAP( \
      BITFIELD_MERGE(UINT32_ENDIANNESS_SWAP(entry), value, offset, length)))

// # Level 1 entry
//
// Memory layout:
// | 31        10 | 9 | 8    5 | 4 | 3 2 | 1      0 |
// | Base address | 0 | Domain | 1 |  00 | Validity |

#define LVL1_ENTRY_DEFAULT 0b00000000000000000000000000010000
#define LVL1_ENTRY_CLEAR(entry) (entry = UINT32_ENDIANNESS_SWAP(LVL1_ENTRY_DEFAULT))

#define LVL1_ENTRY_GET_BASE_ADDRESS(entry) ENTRY_GET(entry, 10, 22)
#define LVL1_ENTRY_SET_BASE_ADDRESS(entry, value) ENTRY_SET(entry, value, 10, 22)

#define LVL1_ENTRY_GET_DOMAIN(entry) ENTRY_GET(entry, 5, 4)
#define LVL1_ENTRY_SET_DOMAIN(entry, value) ENTRY_SET(entry, value, 5, 4)

// ## Domains
#define LVL1_ENTRY_DOMAIN_SYSTEM 0b0000

// ### Domain control values
/* Any access generates a domain fault. */
#define LVL1_ENTRY_DOMAIN_NO_ACCESS 0b00
/* Accesses are checked against the access permission bits in the section or
 * page descriptor. */
#define LVL1_ENTRY_DOMAIN_CLIENT 0b01
// 0b10 is reserved.
/* Accesses are not checked against the access permission bits so a permission
 * fault cannot be generated. */
#define LVL1_ENTRY_DOMAIN_MANAGER 0b11

#define LVL1_ENTRY_GET_TYPE(entry) ENTRY_GET(entry, 0, 2)
#define LVL1_ENTRY_SET_TYPE(entry, value) ENTRY_SET(entry, value, 0, 2)

#define LVL1_TYPE_INVALID 0b00
#define LVL1_TYPE_COARSE_PAGE_TABLE 0b01
// #define LVL1_TYPE_SECTION 0b10
// #define LVL1_TYPE_FINE_PAGE_TABLE 0b11


// # Level 2 entry
//
// Memory layout:
// | 31        12 | 11 10 | 9 8 | 7 6 | 5 4 | 3 | 2 | 1      0 |
// | Base address |   AP3 | AP2 | AP1 | AP0 | C | B | Validity |

#define LVL2_ENTRY_DEFAULT 0b00000000000000000000000000000000
#define LVL2_ENTRY_CLEAR(entry) (entry = UINT32_ENDIANNESS_SWAP(LVL2_ENTRY_DEFAULT))

#define LVL2_ENTRY_GET_BASE_ADDRESS(entry) ENTRY_GET(entry, 12, 20)
#define LVL2_ENTRY_SET_BASE_ADDRESS(entry, value) ENTRY_SET(entry, value, 12, 20)

#define LVL2_ENTRY_GET_AP3(entry) ENTRY_GET(entry, 10, 2)
#define LVL2_ENTRY_SET_AP3(entry, value) ENTRY_SET(entry, value, 10, 2)
#define LVL2_ENTRY_GET_AP2(entry) ENTRY_GET(entry, 8, 2)
#define LVL2_ENTRY_SET_AP2(entry, value) ENTRY_SET(entry, value, 8, 2)
#define LVL2_ENTRY_GET_AP1(entry) ENTRY_GET(entry, 6, 2)
#define LVL2_ENTRY_SET_AP1(entry, value) ENTRY_SET(entry, value, 6, 2)
#define LVL2_ENTRY_GET_AP0(entry) ENTRY_GET(entry, 4, 2)
#define LVL2_ENTRY_SET_AP0(entry, value) ENTRY_SET(entry, value, 4, 2)

// ## Access Permissions
// http://infocenter.arm.com/help/topic/com.arm.doc.ddi0198e/DDI0198E_arm926ejs_r0p5_trm.pdf#page=84&zoom=100,41,652
#define LVL2_ENTRY_AP_ALL_NO_ACCESS 0b00
#define LVL2_ENTRY_AP_PRIVILEDGED READ_ONLY_USER_NO_ACCESS 0b10
#define LVL2_ENTRY_AP_ALL_READ_ONLY 0b01
#define LVL2_ENTRY_AP_ALL_UNPREDICTABLE 0b11

// TODO: Actually, the Access Permission Bits should be used:
// 0 0 0 0 No access No access
// 0 0 1 0 Read-only No access
// 0 0 0 1 Read-only Read-only
// 0 0 1 1 Unpredictable Unpredictable
// 0 1 x x Read/Write No access
// 1 0 x x Read/Write Read-only
// 1 1 x x Read/Write Read/Write

#define LVL2_ENTRY_GET_CACHE_BEHAVIOR(entry) ENTRY_GET(entry, 2, 2)
#define LVL2_ENTRY_SET_CACHE_BEHAVIOR(entry, value) ENTRY_SET(entry, value, 2, 2)

/* DCache disabled. Read from external memory. Write as a nonbuffered store(s)
 * to external memory. DCache is not updated. */
#define LVL2_CACHE_BEHAVIOR_NONCACHEABLE_NONBUFFERABLE 0b00
/* DCache disabled. Read from external memory. Write as a buffered store(s) to
 * external memory. DCache is not updated. */
#define LVL2_CACHE_BEHAVIOR_NONCACHEABLE_BUFFERABLE 0b01
/* DCache enabled:
 * - Read hit: Read from DCache
 * - Read miss: Linefill
 * - Write hit: Write to the DCache, and buffered store to external memory
 * - Write miss: Buffered store to external memory */
#define LVL2_CACHE_BEHAVIOR_CACHEABLE_WRITETHROUGH 0b10
/* DCache enabled:
 * - Read hit: Read from DCache
 * - Read miss: Linefill
 * - Write hit: Write to the DCache only
 * - Write miss: Buffered store to external memory. */
#define LVL2_CACHE_BEHAVIOR_CACHEABLE_WRITEBACK 0b11

#define LVL2_ENTRY_GET_TYPE(entry) ENTRY_GET(entry, 0, 2)
#define LVL2_ENTRY_SET_TYPE(entry, value) ENTRY_SET(entry, value, 0, 2)

/* Generates a page translation fault. */
#define LVL2_ENTRY_TYPE_INVALID 0b00
/* 64 KB */
// #define LVL2_ENTRY_TYPE_LARGE_PAGE 0b01
/* 4 KB */
#define LVL2_ENTRY_TYPE_SMALL_PAGE 0b10
/* 1 KB */
// #define LVL2_ENTRY_TYPE_TINY_PAGE 0b11



// Clearing of whole tables.

void clear_coarse_table(mem_coarse_table_t* table) {
  for (int i = 0; i < MEM_NUM_ENTRIES_COARSE_TABLE; i++) {
    table->entries[i] = LVL1_ENTRY_DEFAULT;
  }
}

void clear_translation_table(mem_translation_table_t* table) {
  for (int i = 0; i < MEM_NUM_ENTRIES_TRANSLATION_TABLE; i++) {
    table->entries[i] = LVL2_ENTRY_DEFAULT;
  }
}

// Adding of entries.
// 
// bool add_translation_table_entry_to_translation_table(mem_translation_table_t* translation_table, mem_translation_table_entry_t* entry)
// {
//   // Find an empty slot (with invalid descriptor bits).
//   for (int i = 0; i < MEM_NUM_ENTRIES_TRANSLATION_T; i++) {
//     mem_translation_table_entry_t table_entry = translation_table->entries[i];

//     if (table_entry.descriptor == MEM_TRANSLATION_DESCRIPTOR_INVALID) {
//       // Insert the new table here.
//       table_entry = *entry;
//       return true;
//     }
//   }
//   return false;
// }

// bool add_coarse_pte_to_coarse_pt(mem_coarse_pt_t* coarse_table, mem_coarse_pte_t* entry)
// {
//   // Find an empty slot (with invalid descriptor bits).
//   for (int i = 0; i < MEM_NUM_ENTRIES_COARSE_PT; i++) {
//     mem_coarse_pte_t table_entry = coarse_table->entries[i];

//     if (table_entry.descriptor == MEM_COARSE_DESCRIPTOR_INVALID) {
//       // Insert the new table here.
//       table_entry = *entry;
//       return true;
//     }
//   }
//   return false;
// }

// bool add_coarse_pte_to_translation_table(mem_translation_table_t* translation_table, mem_coarse_pte_t* entry)
// {
//   // Try adding the entry to an existing coarse pt.
//   for (int i = 0; i < MEM_NUM_ENTRIES_TRANSLATION_T; i++) {
//     mem_translation_table_entry_t table_entry = translation_table->entries[i];
//     if (table_entry.descriptor == MEM_TRANSLATION_DESCRIPTOR_INVALID) {
//       continue;
//     }

//     bool result = add_coarse_pte_to_coarse_pt((mem_coarse_pt_t*)(table_entry.base_address << 14), entry);
//     if (result) {
//       return true;
//     }
//   }

//   // Otherwise, try to create a new coarse pte.
//   mem_translation_table_entry_t table_entry = create_translation_table_entry();
//   if (!add_coarse_pte_to_coarse_pt(&table_entry, entry)) {
//     return false;
//   }

//   mem_coarse_pt_t coarse_table;
//   coarse_entry.base_address = ((uint32_t) &coarse_table) >> 14;
//   return true;

//   // // Find an empty slot (with invalid descriptor bits).
//   // for (int i = 0; i < MEM_NUM_ENTRIES_TRANSLATION_T; i++) {
//   //   mem_translation_table_entry_t *table_entry = &(translation_table->entries[i]);

//   //   if (table_entry->descriptor == MEM_COARSE_DESCRIPTOR_INVALID) {
//   //     // Insert the new table here.
//   //     *table_entry = *entry;
//   //     return true;
//   //   }
//   // }
//   // return false;
// }

// bool add_small_pte_to_translation_table(mem_translation_table_t* translation_table, mem_small_pte_t entry)
// {
//   // Try adding the entry to an existing small pt.
//   for (int i = 0; i < MEM_NUM_ENTRIES_TRANSLATION_T; i++) {
//     mem_translation_table_entry_t table_entry = translation_table->entries[i];
//     if (table_entry.descriptor == MEM_TRANSLATION_DESCRIPTOR_INVALID) {
//       continue;
//     }

//     bool result = add_small_pte_to_coarse_pt((mem_coarse_pt_t*)(table_entry.base_address << 10), entry);
//     if (result) {
//       return true;
//     }
//   }

//   // Otherwise, try to create a new translation table entry.
//   mem_translation_table_entry_t *translation_entry;
//   if (!add_translation_entry_to_translation_table(translation_entry)) {
//     return false;
//   }
  
//   mem_coarse_pt_t coarse_table;
//   translation_entry->base_address = ((uint32_t) &coarse_table) >> 10;

//   mem_small_pt_t small_table;
//   coarse_entry->base_address = ((uint32_t) &small_table) >> 10;
//   return true;
// }



// /* Adds a new entry to the given small page table.
//  * Returns if the the new entry was successfully inserted.
//  */
// bool add_small_pte_to_small_pt(mem_small_pt_t* small_table, mem_small_pte_t* entry)
// {
//   // Find an empty slot (with invalid descriptor bits).
//   for (int i = 0; i < MEM_NUM_ENTRIES_SMALL_PT; i++) {
//     mem_small_pte_t table_entry = small_table->entries[i];

//     if (table_entry.descriptor == MEM_SMALL_DESCRIPTOR_INVALID) {
//       // Insert the new table here.
//       table_entry = *entry;
//       return true;
//     }
//   }
//   return false;
// }

// bool add_small_pte_to_coarse_pt(mem_coarse_pt_t* coarse_table, mem_small_pte_t* entry)
// {
//   // Try adding the entry to an existing small pt.
//   for (int i = 0; i < MEM_NUM_ENTRIES_COARSE_PT; i++) {
//     mem_coarse_pte_t table_entry = coarse_table->entries[i];
//     if (table_entry.descriptor == MEM_COARSE_DESCRIPTOR_INVALID) {
//       continue;
//     }

//     bool result = add_small_pte_to_small_pt((mem_small_pt_t*)(table_entry.base_address << 10), entry);
//     if (result) {
//       return true;
//     }
//   }

//   // Otherwise, try to create a new coarse pte.
//   mem_coarse_pte_t coarse_entry = create_coarse_pte();
//   if (!add_coarse_pte_to_coarse_pt(&coarse_entry, entry)) {
//     return false;
//   }

//   mem_small_pt_t small_table;
//   coarse_entry.base_address = ((uint32_t) &small_table) >> 10;
//   return true;
// }


// void add_translation_entry(translation_table_t* translation_table)
// {
//   translation_table_entry_t* translation_entry;

//   mem_coarse_pt_t *coarse_table = (mem_coarse_pt_t *) 0x12345678;

//   translation_entry.base_address = coarse_table;
//   translation_entry.const_0 = 0;
//   translation_entry.domain = 0; //TODO: proper Domain here!
//   translation_entry.const_1 = 1; //0b1
//   translation_entry.const_00 = 0; //0b00
//   translation_entry.descriptor = 1; //0b01

  
//   // TODO: add entry to table
// }

// /* Creates a new Small-Page-Table on Demand */
// void add_coarse_pte(mem_coarse_pt_t* coarse_table)
// {
//   mem_coarse_pte_t* coarse_entry;

//   mem_small_pt_t *page_table = ()

//   coarse_entry.base_address = page_table.
//   coarse_entry.domain =
//   coarse_entry.const_1 = 1; // 0b1
//   coarse_entry.descriptor = 1; //0b01

//   // TODO: add entry to table
// }


// The kernel page table.
// TODO: Hardcode its address so it aligns with page borders.
#define NUM_COARSE_TABLES MEM_NUM_ENTRIES_TRANSLATION_TABLE
mem_translation_table_t mem_kernel_table;
mem_coarse_table_t all_coarse_tables[NUM_COARSE_TABLES];

void mem_init_kernel_table(void) {
  printf("The kernel table lives at %x\n", (int) &mem_kernel_table);
  printf("The coarse tables live at %x\n", (int) &all_coarse_tables);

  // Clear all tables.
  clear_translation_table(&mem_kernel_table);
  for (int i = 0; i < NUM_COARSE_TABLES; i++) {
    clear_coarse_table(&all_coarse_tables[i]);
  }

  printf("Level 2 entry size: %i\n", sizeof(all_coarse_tables[0].entries[0]));
  mem_lvl2_entry_t st = all_coarse_tables[0].entries[0];
  for (uint8_t i = 0; i < sizeof(st); i++) {
    printf("%x ", *((uint8_t*) ((void*) &st) + i));
  }
  printf("\n");

  printf("Level 1 entry size: %i\n", sizeof(mem_kernel_table.entries[0]));
  mem_lvl1_entry_t lvl1_entry = mem_kernel_table.entries[0];
  for (uint8_t i = 0; i < sizeof(lvl1_entry); i++) {
    printf("%x ", *((uint8_t*) ((void*) &lvl1_entry) + i));
  }
  printf("\n");


  for (int i = 0; i < NUM_COARSE_TABLES; i++) {
    mem_coarse_table_t coarse_table = all_coarse_tables[i];

    mem_lvl1_entry_t lvl1_entry = LVL1_ENTRY_DEFAULT;
    LVL1_ENTRY_SET_BASE_ADDRESS(lvl1_entry, ((uint32_t) &coarse_table) >> 10);
    LVL1_ENTRY_SET_DOMAIN(lvl1_entry, LVL1_ENTRY_DOMAIN_MANAGER);
    LVL1_ENTRY_SET_TYPE(lvl1_entry, LVL1_TYPE_COARSE_PAGE_TABLE);

    for (int j = 0; j < MEM_NUM_ENTRIES_COARSE_TABLE; j++) {
      mem_lvl2_entry_t lvl2_entry = coarse_table.entries[j];
      LVL2_ENTRY_SET_BASE_ADDRESS(lvl2_entry, i << 8 | j);
      LVL2_ENTRY_SET_TYPE(lvl2_entry, LVL2_ENTRY_TYPE_SMALL_PAGE);
    }
  }
}

void mem_init (void) {
  printf("Initializing mem…\n");
  mem_init_kernel_table();

  // // Get the current address of the stack.
  // int a = 42;

  // // Create a new translation table.
  // mem_translation_table_t translation_table;
  // clear_translation_table(&translation_table);

  // mem_small_pt_t small_table;
  // clear_small_table(&small_table);

  // mem_small_pte_t small_table_entry;
  // small_table_entry.ap3 = 0;
  // small_table_entry.ap2 = 0;
  // small_table_entry.ap1 = 0;
  // small_table_entry.ap0 = 0;
  // small_table_entry.base_address = 0x40;
  // small_table_entry.c = 0;
  // small_table_entry.b = 0;
  // small_table_entry.descriptor = MEM_SMALL_DESCRIPTOR_SMALL_PAGE;
  // small_table.entries[0] = small_table_entry;

  // mem_coarse_pt_t coarse_table;
  // clear_coarse_table(&coarse_table);
  
  // mem_coarse_pte_t coarse_table_entry;
  // coarse_table_entry.base_address = ((unsigned int) &small_table) >> 10;
  // coarse_table_entry.domain = 0;
  // coarse_table_entry.const_1 = 1; // 0b1
  // coarse_table_entry.descriptor = 1; //0b01
  // coarse_table.entries[0] = coarse_table_entry;

  // translation_table.entries[0].base_address = (unsigned int) &coarse_table;
  // translation_table.entries[0].domain = 0;
  // translation_table.entries[0].const_1 = 1; // 0b1
  // translation_table.entries[0].descriptor = MEM_TRANSLATION_DESCRIPTOR_COARSE_PAGE_TABLE;

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
    : : "r" (&mem_kernel_table)
  );

  // Configure the Domain Access Control Register (c3).
  //
  // - D15 – D1 (used by programs): initially set to 0b00 for "No access"
  // - D00 (used by the kernel): set to 0b11 for "Manager"
  asm (
    "MCR p15, 0, %0, c3, c0, 0\n"
    : : "r" (LVL1_ENTRY_DOMAIN_MANAGER)
  );

  // TODO: Initialize Pages for Kernel
  printf("Enabling MMU…\n");

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

  // Virtual mem addresses used by the kernel are the same as underlying
  // physical addresses. Hence we don't have to worry about prefetched
  // instructions getting broken.
  printf("Enabled MMU\n");

  int *answer = (int*) 0x400042;
  printf("The translated answer is %x.\n", answer);

  printf("Memory Setup Done.\n");
}
