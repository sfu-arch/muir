package concurrent

import chisel3._
import chisel3.util._
import chisel3.Module
import chisel3.testers._
import chisel3.iotesters.{ChiselFlatSpec, Driver, OrderedDecoupledHWIOTester, PeekPokeTester}
import org.scalatest.{FlatSpec, Matchers}
import muxes._
import config._
import control.{BasicBlockNoMaskNode, BasicBlockNode}
import util._
import interfaces._
import regfile._
import memory._
import stack._
import arbiters._
import loop._
import node._


/**
  * This Object should be initialized at the first step
  * It contains all the transformation from indices to their module's name
  */

object Data_int_exp_FlowParam{

  val entry_pred = Map(
    "active" -> 0
  )


  val for_cond_pred = Map(
    "br_0" -> 0,
    "br_10" -> 1
  )


  val for_inc_pred = Map(
    "br_8" -> 0
  )


  val for_body_pred = Map(
    "br_3" -> 0
  )


  val for_end_pred = Map(
    "br_3" -> 0
  )


  val br_0_brn_bb = Map(
    "for_cond" -> 0
  )


  val br_3_brn_bb = Map(
    "for_body" -> 0,
    "for_end" -> 1
  )


  val br_8_brn_bb = Map(
    "for_inc" -> 0
  )


  val br_10_brn_bb = Map(
    "for_cond" -> 0
  )


  val entry_activate = Map(
    "br_0" -> 0
  )


  val for_cond_activate = Map(
    "phi_1" -> 0,
    "icmp_2" -> 1,
    "br_3" -> 2
  )


  val for_body_activate = Map(
    "load_4" -> 0,
    "load_5" -> 1,
    "mul_6" -> 2,
    "store_7" -> 3,
    "br_8" -> 4
  )


  val for_inc_activate = Map(
    "add_9" -> 0,
    "br_10" -> 1
  )


  val for_end_activate = Map(
    "load_11" -> 0,
    "ret_12" -> 1
  )


  val phi_1_phi_in = Map(
    "const_0" -> 0,
    "add_9" -> 1
  )


  //  %i.0 = phi i32 [ 0, %entry ], [ %inc, %for.inc ], !UID !9
  val phi_1_in = Map( 
    "add_9" -> 0
  )


  //  %cmp = icmp ult i32 %i.0, %exp, !UID !10
  val icmp_2_in = Map( 
    "phi_1" -> 0,
    "data_1" -> 1
  )


  //  br i1 %cmp, label %for.body, label %for.end, !UID !11, !BB_UID !12
  val br_3_in = Map( 
    "icmp_2" -> 0
  )


  //  %0 = load i32, i32* %m, align 4, !UID !13
  val load_4_in = Map( 
    "data_0" -> 0
  )


  //  %1 = load i32, i32* %m, align 4, !UID !14
  val load_5_in = Map( 
    "data_0" -> 0
  )


  //  %mul = mul i32 %1, %0, !UID !15
  val mul_6_in = Map( 
    "load_5" -> 0,
    "load_4" -> 0
  )


  //  store i32 %mul, i32* %m, align 4, !UID !16
  val store_7_in = Map( 
    "mul_6" -> 0,
    "data_0" -> 1
  )


  //  %inc = add i32 %i.0, 1, !UID !19
  val add_9_in = Map( 
    "phi_1" -> 1
  )


  //  %2 = load i32, i32* %m, align 4, !UID !31
  val load_11_in = Map( 
    "data_0" -> 0
  )


  //  ret i32 %2, !UID !32, !BB_UID !33
  val ret_12_in = Map( 
    "load_11" -> 0
  )


}




  /* ================================================================== *
   *                   PRINTING PORTS DEFINITION                        *
   * ================================================================== */


