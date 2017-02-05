/*
 * system.h - SOPC Builder system and BSP software package information
 *
 * Machine generated for CPU 'nios2_qsys' in SOPC Builder design 'niosII_system'
 * SOPC Builder design path: ../../niosII_system.sopcinfo
 *
 * Generated: Sat Feb 04 11:34:12 MST 2017
 */

/*
 * DO NOT MODIFY THIS FILE
 *
 * Changing this file will have subtle consequences
 * which will almost certainly lead to a nonfunctioning
 * system. If you do modify this file, be aware that your
 * changes will be overwritten and lost when this file
 * is generated again.
 *
 * DO NOT MODIFY THIS FILE
 */

/*
 * License Agreement
 *
 * Copyright (c) 2008
 * Altera Corporation, San Jose, California, USA.
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * This agreement shall be governed in all respects by the laws of the State
 * of California and by the laws of the United States of America.
 */

#ifndef __SYSTEM_H_
#define __SYSTEM_H_

/* Include definitions from linker script generator */
#include "linker.h"


/*
 * CPU configuration
 *
 */

#define ALT_CPU_ARCHITECTURE "altera_nios2_qsys"
#define ALT_CPU_BIG_ENDIAN 0
#define ALT_CPU_BREAK_ADDR 0x1908820
#define ALT_CPU_CPU_FREQ 50000000u
#define ALT_CPU_CPU_ID_SIZE 1
#define ALT_CPU_CPU_ID_VALUE 0x00000000
#define ALT_CPU_CPU_IMPLEMENTATION "small"
#define ALT_CPU_DATA_ADDR_WIDTH 0x19
#define ALT_CPU_DCACHE_LINE_SIZE 0
#define ALT_CPU_DCACHE_LINE_SIZE_LOG2 0
#define ALT_CPU_DCACHE_SIZE 0
#define ALT_CPU_EXCEPTION_ADDR 0x1904020
#define ALT_CPU_FLUSHDA_SUPPORTED
#define ALT_CPU_FREQ 50000000
#define ALT_CPU_HARDWARE_DIVIDE_PRESENT 0
#define ALT_CPU_HARDWARE_MULTIPLY_PRESENT 1
#define ALT_CPU_HARDWARE_MULX_PRESENT 0
#define ALT_CPU_HAS_DEBUG_CORE 1
#define ALT_CPU_HAS_DEBUG_STUB
#define ALT_CPU_HAS_JMPI_INSTRUCTION
#define ALT_CPU_ICACHE_LINE_SIZE 32
#define ALT_CPU_ICACHE_LINE_SIZE_LOG2 5
#define ALT_CPU_ICACHE_SIZE 4096
#define ALT_CPU_INST_ADDR_WIDTH 0x19
#define ALT_CPU_NAME "nios2_qsys"
#define ALT_CPU_RESET_ADDR 0x1904000


/*
 * CPU configuration (with legacy prefix - don't use these anymore)
 *
 */

#define NIOS2_BIG_ENDIAN 0
#define NIOS2_BREAK_ADDR 0x1908820
#define NIOS2_CPU_FREQ 50000000u
#define NIOS2_CPU_ID_SIZE 1
#define NIOS2_CPU_ID_VALUE 0x00000000
#define NIOS2_CPU_IMPLEMENTATION "small"
#define NIOS2_DATA_ADDR_WIDTH 0x19
#define NIOS2_DCACHE_LINE_SIZE 0
#define NIOS2_DCACHE_LINE_SIZE_LOG2 0
#define NIOS2_DCACHE_SIZE 0
#define NIOS2_EXCEPTION_ADDR 0x1904020
#define NIOS2_FLUSHDA_SUPPORTED
#define NIOS2_HARDWARE_DIVIDE_PRESENT 0
#define NIOS2_HARDWARE_MULTIPLY_PRESENT 1
#define NIOS2_HARDWARE_MULX_PRESENT 0
#define NIOS2_HAS_DEBUG_CORE 1
#define NIOS2_HAS_DEBUG_STUB
#define NIOS2_HAS_JMPI_INSTRUCTION
#define NIOS2_ICACHE_LINE_SIZE 32
#define NIOS2_ICACHE_LINE_SIZE_LOG2 5
#define NIOS2_ICACHE_SIZE 4096
#define NIOS2_INST_ADDR_WIDTH 0x19
#define NIOS2_RESET_ADDR 0x1904000


