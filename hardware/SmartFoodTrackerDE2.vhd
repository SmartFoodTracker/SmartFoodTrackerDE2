-- Nancy Minderman
-- nancy.minderman@ualberta.ca
-- This file makes extensive use of Altera template structures.
-- This file is the top-level file for lab 1 winter 2014 for version 12.1sp1 on Windows 7

library ieee;

use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.VITAL_Primitives.all;
use work.DE2_CONSTANTS.all;
 
entity SmartFoodTrackerDE2 is port
(
   -- Input ports and clocks
   KEY         :  in    std_logic_vector (3 downto 0);
   SW          :  in    std_logic_vector (0 downto 0);
   CLOCK_50    :  in    std_logic;
   CLOCK_27    :  in    std_logic;

   -- Green leds on board
   LEDG        :  out   DE2_LED_GREEN;

   -- Red leds on board
   LEDR        :  out   DE2_LED_RED;

   -- LCD on board
   LCD_BLON    :  out   std_logic;
   LCD_ON      :  out   std_logic;
   LCD_DATA    :  inout DE2_LCD_DATA_BUS;
   LCD_RS      :  out   std_logic;
   LCD_EN      :  out   std_logic;
   LCD_RW      :  out   std_logic;

   -- SDRAM on board
   DRAM_ADDR   :  out   DE2_SDRAM_ADDR_BUS;
   DRAM_BA_0   :  out   std_logic;
   DRAM_BA_1   :  out   std_logic;
   DRAM_CAS_N  :  out   std_logic;
   DRAM_CKE    :  out   std_logic;
   DRAM_CLK    :  out   std_logic;
   DRAM_CS_N   :  out   std_logic;
   DRAM_DQ     :  inout DE2_SDRAM_DATA_BUS;
   DRAM_LDQM   :  out   std_logic;
   DRAM_UDQM   :  out   std_logic;
   DRAM_RAS_N  :  out   std_logic;
   DRAM_WE_N   :  out   std_logic;

   -- SRAM on board
   SRAM_ADDR   :  out   DE2_SRAM_ADDR_BUS;
   SRAM_DQ     :  inout DE2_SRAM_DATA_BUS;
   SRAM_WE_N   :  out   std_logic;
   SRAM_OE_N   :  out   std_logic;
   SRAM_UB_N   :  out   std_logic;
   SRAM_LB_N   :  out   std_logic;
   SRAM_CE_N   :  out   std_logic;

   -- FLASH
   FL_ADDR     :  out   std_logic_vector (21 downto 0);
   FL_CE_N     :  out   std_logic_vector (0 downto 0);
   FL_OE_N     :  out   std_logic_vector (0 downto 0);
   FL_DQ       :  inout std_logic_vector (7 downto 0);
   FL_RST_N    :  out   std_logic_vector (0 downto 0) := (others => '1');
   FL_WE_N     :  out   std_logic_vector (0 downto 0);

   -- PS2
   PS2_CLK     :  inout std_logic;
   PS2_DAT     :  inout std_logic;

   -- I2C
   I2C_SDAT    :  inout std_logic;
   I2C_SCLK    :  inout std_logic;

   --ENET
   ENET_CMD    :  out   std_logic;
   ENET_CS_N   :  out   std_logic;
   ENET_DATA   :  inout std_logic_vector (15 downto 0);
   ENET_INT    :  in    std_logic;
   ENET_RD_N   :  out   std_logic;
   ENET_RST_N  :  out   std_logic;
   ENET_WR_N   :  out   std_logic;
   ENET_CLK    :  out   std_logic;

   -- AUD
   AUD_ADCLRCK :  inout std_logic;
   AUD_ADCDAT  :  in    std_logic;
   AUD_DACLRCK :  inout std_logic;
   AUD_DACDAT  :  out   std_logic;
   AUD_BCLK    :  inout std_logic
);
end SmartFoodTrackerDE2;

