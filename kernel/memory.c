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

// Clearing of entries.

void
clear_lvl2_entry(mem_lvl2_entry_t* entry)
{
  // TODO: Think about what to do with the ap and c/b bits.
  entry->base_address = 0x0;
  entry->ap3 = 0;
  entry->ap2 = 0;
  entry->ap1 = 0;
  entry->ap0 = 0;
  entry->cache_behavior = MEM_LVL2_CACHE_BEHAVIOR_NONCACHEABLE_NONBUFFERABLE;
  entry->descriptor = MEM_LVL2_DESCRIPTOR_INVALID;
}

void
clear_lvl1_entry(mem_lvl1_entry_t* entry)
{
  entry->base_address = 0x0;
  entry->const_0 = 0;
  entry->domain = MEM_DOMAIN_CONTROL_NO_ACCESS;
  entry->const_1 = 1;
  entry->const_00 = 0;
  entry->descriptor = MEM_LVL1_DESCRIPTOR_INVALID;
}

// Clearing of whole tables.

void
clear_coarse_table(mem_coarse_table_t* table)
{
  for (int i = 0; i < MEM_NUM_ENTRIES_COARSE_TABLE; i++) {
    clear_lvl2_entry(&(table->entries[i]));
  }
}

void
clear_translation_table(mem_translation_table_t* table)
{
  for (int i = 0; i < MEM_NUM_ENTRIES_TRANSLATION_TABLE; i++) {
    clear_lvl1_entry(&(table->entries[i]));
  }
}

// Creation of entries.

mem_lvl2_entry_t create_lvl2_entry() {
  mem_lvl2_entry_t entry;
  clear_lvl2_entry(&entry);
  return entry;
}

mem_lvl1_entry_t create_lvl1_entry() {
  mem_lvl1_entry_t entry;
  clear_lvl1_entry(&entry);
  return entry;
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

void mem_init_kernel_table(void)
{
  printf("The kernel table lives at %x\n", (int) &mem_kernel_table);
  printf("The coarse tables live at %x\n", (int) &all_coarse_tables);
  // printf("The small tables live at %d", (int) &all_small_tables);

  // Clear all tables.
  clear_translation_table(&mem_kernel_table);
  for (int i = 0; i < NUM_COARSE_TABLES; i++) {
    clear_coarse_table(&all_coarse_tables[i]);
  }
  // for (int i = 0; i < NUM_COARSE_TABLES * MEM_NUM_ENTRIES_COARSE_PT; i++) {
  //   clear_small_table(&all_small_tables[i]);
  // }

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

    mem_lvl1_entry_t lvl1_entry = mem_kernel_table.entries[i];
    lvl1_entry.base_address = ((uint32_t) &coarse_table) >> 10; // TODO: Create entry.
    lvl1_entry.descriptor = MEM_LVL1_DESCRIPTOR_COARSE_PAGE_TABLE;
    lvl1_entry.domain = MEM_DOMAIN_CONTROL_MANAGER;

    for (int j = 0; j < MEM_NUM_ENTRIES_COARSE_TABLE; j++) {
      mem_lvl2_entry_t lvl2_entry = coarse_table.entries[j];
      lvl2_entry.base_address = i << 8 | j;
      lvl2_entry.descriptor = MEM_LVL2_DESCRIPTOR_SMALL_PAGE;
    }
  }
}

void
mem_init (void)
{
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
    : : "r" (MEM_DOMAIN_CONTROL_MANAGER)
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
