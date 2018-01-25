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

object Data_test07_FlowParam{

  val entry_pred = Map(
    "active" -> 0
  )


  val for_cond_pred = Map(
    "br0" -> 0,
    "br8" -> 1
  )


  val for_inc_pred = Map(
    "br6" -> 0
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


  val br6_brn_bb = Map(
    "for_inc" -> 0
  )


  val br8_brn_bb = Map(
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
    "sub5" -> 0,
    "br6" -> 1
  )


  val for_inc_activate = Map(
    "add7" -> 0,
    "br8" -> 1
  )


  val for_end_activate = Map(
    "ret9" -> 0
  )


  val phi1_phi_in = Map(
    "const_0" -> 0,
    "add7" -> 1
  )


  val phi2_phi_in = Map(
    "const_0" -> 0,
    "sub5" -> 1
  )


  //  %i.0 = phi i32 [ 0, %entry ], [ %inc, %for.inc ], !UID !11, !ScalaLabel !12
  val phi1_in = Map(
    "add7" -> 0
  )


  //  %j.0 = phi i32 [ 100, %entry ], [ %sub, %for.inc ], !UID !13, !ScalaLabel !14
  val phi2_in = Map(
    "sub5" -> 0
  )


  //  %cmp = icmp slt i32 %i.0, 5, !UID !15, !ScalaLabel !16
  val icmp3_in = Map(
    "phi1" -> 0
  )


  //  br i1 %cmp, label %for.body, label %for.end, !UID !17, !BB_UID !18, !ScalaLabel !19
  val br4_in = Map(
    "icmp3" -> 0
  )


  //  %sub = sub i32 %j.0, 1, !UID !20, !ScalaLabel !21
  val sub5_in = Map(
    "phi2" -> 0
  )


  //  %inc = add nsw i32 %i.0, 1, !UID !25, !ScalaLabel !26
  val add7_in = Map(
    "phi1" -> 1
  )


  //  ret i32 %j.0, !UID !38, !BB_UID !39, !ScalaLabel !40
  val ret9_in = Map(
    "phi2" -> 1
  )


}