/*
 * Define for each module class mastered by the CPU
 *
 */

#define __ALTERA_AVALON_JTAG_UART
#define __ALTERA_AVALON_NEW_SDRAM_CONTROLLER
#define __ALTERA_AVALON_ONCHIP_MEMORY2
#define __ALTERA_AVALON_PIO
#define __ALTERA_AVALON_SYSID_QSYS
#define __ALTERA_AVALON_TIMER
#define __ALTERA_GENERIC_TRISTATE_CONTROLLER
#define __ALTERA_NIOS2_QSYS
#define __ALTERA_UP_AVALON_AUDIO
#define __ALTERA_UP_AVALON_AUDIO_AND_VIDEO_CONFIG
#define __ALTERA_UP_AVALON_CHARACTER_LCD
#define __ALTERA_UP_AVALON_PS2
#define __ALTERA_UP_AVALON_SRAM
#define __DM9000A_IF


/*
 * System configuration
 *
 */

#define ALT_DEVICE_FAMILY "Cyclone II"
#define ALT_IRQ_BASE NULL
#define ALT_LEGACY_INTERRUPT_API_PRESENT
#define ALT_LOG_PORT "/dev/null"
#define ALT_LOG_PORT_BASE 0x0
#define ALT_LOG_PORT_DEV null
#define ALT_LOG_PORT_TYPE ""
#define ALT_NUM_EXTERNAL_INTERRUPT_CONTROLLERS 0
#define ALT_NUM_INTERNAL_INTERRUPT_CONTROLLERS 1
#define ALT_NUM_INTERRUPT_CONTROLLERS 1
#define ALT_STDERR "/dev/jtag_uart"
#define ALT_STDERR_BASE 0x19090b0
#define ALT_STDERR_DEV jtag_uart
#define ALT_STDERR_IS_JTAG_UART
#define ALT_STDERR_PRESENT
#define ALT_STDERR_TYPE "altera_avalon_jtag_uart"
#define ALT_STDIN "/dev/jtag_uart"
#define ALT_STDIN_BASE 0x19090b0
#define ALT_STDIN_DEV jtag_uart
#define ALT_STDIN_IS_JTAG_UART
#define ALT_STDIN_PRESENT
#define ALT_STDIN_TYPE "altera_avalon_jtag_uart"
#define ALT_STDOUT "/dev/jtag_uart"
#define ALT_STDOUT_BASE 0x19090b0
#define ALT_STDOUT_DEV jtag_uart
#define ALT_STDOUT_IS_JTAG_UART
#define ALT_STDOUT_PRESENT
#define ALT_STDOUT_TYPE "altera_avalon_jtag_uart"
#define ALT_SYSTEM_NAME "niosII_system"


/*
 * add_button configuration
 *
 */

#define ADD_BUTTON_BASE 0x1909070
#define ADD_BUTTON_BIT_CLEARING_EDGE_REGISTER 0
#define ADD_BUTTON_BIT_MODIFYING_OUTPUT_REGISTER 0
#define ADD_BUTTON_CAPTURE 0
#define ADD_BUTTON_DATA_WIDTH 1
#define ADD_BUTTON_DO_TEST_BENCH_WIRING 0
#define ADD_BUTTON_DRIVEN_SIM_VALUE 0x0
#define ADD_BUTTON_EDGE_TYPE "NONE"
#define ADD_BUTTON_FREQ 50000000u
#define ADD_BUTTON_HAS_IN 1
#define ADD_BUTTON_HAS_OUT 0
#define ADD_BUTTON_HAS_TRI 0
#define ADD_BUTTON_IRQ 3
#define ADD_BUTTON_IRQ_INTERRUPT_CONTROLLER_ID 0
#define ADD_BUTTON_IRQ_TYPE "LEVEL"
#define ADD_BUTTON_NAME "/dev/add_button"
#define ADD_BUTTON_RESET_VALUE 0x0
#define ADD_BUTTON_SPAN 16
#define ADD_BUTTON_TYPE "altera_avalon_pio"
#define ALT_MODULE_CLASS_add_button altera_avalon_pio


