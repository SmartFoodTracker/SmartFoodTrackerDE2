#!/bin/sh
#
# This file was automatically generated.
#
# It can be overwritten by nios2-flash-programmer-generate or nios2-flash-programmer-gui.
#

#
# Converting ELF File: /afs/ualberta.ca/home/k/o/koshaugh/SmartFoodTrackerDE2/software/Capstone-FIT/Capstone-FIT.elf to: "../flash/Capstone-FIT_tristate_controller.flash"
#
elf2flash --input="/afs/ualberta.ca/home/k/o/koshaugh/SmartFoodTrackerDE2/software/Capstone-FIT/Capstone-FIT.elf" --output="../flash/Capstone-FIT_tristate_controller.flash" --boot="$SOPC_KIT_NIOS2/components/altera_nios2/boot_loader_cfi.srec" --base=0x1400000 --end=0x1800000 --reset=0x1400000 --verbose 

#
# Programming File: "../flash/Capstone-FIT_tristate_controller.flash" To Device: tristate_controller
#
nios2-flash-programmer "../flash/Capstone-FIT_tristate_controller.flash" --base=0x1400000 --sidp=0x19090B8 --id=0x0 --timestamp=1490819245 --device=1 --instance=0 '--cable=USB-Blaster on localhost [1-1.3]' --program --verbose 