abstract class int_expDFIO(implicit val p: Parameters) extends Module with CoreParams {
  val io = IO(new Bundle {
    val data_0 = Flipped(Decoupled(new DataBundle))
    val data_1 = Flipped(Decoupled(new DataBundle))
    val entry  = Flipped(Decoupled(new ControlBundle))
    val pred = Decoupled(new Bool())
    val start = Input(new Bool())
    val result = Decoupled(new DataBundle)
  })
}




  /* ================================================================== *
   *                   PRINTING MODULE DEFINITION                       *
   * ================================================================== */


class int_expDF(implicit p: Parameters) extends int_expDFIO()(p) {



  /* ================================================================== *
   *                   PRINTING MEMORY SYSTEM                           *
   * ================================================================== */


	val StackPointer = Module(new Stack(NumOps = 1))

	val RegisterFile = Module(new TypeStackFile(ID=0,Size=32,NReads=3,NWrites=1)
		            (WControl=new WriteMemoryController(NumOps=1,BaseSize=2,NumEntries=2))
		            (RControl=new ReadMemoryController(NumOps=3,BaseSize=2,NumEntries=2)))

	val CacheMem = Module(new UnifiedController(ID=0,Size=32,NReads=3,NWrites=1)
		            (WControl=new WriteMemoryController(NumOps=1,BaseSize=2,NumEntries=2))
		            (RControl=new ReadMemoryController(NumOps=3,BaseSize=2,NumEntries=2))
		            (RWArbiter=new ReadWriteArbiter()))


  /* ================================================================== *
   *                   PRINTING LOOP HEADERS                            *
   * ================================================================== */


  val loop_L_6_start = Module(new LoopStart(NumInputs = 2, NumOuts = 2, ID = 0)(p))



  /* ================================================================== *
   *                   PRINTING BASICBLOCKS                             *
   * ================================================================== */


  //Initializing BasicBlocks: 

  val entry = Module(new BasicBlockNoMaskNode(NumInputs = 1, NumOuts = 1, BID = 0)(p))

  val for_cond = Module(new BasicBlockNode(NumInputs = 2, NumOuts = 6, NumPhi = 1, BID = 1)(p))

  val for_body = Module(new BasicBlockNoMaskNode(NumInputs = 1, NumOuts = 5, BID = 2)(p))

  val for_inc = Module(new BasicBlockNoMaskNode(NumInputs = 1, NumOuts = 2, BID = 3)(p))

  val for_end = Module(new BasicBlockNoMaskNode(NumInputs = 1, NumOuts = 2, BID = 4)(p))






  /* ================================================================== *
   *                   PRINTING INSTRUCTIONS                            *
   * ================================================================== */


  //Initializing Instructions: 

  // [BasicBlock]entry:

  //  br label %for.cond, !UID !7, !BB_UID !8
  val br_0 = Module (new UBranchNode(ID = 0)(p))



  // [BasicBlock]for.cond:

  //  %i.0 = phi i32 [ 0, %entry ], [ %inc, %for.inc ], !UID !9
  val phi_1 = Module (new PhiNode(NumInputs = 2, NumOuts = 2, ID = 1)(p))


  //  %cmp = icmp ult i32 %i.0, %exp, !UID !10
  val icmp_2 = Module (new IcmpNode(NumOuts = 1, ID = 2, opCode = "ULT")(sign=false)(p))


  //  br i1 %cmp, label %for.body, label %for.end, !UID !11, !BB_UID !12
  val br_3 = Module (new CBranchNode(ID = 3)(p))

  val for_cond_expand = Module(new ExpandNode(NumOuts=3, ID=0))



  // [BasicBlock]for.body:

  //  %0 = load i32, i32* %m, align 4, !UID !13
  val load_4 = Module(new UnTypLoad(NumPredOps=0, NumSuccOps=0, NumOuts=1,ID=4,RouteID=0))


  //  %1 = load i32, i32* %m, align 4, !UID !14
  val load_5 = Module(new UnTypLoad(NumPredOps=0, NumSuccOps=0, NumOuts=1,ID=5,RouteID=1))


