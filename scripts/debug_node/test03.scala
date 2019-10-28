package dandelion.generator

import chisel3._
import chisel3.util._
import chisel3.Module._
import chisel3.testers._
import chisel3.iotesters._
import dandelion.accel._
import dandelion.arbiters._
import dandelion.config._
import dandelion.control._
import dandelion.fpu._
import dandelion.interfaces._
import dandelion.junctions._
import dandelion.loop._
import dandelion.memory._
import dandelion.memory.stack._
import dandelion.node._
import muxes._
import org.scalatest._
import regfile._
import util._


  /* ================================================================== *
   *                   PRINTING PORTS DEFINITION                        *
   * ================================================================== */

abstract class test03DFIO(implicit val p: Parameters) extends Module with CoreParams {
  val io = IO(new Bundle {
    val in = Flipped(Decoupled(new Call(List(32, 32))))
    val MemResp = Flipped(Valid(new MemResp))
    val MemReq = Decoupled(new MemReq)
    val out = Decoupled(new Call(List(32)))
  })
}

class test03DF(implicit p: Parameters) extends test03DFIO()(p) {


  /* ================================================================== *
   *                   PRINTING MEMORY MODULES                          *
   * ================================================================== */

  //Remember if there is no mem operation io memreq/memresp should be grounded
  io.MemReq <> DontCare
  io.MemResp <> DontCare

  val InputSplitter = Module(new SplitCallNew(List(3, 3)))
  InputSplitter.io.In <> io.in



  /* ================================================================== *
   *                   PRINTING LOOP HEADERS                            *
   * ================================================================== */



  /* ================================================================== *
   *                   PRINTING BASICBLOCK NODES                        *
   * ================================================================== */

  val bb_entry0 = Module(new BasicBlockNoMaskFastNode(NumInputs = 1, NumOuts = 9, BID = 0))



  /* ================================================================== *
   *                   PRINTING INSTRUCTION NODES                       *
   * ================================================================== */

  //  %cmp = icmp slt i32 %b, %a, !dbg !18, !UID !20
  val icmp_cmp0 = Module(new IcmpNode(NumOuts = 2, ID = 0, opCode = "ult")(sign = false))

  //  %sub = select i1 %cmp, i32 %b, i32 0, !dbg !21, !UID !22
  val select_sub1 = Module(new SelectNode(NumOuts = 1, ID = 1))

  //  %a.addr.0 = sub nsw i32 %a, %sub, !dbg !21, !UID !23
  val binaryOp_a_addr_02 = Module(new ComputeNode(NumOuts = 1, ID = 2, opCode = "sub")(sign = false, Debug = false))

  //  %sub1 = select i1 %cmp, i32 0, i32 %a, !dbg !21, !UID !24
  val select_sub13 = Module(new SelectNode(NumOuts = 1, ID = 3))

  //  %b.addr.0 = sub nsw i32 %b, %sub1, !dbg !21, !UID !25
  val binaryOp_b_addr_04 = Module(new ComputeNode(NumOuts = 1, ID = 4, opCode = "sub")(sign = false, Debug = false))

  //  %mul = mul nsw i32 %a.addr.0, %b.addr.0, !dbg !26, !UID !27
  val binaryOp_mul5 = Module(new ComputeNode(NumOuts = 1, ID = 5, opCode = "mul")(sign = false, Debug = false))

  //  ret i32 %mul, !dbg !28, !UID !29, !BB_UID !30
  val ret_6 = Module(new RetNode2(retTypes = List(32), ID = 6))



  /* ================================================================== *
   *                   PRINTING CONSTANTS NODES                         *
   * ================================================================== */

  //i32 0
  val const0 = Module(new ConstFastNode(value = 0, ID = 0))

  //i32 0
  val const1 = Module(new ConstFastNode(value = 0, ID = 1))



  /* ================================================================== *
   *                   BASICBLOCK -> PREDICATE INSTRUCTION              *
   * ================================================================== */

  bb_entry0.io.predicateIn(0) <> InputSplitter.io.Out.enable



  /* ================================================================== *
   *                   BASICBLOCK -> PREDICATE LOOP                     *
   * ================================================================== */



  /* ================================================================== *
   *                   PRINTING PARALLEL CONNECTIONS                    *
   * ================================================================== */



  /* ================================================================== *
   *                   LOOP -> PREDICATE INSTRUCTION                    *
   * ================================================================== */



  /* ================================================================== *
   *                   ENDING INSTRUCTIONS                              *
   * ================================================================== */



