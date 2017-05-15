/* Entry Point */
ENTRY(ResetISR)

/* Highest address of the user mode stack */
_estack = 0x2000C000;    /* end of AHB16 RAM */

/* Generate a link error if heap and stack don't fit into RAM */
_Min_Heap_Size = 0x2000;      /* required amount of heap  */
_Min_Stack_Size = 0x2000; /* required amount of stack */

/* Specify the memory areas */
MEMORY
{
  FLASH_A (rx)   : ORIGIN = 0x1A000000, LENGTH = 512K
  FLASH_B (rx) 	 : ORIGIN = 0x1B000000, LENGTH = 512K
  RAMLOC32 (xrw)  : ORIGIN = 0x10000000, LENGTH = 32K
  RAMLOC40 (xrw)  : ORIGIN = 0x10080000, LENGTH = 40K
  RAMAHB16 (xrw)  : ORIGIN = 0x20008000, LENGTH = 16K
  RAMAHB32 (xrw)  : ORIGIN = 0x20000000, LENGTH = 32K
  RAMETB16 (xrw)  : ORIGIN = 0x2000C000, LENGTH = 16K
  /*OSHEAP  (xrw)   : ORIGIN = 0x28000000, LENGTH = 256K*/ /* 256K of OS heap*/
  EXTSRAM (xrw)   : ORIGIN = 0x28000000, LENGTH = 32M /* 31768K of general purpose RAM */
}

/* Define output sections */
SECTIONS
{
  /* The startup code goes first into FLASH */
  .isr_vector :
  {
    . = ALIGN(4);
    _isr_vector = .;     /* define isr_vector start address */
    KEEP(*(.isr_vector)) /* Startup code */
    . = ALIGN(4);
  } >FLASH_A

  /* The program code and other data goes into FLASH */
  .text :
  {
    . = ALIGN(4);
	*(.after_vectors*) /* .after_vectors section */
    *(.text)           /* .text sections (code) */
    *(.text*)          /* .text* sections (code) */
    *(.rodata)         /* .rodata sections (constants, strings, etc.) */
    *(.rodata*)        /* .rodata* sections (constants, strings, etc.) */
    *(.glue_7)         /* glue arm to thumb code */
    *(.glue_7t)        /* glue thumb to arm code */
    *(.eh_frame)

    KEEP (*(.init))
    KEEP (*(.fini))

    . = ALIGN(4);
    _etext = .;        /* define a global symbols at end of code */
  } >FLASH_A


   .ARM.extab   : { *(.ARM.extab* .gnu.linkonce.armextab.*) } >FLASH_A
    .ARM : {
    __exidx_start = .;
      *(.ARM.exidx*)
      __exidx_end = .;
    } >FLASH_A

  .preinit_array     :
  {
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array*))
    PROVIDE_HIDDEN (__preinit_array_end = .);
  } >FLASH_A
  .init_array :
  {
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array*))
    PROVIDE_HIDDEN (__init_array_end = .);
  } >FLASH_A
  .fini_array :
  {
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(SORT(.fini_array.*)))
    KEEP (*(.fini_array*))
    PROVIDE_HIDDEN (__fini_array_end = .);
  } >FLASH_A

  /* used by the startup to initialize data */
  _sidata = LOADADDR(.data);

  /* Initialized data sections goes into RAM, load LMA copy after code */
  .data : 
  {
    . = ALIGN(4);
    _sdata = .;        /* create a global symbol at data start */
    *(.data)           /* .data sections */
    *(.data*)          /* .data* sections */
    . = ALIGN(4);
    _edata = .;        /* define a global symbol at data end */
  } > RAMLOC40 AT> FLASH_A /*RAMAHB32 AT> FLASH_A*/

  /* Uninitialized data section */
  . = ALIGN(4);
  .bss :
  {
    /* This is used by the startup in order to initialize the .bss secion */
    _sbss = .;         /* define a global symbol at bss start */
    __bss_start__ = _sbss;
    *(.bss)
    *(.bss*)
    *(COMMON)

    . = ALIGN(4);
    _ebss = .;         /* define a global symbol at bss end */
    __bss_end__ = _ebss;
  } >EXTSRAM /*RAMLOC40*/ /* RAMLOC32 */

  /* User_heap_stack section, used to check that there is enough RAM left */
  ._user_heap_stack :
  {
    . = ALIGN(4);
    . = . + _Min_Heap_Size;
    . = . + _Min_Stack_Size;
    . = ALIGN(4);
  } >RAMAHB16

  /* MEMORY_bank1 section, code must be located here explicitly            */
  /* Example: extern int foo(void) __attribute__ ((section (".osheap"))); */
  .memory_osheap :
  {
    *(.osheap)        /* .osheap sections */
    *(.osheap*)       /* .osheap* sections */
  } > RAMLOC32 /*EXTSRAM*/
  
  PROVIDE(__HEAP_START = ORIGIN(RAMAHB16));
  
  /* MEMORY_bank1 section, code must be located here explicitly            */
  /* Example: extern int foo(void) __attribute__ ((section (".osheap"))); */
  .memory_extsram :
  {
    *(.extram)        /* .mb1text sections (code) */
    *(.extram*)       /* .mb1text* sections (code)  */
  } >EXTSRAM

  /* Remove information from the standard libraries */
  /DISCARD/ :
  {
    libc.a ( * )
    libm.a ( * )
    libgcc.a ( * )
  }

  .ARM.attributes 0 : { *(.ARM.attributes) }
}