package dataflow

import chisel3._
import chisel3.util._
import chisel3.Module
import chisel3.testers._
import chisel3.iotesters._
import org.scalatest.{FlatSpec, Matchers}
import muxes._
import config._
import control._
import util._
import interfaces._
import regfile._
import memory._
import stack._
import arbiters._
import loop._
import accel._
import node._


/**
  * This Object should be initialized at the first step
  * It contains all the transformation from indices to their module's name
  */

object Data_test03_FlowParam{

  val entry_pred = Map(
    "active" -> 0
  )


  val for_cond_pred = Map(
    "br0" -> 0,
    "br9" -> 1
  )


  val for_inc_pred = Map(
    "br7" -> 0
  )


  val for_body_pred = Map(
    "br4" -> 0
  )


  val for_end_pred = Map(
    "br4" -> 0
  )


  val br0_brn_bb = Map(
    "for_cond" -> 0
  )


  val br4_brn_bb = Map(
    "for_body" -> 0,
    "for_end" -> 1
  )


  val br7_brn_bb = Map(
    "for_inc" -> 0
  )


  val br9_brn_bb = Map(
    "for_cond" -> 0
  )


  val entry_activate = Map(
    "br0" -> 0
  )


  val for_cond_activate = Map(
    "phi1" -> 0,
    "phi2" -> 1,
    "icmp3" -> 2,
    "br4" -> 3
  )


  val for_body_activate = Map(
    "add5" -> 0,
    "mul6" -> 1,
    "br7" -> 2
  )


  val for_inc_activate = Map(
    "add8" -> 0,
    "br9" -> 1
  )


  val for_end_activate = Map(
    "ret10" -> 0
  )


  val phi1_phi_in = Map(
    "data_0" -> 0,
    "mul6" -> 1
  )


  val phi2_phi_in = Map(
    "const_0" -> 0,
    "add8" -> 1
  )


  //  %sum.0 = phi i32 [ %a, %entry ], [ %mul, %for.inc ], !UID !11, !ScalaLabel !12
  val phi1_in = Map(
    "data_0" -> 0,
    "mul6" -> 0
  )


  //  %i.0 = phi i32 [ 0, %entry ], [ %inc, %for.inc ], !UID !13, !ScalaLabel !14
  val phi2_in = Map(
    "add8" -> 0
  )


  //  %cmp = icmp slt i32 %i.0, %n, !UID !15, !ScalaLabel !16
  val icmp3_in = Map(
    "phi2" -> 0,
    "data_2" -> 0
  )


  //  br i1 %cmp, label %for.body, label %for.end, !UID !17, !BB_UID !18, !ScalaLabel !19
  val br4_in = Map(
    "icmp3" -> 0
  )


  //  %add = add i32 %sum.0, %a, !UID !20, !ScalaLabel !21
  val add5_in = Map(
    "phi1" -> 0,
    "data_0" -> 1
  )


  //  %mul = mul i32 %add, %b, !UID !22, !ScalaLabel !23
  val mul6_in = Map(
    "add5" -> 0,
    "data_1" -> 0
  )


  //  %inc = add nsw i32 %i.0, 1, !UID !27, !ScalaLabel !28
  val add8_in = Map(
    "phi2" -> 1
  )


  //  ret i32 %sum.0, !UID !41, !BB_UID !42, !ScalaLabel !43
  val ret10_in = Map(
    "phi1" -> 1
  )


}




  /* ================================================================== *
   *                   PRINTING PORTS DEFINITION                        *
   * ================================================================== */


abstract class test03DFIO(implicit val p: Parameters) extends Module with CoreParams {
  val io = IO(new Bundle {
    val data_0 = Flipped(Decoupled(new DataBundle))
    val data_1 = Flipped(Decoupled(new DataBundle))
    val data_2 = Flipped(Decoupled(new DataBundle))
    val entry  = Flipped(Decoupled(new ControlBundle))
    val start = Input(new Bool())

    val CacheResp = Flipped(Valid(new CacheRespT))
    val CacheReq = Decoupled(new CacheReq)
    val result = Decoupled(new DataBundle)
  })
}




  /* ================================================================== *
   *                   PRINTING MODULE DEFINITION                       *
   * ================================================================== */


