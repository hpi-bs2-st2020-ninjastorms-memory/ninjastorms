
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
#include "kernel/utilities.h"

#include "syscall.h"

#include <stdio.h>
#include <errno.h>

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
  int my_pid = get_pid();
  printf("b: My pid: %i\n",my_pid);
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
  int my_pid = get_pid();
  printf("c: My pid: %i, my parent is pid %i\n",my_pid, get_parent_pid());
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
  int my_pid = get_pid();
  printf("d: My pid: %i\n",my_pid);
  create_process(&task_c);
  unsigned int n = 0;
  /* Testing for permissions
  int direct_call_result = add_task(&task_c);
  if(direct_call_result<0){
    printf("add task failed with errno %i \n",errno);
  }*/
  while (1)
    {
      printf("  task d: %i\n", n++);
      volatile int i;
      for (i = 0; i < 10000000; ++i);
      if(n>5){
          //That's enough!
          shutdown();
      }
    }
}


static void
user_mode_init(void)
{
    printf("User mode initialized with pid: %i", get_pid());
    int a_pid = create_process(&task_a);
    printf("Starting a, assigned pid: %i", a_pid);
    create_process(&task_b);
    create_process(&task_d);
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

  add_task(&user_mode_init);
  start_scheduler();

  puts("All done. ninjastorms out!");

  return 0;
}
