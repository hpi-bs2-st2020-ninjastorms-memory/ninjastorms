
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

#include "main.h"

#include "kernel/drivers/button.h"
#include "kernel/scheduler.h"
#include "memory.h"

#include <stdio.h>

static void
task_a (void)
{
  unsigned int n = 0;

  while (1)
    {
      printf("  task a: %i\n", n++);
      volatile int i;
      for (i = 0; i < 10000000; ++i);
    }
}

static void
task_b (void)
{
  unsigned int n = 0;

  while (1)
    {
      printf("  task b: %i\n", n++);
      volatile int i;
      for (i = 0; i < 10000000; ++i);
    }
}

static void
task_c (void)
{
  unsigned int n = 0;

  while (1)
    {
      printf("  task c: %i\n", n++);
      volatile int i;
      for (i = 0; i < 10000000; ++i);
    }
}

static void
task_d (void)
{
  unsigned int n = 0;

  while (1)
    {
      printf("  task d: %i\n", n++);
      volatile int i;
      for (i = 0; i < 10000000; ++i);
    }
}


static void
call_software_interrupt_test(void)
{
    puts("Testing swi");
    //Change mode
    //unsigned int* cpsr = (unsigned int*) 0x10;
    //printf("cpsr content is %x before", *cpsr);
    //*cpsr &= 0x37777777760; //mask 
    //*cpsr |= 0x10;
    //printf("and %x after", *cpsr);
    /*asm(
        "mov r0, #0x10\n"
        "bfm cpsr, r0, #26, #31 \n"
        //"bfxil cpsr, r0, #26, #5\n" //switch to user mode
        "svc 42\n"
    );*/
    asm(
        "mov r11, #10000\n"
        "msr cpsr, r11\n"
        "svc 42"
    );
}

char shuriken[] =
"                 /\\\n"
"                /  \\\n"
"                |  |\n"
"              __/()\\__\n"
"             /   /\\   \\\n"
"            /___/  \\___\\\n";

int
kernel_main (void)
{
  puts("This is ninjastorms OS");
  puts("  shuriken ready");
  puts(shuriken);

  //add_task(&task_a);
  //add_task(&task_b);
  //add_task(&task_c);
  //add_task(&task_d);
  add_task(&call_software_interrupt_test);

  start_scheduler();


  puts("All done. ninjastorms out!");

  return 0;
}
