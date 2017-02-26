# SmartFoodTrackerDE2  
Quartus hardware configuration and associated NiosII software projects for the Altera DE2 development platform.  

## Usage  
There are no settings commited for flashing these projects permenantly. To temporarily run these projects on your DE2 board launch Quartus 12.1:  
```
$ ./launch_quartus.sh
```
From Quartus compile the hardware configuration and program the board with the generated *.sof file.  

Next, launch the Eclipse development environment and import all projects from /software/. "Generate BSP" and compile the board support packages, compile given project and Run as Nios II hardware throught the JTAG programmer.  

## Overview of Repository  
`config/`: Miscellaneous environment settings suitable for developing this project.  
`hardware/`: All required hardware for the project (Qsys config, vhdl top-level, third-party verilog, etc.).  
`software/`: All MicroC/OS-II software projects (for more information about these projects, view their readmes).  