class test03DF(implicit p: Parameters) extends test03DFIO()(p) {



  /* ================================================================== *
   *                   PRINTING MEMORY SYSTEM                           *
   * ================================================================== */


	val StackPointer = Module(new Stack(NumOps = 1))

	val RegisterFile = Module(new TypeStackFile(ID=0,Size=32,NReads=2,NWrites=2)
		            (WControl=new WriteMemoryController(NumOps=2,BaseSize=2,NumEntries=2))
		            (RControl=new ReadMemoryController(NumOps=2,BaseSize=2,NumEntries=2)))

	val CacheMem = Module(new UnifiedController(ID=0,Size=32,NReads=2,NWrites=2)
		            (WControl=new WriteMemoryController(NumOps=2,BaseSize=2,NumEntries=2))
		            (RControl=new ReadMemoryController(NumOps=2,BaseSize=2,NumEntries=2))
		            (RWArbiter=new ReadWriteArbiter()))

  io.CacheReq <> CacheMem.io.CacheReq
  CacheMem.io.CacheResp <> io.CacheResp



  /* ================================================================== *
   *                   PRINTING LOOP HEADERS                            *
   * ================================================================== */


  val loop_L_7_liveIN_0 = Module(new LiveInNode(NumOuts = 2, ID = 0))
  val loop_L_7_liveIN_1 = Module(new LiveInNode(NumOuts = 1, ID = 0))
  val loop_L_7_liveIN_2 = Module(new LiveInNode(NumOuts = 1, ID = 0))
  val loop_L_7_end   = Module(new LoopEnd(NumInputs = 1, NumOuts = 1, ID = 0)(p))



  /* ================================================================== *
   *                   PRINTING BASICBLOCKS                             *
   * ================================================================== */


  //Initializing BasicBlocks: 

  val entry = Module(new BasicBlockNoMaskNode(NumInputs = 1, NumOuts = 1, BID = 0)(p))

  val for_cond = Module(new BasicBlockNode(NumInputs = 2, NumOuts = 8, NumPhi = 2, BID = 1)(p))

  val for_body = Module(new BasicBlockNoMaskNode(NumInputs = 1, NumOuts = 3, BID = 2)(p))

  val for_inc = Module(new BasicBlockNoMaskNode(NumInputs = 1, NumOuts = 2, BID = 3)(p))

  val for_end = Module(new BasicBlockNoMaskNode(NumInputs = 1, NumOuts = 1, BID = 4)(p))






  /* ================================================================== *
   *                   PRINTING INSTRUCTIONS                            *
   * ================================================================== */


  //Initializing Instructions: 

  // [BasicBlock]  entry:

  //  br label %for.cond, !UID !8, !BB_UID !9, !ScalaLabel !10
  val br0 = Module (new UBranchNode(ID = 0)(p))



  // [BasicBlock]  for.cond:

  //  %sum.0 = phi i32 [ %a, %entry ], [ %mul, %for.inc ], !UID !11, !ScalaLabel !12
  val phi1 = Module (new PhiNode(NumInputs = 2, NumOuts = 2, ID = 1)(p))


  //  %i.0 = phi i32 [ 0, %entry ], [ %inc, %for.inc ], !UID !13, !ScalaLabel !14
  val phi2 = Module (new PhiNode(NumInputs = 2, NumOuts = 2, ID = 2)(p))


  //  %cmp = icmp slt i32 %i.0, %n, !UID !15, !ScalaLabel !16
  val icmp3 = Module (new IcmpNode(NumOuts = 1, ID = 3, opCode = "ULT")(sign=false)(p))


  //  br i1 %cmp, label %for.body, label %for.end, !UID !17, !BB_UID !18, !ScalaLabel !19
  val br4 = Module (new CBranchNode(ID = 4)(p))

  val for_cond_expand = Module(new ExpandNode(NumOuts=4, ID=0))



  // [BasicBlock]  for.body:

  //  %add = add i32 %sum.0, %a, !UID !20, !ScalaLabel !21
  val add5 = Module (new ComputeNode(NumOuts = 1, ID = 5, opCode = "add")(sign=false)(p))