/*
 * audio_config configuration
 *
 */

#define ALT_MODULE_CLASS_audio_config altera_up_avalon_audio_and_video_config
#define AUDIO_CONFIG_BASE 0x1909020
#define AUDIO_CONFIG_IRQ -1
#define AUDIO_CONFIG_IRQ_INTERRUPT_CONTROLLER_ID -1
#define AUDIO_CONFIG_NAME "/dev/audio_config"
#define AUDIO_CONFIG_SPAN 16
#define AUDIO_CONFIG_TYPE "altera_up_avalon_audio_and_video_config"


/*
 * audio_core configuration
 *
 */

#define ALT_MODULE_CLASS_audio_core altera_up_avalon_audio
#define AUDIO_CORE_BASE 0x1909030
#define AUDIO_CORE_IRQ 7
#define AUDIO_CORE_IRQ_INTERRUPT_CONTROLLER_ID 0
#define AUDIO_CORE_NAME "/dev/audio_core"
#define AUDIO_CORE_SPAN 16
#define AUDIO_CORE_TYPE "altera_up_avalon_audio"


/*
 * barcode_scanner_ps2 configuration
 *
 */

#define ALT_MODULE_CLASS_barcode_scanner_ps2 altera_up_avalon_ps2
#define BARCODE_SCANNER_PS2_BASE 0x19090a8
#define BARCODE_SCANNER_PS2_IRQ 6
#define BARCODE_SCANNER_PS2_IRQ_INTERRUPT_CONTROLLER_ID 0
#define BARCODE_SCANNER_PS2_NAME "/dev/barcode_scanner_ps2"
#define BARCODE_SCANNER_PS2_SPAN 8
#define BARCODE_SCANNER_PS2_TYPE "altera_up_avalon_ps2"


/*
 * cancel_button configuration
 *
 */

#define ALT_MODULE_CLASS_cancel_button altera_avalon_pio
#define CANCEL_BUTTON_BASE 0x1909050
#define CANCEL_BUTTON_BIT_CLEARING_EDGE_REGISTER 0
#define CANCEL_BUTTON_BIT_MODIFYING_OUTPUT_REGISTER 0
#define CANCEL_BUTTON_CAPTURE 0
#define CANCEL_BUTTON_DATA_WIDTH 1
#define CANCEL_BUTTON_DO_TEST_BENCH_WIRING 0
#define CANCEL_BUTTON_DRIVEN_SIM_VALUE 0x0
#define CANCEL_BUTTON_EDGE_TYPE "NONE"
#define CANCEL_BUTTON_FREQ 50000000u
#define CANCEL_BUTTON_HAS_IN 1
#define CANCEL_BUTTON_HAS_OUT 0
#define CANCEL_BUTTON_HAS_TRI 0
#define CANCEL_BUTTON_IRQ 5
#define CANCEL_BUTTON_IRQ_INTERRUPT_CONTROLLER_ID 0
#define CANCEL_BUTTON_IRQ_TYPE "LEVEL"
#define CANCEL_BUTTON_NAME "/dev/cancel_button"
#define CANCEL_BUTTON_RESET_VALUE 0x0
#define CANCEL_BUTTON_SPAN 16
#define CANCEL_BUTTON_TYPE "altera_avalon_pio"


/*
 * character_lcd configuration
 *
 */

#define ALT_MODULE_CLASS_character_lcd altera_up_avalon_character_lcd
#define CHARACTER_LCD_BASE 0x19090c0
#define CHARACTER_LCD_IRQ -1
#define CHARACTER_LCD_IRQ_INTERRUPT_CONTROLLER_ID -1
#define CHARACTER_LCD_NAME "/dev/character_lcd"
#define CHARACTER_LCD_SPAN 2
#define CHARACTER_LCD_TYPE "altera_up_avalon_character_lcd"


/*
 * dm9000a_if_ethernet configuration
 *
 */

