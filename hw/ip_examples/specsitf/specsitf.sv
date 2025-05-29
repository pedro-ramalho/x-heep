// specsitf.sv (Corrected)
module specsitf #(
    parameter type reg_req_t  = logic,
    parameter type reg_rsp_t  = logic,
    parameter type obi_req_t  = logic,
    parameter type obi_resp_t = logic
) (
    input logic clk_i,
    input logic rst_ni,
    
    input reg_req_t reg_req_i,
    output reg_rsp_t reg_rsp_o,

    output obi_req_t  obi_req_o,
    input  obi_resp_t obi_resp_i
);

  logic [31:0] addr, ctrl = '0, st = '0;

  /* OBI registers managed by the accelerator; in this scenario, the accelerator acts as the bus master */
  // These registers hold the *requested* values from Java.
  // They should be directly assigned to obi_req_o signals.
  logic [31:0] acc_mem_req_type;  
  logic [31:0] acc_mem_req_addr;  
  logic [31:0] acc_mem_req_wdata;
  // logic acc_mem_has_req = 0; // This flag is not needed with the direct assignment below.


  import "DPI-C" function void specsitf_comm_init();
  import "DPI-C" function void specsitf_comm_free();
  import "DPI-C" function void specsitf_comm_send(
    int addr,
    int ctrl,
    int obi_gnt,
    int obi_rvalid,
    int obi_rdata
  );
  import "DPI-C" function void specsitf_comm_recv(
    output int st,
    output int mem_req_type,
    output int mem_req_addr,
    output int mem_req_wdata
  );

  initial begin
    specsitf_comm_init();
    $display("[%0t] Verilog: specsitf_comm_init called.", $time);
  end
  final begin
    specsitf_comm_free();
    $display("[%0t] Verilog: specsitf_comm_free called.", $time);
  end

  /* memory-mapped register write logic (Host to Accelerator) */
  always_ff @(posedge clk_i or negedge rst_ni) begin
    if (!rst_ni) begin
      addr <= '0;
      ctrl <= '0;
      $display("[%0t] Verilog: Reset asserted. addr=%h, ctrl=%h", $time, addr, ctrl);
    end else if (reg_req_i.valid && reg_req_i.write) begin
      case (reg_req_i.addr[7:2])
        0: begin addr <= reg_req_i.wdata; $display("[%0t] Verilog: Host wrote addr=%h (Reg 0)", $time, reg_req_i.wdata); end
        1: begin ctrl <= reg_req_i.wdata; $display("[%0t] Verilog: Host wrote ctrl=%h (Reg 1)", $time, reg_req_i.wdata); end
        default: $display("[%0t] Verilog: Host wrote to invalid Reg %h", $time, reg_req_i.addr[7:2]);
      endcase
    end
  end

  /* memory-mapped register read logic (Host from Accelerator) */
  always_comb begin
    reg_rsp_o = '{ready: 1'b1, error: 1'b0, rdata: '0};

    if (reg_req_i.valid && !reg_req_i.write) begin
      case (reg_req_i.addr[7:2])
        0: reg_rsp_o.rdata = addr;
        1: reg_rsp_o.rdata = ctrl;
        2: reg_rsp_o.rdata = st; // Host reads st at offset 2
        default: reg_rsp_o.error = 1'b1;  
      endcase
      // $display("[%0t] Verilog: Host read Reg %h, rdata=%h", $time, reg_req_i.addr[7:2], reg_rsp_o.rdata); // Excessive logging
    end
  end

  /* DPI-C interface call and internal OBI request register updates */
  always_ff @(posedge clk_i or negedge rst_ni) begin  
    int tmp_st, tmp_mem_req_type, tmp_mem_req_addr, tmp_mem_req_wdata;

    if (!rst_ni) begin
      st <= '0;
      acc_mem_req_type <= '0;
      acc_mem_req_addr <= '0;
      acc_mem_req_wdata <= '0;
    end else begin
      // Send current inputs to Java (previous cycle's OBI response)
      specsitf_comm_send(
        addr, // Value of addr register from host
        ctrl, // Value of ctrl register from host
        {31'b0, obi_resp_i.gnt},    /* cast gnt to 32-bit    */
        {31'b0, obi_resp_i.rvalid}, /* cast rvalid to 32-bit */
        obi_resp_i.rdata
      );
      
      // Receive outputs from Java (Java's decision for current cycle's OBI request and status)
      specsitf_comm_recv(tmp_st, tmp_mem_req_type, tmp_mem_req_addr, tmp_mem_req_wdata);

      // Update internal status register from Java
      st <= tmp_st;
      
      // Update internal OBI request registers from Java.
      // These are then used by the 'assign' statements below to drive OBI.
      acc_mem_req_type <= tmp_mem_req_type;
      acc_mem_req_addr <= tmp_mem_req_addr;
      acc_mem_req_wdata <= tmp_mem_req_wdata;
    end
  end

  /* output OBI signals - driven by internal registers, continuous assignment */
  // These assignments MUST be continuous to hold 'req' high as long as Java wants it.
  assign obi_req_o.req = (acc_mem_req_type == 1 || acc_mem_req_type == 2); // Correct!
  assign obi_req_o.we = (acc_mem_req_type == 2); // Correct!
  assign obi_req_o.be = 4'b1111; // Correct!
  assign obi_req_o.addr = acc_mem_req_addr; // Correct!
  assign obi_req_o.wdata = acc_mem_req_wdata; // Correct!

  // Debugging OBI signals
  always @(posedge clk_i) begin
    if (rst_ni) begin
      $display("[%0t] Verilog OBI_OUT: req=%b, we=%b, addr=%h, wdata=%h (From Java: type=%0d, addr=%h, wdata=%h)",
               $time, obi_req_o.req, obi_req_o.we, obi_req_o.addr, obi_req_o.wdata,
               acc_mem_req_type, acc_mem_req_addr, acc_mem_req_wdata);
      $display("[%0t] Verilog OBI_IN : gnt=%b, rvalid=%b, rdata=%h",
               $time, obi_resp_i.gnt, obi_resp_i.rvalid, obi_resp_i.rdata);
    end
  end

endmodule