  //  %mul = mul i32 %add, %b, !UID !22, !ScalaLabel !23
  val mul6 = Module (new ComputeNode(NumOuts = 1, ID = 6, opCode = "mul")(sign=false)(p))


  //  br label %for.inc, !UID !24, !BB_UID !25, !ScalaLabel !26
  val br7 = Module (new UBranchNode(ID = 7)(p))



  // [BasicBlock]  for.inc:

  //  %inc = add nsw i32 %i.0, 1, !UID !27, !ScalaLabel !28
  val add8 = Module (new ComputeNode(NumOuts = 1, ID = 8, opCode = "add")(sign=false)(p))


  //  br label %for.cond, !llvm.loop !29, !UID !38, !BB_UID !39, !ScalaLabel !40
  val br9 = Module (new UBranchNode(ID = 9)(p))



  // [BasicBlock]  for.end:

  //  ret i32 %sum.0, !UID !41, !BB_UID !42, !ScalaLabel !43
  val ret10 = Module(new RetNode(NumOuts=1, ID=10))







  /* ================================================================== *
   *                   INITIALIZING PARAM                               *
   * ================================================================== */


  /**
    * Instantiating parameters
    */
  val param = Data_test03_FlowParam



  /* ================================================================== *
   *                   CONNECTING BASIC BLOCKS TO PREDICATE INSTRUCTIONS*
   * ================================================================== */


  /**
     * Connecting basic blocks to predicate instructions
     */


  entry.io.predicateIn(0) <> io.entry

  /**
    * Connecting basic blocks to predicate instructions
    */

  //Connecting br0 to for_cond
  for_cond.io.predicateIn(param.for_cond_pred("br0")) <> br0.io.Out(param.br0_brn_bb("for_cond"))


  //Connecting br4 to for_body
  for_body.io.predicateIn(param.for_body_pred("br4")) <> br4.io.Out(param.br4_brn_bb("for_body"))


  //Connecting br4 to for_end
  for_cond_expand.io.InData <> br4.io.Out(param.br4_brn_bb("for_end"))
  for_end.io.predicateIn(param.for_end_pred("br4")) <> for_cond_expand.io.Out(0)


  //Connecting br7 to for_inc
  for_inc.io.predicateIn(param.for_inc_pred("br7")) <> br7.io.Out(param.br7_brn_bb("for_inc"))


  //Connecting br9 to for_cond
  for_cond.io.predicateIn(param.for_cond_pred("br9")) <> br9.io.Out(param.br9_brn_bb("for_cond"))




  /* ================================================================== *
   *                   CONNECTING BASIC BLOCKS TO INSTRUCTIONS          *
   * ================================================================== */


  /**
    * Wiring enable signals to the instructions
    */
  //Wiring enable signals

  br0.io.enable <> entry.io.Out(param.entry_activate("br0"))



  phi1.io.enable <> for_cond.io.Out(param.for_cond_activate("phi1"))

  phi2.io.enable <> for_cond.io.Out(param.for_cond_activate("phi2"))

  icmp3.io.enable <> for_cond.io.Out(param.for_cond_activate("icmp3"))

  br4.io.enable <> for_cond.io.Out(param.for_cond_activate("br4"))

  for_cond_expand.io.enable <> for_cond.io.Out(7)

  loop_L_7_liveIN_0.io.enable <> for_cond.io.Out(4)
  loop_L_7_liveIN_1.io.enable <> for_cond.io.Out(5)
  loop_L_7_liveIN_2.io.enable <> for_cond.io.Out(6)

  loop_L_7_liveIN_0.io.Finish <> for_cond_expand.io.Out(1)
  loop_L_7_liveIN_1.io.Finish <> for_cond_expand.io.Out(2)
  loop_L_7_liveIN_2.io.Finish <> for_cond_expand.io.Out(3)



  add5.io.enable <> for_body.io.Out(param.for_body_activate("add5"))

  mul6.io.enable <> for_body.io.Out(param.for_body_activate("mul6"))

  br7.io.enable <> for_body.io.Out(param.for_body_activate("br7"))



  add8.io.enable <> for_inc.io.Out(param.for_inc_activate("add8"))