  /* ================================================================== *
   *                   PRINTING PORTS DEFINITION                        *
   * ================================================================== */


abstract class test07DFIO(implicit val p: Parameters) extends Module with CoreParams {
  val io = IO(new Bundle {
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


class test07DF(implicit p: Parameters) extends test07DFIO()(p) {



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


  val loop_L_3_end   = Module(new LoopEnd(NumInputs = 1, NumOuts = 1, ID = 0)(p))



  /* ================================================================== *
   *                   PRINTING BASICBLOCKS                             *
   * ================================================================== */


  //Initializing BasicBlocks: 

  val entry = Module(new BasicBlockNoMaskNode(NumInputs = 1, NumOuts = 1, BID = 0)(p))

  val for_cond = Module(new BasicBlockNode(NumInputs = 2, NumOuts = 5, NumPhi = 2, BID = 1)(p))

  val for_body = Module(new BasicBlockNoMaskNode(NumInputs = 1, NumOuts = 2, BID = 2)(p))

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

  //  %i.0 = phi i32 [ 0, %entry ], [ %inc, %for.inc ], !UID !11, !ScalaLabel !12
  val phi1 = Module (new PhiNode(NumInputs = 2, NumOuts = 2, ID = 1)(p))


  //  %j.0 = phi i32 [ 100, %entry ], [ %sub, %for.inc ], !UID !13, !ScalaLabel !14
  val phi2 = Module (new PhiNode(NumInputs = 2, NumOuts = 2, ID = 2)(p))


  //  %cmp = icmp slt i32 %i.0, 5, !UID !15, !ScalaLabel !16
  val icmp3 = Module (new IcmpNode(NumOuts = 1, ID = 3, opCode = "ULT")(sign=false)(p))


  //  br i1 %cmp, label %for.body, label %for.end, !UID !17, !BB_UID !18, !ScalaLabel !19
  val br4 = Module (new CBranchNode(ID = 4)(p))

  val for_cond_expand = Module(new ExpandNode(NumOuts=1, ID=0))



  // [BasicBlock]  for.body:

  //  %sub = sub i32 %j.0, 1, !UID !20, !ScalaLabel !21
  val sub5 = Module (new ComputeNode(NumOuts = 1, ID = 5, opCode = "sub")(sign=false)(p))


  //  br label %for.inc, !UID !22, !BB_UID !23, !ScalaLabel !24
  val br6 = Module (new UBranchNode(ID = 6)(p))



  // [BasicBlock]  for.inc:

  //  %inc = add nsw i32 %i.0, 1, !UID !25, !ScalaLabel !26
  val add7 = Module (new ComputeNode(NumOuts = 1, ID = 7, opCode = "add")(sign=false)(p))


  //  br label %for.cond, !llvm.loop !27, !UID !35, !BB_UID !36, !ScalaLabel !37
  val br8 = Module (new UBranchNode(ID = 8)(p))



  // [BasicBlock]  for.end:

  //  ret i32 %j.0, !UID !38, !BB_UID !39, !ScalaLabel !40
  val ret9 = Module(new RetNode(NumOuts=1, ID=9))







  /* ================================================================== *
   *                   INITIALIZING PARAM                               *
   * ================================================================== */


  /**
    * Instantiating parameters
    */
  val param = Data_test07_FlowParam



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


  //Connecting br6 to for_inc
  for_inc.io.predicateIn(param.for_inc_pred("br6")) <> br6.io.Out(param.br6_brn_bb("for_inc"))


  //Connecting br8 to for_cond
  for_cond.io.predicateIn(param.for_cond_pred("br8")) <> br8.io.Out(param.br8_brn_bb("for_cond"))




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

  for_cond_expand.io.enable <> for_cond.io.Out(4)





  sub5.io.enable <> for_body.io.Out(param.for_body_activate("sub5"))

  br6.io.enable <> for_body.io.Out(param.for_body_activate("br6"))



  add7.io.enable <> for_inc.io.Out(param.for_inc_activate("add7"))

  br8.io.enable <> for_inc.io.Out(param.for_inc_activate("br8"))



  ret9.io.enable <> for_end.io.Out(param.for_end_activate("ret9"))





  /* ================================================================== *
   *                   DUMPING PHI NODES                                *
   * ================================================================== */


  /**
    * Connecting PHI Masks
    */
  //Connect PHI node

  phi1.io.InData(param.phi1_phi_in("const_0")).bits.data := 0.U
  phi1.io.InData(param.phi1_phi_in("const_0")).bits.predicate := true.B
  phi1.io.InData(param.phi1_phi_in("const_0")).valid := true.B

  phi1.io.InData(param.phi1_phi_in("add7")) <> add7.io.Out(param.phi1_in("add7"))

  phi2.io.InData(param.phi2_phi_in("const_0")).bits.data := 100.U
  phi2.io.InData(param.phi2_phi_in("const_0")).bits.predicate := true.B
  phi2.io.InData(param.phi2_phi_in("const_0")).valid := true.B

  phi2.io.InData(param.phi2_phi_in("sub5")) <> sub5.io.Out(param.phi2_in("sub5"))

  /**
    * Connecting PHI Masks
    */
  //Connect PHI node

  phi1.io.Mask <> for_cond.io.MaskBB(0)

  phi2.io.Mask <> for_cond.io.MaskBB(1)



  /* ================================================================== *
   *                   CONNECTING LOOPHEADERS                           *
   * ================================================================== */


  loop_L_3_end.io.inputArg(0) <> phi2.io.Out(0)



  /* ================================================================== *
   *                   DUMPING DATAFLOW                                 *
   * ================================================================== */


  /**
    * Connecting Dataflow signals
    */

  // Wiring instructions
  icmp3.io.LeftIO <> phi1.io.Out(param.icmp3_in("phi1"))

  // Wiring constant
  icmp3.io.RightIO.bits.data := 5.U
  icmp3.io.RightIO.bits.predicate := true.B
  icmp3.io.RightIO.valid := true.B

  // Wiring Branch instruction
  br4.io.CmpIO <> icmp3.io.Out(param.br4_in("icmp3"))

  // Wiring instructions
  sub5.io.LeftIO <> phi2.io.Out(param.sub5_in("phi2"))

  // Wiring constant
  sub5.io.RightIO.bits.data := 1.U
  sub5.io.RightIO.bits.predicate := true.B
  sub5.io.RightIO.valid := true.B

  // Wiring instructions
  add7.io.LeftIO <> phi1.io.Out(param.add7_in("phi1"))

  // Wiring constant
  add7.io.RightIO.bits.data := 1.U
  add7.io.RightIO.bits.predicate := true.B
  add7.io.RightIO.valid := true.B

  // Wiring return instructions
  ret9.io.InputIO <> phi2.io.Out(param.ret9_in("phi2"))
  io.result <> ret9.io.Out(0)


}

import java.io.{File, FileWriter}
object test07Main extends App {
  val dir = new File("RTL/test07") ; dir.mkdirs
  implicit val p = config.Parameters.root((new MiniConfig).toInstance)
  val chirrtl = firrtl.Parser.parse(chisel3.Driver.emit(() => new test07DF()))

  val verilogFile = new File(dir, s"${chirrtl.main}.v")
  val verilogWriter = new FileWriter(verilogFile)
  val compileResult = (new firrtl.VerilogCompiler).compileAndEmit(firrtl.CircuitState(chirrtl, firrtl.ChirrtlForm))
  val compiledStuff = compileResult.getEmittedCircuit
  verilogWriter.write(compiledStuff.value)
  verilogWriter.close()
}

