ENTRY(Reset_Handler)

/* defined in oxc_base.h: extern  const int _sdata, _edata, _sbss, _ebss, _end, _estack */

_estack = ORIGIN(RAM) + _RAM_SIZE;  /* stack starts at over_end of RAM */
/* to check segment sizes */
_Min_Heap_Size = 0x400;
_Min_Stack_Size = 0x500;


SECTIONS
{
  /* FLASH start: IRQ (irs) vectors */
  .isr_vector :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector))
    . = ALIGN(4);
  } >FLASH

  .text :
  {
    . = ALIGN(4);
    *(.text)       /* .text sections (code)          */
    *(.text*)      /* .text* sections (splited code) */
    *(.glue_7)     /* unused glue arm to thumb       */
    *(.glue_7t)    /* unused glue thumb to arm       */
    *(.eh_frame)   /* C++ exceptions handlers        */

    KEEP (*(.init))
    KEEP (*(.fini))

    . = ALIGN(4);
    _etext = . ;        /* end of code */
  } >FLASH

  .rodata :
  {
    . = ALIGN(4);
    *(.rodata)
    *(.rodata*)
    . = ALIGN(4);
  } >FLASH

  /* aux sentions for r/o data: auto-registering ....*/
  .rodata_aux0 :
  {
    . = ALIGN(4);
    *(.rodata_aux0)
    *(.rodata_aux0*)
    . = ALIGN(4);
  } >FLASH

  .rodata_aux1 :
  {
    . = ALIGN(4);
    *(.rodata_aux1)
    *(.rodata_aux1*)
    . = ALIGN(4);
  } >FLASH

  .ARM.extab : { *(.ARM.extab* .gnu.linkonce.armextab.*) } >FLASH /* unused? */
  .ARM : {
    __exidx_start = .;
    *(.ARM.exidx*)
    __exidx_end = .;
  } >FLASH

  .preinit_array :
  {
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array*))
    PROVIDE_HIDDEN (__preinit_array_end = .);
  } >FLASH

  .init_array :
  {
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array*))
    PROVIDE_HIDDEN (__init_array_end = .);
  } >FLASH

  .fini_array :
  {
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(SORT(.fini_array.*)))
    KEEP (*(.fini_array*))
    PROVIDE_HIDDEN (__fini_array_end = .);
  } >FLASH

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
    _edata = .;        /* data end */
  } >RAM AT> FLASH

  _siccmram = LOADADDR(.ccmram);

  .ccmram :
  {
    . = ALIGN(4);
    _sccmram = .;
    *(.ccmram)
    *(.ccmram*)

    . = ALIGN(4);
    _eccmram = .;
  } >CCMRAM AT> FLASH

  /* Classic uninitialized data (BSS) */
  . = ALIGN(4);
  .bss :
  {
    /* This is used by the startup in order to initialize the .bss secion */
    _sbss = .;             /* bss start */
    __bss_start__ = _sbss; /* alias to _sbss */
    *(.bss)
    *(.bss*)
    *(COMMON)

    . = ALIGN(4);
    _ebss = .;           /* bss end */
    __bss_end__ = _ebss; /* alias to _ebss */
  } >RAM

  ._user_heap_stack :
  {
    . = ALIGN(8);
    PROVIDE ( end = . );
    PROVIDE ( _end = . );
    . = . + _Min_Heap_Size;
    . = . + _Min_Stack_Size;
    . = ALIGN(8);
  } >RAM


  /* Remove debug information from the common libraries */
  /DISCARD/ :
  {
    libgcc.a ( * )
    libc.a ( * )
    libm.a ( * )
  }

  .ARM.attributes 0 : { *(.ARM.attributes) }
}