  br9.io.enable <> for_inc.io.Out(param.for_inc_activate("br9"))



  ret10.io.enable <> for_end.io.Out(param.for_end_activate("ret10"))





  /* ================================================================== *
   *                   DUMPING PHI NODES                                *
   * ================================================================== */


  /**
    * Connecting PHI Masks
    */
  //Connect PHI node

  // Wiring Live in to PHI node

  phi1.io.InData(param.phi1_phi_in("data_0")) <> loop_L_7_liveIN_0.io.Out(param.phi1_in("data_0"))

  phi1.io.InData(param.phi1_phi_in("mul6")) <> mul6.io.Out(param.phi1_in("mul6"))

  phi2.io.InData(param.phi2_phi_in("const_0")).bits.data := 0.U
  phi2.io.InData(param.phi2_phi_in("const_0")).bits.predicate := true.B
  phi2.io.InData(param.phi2_phi_in("const_0")).valid := true.B

  phi2.io.InData(param.phi2_phi_in("add8")) <> add8.io.Out(param.phi2_in("add8"))

  /**
    * Connecting PHI Masks
    */
  //Connect PHI node

  phi1.io.Mask <> for_cond.io.MaskBB(0)

  phi2.io.Mask <> for_cond.io.MaskBB(1)



  /* ================================================================== *
   *                   CONNECTING LOOPHEADERS                           *
   * ================================================================== */


  // Connecting function argument to the loop header
  //i32 %a
  loop_L_7_liveIN_0.io.InData <> io.data_0

  // Connecting function argument to the loop header
  //i32 %b
  loop_L_7_liveIN_1.io.InData <> io.data_1

  // Connecting function argument to the loop header
  //i32 %n
  loop_L_7_liveIN_2.io.InData <> io.data_2

  loop_L_7_end.io.inputArg(0) <> phi1.io.Out(0)



  /* ================================================================== *
   *                   DUMPING DATAFLOW                                 *
   * ================================================================== */


  /**
    * Connecting Dataflow signals
    */

  // Wiring instructions
  icmp3.io.LeftIO <> phi2.io.Out(param.icmp3_in("phi2"))

  // Wiring Binary instruction to the loop header
  icmp3.io.RightIO <> loop_L_7_liveIN_2.io.Out(param.icmp3_in("data_2"))

  // Wiring Branch instruction
  br4.io.CmpIO <> icmp3.io.Out(param.br4_in("icmp3"))

  // Wiring instructions
  add5.io.LeftIO <> phi1.io.Out(param.add5_in("phi1"))

  // Wiring Binary instruction to the loop header
  add5.io.RightIO <> loop_L_7_liveIN_0.io.Out(param.add5_in("data_0"))

  // Wiring instructions
  mul6.io.LeftIO <> add5.io.Out(param.mul6_in("add5"))

  // Wiring Binary instruction to the loop header
  mul6.io.RightIO <> loop_L_7_liveIN_1.io.Out(param.mul6_in("data_1"))

  // Wiring instructions
  add8.io.LeftIO <> phi2.io.Out(param.add8_in("phi2"))

  // Wiring constant
  add8.io.RightIO.bits.data := 1.U
  add8.io.RightIO.bits.predicate := true.B
  add8.io.RightIO.valid := true.B

  // Wiring return instructions
  ret10.io.InputIO <> phi1.io.Out(param.ret10_in("phi1"))
  io.result <> ret10.io.Out(0)


}

import java.io.{File, FileWriter}
object test03Main extends App {
  val dir = new File("RTL/test03") ; dir.mkdirs
  implicit val p = config.Parameters.root((new MiniConfig).toInstance)
  val chirrtl = firrtl.Parser.parse(chisel3.Driver.emit(() => new test03DF()))

  val verilogFile = new File(dir, s"${chirrtl.main}.v")
  val verilogWriter = new FileWriter(verilogFile)
  val compileResult = (new firrtl.VerilogCompiler).compileAndEmit(firrtl.CircuitState(chirrtl, firrtl.ChirrtlForm))
  val compiledStuff = compileResult.getEmittedCircuit
  verilogWriter.write(compiledStuff.value)
  verilogWriter.close()
}

