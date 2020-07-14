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


#define ENTRY_GET(entry, offset, length) (BITFIELD_GET(entry, offset, length))
#define ENTRY_SET(entry, value, offset, length) \
    ((entry) = BITFIELD_MERGE(entry, value, offset, length))

// # Level 1 entry
//
// Memory layout:
// | 31        10 | 9 | 8    5 | 4 | 3 2 | 1      0 |
// | Base address | 0 | Domain | 1 |  00 | Validity |

#define LVL1_ENTRY_DEFAULT 0b00000000000000000000000000010000

#define LVL1_ENTRY_GET_BASE_ADDRESS(entry) ENTRY_GET(entry, 10, 22)
#define LVL1_ENTRY_GET_COARSE_TABLE(entry) \
    ((mem_coarse_table_t*) (LVL1_ENTRY_GET_BASE_ADDRESS(entry) << 10))
#define LVL1_ENTRY_SET_BASE_ADDRESS(entry, value) \
    ENTRY_SET(entry, value, 10, 22)
#define LVL1_ENTRY_SET_COARSE_TABLE(entry, value) \
    LVL1_ENTRY_SET_BASE_ADDRESS(entry, ((uint32_t) (value)) >> 10)

#define LVL1_ENTRY_GET_DOMAIN(entry) ENTRY_GET(entry, 5, 4)
#define LVL1_ENTRY_SET_DOMAIN(entry, value) ENTRY_SET(entry, value, 5, 4)

// ## Domains
#define LVL1_ENTRY_DOMAIN_SYSTEM 0b0000

// ### Domain control values
// Any access generates a domain fault.
#define LVL1_ENTRY_DOMAIN_NO_ACCESS 0b00
// Accesses are checked against the access permission bits in the section or
// page descriptor.
#define LVL1_ENTRY_DOMAIN_CLIENT 0b01
// 0b10 is reserved.
// Accesses are not checked against the access permission bits so a permission
// fault cannot be generated.
#define LVL1_ENTRY_DOMAIN_MANAGER 0b11

#define LVL1_ENTRY_GET_TYPE(entry) ENTRY_GET(entry, 0, 2)
#define LVL1_ENTRY_SET_TYPE(entry, value) ENTRY_SET(entry, value, 0, 2)

#define LVL1_ENTRY_TYPE_INVALID 0b00
#define LVL1_ENTRY_TYPE_COARSE_PAGE_TABLE 0b01
#define LVL1_ENTRY_TYPE_SECTION 0b10
#define LVL1_ENTRY_TYPE_FINE_PAGE_TABLE 0b11


// # Level 2 entry
//
// Memory layout:
// | 31        12 | 11 10 | 9 8 | 7 6 | 5 4 | 3 | 2 | 1      0 |
// | Base address |   AP3 | AP2 | AP1 | AP0 | C | B | Validity |

#define LVL2_ENTRY_DEFAULT 0b00000000000000000000000000000000

#define LVL2_ENTRY_GET_BASE_ADDRESS(entry) ENTRY_GET(entry, 12, 20)
#define LVL2_ENTRY_GET_BASE_POINTER(entry) \
    ((void*) LVL2_ENTRY_GET_BASE_ADDRESS(entry) << 12)
#define LVL2_ENTRY_SET_BASE_ADDRESS(entry, value) \
    ENTRY_SET(entry, value, 12, 20)
#define LVL2_ENTRY_SET_BASE_POINTER(entry, value) \
    LVL2_ENTRY_SET_BASE_ADDRESS(entry, ((uint32_t) (value)) >> 12)

#define LVL2_ENTRY_GET_AP3(entry) ENTRY_GET(entry, 10, 2)
#define LVL2_ENTRY_SET_AP3(entry, value) ENTRY_SET(entry, value, 10, 2)
#define LVL2_ENTRY_GET_AP2(entry) ENTRY_GET(entry, 8, 2)
#define LVL2_ENTRY_SET_AP2(entry, value) ENTRY_SET(entry, value, 8, 2)
#define LVL2_ENTRY_GET_AP1(entry) ENTRY_GET(entry, 6, 2)
#define LVL2_ENTRY_SET_AP1(entry, value) ENTRY_SET(entry, value, 6, 2)
#define LVL2_ENTRY_GET_AP0(entry) ENTRY_GET(entry, 4, 2)
#define LVL2_ENTRY_SET_AP0(entry, value) ENTRY_SET(entry, value, 4, 2)