  //  %mul = mul i32 %1, %0, !UID !15
  val mul_6 = Module (new ComputeNode(NumOuts = 1, ID = 6, opCode = "mul")(sign=false)(p))


  //  store i32 %mul, i32* %m, align 4, !UID !16
  val store_7 = Module(new UnTypStore(NumPredOps=0, NumSuccOps=0, NumOuts=1,ID=7,RouteID=0))


  //  br label %for.inc, !UID !17, !BB_UID !18
  val br_8 = Module (new UBranchNode(ID = 8)(p))



  // [BasicBlock]for.inc:

  //  %inc = add i32 %i.0, 1, !UID !19
  val add_9 = Module (new ComputeNode(NumOuts = 1, ID = 9, opCode = "add")(sign=false)(p))


  //  br label %for.cond, !llvm.loop !20, !UID !29, !BB_UID !30
  val br_10 = Module (new UBranchNode(ID = 10)(p))



  // [BasicBlock]for.end:

  //  %2 = load i32, i32* %m, align 4, !UID !31
  val load_11 = Module(new UnTypLoad(NumPredOps=0, NumSuccOps=0, NumOuts=1,ID=11,RouteID=2))


  //  ret i32 %2, !UID !32, !BB_UID !33
  val ret_12 = Module(new RetNode(NumOuts=1, ID=12))







  /* ================================================================== *
   *                   INITIALIZING PARAM                               *
   * ================================================================== */


  /**
    * Instantiating parameters
    */
  val param = Data_int_exp_FlowParam



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

  //Connecting br_0 to for_cond
  for_cond.io.predicateIn(param.for_cond_pred("br_0")) <> br_0.io.Out(param.br_0_brn_bb("for_cond"))


  //Connecting br_3 to for_body
  for_body.io.predicateIn(param.for_body_pred("br_3")) <> br_3.io.Out(param.br_3_brn_bb("for_body"))


  //Connecting br_3 to for_end
  for_cond_expand.io.InData <> br_3.io.Out(param.br_3_brn_bb("for_end"))
  for_end.io.predicateIn(param.for_end_pred("br_3")) <> for_cond_expand.io.Out(0)


  //Connecting br_8 to for_inc
  for_inc.io.predicateIn(param.for_inc_pred("br_8")) <> br_8.io.Out(param.br_8_brn_bb("for_inc"))


  //Connecting br_10 to for_cond
  for_cond.io.predicateIn(param.for_cond_pred("br_10")) <> br_10.io.Out(param.br_10_brn_bb("for_cond"))



  // There is no detach instruction




  /* ================================================================== *
   *                   CONNECTING BASIC BLOCKS TO INSTRUCTIONS          *
   * ================================================================== */


  /**
    * Wireing enable signals to the instructions
    */
  //Wiring enable signals

  br_0.io.enable <> entry.io.Out(param.entry_activate("br_0"))



  phi_1.io.enable <> for_cond.io.Out(param.for_cond_activate("phi_1"))

  icmp_2.io.enable <> for_cond.io.Out(param.for_cond_activate("icmp_2"))

  br_3.io.enable <> for_cond.io.Out(param.for_cond_activate("br_3"))

  for_cond_expand.io.enable <> for_cond.io.Out(5)

  loop_L_6_start.io.enableSignal(0) <> for_cond.io.Out(3)
  loop_L_6_start.io.enableSignal(1) <> for_cond.io.Out(4)

  loop_L_6_start.io.Finish(0) <> for_cond_expand.io.Out(1)
  loop_L_6_start.io.Finish(1) <> for_cond_expand.io.Out(2)



  load_4.io.enable <> for_body.io.Out(param.for_body_activate("load_4"))

  load_5.io.enable <> for_body.io.Out(param.for_body_activate("load_5"))

  mul_6.io.enable <> for_body.io.Out(param.for_body_activate("mul_6"))

  store_7.io.enable <> for_body.io.Out(param.for_body_activate("store_7"))

  br_8.io.enable <> for_body.io.Out(param.for_body_activate("br_8"))



