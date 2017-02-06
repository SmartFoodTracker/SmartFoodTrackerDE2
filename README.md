# SmartFoodTrackerDE2
Hardware configuration and associated MicroC/OS-II software projects for the Altera DE2 development platform

## Usage
There are no settings commited for flashing these projects permenantly. To temporarily run these projects on your DE2 board launch Quartus 12.1:
```
$ ./launch_quartus.sh
```
From Quartus compile the hardware configuration and program the board with the generated *.sof file

Next, launch the Eclipse development environment and import a project from /software/... and its associated *_bsp project. Generate the bsp from the _bsp project, compile both projects and Run as Nios II hardware throught the JTAG programmer.

## Overview of Repository  

`hardware/`: all required hardware for the project (Qsys config, vhdl top-level, third party verilog, etc.)  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;`DM9000A_IF.v` and `DM9000A_IF_hw.tcl`: Verilog and generation script for our ethernet component (source: _____)  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;`SmartFoodTrackerDE2.vhd`: Top level vhdl component representing our Qsys configuration  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;`niosII_system.qsys`: Qsys configuration

`software/`: all MicroC/OS-II software projects (for more information about these projects, view their readmes)
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;`Capstone-FIT/`: Bradshaw's ethernet testing project, work in progress  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;`barcode_scanner/`: Kyle's barcode scanner test project