// ## Access Permissions
//
// http://infocenter.arm.com/help/topic/com.arm.doc.ddi0198e/DDI0198E_arm926ejs_r0p5_trm.pdf#page=84&zoom=100,41,652
//
// | S | R | Privileged permissions | user permissions |
// |:-:|:-:|:-----------------------|:-----------------|
// | 0 | 0 | No access              | No access        |
// | 1 | 0 | Read-only              | No access        |
// | 0 | 1 | Read-only              | Read-only        |
// | 1 | 1 | Unpredictable          | Unpredictable    |
#define LVL2_ENTRY_AP_PROTECTION_DEPENDENT 0b00
// Privileged permissions: read/write · user permissions: no access
#define LVL2_ENTRY_AP_RW_NONE 0b10
// Privileged permissions: read/write · user permissions: read-only
#define LVL2_ENTRY_AP_RW_R 0b01
// Privileged permissions: read/write · user permissions: read/write
#define LVL2_ENTRY_AP_RW_RW 0b11

#define LVL2_ENTRY_GET_CACHE_BEHAVIOR(entry) ENTRY_GET(entry, 2, 2)
#define LVL2_ENTRY_SET_CACHE_BEHAVIOR(entry, value) ENTRY_SET(entry, value, 2, 2)

// DCache disabled. Read from external memory. Write as a nonbuffered store(s)
// to external memory. DCache is not updated.
#define LVL2_CACHE_BEHAVIOR_NONCACHEABLE_NONBUFFERABLE 0b00
// DCache disabled. Read from external memory. Write as a buffered store(s) to
// external memory. DCache is not updated.
#define LVL2_CACHE_BEHAVIOR_NONCACHEABLE_BUFFERABLE 0b01
// DCache enabled:
// - Read hit: Read from DCache
// - Read miss: Linefill
// - Write hit: Write to the DCache, and buffered store to external memory
// - Write miss: Buffered store to external memory
#define LVL2_CACHE_BEHAVIOR_CACHEABLE_WRITETHROUGH 0b10
// DCache enabled:
// - Read hit: Read from DCache
// - Read miss: Linefill
// - Write hit: Write to the DCache only
// - Write miss: Buffered store to external memory
#define LVL2_CACHE_BEHAVIOR_CACHEABLE_WRITEBACK 0b11

#define LVL2_ENTRY_GET_TYPE(entry) ENTRY_GET(entry, 0, 2)
#define LVL2_ENTRY_SET_TYPE(entry, value) ENTRY_SET(entry, value, 0, 2)

// Generates a page translation fault.
#define LVL2_ENTRY_TYPE_INVALID 0b00
// 64 KB
#define LVL2_ENTRY_TYPE_LARGE_PAGE 0b01
// 4 KB
#define LVL2_ENTRY_TYPE_SMALL_PAGE 0b10
// 1 KB
#define LVL2_ENTRY_TYPE_TINY_PAGE 0b11

// Fault status codes that can be reported by the MMU in the Fault Status
// Register (FSR).
//
// https://developer.arm.com/documentation/ddi0198/e/memory-management-unit/mmu-faults-and-cpu-aborts/fault-address-and-fault-status-registers?lang=en
#define MMU_FAULT_STATUS_TRANSLATION_PAGE 0b0111


void dump_uint32(uint32_t value) {
  printf("0x");
  for (uint8_t i = 0; i < sizeof(value) * 2; i++) {
    // printf("%x", *((uint8_t*) ((void*) &value) + i));
    printf("%x", (value >> (4 * (7 - i))) & 0xf);
    if (i % 2 == 1 && i > 0) {
      printf(" ");
    }
  }
}

void dump_bin_uint8(uint8_t value) {
  for (int i = 7; i >= 0; i--) {
    printf("%i", (value >> i) & 0b1);
  }
}

