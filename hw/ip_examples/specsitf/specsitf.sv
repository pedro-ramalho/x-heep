module specsitf #(
    parameter type reg_req_t  = logic,
    parameter type reg_rsp_t  = logic,
    parameter type obi_req_t  = logic,
    parameter type obi_resp_t = logic
) (
    input logic clk_i,
    input logic rst_ni,

    input  reg_req_t reg_req_i,
    output reg_rsp_t reg_rsp_o,

    output obi_req_t  read_channel_req_o,
    input  obi_resp_t read_channel_resp_i,

    output obi_req_t  write_channel_req_o,
    input  obi_resp_t write_channel_resp_i
);

  /* mandatory registers that control the state of the CPU and simulator */
  logic [31:0] control = '0, status = '0;

  /* user registers exposed through MMIO */
  logic [31:0] read_address, write_address, threshold, data_size;

  /* OBI registers managed by the accelerator for read requests */
  logic [31:0] read_channel_req_type;
  logic [31:0] read_channel_req_addr;

  /* OBI registers managed by the accelerator for write requests */
  logic [31:0] write_channel_req_type;
  logic [31:0] write_channel_req_addr;
  logic [31:0] write_channel_req_data;

  import "DPI-C" function void specsitf_comm_init();
  import "DPI-C" function void specsitf_comm_free();
  import "DPI-C" function void specsitf_comm_send(
    int read_address,
    int write_address,
    int threshold,
    int data_size,
    int control,
    int read_req_gnt,
    int read_req_rvalid,
    int read_req_rdata,
    int write_req_gnt
  );
  import "DPI-C" function void specsitf_comm_recv(
    output int st,
    output int read_channel_req_type,
    output int read_channel_req_addr,
    output int write_channel_req_type,
    output int write_channel_req_addr,
    output int write_channel_req_data
  );

  initial begin
    specsitf_comm_init();
  end
  final begin
    specsitf_comm_free();
  end

  /* MM register write logic (host to accelerator) */
  always_ff @(posedge clk_i or negedge rst_ni) begin
    if (!rst_ni) begin
      read_address <= '0;
      write_address <= '0;
      threshold <= '0;
      data_size <= '0;
      control <= '0;
    end else if (reg_req_i.valid && reg_req_i.write) begin
      case (reg_req_i.addr[7:2])
        0: begin
          read_address <= reg_req_i.wdata;
        end
        1: begin
          write_address <= reg_req_i.wdata;
        end
        2: begin
          threshold <= reg_req_i.wdata;
        end
        3: begin
          data_size <= reg_req_i.wdata;
        end
        4: begin
          control <= reg_req_i.wdata;
        end
        default: $display("[%0t] Verilog-MMIO-WRITE: Invalid Reg %h", $time, reg_req_i.addr[7:2]);
      endcase
    end
  end

  /* MM register read logic (host from accelerator) */
  always_comb begin
    reg_rsp_o = '{ready: 1'b1, error: 1'b0, rdata: '0};

    if (reg_req_i.valid && !reg_req_i.write) begin
      case (reg_req_i.addr[7:2])
        5: reg_rsp_o.rdata = status;
        default: reg_rsp_o.error = 1'b1;
      endcase
    end
  end

  /* DPI-C interface call and internal OBI request register updates */
  always_ff @(posedge clk_i or negedge rst_ni) begin
    int _status;
    int _read_channel_req_type, _read_channel_req_addr;
    int _write_channel_req_type, _write_channel_req_addr, _write_channel_req_data;

    if (!rst_ni) begin
      status <= '0;

      read_channel_req_type <= '0;
      read_channel_req_addr <= '0;

      write_channel_req_type <= '0;
      write_channel_req_addr <= '0;
      write_channel_req_data <= '0;

      $display("[%0t] Verilog-DPI: Reset asserted. All DPI outputs reset.", $time);
    end else begin

      // Log inputs to DPI-C before sending
      // $display(
      //     "[%0t] Verilog-DPI-SEND-INPUTS: addr=%h, ctrl=%h, read_gnt=%0d, read_rvalid=%0d, read_rdata=%h, write_gnt=%0d",
      //     $time, address, control, read_channel_resp_i.gnt, read_channel_resp_i.rvalid,
      //     read_channel_resp_i.rdata, write_channel_resp_i.gnt);

      /* send outputs to simulator (Java) */
      specsitf_comm_send(read_address, write_address, threshold, data_size, control, {
                         31'b0, read_channel_resp_i.gnt}, {31'b0, read_channel_resp_i.rvalid},
                         read_channel_resp_i.rdata, {31'b0, write_channel_resp_i.gnt});

      /* recv outputs from simulator (Java) */
      specsitf_comm_recv(_status, _read_channel_req_type, _read_channel_req_addr,
                         _write_channel_req_type, _write_channel_req_addr, _write_channel_req_data);

      // Log raw outputs received from DPI-C before registering
      $display(
          "[%0t] Verilog-DPI-RECV-RAW: _status=%0d, _read_type=%0d, _read_addr=%h, _write_type=%0d, _write_addr=%h, _write_data=%h",
          $time, _status, _read_channel_req_type, _read_channel_req_addr, _write_channel_req_type,
          _write_channel_req_addr, _write_channel_req_data);

      status <= _status;

      read_channel_req_type <= _read_channel_req_type;
      read_channel_req_addr <= _read_channel_req_addr;

      write_channel_req_type <= _write_channel_req_type;
      write_channel_req_addr <= _write_channel_req_addr;
      write_channel_req_data <= _write_channel_req_data;

      // Log values of registered internal signals (which will drive OBI outputs next cycle)
      $display(
          "[%0t] Verilog-DPI-REGISTERED: status=%0d, read_type=%0d, read_addr=%h, write_type=%0d, write_addr=%h, write_data=%h",
          $time, status, read_channel_req_type, read_channel_req_addr, write_channel_req_type,
          write_channel_req_addr, write_channel_req_data);
    end
  end

  /* assign read channel signals */
  assign read_channel_req_o.req = (read_channel_req_type == 1);
  assign read_channel_req_o.we = 0;
  assign read_channel_req_o.be = 4'b1111;
  assign read_channel_req_o.addr = read_channel_req_addr;
  assign read_channel_req_o.wdata = 0;

  /* assign write channel signals (Still has the bug, please fix this for correct write operation!) */
  assign write_channel_req_o.req = (write_channel_req_type == 1);
  assign write_channel_req_o.we = 1;
  assign write_channel_req_o.be = 4'b1111;
  assign write_channel_req_o.addr = write_channel_req_addr;
  assign write_channel_req_o.wdata = write_channel_req_data;

  always @(posedge clk_i) begin
    if (rst_ni) begin
      $display(
          "[%0t] Verilog-OBI-OUT: READ_REQ: req=%0d, we=%0d, addr=%h | READ_RESP_IN: gnt=%0d, rvalid=%0d, rdata=%h",
          $time, read_channel_req_o.req, read_channel_req_o.we, read_channel_req_o.addr,
          read_channel_resp_i.gnt, read_channel_resp_i.rvalid, read_channel_resp_i.rdata);

      $display(
          "[%0t] Verilog-OBI-OUT: WRITE_REQ: req=%0d, we=%0d, addr=%h, wdata=%h | WRITE_RESP_IN: gnt=%0d",
          $time, write_channel_req_o.req, write_channel_req_o.we, write_channel_req_o.addr,
          write_channel_req_o.wdata, write_channel_resp_i.gnt);
    end
  end

endmodule