architecture structure of SmartFoodTrackerDE2 is

   component niosII_system is port
   (
            clk_clk                                                               : in    std_logic                     := 'X';             -- clk
            reset_reset_n                                                         : in    std_logic                     := 'X';             -- reset_n
            green_leds_external_connection_export                                 : out   DE2_LED_GREEN;                     -- export
            switch_external_connection_export                                     : in    std_logic                     := 'X';             -- export
            add_button_external_connection_export                                 : in    std_logic                     := 'X';             -- export
            remove_button_external_connection_export                              : in    std_logic                     := 'X';             -- export
            red_leds_external_connection_export                                   : out   DE2_LED_RED;                     -- export
            altpll_c0_clk                                                         : out   std_logic;                                        -- clk
            altpll_c2_clk                                                         : out   std_logic;                                         -- clk
            sdram_wire_addr                                                       : out   DE2_SDRAM_ADDR_BUS;                    -- addr
            sdram_wire_ba                                                         : out   std_logic_vector(1 downto 0);                     -- ba
            sdram_wire_cas_n                                                      : out   std_logic;                                        -- cas_n
            sdram_wire_cke                                                        : out   std_logic;                                        -- cke
            sdram_wire_cs_n                                                       : out   std_logic;                                        -- cs_n
            sdram_wire_dq                                                         : inout DE2_SDRAM_DATA_BUS := (others => 'X'); -- dq
            sdram_wire_dqm                                                        : out   std_logic_vector(1 downto 0);                     -- dqm
            sdram_wire_ras_n                                                      : out   std_logic;                                        -- ras_n
            sdram_wire_we_n                                                       : out   std_logic;                                        -- we_n
            sram_external_interface_DQ                                            : inout DE2_SRAM_DATA_BUS := (others => 'X'); -- DQ
            sram_external_interface_ADDR                                          : out   DE2_SRAM_ADDR_BUS;                    -- ADDR
            sram_external_interface_LB_N                                          : out   std_logic;                                        -- LB_N
            sram_external_interface_UB_N                                          : out   std_logic;                                        -- UB_N
            sram_external_interface_CE_N                                          : out   std_logic;                                        -- CE_N
            sram_external_interface_OE_N                                          : out   std_logic;                                        -- OE_N
            sram_external_interface_WE_N                                          : out   std_logic;                                        -- WE_N
            character_lcd_external_interface_DATA                                 : inout DE2_LCD_DATA_BUS              := (others => 'X'); -- DATA
            character_lcd_external_interface_ON                                   : out   std_logic;                                        -- ON
            character_lcd_external_interface_BLON                                 : out   std_logic;                                        -- BLON
            character_lcd_external_interface_EN                                   : out   std_logic;                                        -- EN
            character_lcd_external_interface_RS                                   : out   std_logic;                                        -- RS
            character_lcd_external_interface_RW                                   : out   std_logic;                                        -- RW
            cancel_button_external_connection_export                              : in    std_logic                     := 'X';             -- export
            barcode_scanner_ps2_external_interface_CLK                            : inout std_logic                     := 'X';             -- CLK
            barcode_scanner_ps2_external_interface_DAT                            : inout std_logic                     := 'X';             -- DAT
            audio_core_external_interface_ADCDAT                                  : in    std_logic                     := 'X';             -- ADCDAT
            audio_core_external_interface_ADCLRCK                                 : in    std_logic                     := 'X';             -- ADCLRCK
            audio_core_external_interface_BCLK                                    : in    std_logic                     := 'X';             -- BCLK
            audio_core_external_interface_DACDAT                                  : out   std_logic;                                        -- DACDAT
            audio_core_external_interface_DACLRCK                                 : in    std_logic                     := 'X';             -- DACLRCK
            audio_config_external_interface_SDAT                                  : inout std_logic                     := 'X';             -- SDAT
            audio_config_external_interface_SCLK                                  : out   std_logic;                                        -- SCLK
            dm9000a_if_ethernet_s1_export_DATA                                    : inout std_logic_vector(15 downto 0) := (others => 'X'); -- DATA
            dm9000a_if_ethernet_s1_export_CMD                                     : out   std_logic;                                        -- CMD
            dm9000a_if_ethernet_s1_export_RD_N                                    : out   std_logic;                                        -- RD_N
            dm9000a_if_ethernet_s1_export_WR_N                                    : out   std_logic;                                        -- WR_N
            dm9000a_if_ethernet_s1_export_CS_N                                    : out   std_logic;                                        -- CS_N
            dm9000a_if_ethernet_s1_export_RST_N                                   : out   std_logic;                                        -- RST_N
            dm9000a_if_ethernet_s1_export_INT                                     : in    std_logic                     := 'X';             -- INT
            tristate_conduit_bridge_out_tristate_controller_tcm_read_n_out        : out   std_logic_vector(0 downto 0);                     -- tristate_controller_tcm_read_n_out
            tristate_conduit_bridge_out_tristate_controller_tcm_address_out       : out   std_logic_vector(21 downto 0);                    -- tristate_controller_tcm_address_out
            tristate_conduit_bridge_out_tristate_controller_tcm_data_out          : inout std_logic_vector(7 downto 0)  := (others => 'X'); -- tristate_controller_tcm_data_outtristate_conduit_bridge_out_tristate_controller_tcm_write_n_out       : out   std_logic_vector(0 downto 0);                     -- tristate_controller_tcm_write_n_out
            tristate_conduit_bridge_out_tristate_controller_tcm_chipselect_n_out  : out   std_logic_vector(0 downto 0);                     -- tristate_controller_tcm_chipselect_n_out
            audio_clock_secondary_clk_in_clk                                      : in    std_logic                     := 'X';             -- clk
            audio_clock_secondary_clk_in_reset_reset_n                            : in    std_logic                     := 'X'              -- reset_n
    );
   end component niosII_system;

   signal BA   :  std_logic_vector (1 downto 0);
   signal DQM  :  std_logic_vector (1 downto 0);