void dump_bin_uint32(uint32_t value) {
  printf("0b");
  for (int i = 31; i >= 0; i--) {
    printf("%i", (value >> i) & 0b1);
    if (i % 8 == 0 && i > 0) {
      printf(" ");
    }
  }
}

// Clearing of whole tables.

void mem_clear_coarse_table(mem_coarse_table_t* table) {
  for (int i = 0; i < MEM_NUM_ENTRIES_COARSE_TABLE; i++) {
    table->entries[i] = LVL2_ENTRY_DEFAULT;
  }
}

void mem_clear_translation_table(mem_translation_table_t* table) {
  for (int i = 0; i < MEM_NUM_ENTRIES_TRANSLATION_TABLE; i++) {
    table->entries[i] = LVL1_ENTRY_DEFAULT;
  }
}

// Creation of tables.

mem_coarse_table_t* mem_create_coarse_table() {
  // TODO
}

// Adding of entries.

// Adds a new level 2 [entry] to the existing coarse [table].
//
// The [address_in_page] should be an address in the page that is to be added.
// | 12 bits | 8 bits | 12 bits |
void mem_add_lvl2_entry_to_coarse_table(
  mem_coarse_table_t* table,
  void* address_in_page,
  mem_lvl2_entry_t entry
) {
  printf("Adding a lvl2 entry to the coarse table for address 0x%x.\n",
      address_in_page);
  uint32_t index = (uint32_t) address_in_page >> 12 & 0xFF;

  if (LVL2_ENTRY_GET_TYPE(table->entries[index]) != LVL2_ENTRY_TYPE_INVALID) {
    printf("Fatal: There's already an entry for page base address 0x%x in the "
      "coarse page table. This indicates that the table is not setup "
      "correctly, because we shouldn't need to virtualize the same address "
      "twice.\n",
      address_in_page);
    return;
  }

  table->entries[index] = entry;
}

// Adds a new level 1 [entry] to the existing translation [table].
//
// The [address_in_page] should be an address in the page that is to be added.
// | 12 bits | 8 bits | 12 bits |
void mem_add_lvl1_entry_to_translation_table(
  mem_translation_table_t* table,
  void* address_in_page,
  mem_lvl1_entry_t entry
) {
  printf("Adding a lvl1 entry to the translation table for address 0x%x.\n",
      address_in_page);
  uint32_t index = (uint32_t) address_in_page >> 20;

  if (LVL1_ENTRY_GET_TYPE(table->entries[index]) == LVL1_ENTRY_TYPE_INVALID) {
    printf("Fatal: There's already an entry for page base address 0x%x in the "
      "translation table.\n"
      "This indicates that the table is not setup correctly, because we "
      "shouldn't need to virtualize the same address twice.\n",
      address_in_page);
    return;
  }

  table->entries[index] = entry;
}

void mem_add_lvl2_entry_to_translation_table(
  mem_translation_table_t* table,
  void* address_in_page,
  mem_lvl2_entry_t entry
) {
  printf("Adding a lvl2 entry to the translation table for address 0x%x.\n",
      address_in_page);
  uint32_t index = (uint32_t) address_in_page >> 20;
  mem_lvl1_entry_t table_entry = table->entries[index];

  // If a translation table already exists for this entry, add it to the
  // corresponding coarse table.
  if (LVL1_ENTRY_GET_TYPE(table_entry) != LVL1_ENTRY_TYPE_INVALID) {
    mem_coarse_table_t* coarse_table = LVL1_ENTRY_GET_COARSE_TABLE(table_entry);
    mem_add_lvl2_entry_to_coarse_table(coarse_table, address_in_page, entry);
    return;
  }

  // Create a new coarse table and create an entry in the translation table that
  // points to it.
  mem_coarse_table_t* coarse_table = mem_create_coarse_table();
  mem_add_lvl2_entry_to_coarse_table(coarse_table, address_in_page, entry);
  
  LVL1_ENTRY_SET_COARSE_TABLE(table_entry, coarse_table);
  LVL1_ENTRY_SET_DOMAIN(table_entry, LVL1_ENTRY_DOMAIN_SYSTEM);
  LVL1_ENTRY_SET_TYPE(table_entry, LVL1_ENTRY_TYPE_COARSE_PAGE_TABLE);
  mem_add_lvl1_entry_to_translation_table(table, address_in_page, table_entry);
}


