// niosII_system_tristate_conduit_pin_sharer.v

// This file was auto-generated from altera_tristate_conduit_pin_sharer_hw.tcl.  If you edit it your changes
// will probably be lost.
// 
// Generated using ACDS version 12.1sp1 243 at 2017.02.19.16:32:42

`timescale 1 ps / 1 ps
module niosII_system_tristate_conduit_pin_sharer (
		input  wire        clk_clk,                                   //   clk.clk
		input  wire        reset_reset,                               // reset.reset
		output wire        request,                                   //   tcm.request
		input  wire        grant,                                     //      .grant
		output wire [0:0]  tristate_controller_tcm_byteenable_out,    //      .tristate_controller_tcm_byteenable_out_out
		output wire [21:0] tristate_controller_tcm_address_out,       //      .tristate_controller_tcm_address_out_out
		output wire [0:0]  tristate_controller_tcm_read_n_out,        //      .tristate_controller_tcm_read_n_out_out
		output wire [0:0]  tristate_controller_tcm_write_n_out,       //      .tristate_controller_tcm_write_n_out_out
		output wire [7:0]  tristate_controller_tcm_data_out,          //      .tristate_controller_tcm_data_out_out
		input  wire [7:0]  tristate_controller_tcm_data_in,           //      .tristate_controller_tcm_data_out_in
		output wire        tristate_controller_tcm_data_outen,        //      .tristate_controller_tcm_data_out_outen
		output wire [0:0]  tristate_controller_tcm_chipselect_n_out,  //      .tristate_controller_tcm_chipselect_n_out_out
		output wire [0:0]  tristate_controller_tcm_begintransfer_out, //      .tristate_controller_tcm_begintransfer_out_out
		input  wire        tcs0_request,                              //  tcs0.request
		output wire        tcs0_grant,                                //      .grant
		input  wire [0:0]  tcs0_byteenable_out,                       //      .byteenable_out
		input  wire [21:0] tcs0_address_out,                          //      .address_out
		input  wire [0:0]  tcs0_read_n_out,                           //      .read_n_out
		input  wire [0:0]  tcs0_write_n_out,                          //      .write_n_out
		input  wire [7:0]  tcs0_data_out,                             //      .data_out
		output wire [7:0]  tcs0_data_in,                              //      .data_in
		input  wire        tcs0_data_outen,                           //      .data_outen
		input  wire [0:0]  tcs0_chipselect_n_out,                     //      .chipselect_n_out
		input  wire [0:0]  tcs0_begintransfer_out                     //      .begintransfer_out
	);

	wire  [0:0] arbiter_grant_data;        // arbiter:next_grant -> pin_sharer:next_grant
	wire        arbiter_grant_ready;       // pin_sharer:ack -> arbiter:ack
	wire        pin_sharer_tcs0_arb_valid; // pin_sharer:arb_tristate_controller_tcm -> arbiter:sink0_valid

	niosII_system_tristate_conduit_pin_sharer_pin_sharer pin_sharer (
		.clk                                       (clk_clk),                                   //      clk.clk
		.reset                                     (reset_reset),                               //    reset.reset
		.request                                   (request),                                   //      tcm.request
		.grant                                     (grant),                                     //         .grant
		.tristate_controller_tcm_byteenable_out    (tristate_controller_tcm_byteenable_out),    //         .tristate_controller_tcm_byteenable_out_out
		.tristate_controller_tcm_address_out       (tristate_controller_tcm_address_out),       //         .tristate_controller_tcm_address_out_out
		.tristate_controller_tcm_read_n_out        (tristate_controller_tcm_read_n_out),        //         .tristate_controller_tcm_read_n_out_out
		.tristate_controller_tcm_write_n_out       (tristate_controller_tcm_write_n_out),       //         .tristate_controller_tcm_write_n_out_out
		.tristate_controller_tcm_data_out          (tristate_controller_tcm_data_out),          //         .tristate_controller_tcm_data_out_out
		.tristate_controller_tcm_data_in           (tristate_controller_tcm_data_in),           //         .tristate_controller_tcm_data_out_in
		.tristate_controller_tcm_data_outen        (tristate_controller_tcm_data_outen),        //         .tristate_controller_tcm_data_out_outen
		.tristate_controller_tcm_chipselect_n_out  (tristate_controller_tcm_chipselect_n_out),  //         .tristate_controller_tcm_chipselect_n_out_out
		.tristate_controller_tcm_begintransfer_out (tristate_controller_tcm_begintransfer_out), //         .tristate_controller_tcm_begintransfer_out_out
		.tcs0_request                              (tcs0_request),                              //     tcs0.request
		.tcs0_grant                                (tcs0_grant),                                //         .grant
		.tcs0_tcm_byteenable_out                   (tcs0_byteenable_out),                       //         .byteenable_out
		.tcs0_tcm_address_out                      (tcs0_address_out),                          //         .address_out
		.tcs0_tcm_read_n_out                       (tcs0_read_n_out),                           //         .read_n_out
		.tcs0_tcm_write_n_out                      (tcs0_write_n_out),                          //         .write_n_out
		.tcs0_tcm_data_out                         (tcs0_data_out),                             //         .data_out
		.tcs0_tcm_data_in                          (tcs0_data_in),                              //         .data_in
		.tcs0_tcm_data_outen                       (tcs0_data_outen),                           //         .data_outen
		.tcs0_tcm_chipselect_n_out                 (tcs0_chipselect_n_out),                     //         .chipselect_n_out
		.tcs0_tcm_begintransfer_out                (tcs0_begintransfer_out),                    //         .begintransfer_out
		.ack                                       (arbiter_grant_ready),                       //    grant.ready
		.next_grant                                (arbiter_grant_data),                        //         .data
		.arb_tristate_controller_tcm               (pin_sharer_tcs0_arb_valid)                  // tcs0_arb.valid
	);

	niosII_system_tristate_conduit_pin_sharer_arbiter arbiter (
		.clk         (clk_clk),                   //       clk.clk
		.reset       (reset_reset),               // clk_reset.reset
		.ack         (arbiter_grant_ready),       //     grant.ready
		.next_grant  (arbiter_grant_data),        //          .data
		.sink0_valid (pin_sharer_tcs0_arb_valid)  //     sink0.valid
	);

endmodule