#define ALT_MODULE_CLASS_dm9000a_if_ethernet DM9000A_IF
#define DM9000A_IF_ETHERNET_BASE 0x19090a0
#define DM9000A_IF_ETHERNET_IRQ 8
#define DM9000A_IF_ETHERNET_IRQ_INTERRUPT_CONTROLLER_ID 0
#define DM9000A_IF_ETHERNET_NAME "/dev/dm9000a_if_ethernet"
#define DM9000A_IF_ETHERNET_SPAN 8
#define DM9000A_IF_ETHERNET_TYPE "DM9000A_IF"


/*
 * green_leds configuration
 *
 */

#define ALT_MODULE_CLASS_green_leds altera_avalon_pio
#define GREEN_LEDS_BASE 0x1909090
#define GREEN_LEDS_BIT_CLEARING_EDGE_REGISTER 0
#define GREEN_LEDS_BIT_MODIFYING_OUTPUT_REGISTER 0
#define GREEN_LEDS_CAPTURE 0
#define GREEN_LEDS_DATA_WIDTH 8
#define GREEN_LEDS_DO_TEST_BENCH_WIRING 0
#define GREEN_LEDS_DRIVEN_SIM_VALUE 0x0
#define GREEN_LEDS_EDGE_TYPE "NONE"
#define GREEN_LEDS_FREQ 50000000u
#define GREEN_LEDS_HAS_IN 0
#define GREEN_LEDS_HAS_OUT 1
#define GREEN_LEDS_HAS_TRI 0
#define GREEN_LEDS_IRQ -1
#define GREEN_LEDS_IRQ_INTERRUPT_CONTROLLER_ID -1
#define GREEN_LEDS_IRQ_TYPE "NONE"
#define GREEN_LEDS_NAME "/dev/green_leds"
#define GREEN_LEDS_RESET_VALUE 0x0
#define GREEN_LEDS_SPAN 16
#define GREEN_LEDS_TYPE "altera_avalon_pio"


/*
 * hal configuration
 *
 */

#define ALT_MAX_FD 32
#define ALT_SYS_CLK TIMER
#define ALT_TIMESTAMP_CLK none


/*
 * jtag_uart configuration
 *
 */

#define ALT_MODULE_CLASS_jtag_uart altera_avalon_jtag_uart
#define JTAG_UART_BASE 0x19090b0
#define JTAG_UART_IRQ 1
#define JTAG_UART_IRQ_INTERRUPT_CONTROLLER_ID 0
#define JTAG_UART_NAME "/dev/jtag_uart"
#define JTAG_UART_READ_DEPTH 64
#define JTAG_UART_READ_THRESHOLD 8
#define JTAG_UART_SPAN 8
#define JTAG_UART_TYPE "altera_avalon_jtag_uart"
#define JTAG_UART_WRITE_DEPTH 64
#define JTAG_UART_WRITE_THRESHOLD 8


/*
 * onchip_memory2 configuration
 *
 */

#define ALT_MODULE_CLASS_onchip_memory2 altera_avalon_onchip_memory2
#define ONCHIP_MEMORY2_ALLOW_IN_SYSTEM_MEMORY_CONTENT_EDITOR 0
#define ONCHIP_MEMORY2_ALLOW_MRAM_SIM_CONTENTS_ONLY_FILE 0
#define ONCHIP_MEMORY2_BASE 0x1904000
#define ONCHIP_MEMORY2_CONTENTS_INFO ""
#define ONCHIP_MEMORY2_DUAL_PORT 0
#define ONCHIP_MEMORY2_GUI_RAM_BLOCK_TYPE "Automatic"
#define ONCHIP_MEMORY2_INIT_CONTENTS_FILE "onchip_memory2"
#define ONCHIP_MEMORY2_INIT_MEM_CONTENT 1
#define ONCHIP_MEMORY2_INSTANCE_ID "NONE"
#define ONCHIP_MEMORY2_IRQ -1
#define ONCHIP_MEMORY2_IRQ_INTERRUPT_CONTROLLER_ID -1
#define ONCHIP_MEMORY2_NAME "/dev/onchip_memory2"
#define ONCHIP_MEMORY2_NON_DEFAULT_INIT_FILE_ENABLED 0
#define ONCHIP_MEMORY2_RAM_BLOCK_TYPE "Auto"
#define ONCHIP_MEMORY2_READ_DURING_WRITE_MODE "DONT_CARE"
#define ONCHIP_MEMORY2_SINGLE_CLOCK_OP 0
#define ONCHIP_MEMORY2_SIZE_MULTIPLE 1
#define ONCHIP_MEMORY2_SIZE_VALUE 16384u
#define ONCHIP_MEMORY2_SPAN 16384
#define ONCHIP_MEMORY2_TYPE "altera_avalon_onchip_memory2"
#define ONCHIP_MEMORY2_WRITABLE 1