  add_9.io.enable <> for_inc.io.Out(param.for_inc_activate("add_9"))

  br_10.io.enable <> for_inc.io.Out(param.for_inc_activate("br_10"))



  load_11.io.enable <> for_end.io.Out(param.for_end_activate("load_11"))

  ret_12.io.enable <> for_end.io.Out(param.for_end_activate("ret_12"))





  /* ================================================================== *
   *                   DUMPING PHI NODES                                *
   * ================================================================== */


  /**
    * Connecting PHI Masks
    */
  //Connect PHI node

  phi_1.io.InData(param.phi_1_phi_in("const_0")).bits.data := 0.U
  phi_1.io.InData(param.phi_1_phi_in("const_0")).bits.predicate := true.B
  phi_1.io.InData(param.phi_1_phi_in("const_0")).valid := true.B

  phi_1.io.InData(param.phi_1_phi_in("add_9")) <> add_9.io.Out(0)

  /**
    * Connecting PHI Masks
    */
  //Connect PHI node

  phi_1.io.Mask <> for_cond.io.MaskBB(0)



  /* ================================================================== *
   *                   CONNECTING LOOPHEADERS                           *
   * ================================================================== */


  // Connecting function argument to the loop header
  //i32* %m
  loop_L_6_start.io.inputArg(0) <> io.data_0

  // Connecting function argument to the loop header
  //i32 %exp
  loop_L_6_start.io.inputArg(1) <> io.data_1



  /* ================================================================== *
   *                   DUMPING DATAFLOW                                 *
   * ================================================================== */


  /**
    * Connecting Dataflow signals
    */

  // Wiring instructions
  icmp_2.io.LeftIO <> phi_1.io.Out(param.icmp_2_in("phi_1"))

  // Wiring Binary instruction to the loop header
  icmp_2.io.RightIO <> loop_L_6_start.io.outputArg(1)

  // Wiring Branch instruction
  br_3.io.CmpIO <> icmp_2.io.Out(param.br_3_in("icmp_2"))

  // Wiring Load instruction to the function argument
  load_4.io.GepAddr <> io.data_0
  load_4.io.memResp <> RegisterFile.io.ReadOut(0)
  RegisterFile.io.ReadIn(0) <> load_4.io.memReq




  // Wiring Load instruction to the function argument
  load_5.io.GepAddr <> io.data_0
  load_5.io.memResp <> RegisterFile.io.ReadOut(1)
  RegisterFile.io.ReadIn(1) <> load_5.io.memReq




  // Wiring instructions
  mul_6.io.LeftIO <> load_5.io.Out(param.mul_6_in("load_5"))

  // Wiring instructions
  mul_6.io.RightIO <> load_4.io.Out(param.mul_6_in("load_4"))

  // Wiring Store instruction to the parent instruction
  store_7.io.GepAddr <> mul_6.io.Out(param.store_7_in("mul_6"))


  store_7.io.inData <> io.data_0
  store_7.io.memResp  <> RegisterFile.io.WriteOut(0)
  RegisterFile.io.WriteIn(0) <> store_7.io.memReq
  store_7.io.Out(0).ready := true.B




  // Wiring instructions
  add_9.io.LeftIO <> phi_1.io.Out(param.add_9_in("phi_1"))

  // Wiring constant
  add_9.io.RightIO.bits.data := 1.U
  add_9.io.RightIO.bits.predicate := true.B
  add_9.io.RightIO.valid := true.B

  // Wiring Load instruction to the function argument
  load_11.io.GepAddr <> io.data_0
  load_11.io.memResp <> CacheMem.io.ReadOut(2)
  RegisterFile.io.ReadIn(2) <> load_11.io.memReq




  // Wiring return instructions
  ret_12.io.InputIO <> load_11.io.Out(param.ret_12_in("load_11"))
  io.result <> ret_12.io.Out(0)

  io.pred.valid := false.B
  io.pred.bits := false.B


}

