
/* The entry point of the kernel (see boot.S) */
ENTRY(Start)

SECTIONS
{
  .text :
  {
    KEEP (*(.text.boot))
    *(.text .text.*)
  }
  .rodata :
  {
    *(.rodata .rodata.*)
  }

  . = ALIGN (CONSTANT (MAXPAGESIZE)) - ((CONSTANT (MAXPAGESIZE) - .) & (CONSTANT (MAXPAGESIZE) - 1));
  . = DATA_SEGMENT_ALIGN (CONSTANT (MAXPAGESIZE), CONSTANT (COMMONPAGESIZE));

  .preinit_array :
  {
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array))
    PROVIDE_HIDDEN (__preinit_array_end = .);
  }
  .init_array :
  {
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP (*(.init_array))
    PROVIDE_HIDDEN (__init_array_end = .);
  }
  .fini_array :
  {
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(.fini_array))
    PROVIDE_HIDDEN (__fini_array_end = .);
  }

  .minimal_memory_management_part :
  {
    . = ALIGN(4K);
    _minimal_memory_management_part_start = .;

    libc/*.o;
    kernel/*.o;
    //kernel/memory.o;
    //kernel/interrupt.o;
    //kernel/interrupt_handler.o;

    _minimal_memory_management_part_end = .;
  }

  .data :
  {
    *(.data)

    . = ALIGN(16K);
    *(.kernel_page_table)

    . = ALIGN(1K);
    *(.all_coarse_page_tables)
    //_data_end = .;
  }

  __bss_start = .;
  .bss :
  {
    *(.bss .bss.*)
    . = ALIGN(. != 0 ? 32 / 8 : 1);
    . += 8K;
  }
  __bss_end = .;

  . = ALIGN(4);
  __end = .;
}
