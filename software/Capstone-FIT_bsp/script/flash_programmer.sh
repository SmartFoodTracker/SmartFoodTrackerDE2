#!/bin/sh
#
# This file was automatically generated.
#
# It can be overwritten by nios2-flash-programmer-generate or nios2-flash-programmer-gui.
#

#
# Converting ELF File: Capstone-FIT.elf to: "../flash/Capstone-FIT_tristate_controller.flash"
#
elf2flash --input="Capstone-FIT.elf" --output="../flash/Capstone-FIT_tristate_controller.flash" --boot="$SOPC_KIT_NIOS2/components/altera_nios2/boot_loader_cfi.srec" --base=0x1400000 --end=0x1800000 --reset=0x1400000 

#
# Programming File: "../flash/Capstone-FIT_tristate_controller.flash" To Device: tristate_controller
#
nios2-flash-programmer "../flash/Capstone-FIT_tristate_controller.flash" --base=0x1400000 --sidp=0x19090B8 --id=0x0 --timestamp=1486160258 --device=1 --instance=0 '--cable=USB-Blaster on localhost [2-1.7]' --program 