  /* ================================================================== *
   *                   LOOP INPUT DATA DEPENDENCIES                     *
   * ================================================================== */



  /* ================================================================== *
   *                   LOOP DATA LIVE-IN DEPENDENCIES                   *
   * ================================================================== */



  /* ================================================================== *
   *                   LOOP DATA LIVE-OUT DEPENDENCIES                  *
   * ================================================================== */



  /* ================================================================== *
   *                   LOOP LIVE OUT DEPENDENCIES                       *
   * ================================================================== */



  /* ================================================================== *
   *                   LOOP CARRY DEPENDENCIES                          *
   * ================================================================== */



  /* ================================================================== *
   *                   LOOP DATA CARRY DEPENDENCIES                     *
   * ================================================================== */



  /* ================================================================== *
   *                   BASICBLOCK -> ENABLE INSTRUCTION                 *
   * ================================================================== */

  const0.io.enable <> bb_entry0.io.Out(0)

  const1.io.enable <> bb_entry0.io.Out(1)

  icmp_cmp0.io.enable <> bb_entry0.io.Out(2)


  select_sub1.io.enable <> bb_entry0.io.Out(3)


  binaryOp_a_addr_02.io.enable <> bb_entry0.io.Out(4)


  select_sub13.io.enable <> bb_entry0.io.Out(5)


  binaryOp_b_addr_04.io.enable <> bb_entry0.io.Out(6)


  binaryOp_mul5.io.enable <> bb_entry0.io.Out(7)


  ret_6.io.In.enable <> bb_entry0.io.Out(8)




  /* ================================================================== *
   *                   CONNECTING PHI NODES                             *
   * ================================================================== */



  /* ================================================================== *
   *                   PRINT ALLOCA OFFSET                              *
   * ================================================================== */



  /* ================================================================== *
   *                   CONNECTING MEMORY CONNECTIONS                    *
   * ================================================================== */



  /* ================================================================== *
   *                   PRINT SHARED CONNECTIONS                         *
   * ================================================================== */



  /* ================================================================== *
   *                   CONNECTING DATA DEPENDENCIES                     *
   * ================================================================== */

  select_sub1.io.InData2 <> const0.io.Out

  select_sub13.io.InData1 <> const1.io.Out

  select_sub1.io.Select <> icmp_cmp0.io.Out(0)

  select_sub13.io.Select <> icmp_cmp0.io.Out(1)

  binaryOp_a_addr_02.io.RightIO <> select_sub1.io.Out(0)

  binaryOp_mul5.io.LeftIO <> binaryOp_a_addr_02.io.Out(0)

  binaryOp_b_addr_04.io.RightIO <> select_sub13.io.Out(0)

  binaryOp_mul5.io.RightIO <> binaryOp_b_addr_04.io.Out(0)

  ret_6.io.In.data("field0") <> binaryOp_mul5.io.Out(0)

  icmp_cmp0.io.RightIO <> InputSplitter.io.Out.data.elements("field0")(0)

  binaryOp_a_addr_02.io.LeftIO <> InputSplitter.io.Out.data.elements("field0")(1)

  select_sub13.io.InData2 <> InputSplitter.io.Out.data.elements("field0")(2)

  icmp_cmp0.io.LeftIO <> InputSplitter.io.Out.data.elements("field1")(0)

  select_sub1.io.InData1 <> InputSplitter.io.Out.data.elements("field1")(1)

  binaryOp_b_addr_04.io.LeftIO <> InputSplitter.io.Out.data.elements("field1")(2)



  /* ================================================================== *
   *                   PRINTING OUTPUT INTERFACE                        *
   * ================================================================== */

  io.out <> ret_6.io.Out

}

import java.io.{File, FileWriter}

object test03Top extends App {
  val dir = new File("RTL/test03Top");
  dir.mkdirs
  implicit val p = config.Parameters.root((new MiniConfig).toInstance)
  val chirrtl = firrtl.Parser.parse(chisel3.Driver.emit(() => new test03DF()))

  val verilogFile = new File(dir, s"${chirrtl.main}.v")
  val verilogWriter = new FileWriter(verilogFile)
  val compileResult = (new firrtl.VerilogCompiler).compileAndEmit(firrtl.CircuitState(chirrtl, firrtl.ChirrtlForm))
  val compiledStuff = compileResult.getEmittedCircuit
  verilogWriter.write(compiledStuff.value)
  verilogWriter.close()
}