/*
 * red_leds configuration
 *
 */

#define ALT_MODULE_CLASS_red_leds altera_avalon_pio
#define RED_LEDS_BASE 0x1909040
#define RED_LEDS_BIT_CLEARING_EDGE_REGISTER 0
#define RED_LEDS_BIT_MODIFYING_OUTPUT_REGISTER 0
#define RED_LEDS_CAPTURE 0
#define RED_LEDS_DATA_WIDTH 8
#define RED_LEDS_DO_TEST_BENCH_WIRING 0
#define RED_LEDS_DRIVEN_SIM_VALUE 0x0
#define RED_LEDS_EDGE_TYPE "NONE"
#define RED_LEDS_FREQ 50000000u
#define RED_LEDS_HAS_IN 0
#define RED_LEDS_HAS_OUT 1
#define RED_LEDS_HAS_TRI 0
#define RED_LEDS_IRQ -1
#define RED_LEDS_IRQ_INTERRUPT_CONTROLLER_ID -1
#define RED_LEDS_IRQ_TYPE "NONE"
#define RED_LEDS_NAME "/dev/red_leds"
#define RED_LEDS_RESET_VALUE 0x0
#define RED_LEDS_SPAN 16
#define RED_LEDS_TYPE "altera_avalon_pio"


/*
 * remove_button configuration
 *
 */

#define ALT_MODULE_CLASS_remove_button altera_avalon_pio
#define REMOVE_BUTTON_BASE 0x1909060
#define REMOVE_BUTTON_BIT_CLEARING_EDGE_REGISTER 0
#define REMOVE_BUTTON_BIT_MODIFYING_OUTPUT_REGISTER 0
#define REMOVE_BUTTON_CAPTURE 0
#define REMOVE_BUTTON_DATA_WIDTH 1
#define REMOVE_BUTTON_DO_TEST_BENCH_WIRING 0
#define REMOVE_BUTTON_DRIVEN_SIM_VALUE 0x0
#define REMOVE_BUTTON_EDGE_TYPE "NONE"
#define REMOVE_BUTTON_FREQ 50000000u
#define REMOVE_BUTTON_HAS_IN 1
#define REMOVE_BUTTON_HAS_OUT 0
#define REMOVE_BUTTON_HAS_TRI 0
#define REMOVE_BUTTON_IRQ 4
#define REMOVE_BUTTON_IRQ_INTERRUPT_CONTROLLER_ID 0
#define REMOVE_BUTTON_IRQ_TYPE "LEVEL"
#define REMOVE_BUTTON_NAME "/dev/remove_button"
#define REMOVE_BUTTON_RESET_VALUE 0x0
#define REMOVE_BUTTON_SPAN 16
#define REMOVE_BUTTON_TYPE "altera_avalon_pio"


/*
 * sdram configuration
 *
 */

#define ALT_MODULE_CLASS_sdram altera_avalon_new_sdram_controller
#define SDRAM_BASE 0x800000
#define SDRAM_CAS_LATENCY 3
#define SDRAM_CONTENTS_INFO ""
#define SDRAM_INIT_NOP_DELAY 0.0
#define SDRAM_INIT_REFRESH_COMMANDS 2
#define SDRAM_IRQ -1
#define SDRAM_IRQ_INTERRUPT_CONTROLLER_ID -1
#define SDRAM_IS_INITIALIZED 1
#define SDRAM_NAME "/dev/sdram"
#define SDRAM_POWERUP_DELAY 100.0
#define SDRAM_REFRESH_PERIOD 15.625
#define SDRAM_REGISTER_DATA_IN 1
#define SDRAM_SDRAM_ADDR_WIDTH 0x16
#define SDRAM_SDRAM_BANK_WIDTH 2
#define SDRAM_SDRAM_COL_WIDTH 8
#define SDRAM_SDRAM_DATA_WIDTH 16
#define SDRAM_SDRAM_NUM_BANKS 4
#define SDRAM_SDRAM_NUM_CHIPSELECTS 1
#define SDRAM_SDRAM_ROW_WIDTH 12
#define SDRAM_SHARED_DATA 0
#define SDRAM_SIM_MODEL_BASE 0
#define SDRAM_SPAN 8388608
#define SDRAM_STARVATION_INDICATOR 0
#define SDRAM_TRISTATE_BRIDGE_SLAVE ""
#define SDRAM_TYPE "altera_avalon_new_sdram_controller"
#define SDRAM_T_AC 5.5
#define SDRAM_T_MRD 3
#define SDRAM_T_RCD 20.0
#define SDRAM_T_RFC 70.0
#define SDRAM_T_RP 20.0
#define SDRAM_T_WR 14.0


