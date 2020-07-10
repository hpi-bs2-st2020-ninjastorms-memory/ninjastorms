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


// An entry for the root translation table.
//
// Reference: http://infocenter.arm.com/help/topic/com.arm.doc.ddi0198e/DDI0198E_arm926ejs_r0p5_trm.pdf#page=72
typedef uint32_t mem_lvl1_entry_t;

#define MEM_NUM_ENTRIES_TRANSLATION_TABLE 4096

typedef struct mem_translation_table_t {
  mem_lvl1_entry_t entries[MEM_NUM_ENTRIES_TRANSLATION_TABLE];
} mem_translation_table_t;


// An entry for the second level, i.e. a small page.
//
// Reference: http://infocenter.arm.com/help/topic/com.arm.doc.ddi0198e/DDI0198E_arm926ejs_r0p5_trm.pdf#page=78
typedef uint32_t mem_lvl2_entry_t;

#define MEM_NUM_ENTRIES_COARSE_TABLE 256

// A coarse page table.
//
// Reference: http://infocenter.arm.com/help/topic/com.arm.doc.ddi0198e/DDI0198E_arm926ejs_r0p5_trm.pdf#page=71
typedef struct mem_coarse_table_t {
  mem_lvl2_entry_t entries[MEM_NUM_ENTRIES_COARSE_TABLE]; // 4 KiB / 32 bits
} mem_coarse_table_t;


void mem_init(void);

void mem_debug_interrupt(void);

void mem_interrupt_handler_data_abort(void);