// The kernel page table.
#define NUM_COARSE_TABLES MEM_NUM_ENTRIES_TRANSLATION_TABLE

__attribute((__section__("kernel_page_table")))
mem_translation_table_t mem_kernel_table;

__attribute((__section__("all_coarse_page_tables")))
mem_coarse_table_t all_coarse_tables[NUM_COARSE_TABLES];

const void* _start_kernel_page_table;
const void* _start_all_coarse_page_table;

void mem_init_kernel_table(void) {
  printf("_start_kernel_page_table: 0x%x\n", &mem_kernel_table);
  printf("_start_all_coarse_page_table: 0x%x\n", &all_coarse_tables);

  // Clear all tables.
  mem_clear_translation_table(&mem_kernel_table);
  for (uint16_t i = 0; i < NUM_COARSE_TABLES; i++) {
    mem_clear_coarse_table(&all_coarse_tables[i]);
  }

  mem_lvl1_entry_t lvl1_entry = mem_kernel_table.entries[0];
  for (uint16_t i = 0; i < NUM_COARSE_TABLES; i++) {
    mem_coarse_table_t *coarse_table = &all_coarse_tables[i];

    mem_lvl1_entry_t lvl1_entry = mem_kernel_table.entries[i];
    LVL1_ENTRY_SET_COARSE_TABLE(lvl1_entry, coarse_table);
    LVL1_ENTRY_SET_DOMAIN(lvl1_entry, LVL1_ENTRY_DOMAIN_SYSTEM);
    LVL1_ENTRY_SET_TYPE(lvl1_entry, LVL1_ENTRY_TYPE_COARSE_PAGE_TABLE);
    mem_kernel_table.entries[i] = lvl1_entry;

    for (uint16_t j = 0; j < MEM_NUM_ENTRIES_COARSE_TABLE; j++) {
      mem_lvl2_entry_t lvl2_entry = coarse_table->entries[j];
      LVL2_ENTRY_SET_BASE_POINTER(lvl2_entry, (i << 8 | j) << 12);
      LVL2_ENTRY_SET_AP3(lvl2_entry, LVL2_ENTRY_AP_RW_RW);
      LVL2_ENTRY_SET_AP2(lvl2_entry, LVL2_ENTRY_AP_RW_RW);
      LVL2_ENTRY_SET_AP1(lvl2_entry, LVL2_ENTRY_AP_RW_RW);
      LVL2_ENTRY_SET_AP0(lvl2_entry, LVL2_ENTRY_AP_RW_RW);
      if (i == 0b101010101010 && j == 0b10101010) {
        LVL2_ENTRY_SET_TYPE(lvl2_entry, LVL2_ENTRY_TYPE_INVALID);
      } else {
        LVL2_ENTRY_SET_TYPE(lvl2_entry, LVL2_ENTRY_TYPE_SMALL_PAGE);
      }
      coarse_table->entries[j] = lvl2_entry;
    }
  }
}

void dump_memory_uint32(uint32_t* buffer) {
  for (int i = 0; i < 32; i++) {
    uint32_t value = *(buffer + i);
    dump_uint32(value);
    printf("    ");
    dump_bin_uint32(value);
    printf("\n");
  }
}

void dump_memory_uint8(uint8_t* buffer) {
  for (int i = 0; i < 32; i++) {
    for (int i = 0; i < 4; i++) {
      uint8_t value = *(buffer + i);
      // dump_uint8(value);
      // printf("    ");
      dump_bin_uint8(value);
      printf(" ");
    }
    printf("\n");
  }
}