/*
 * sram configuration
 *
 */

#define ALT_MODULE_CLASS_sram altera_up_avalon_sram
#define SRAM_BASE 0x1880000
#define SRAM_IRQ -1
#define SRAM_IRQ_INTERRUPT_CONTROLLER_ID -1
#define SRAM_NAME "/dev/sram"
#define SRAM_SPAN 524288
#define SRAM_TYPE "altera_up_avalon_sram"


/*
 * switch configuration
 *
 */

#define ALT_MODULE_CLASS_switch altera_avalon_pio
#define SWITCH_BASE 0x1909080
#define SWITCH_BIT_CLEARING_EDGE_REGISTER 1
#define SWITCH_BIT_MODIFYING_OUTPUT_REGISTER 0
#define SWITCH_CAPTURE 1
#define SWITCH_DATA_WIDTH 1
#define SWITCH_DO_TEST_BENCH_WIRING 0
#define SWITCH_DRIVEN_SIM_VALUE 0x0
#define SWITCH_EDGE_TYPE "ANY"
#define SWITCH_FREQ 50000000u
#define SWITCH_HAS_IN 1
#define SWITCH_HAS_OUT 0
#define SWITCH_HAS_TRI 0
#define SWITCH_IRQ 2
#define SWITCH_IRQ_INTERRUPT_CONTROLLER_ID 0
#define SWITCH_IRQ_TYPE "EDGE"
#define SWITCH_NAME "/dev/switch"
#define SWITCH_RESET_VALUE 0x0
#define SWITCH_SPAN 16
#define SWITCH_TYPE "altera_avalon_pio"


/*
 * sysid_qsys configuration
 *
 */

#define ALT_MODULE_CLASS_sysid_qsys altera_avalon_sysid_qsys
#define SYSID_QSYS_BASE 0x19090b8
#define SYSID_QSYS_ID 0
#define SYSID_QSYS_IRQ -1
#define SYSID_QSYS_IRQ_INTERRUPT_CONTROLLER_ID -1
#define SYSID_QSYS_NAME "/dev/sysid_qsys"
#define SYSID_QSYS_SPAN 8
#define SYSID_QSYS_TIMESTAMP 1486233023
#define SYSID_QSYS_TYPE "altera_avalon_sysid_qsys"


/*
 * timer configuration
 *
 */

#define ALT_MODULE_CLASS_timer altera_avalon_timer
#define TIMER_ALWAYS_RUN 0
#define TIMER_BASE 0x1909000
#define TIMER_COUNTER_SIZE 32
#define TIMER_FIXED_PERIOD 0
#define TIMER_FREQ 50000000u
#define TIMER_IRQ 0
#define TIMER_IRQ_INTERRUPT_CONTROLLER_ID 0
#define TIMER_LOAD_VALUE 49999ull
#define TIMER_MULT 0.0010
#define TIMER_NAME "/dev/timer"
#define TIMER_PERIOD 1
#define TIMER_PERIOD_UNITS "ms"
#define TIMER_RESET_OUTPUT 0
#define TIMER_SNAPSHOT 1
#define TIMER_SPAN 32
#define TIMER_TICKS_PER_SEC 1000u
#define TIMER_TIMEOUT_PULSE_OUTPUT 0
#define TIMER_TYPE "altera_avalon_timer"


/*
 * tristate_controller configuration
 *
 */