begin

   DRAM_BA_1   <= BA(1);
   DRAM_BA_0   <= BA(0);

   DRAM_UDQM   <= DQM(1);
   DRAM_LDQM   <= DQM(0);

   u0 : component niosII_system port map
   (
            clk_clk                                                               => CLOCK_50,                                                               --                                    clk.clk
            reset_reset_n                                                         => KEY(0),                                                         --                                  reset.reset_n
            green_leds_external_connection_export                                 => LEDG,                                 --         green_leds_external_connection.export
            switch_external_connection_export                                     => SW(0),                                     --             switch_external_connection.export
            add_button_external_connection_export                                 => KEY(3),                                 --         add_button_external_connection.export
            remove_button_external_connection_export                              => KEY(2),                                      --              pio_0_external_connection.export
            red_leds_external_connection_export                                   => LEDR,                                   --           red_leds_external_connection.export
            altpll_c0_clk                                                         => DRAM_CLK,                                                         --                              altpll_c0.clk
            altpll_c2_clk                                                         => ENET_CLK,                                                          --                              altpll_c2.clk
            sdram_wire_addr                                                       => DRAM_ADDR,                                                       --                             sdram_wire.addr
            sdram_wire_ba                                                         => BA,                                                         --                                       .ba
            sdram_wire_cas_n                                                      => DRAM_CAS_N,                                                      --                                       .cas_n
            sdram_wire_cke                                                        => DRAM_CKE,                                                        --                                       .cke
            sdram_wire_cs_n                                                       => DRAM_CS_N,                                                       --                                       .cs_n
            sdram_wire_dq                                                         => DRAM_DQ,                                                         --                                       .dq
            sdram_wire_dqm                                                        => DQM,                                                        --                                       .dqm
            sdram_wire_ras_n                                                      => DRAM_RAS_N,                                                      --                                       .ras_n
            sdram_wire_we_n                                                       => DRAM_WE_N,                                                       --                                       .we_n
            sram_external_interface_DQ                                            => SRAM_DQ,                                            --                sram_external_interface.DQ
            sram_external_interface_ADDR                                          => SRAM_ADDR,                                          --                                       .ADDR
            sram_external_interface_LB_N                                          => SRAM_LB_N,                                          --                                       .LB_N
            sram_external_interface_UB_N                                          => SRAM_UB_N,                                          --                                       .UB_N
            sram_external_interface_CE_N                                          => SRAM_CE_N,                                          --                                       .CE_N
            sram_external_interface_OE_N                                          => SRAM_OE_N,                                          --                                       .OE_N
            sram_external_interface_WE_N                                          => SRAM_WE_N,                                          --                                       .WE_N
            character_lcd_external_interface_DATA                                 => LCD_DATA,                                 --       character_lcd_external_interface.DATA
            character_lcd_external_interface_ON                                   => LCD_ON,                                   --                                       .ON
            character_lcd_external_interface_BLON                                 => LCD_BLON,                                 --                                       .BLON
            character_lcd_external_interface_EN                                   => LCD_EN,                                   --                                       .EN
            character_lcd_external_interface_RS                                   => LCD_RS,                                   --                                       .RS
            character_lcd_external_interface_RW                                   => LCD_RW,                                   --                                       .RW
            cancel_button_external_connection_export                              => KEY(1),                              --      cancel_button_external_connection.export
            barcode_scanner_ps2_external_interface_CLK                            => PS2_CLK,                            -- barcode_scanner_ps2_external_interface.CLK
            barcode_scanner_ps2_external_interface_DAT                            => PS2_DAT,                            --                                       .DAT
            audio_core_external_interface_ADCDAT                                  => AUD_ADCDAT,                                  --          audio_core_external_interface.ADCDAT
            audio_core_external_interface_ADCLRCK                                 => AUD_ADCLRCK,                                 --                                       .ADCLRCK
            audio_core_external_interface_BCLK                                    => AUD_BCLK,                                    --                                       .BCLK
            audio_core_external_interface_DACDAT                                  => AUD_DACDAT,                                  --                                       .DACDAT
            audio_core_external_interface_DACLRCK                                 => AUD_DACLRCK,                                 --                                       .DACLRCK
            audio_config_external_interface_SDAT                                  => I2C_SDAT,                                  --        audio_config_external_interface.SDAT
            audio_config_external_interface_SCLK                                  => I2C_SCLK,                                  --                                       .SCLK
            dm9000a_if_ethernet_s1_export_DATA                                    => ENET_DATA,                                    --          dm9000a_if_ethernet_s1_export.DATA
            dm9000a_if_ethernet_s1_export_CMD                                     => ENET_CMD,                                     --                                       .CMD
            dm9000a_if_ethernet_s1_export_RD_N                                    => ENET_RD_N,                                    --                                       .RD_N
            dm9000a_if_ethernet_s1_export_WR_N                                    => ENET_WR_N,                                    --                                       .WR_N
            dm9000a_if_ethernet_s1_export_CS_N                                    => ENET_CS_N,                                    --                                       .CS_N
            dm9000a_if_ethernet_s1_export_RST_N                                   => ENET_RST_N,                                   --                                       .RST_N
            dm9000a_if_ethernet_s1_export_INT                                     => ENET_INT,                                     --                                       .INT
            tristate_conduit_bridge_out_tristate_controller_tcm_read_n_out        => FL_OE_N,        --            tristate_conduit_bridge_out.tristate_controller_tcm_read_n_out
            tristate_conduit_bridge_out_tristate_controller_tcm_address_out       => FL_ADDR,       --                                       .tristate_controller_tcm_address_out
            tristate_conduit_bridge_out_tristate_controller_tcm_data_out          => FL_DQ,          --                                       .tristate_controller_tcm_data_outristate_conduit_bridge_out_tristate_controller_tcm_write_n_out       => FL_WE_N,       --                                       .tristate_controller_tcm_write_n_out
            tristate_conduit_bridge_out_tristate_controller_tcm_chipselect_n_out  => FL_CE_N,  --                                       .tristate_controller_tcm_chipselect_n_out
            audio_clock_secondary_clk_in_clk                                      => CLOCK_27,                                      --           audio_clock_secondary_clk_in.clk
            audio_clock_secondary_clk_in_reset_reset_n                            => KEY(0)                             --     audio_clock_secondary_clk_in_reset.reset_n
   );

end structure;

library ieee;

use ieee.std_logic_1164.all;

package DE2_CONSTANTS is

   type DE2_SDRAM_ADDR_BUS   is array(11 downto 0)   of std_logic;
   type DE2_SDRAM_DATA_BUS   is array(15 downto 0)   of std_logic;
   type DE2_LCD_DATA_BUS     is array(7 downto 0)    of std_logic;
   type DE2_LED_GREEN        is array(7 downto 0)    of std_logic;
   type DE2_LED_RED          is array(7 downto 0)    of std_logic;
   type DE2_SRAM_ADDR_BUS    is array(17 downto 0)   of std_logic;
   type DE2_SRAM_DATA_BUS    is array(15 downto 0)   of std_logic;

end DE2_CONSTANTS;