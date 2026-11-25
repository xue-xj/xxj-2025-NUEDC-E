ENTRY(main)
MEMORY { FLASH (rx) : ORIGIN = 0x00000000, LENGTH = 128K
         RAM (rwx) : ORIGIN = 0x20200000, LENGTH = 32K }
SECTIONS {
    .text : { *(.text*) *(.rodata*) } > FLASH
    .ARM.exidx : { *(.ARM.exidx*) } > FLASH
    .data : { *(.data*) } > RAM AT > FLASH
    .bss (NOLOAD) : { *(.bss*) *(COMMON) } > RAM
}