void mem_init(void) {
  printf("Initializing memory…\n");
  mem_init_kernel_table();
  // printf("Dumping kernel table…\n");
  // printf("\n");
  // printf("Translation table:\n");
  // dump_memory_uint8((uint8_t*) mem_kernel_table);
  // printf("\n");
  // printf("First coarse table:\n");
  // dump_memory_uint8((uint8_t*) all_coarse_tables);

  // Write the address of the translation table to 0xc2. The register c2
  // (Control Register 2) is the Translation Table Base Register (TTBR), for the
  // base address of the first-level translation table.
  // http://infocenter.arm.com/help/topic/com.arm.doc.ddi0198e/DDI0198E_arm926ejs_r0p5_trm.pdf#page=70
  // http://infocenter.arm.com/help/topic/com.arm.doc.ddi0198e/DDI0198E_arm926ejs_r0p5_trm.pdf#page=92
  asm (
    "MCR p15, 0, %0, c2, c0, 0\n" // write TTBR
    : : "r" (&mem_kernel_table)
  );

  // Configure the Domain Access Control Register (c3).
  //
  // - D15 – D1 (used by programs): initially set to 0b00 for "No access"
  // - D00 (used by the kernel): set to 0b11 for "Manager"
  asm (
    "MCR p15, 0, %0, c3, c0, 0\n"
    : : "r" (LVL1_ENTRY_DOMAIN_MANAGER << (LVL1_ENTRY_DOMAIN_SYSTEM * 2))
  );

  printf("Enabling MMU…\n");
  uint32_t c1, c2, c3;
  asm (
    "MRC p15, 0, R1, c1, c0, 0\n" // Read control register
    "ORR R1, #0x1\n"              // Sets M bit from read Control Register to 1
    "MCR p15, 0, R1, c1, c0, 0\n" // Write control register and enables MMU
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
    "MRC p15, 0, %0, c1, c0, 0\n" // Read control register
    "ORR %0, #0x1\n"              // Sets M bit from read Control Register to 1
    "MRC p15, 0, %1, c2, c0, 0\n"
    "MRC p15, 0, %2, c3, c0, 0\n"
    ""
    : "=r" (c1), "=r" (c2), "=r" (c3)
  );

  // Virtual mem addresses used by the kernel are the same as underlying
  // physical addresses. Hence we don't have to worry about prefetched
  // instructions getting broken.
  printf("Memory Setup Done.\n");
}

void mem_test_data_abort(void) {
  uint8_t* forbiddenAddress = (uint8_t*) 0b10101010101010101010101010101010;
  printf("Accessing the forbidden byte…\n");
  uint8_t forbiddenByte = *forbiddenAddress;
  printf("Forbidden byte: %i\n", forbiddenByte);
  printf("Accessing the forbidden byte again…\n");
  forbiddenByte = *forbiddenAddress;
}

void mem_interrupt_handler_data_abort(void) {
  printf("Handling data abort…\n");

  // Read Fault Status Register.
  uint32_t fault_status;
  uint32_t fault_address;
  asm(
    "MRC p15, 0, %0, c5, c0, 0\n" // read FSR
    "MRC p15, 0, %1, c6, c0, 0\n" // read FAR
    : "=r" (fault_status),
      "=r" (fault_address)
  );
  // dump_uint32(fault_address);
  // printf("\n");
  // dump_uint32(fault_status);
  // printf("\n");

  if (fault_status == MMU_FAULT_STATUS_TRANSLATION_PAGE) {
    mem_lvl2_entry_t entry = LVL2_ENTRY_DEFAULT;
    LVL2_ENTRY_SET_BASE_POINTER(entry, fault_address);
    LVL2_ENTRY_SET_AP3(entry, LVL2_ENTRY_AP_RW_RW);
    LVL2_ENTRY_SET_AP2(entry, LVL2_ENTRY_AP_RW_RW);
    LVL2_ENTRY_SET_AP1(entry, LVL2_ENTRY_AP_RW_RW);
    LVL2_ENTRY_SET_AP0(entry, LVL2_ENTRY_AP_RW_RW);
    LVL2_ENTRY_SET_TYPE(entry, LVL2_ENTRY_TYPE_SMALL_PAGE);

    mem_add_lvl2_entry_to_translation_table(&mem_kernel_table,
        (void*) fault_address, entry);
  } else {
    printf("Fatal: Unknown MMU fault status code: 0x%x\n", fault_status);
    return;
  }

  asm("SUBS PC, R14, #0");
  // TODO: use #8 when handling data abort
}
// https://www.scss.tcd.ie/~waldroj/3d1/arm_arm.pdf#page=58