#define ALT_MODULE_CLASS_tristate_controller altera_generic_tristate_controller
#define TRISTATE_CONTROLLER_BASE 0x1400000
#define TRISTATE_CONTROLLER_IRQ -1
#define TRISTATE_CONTROLLER_IRQ_INTERRUPT_CONTROLLER_ID -1
#define TRISTATE_CONTROLLER_NAME "/dev/tristate_controller"
#define TRISTATE_CONTROLLER_SPAN 4194304
#define TRISTATE_CONTROLLER_TYPE "altera_generic_tristate_controller"


/*
 * ucosii configuration
 *
 */

#define OS_ARG_CHK_EN 1
#define OS_CPU_HOOKS_EN 1
#define OS_DEBUG_EN 1
#define OS_EVENT_NAME_SIZE 32
#define OS_FLAGS_NBITS 16
#define OS_FLAG_ACCEPT_EN 1
#define OS_FLAG_DEL_EN 1
#define OS_FLAG_EN 1
#define OS_FLAG_NAME_SIZE 32
#define OS_FLAG_QUERY_EN 1
#define OS_FLAG_WAIT_CLR_EN 1
#define OS_LOWEST_PRIO 20
#define OS_MAX_EVENTS 60
#define OS_MAX_FLAGS 20
#define OS_MAX_MEM_PART 60
#define OS_MAX_QS 20
#define OS_MAX_TASKS 10
#define OS_MBOX_ACCEPT_EN 1
#define OS_MBOX_DEL_EN 1
#define OS_MBOX_EN 1
#define OS_MBOX_POST_EN 1
#define OS_MBOX_POST_OPT_EN 1
#define OS_MBOX_QUERY_EN 1
#define OS_MEM_EN 1
#define OS_MEM_NAME_SIZE 32
#define OS_MEM_QUERY_EN 1
#define OS_MUTEX_ACCEPT_EN 1
#define OS_MUTEX_DEL_EN 1
#define OS_MUTEX_EN 1
#define OS_MUTEX_QUERY_EN 1
#define OS_Q_ACCEPT_EN 1
#define OS_Q_DEL_EN 1
#define OS_Q_EN 1
#define OS_Q_FLUSH_EN 1
#define OS_Q_POST_EN 1
#define OS_Q_POST_FRONT_EN 1
#define OS_Q_POST_OPT_EN 1
#define OS_Q_QUERY_EN 1
#define OS_SCHED_LOCK_EN 1
#define OS_SEM_ACCEPT_EN 1
#define OS_SEM_DEL_EN 1
#define OS_SEM_EN 1
#define OS_SEM_QUERY_EN 1
#define OS_SEM_SET_EN 1
#define OS_TASK_CHANGE_PRIO_EN 1
#define OS_TASK_CREATE_EN 1
#define OS_TASK_CREATE_EXT_EN 1
#define OS_TASK_DEL_EN 1
#define OS_TASK_IDLE_STK_SIZE 512
#define OS_TASK_NAME_SIZE 32
#define OS_TASK_PROFILE_EN 1
#define OS_TASK_QUERY_EN 1
#define OS_TASK_STAT_EN 1
#define OS_TASK_STAT_STK_CHK_EN 1
#define OS_TASK_STAT_STK_SIZE 512
#define OS_TASK_SUSPEND_EN 1
#define OS_TASK_SW_HOOK_EN 1
#define OS_TASK_TMR_PRIO 0
#define OS_TASK_TMR_STK_SIZE 512
#define OS_THREAD_SAFE_NEWLIB 1
#define OS_TICKS_PER_SEC TIMER_TICKS_PER_SEC
#define OS_TICK_STEP_EN 1
#define OS_TIME_DLY_HMSM_EN 1
#define OS_TIME_DLY_RESUME_EN 1
#define OS_TIME_GET_SET_EN 1
#define OS_TIME_TICK_HOOK_EN 1
#define OS_TMR_CFG_MAX 16
#define OS_TMR_CFG_NAME_SIZE 16
#define OS_TMR_CFG_TICKS_PER_SEC 10
#define OS_TMR_CFG_WHEEL_SIZE 2
#define OS_TMR_EN 0

#endif /* __SYSTEM_H_ */
