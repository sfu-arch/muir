package dataflow

import FPU._
import accel._
import arbiters._
import chisel3._
import chisel3.util._
import chisel3.Module._
import chisel3.testers._
import chisel3.iotesters._
import config._
import control._
import interfaces._
import junctions._
import loop._
import memory._
import muxes._
import node._
import org.scalatest._
import regfile._
import stack._
import util._


  /* ================================================================== *
   *                   PRINTING PORTS DEFINITION                        *
   * ================================================================== */

abstract class mergeDFIO(implicit val p: Parameters) extends Module with CoreParams {
  val io = IO(new Bundle {
    val in = Flipped(Decoupled(new Call(List(32, 32, 32, 32))))
    val MemResp = Flipped(Valid(new MemResp))
    val MemReq = Decoupled(new MemReq)
    val out = Decoupled(new Call(List()))
  })
}

class mergeDF(implicit p: Parameters) extends mergeDFIO()(p) {


  /* ================================================================== *
   *                   PRINTING MEMORY MODULES                          *
   * ================================================================== */

  val MemCtrl = Module(new UnifiedController(ID = 0, Size = 32, NReads = 3, NWrites = 3)
  (WControl = new WriteMemoryController(NumOps = 3, BaseSize = 2, NumEntries = 2))
  (RControl = new ReadMemoryController(NumOps = 3, BaseSize = 2, NumEntries = 2))
  (RWArbiter = new ReadWriteArbiter()))

  io.MemReq <> MemCtrl.io.MemReq
  MemCtrl.io.MemResp <> io.MemResp

  val StackPointer = Module(new Stack(NumOps = 1))

  val InputSplitter = Module(new SplitCallNew(List(3, 8, 6, 6)))
  InputSplitter.io.In <> io.in



  /* ================================================================== *
   *                   PRINTING LOOP HEADERS                            *
   * ================================================================== */

  val Loop_0 = Module(new LoopBlockNode(NumIns = List(1, 1, 1, 2, 1, 1), NumOuts = List(), NumCarry = List(1, 1, 1), NumExits = 1, ID = 0))

  val Loop_1 = Module(new LoopBlockNode(NumIns = List(1, 1, 1, 1, 1), NumOuts = List(), NumCarry = List(1), NumExits = 1, ID = 1))



  /* ================================================================== *
   *                   PRINTING BASICBLOCK NODES                        *
   * ================================================================== */

  val bb_entry0 = Module(new BasicBlockNoMaskFastNode(NumInputs = 1, NumOuts = 4, BID = 0))

  val bb_merge_label2_loopexit1 = Module(new BasicBlockNoMaskFastNode(NumInputs = 1, NumOuts = 15, BID = 1))

  val bb_merge_label22 = Module(new BasicBlockNoMaskFastNode(NumInputs = 2, NumOuts = 2, BID = 2))

  val bb_for_body5_lr_ph3 = Module(new BasicBlockNoMaskFastNode(NumInputs = 1, NumOuts = 6, BID = 3))

  val bb_for_body54 = Module(new BasicBlockNode(NumInputs = 2, NumOuts = 13, NumPhi = 1, BID = 4))

  val bb_for_end14_loopexit5 = Module(new BasicBlockNoMaskFastNode(NumInputs = 1, NumOuts = 1, BID = 5))

  val bb_for_end146 = Module(new BasicBlockNoMaskFastNode(NumInputs = 2, NumOuts = 2, BID = 6))

  val bb_for_body17_lr_ph7 = Module(new BasicBlockNoMaskFastNode(NumInputs = 1, NumOuts = 3, BID = 7))

  val bb_for_body178 = Module(new BasicBlockNode(NumInputs = 2, NumOuts = 14, NumPhi = 3, BID = 8))

  val bb_if_then9 = Module(new BasicBlockNoMaskFastNode(NumInputs = 1, NumOuts = 4, BID = 9))

  val bb_if_else10 = Module(new BasicBlockNoMaskFastNode(NumInputs = 1, NumOuts = 4, BID = 10))

  val bb_if_end11 = Module(new BasicBlockNode(NumInputs = 2, NumOuts = 6, NumPhi = 2, BID = 11))

  val bb_for_end30_loopexit12 = Module(new BasicBlockNoMaskFastNode(NumInputs = 1, NumOuts = 1, BID = 12))

  val bb_for_end3013 = Module(new BasicBlockNoMaskFastNode(NumInputs = 2, NumOuts = 1, BID = 13))



  /* ================================================================== *
   *                   PRINTING INSTRUCTION NODES                       *
   * ================================================================== */

  //  %temp = alloca [2048 x i32], align 16, !UID !76
  val alloca_temp0 = Module(new AllocaNode(NumOuts=4, ID = 0, RouteID=0))

  //  %0 = bitcast [2048 x i32]* %temp to i8*, !dbg !81, !UID !82
  val bitcast_1 = Module(new BitCastNode(NumOuts = 0, ID = 1))

  //  %cmp67 = icmp sgt i32 %start, %m, !dbg !85, !UID !88
  val icmp_cmp672 = Module(new IcmpNode(NumOuts = 1, ID = 2, opCode = "ugt")(sign = false))

  //  br i1 %cmp67, label %merge_label2, label %merge_label2.loopexit, !dbg !89, !UID !90, !BB_UID !91
  val br_3 = Module(new CBranchNodeVariable(NumTrue = 1, NumFalse = 1, NumPredecessor = 0, ID = 3))

  //  %1 = sext i32 %start to i64, !dbg !89, !UID !92
  val sext4 = Module(new SextNode(NumOuts = 2))

  //  %scevgep = getelementptr [2048 x i32], [2048 x i32]* %temp, i64 0, i64 %1, !dbg !89, !UID !93
  val Gep_scevgep5 = Module(new GepNode(NumIns = 2, NumOuts = 1, ID = 5)(ElementSize = 4, ArraySize = List(8192)))

  //  %scevgep73 = bitcast i32* %scevgep to i8*, !UID !94
  val bitcast_scevgep736 = Module(new BitCastNode(NumOuts = 0, ID = 6))

  //  %scevgep74 = getelementptr i32, i32* %a, i64 %1, !dbg !89, !UID !95
  val Gep_scevgep747 = Module(new GepNode(NumIns = 1, NumOuts = 1, ID = 7)(ElementSize = 4, ArraySize = List()))

  //  %scevgep7475 = bitcast i32* %scevgep74 to i8*, !UID !96
  val bitcast_scevgep74758 = Module(new BitCastNode(NumOuts = 0, ID = 8))

  //  %2 = icmp sgt i32 %m, %start, !dbg !89, !UID !97
  val icmp_9 = Module(new IcmpNode(NumOuts = 1, ID = 9, opCode = "ugt")(sign = false))

  //  %smax = select i1 %2, i32 %m, i32 %start, !dbg !89, !UID !98
  val select_smax10 = Module(new SelectNode(NumOuts = 1, ID = 10))

  //  %3 = sub i32 %smax, %start, !dbg !89, !UID !99
  val binaryOp_11 = Module(new ComputeNode(NumOuts = 1, ID = 11, opCode = "sub")(sign = false))

  //  %4 = zext i32 %3 to i64, !dbg !89, !UID !100
  val sext12 = Module(new ZextNode(NumOuts = 1))

  //  %5 = shl nuw nsw i64 %4, 2, !dbg !89, !UID !101
  val binaryOp_13 = Module(new ComputeNode(NumOuts = 1, ID = 13, opCode = "shl")(sign = false))

  //  %6 = add nuw nsw i64 %5, 4, !dbg !89, !UID !102
  val binaryOp_14 = Module(new ComputeNode(NumOuts = 0, ID = 14, opCode = "add")(sign = false))

  //  br label %merge_label2, !dbg !105, !UID !107, !BB_UID !108
  val br_15 = Module(new UBranchNode(ID = 15))

  //  %cmp465 = icmp slt i32 %m, %stop, !dbg !110, !UID !112
  val icmp_cmp46516 = Module(new IcmpNode(NumOuts = 1, ID = 16, opCode = "ult")(sign = false))

  //  br i1 %cmp465, label %for.body5.lr.ph, label %for.end14, !dbg !113, !UID !114, !BB_UID !115
  val br_17 = Module(new CBranchNodeVariable(NumTrue = 1, NumFalse = 1, NumPredecessor = 0, ID = 17))

  //  %add = add nsw i32 %m, 1, !dbg !105, !UID !116
  val binaryOp_add18 = Module(new ComputeNode(NumOuts = 1, ID = 18, opCode = "add")(sign = false))

  //  %add9 = add nsw i32 %add, %stop, !UID !117
  val binaryOp_add919 = Module(new ComputeNode(NumOuts = 1, ID = 19, opCode = "add")(sign = false))

  //  %7 = sext i32 %m to i64, !dbg !113, !UID !118
  val sext20 = Module(new SextNode(NumOuts = 1))

  //  %8 = sext i32 %stop to i64, !dbg !113, !UID !119
  val sext21 = Module(new SextNode(NumOuts = 1))

  //  br label %for.body5, !dbg !113, !UID !120, !BB_UID !121
  val br_22 = Module(new UBranchNode(ID = 22))

  //  %indvars.iv69.in = phi i64 [ %indvars.iv69, %for.body5 ], [ %7, %for.body5.lr.ph ], !UID !122
  val phiindvars_iv69_in23 = Module(new PhiFastNode(NumInputs = 2, NumOutputs = 1, ID = 23, Res = false))

  //  %indvars.iv69 = add i64 %indvars.iv69.in, 1, !UID !123
  val binaryOp_indvars_iv6924 = Module(new ComputeNode(NumOuts = 4, ID = 24, opCode = "add")(sign = false))

  //  %arrayidx7 = getelementptr inbounds i32, i32* %a, i64 %indvars.iv69, !dbg !124, !UID !126
  val Gep_arrayidx725 = Module(new GepNode(NumIns = 1, NumOuts = 1, ID = 25)(ElementSize = 4, ArraySize = List()))

  //  %9 = load i32, i32* %arrayidx7, align 4, !dbg !124, !tbaa !127, !UID !131
  val ld_26 = Module(new UnTypLoad(NumPredOps = 0, NumSuccOps = 0, NumOuts = 1, ID = 26, RouteID = 0))

  //  %10 = trunc i64 %indvars.iv69 to i32, !dbg !132, !UID !133
  val trunc27 = Module(new TruncNode(NumOuts = 1))

  //  %sub = sub i32 %add9, %10, !dbg !132, !UID !134
  val binaryOp_sub28 = Module(new ComputeNode(NumOuts = 1, ID = 28, opCode = "sub")(sign = false))

  //  %idxprom10 = sext i32 %sub to i64, !dbg !135, !UID !136
  val sextidxprom1029 = Module(new SextNode(NumOuts = 1))

  //  %arrayidx11 = getelementptr inbounds [2048 x i32], [2048 x i32]* %temp, i64 0, i64 %idxprom10, !dbg !135, !UID !137
  val Gep_arrayidx1130 = Module(new GepNode(NumIns = 2, NumOuts = 1, ID = 30)(ElementSize = 4, ArraySize = List(8192)))

  //  store i32 %9, i32* %arrayidx11, align 4, !dbg !138, !tbaa !127, !UID !139
  val st_31 = Module(new UnTypStore(NumPredOps = 0, NumSuccOps = 0, ID = 31, RouteID = 0))

  //  %cmp4 = icmp slt i64 %indvars.iv69, %8, !dbg !110, !UID !140
  val icmp_cmp432 = Module(new IcmpNode(NumOuts = 1, ID = 32, opCode = "ult")(sign = false))

  //  br i1 %cmp4, label %for.body5, label %for.end14.loopexit, !dbg !113, !llvm.loop !141, !UID !143, !BB_UID !144
  val br_33 = Module(new CBranchNodeVariable(NumTrue = 1, NumFalse = 1, NumPredecessor = 0, ID = 33))

  //  br label %for.end14, !dbg !145
  val br_34 = Module(new UBranchNode(ID = 34))

  //  %cmp1661 = icmp sgt i32 %start, %stop, !dbg !145, !UID !147
  val icmp_cmp166135 = Module(new IcmpNode(NumOuts = 1, ID = 35, opCode = "ugt")(sign = false))

  //  br i1 %cmp1661, label %for.end30, label %for.body17.lr.ph, !dbg !148, !UID !149, !BB_UID !150
  val br_36 = Module(new CBranchNodeVariable(NumTrue = 1, NumFalse = 1, NumPredecessor = 0, ID = 36))

  //  %11 = sext i32 %start to i64, !dbg !148, !UID !151
  val sext37 = Module(new SextNode(NumOuts = 1))

  //  %12 = sext i32 %stop to i64, !dbg !148, !UID !152
  val sext38 = Module(new SextNode(NumOuts = 1))

  //  br label %for.body17, !dbg !148, !UID !153, !BB_UID !154
  val br_39 = Module(new UBranchNode(ID = 39))

  //  %indvars.iv = phi i64 [ %indvars.iv.next, %if.end ], [ %11, %for.body17.lr.ph ], !UID !155
  val phiindvars_iv40 = Module(new PhiFastNode(NumInputs = 2, NumOutputs = 3, ID = 40, Res = false))

  //  %i.164 = phi i32 [ %i.2, %if.end ], [ %start, %for.body17.lr.ph ], !UID !156
  val phii_16441 = Module(new PhiFastNode(NumInputs = 2, NumOutputs = 3, ID = 41, Res = false))

  //  %j.163 = phi i32 [ %j.2, %if.end ], [ %stop, %for.body17.lr.ph ], !UID !157
  val phij_16342 = Module(new PhiFastNode(NumInputs = 2, NumOutputs = 3, ID = 42, Res = false))

  //  %idxprom18 = sext i32 %j.163 to i64, !dbg !158, !UID !159
  val sextidxprom1843 = Module(new SextNode(NumOuts = 1))

  //  %arrayidx19 = getelementptr inbounds [2048 x i32], [2048 x i32]* %temp, i64 0, i64 %idxprom18, !dbg !158, !UID !160
  val Gep_arrayidx1944 = Module(new GepNode(NumIns = 2, NumOuts = 1, ID = 44)(ElementSize = 4, ArraySize = List(8192)))

  //  %13 = load i32, i32* %arrayidx19, align 4, !dbg !158, !tbaa !127, !UID !161
  val ld_45 = Module(new UnTypLoad(NumPredOps = 0, NumSuccOps = 0, NumOuts = 2, ID = 45, RouteID = 1))

  //  %idxprom20 = sext i32 %i.164 to i64, !dbg !163, !UID !164
  val sextidxprom2046 = Module(new SextNode(NumOuts = 1))

  //  %arrayidx21 = getelementptr inbounds [2048 x i32], [2048 x i32]* %temp, i64 0, i64 %idxprom20, !dbg !163, !UID !165
  val Gep_arrayidx2147 = Module(new GepNode(NumIns = 2, NumOuts = 1, ID = 47)(ElementSize = 4, ArraySize = List(8192)))

  //  %14 = load i32, i32* %arrayidx21, align 4, !dbg !163, !tbaa !127, !UID !166
  val ld_48 = Module(new UnTypLoad(NumPredOps = 0, NumSuccOps = 0, NumOuts = 2, ID = 48, RouteID = 2))

  //  %cmp22 = icmp slt i32 %13, %14, !dbg !168, !UID !170
  val icmp_cmp2249 = Module(new IcmpNode(NumOuts = 1, ID = 49, opCode = "ult")(sign = false))

  //  %arrayidx24 = getelementptr inbounds i32, i32* %a, i64 %indvars.iv, !UID !171
  val Gep_arrayidx2450 = Module(new GepNode(NumIns = 1, NumOuts = 2, ID = 50)(ElementSize = 4, ArraySize = List()))

  //  br i1 %cmp22, label %if.then, label %if.else, !dbg !172, !UID !173, !BB_UID !174
  val br_51 = Module(new CBranchNodeVariable(NumTrue = 1, NumFalse = 1, NumPredecessor = 0, ID = 51))

  //  store i32 %13, i32* %arrayidx24, align 4, !dbg !175, !tbaa !127, !UID !177
  val st_52 = Module(new UnTypStore(NumPredOps = 0, NumSuccOps = 0, ID = 52, RouteID = 1))

  //  %dec = add nsw i32 %j.163, -1, !dbg !178, !UID !179
  val binaryOp_dec53 = Module(new ComputeNode(NumOuts = 1, ID = 53, opCode = "add")(sign = false))

  //  br label %if.end, !dbg !180, !UID !181, !BB_UID !182
  val br_54 = Module(new UBranchNode(ID = 54))

  //  store i32 %14, i32* %arrayidx24, align 4, !dbg !183, !tbaa !127, !UID !185
  val st_55 = Module(new UnTypStore(NumPredOps = 0, NumSuccOps = 0, ID = 55, RouteID = 2))

  //  %inc27 = add nsw i32 %i.164, 1, !dbg !186, !UID !187
  val binaryOp_inc2756 = Module(new ComputeNode(NumOuts = 1, ID = 56, opCode = "add")(sign = false))

  //  br label %if.end, !UID !188, !BB_UID !189
  val br_57 = Module(new UBranchNode(ID = 57))

  //  %j.2 = phi i32 [ %dec, %if.then ], [ %j.163, %if.else ], !UID !190
  val phij_258 = Module(new PhiFastNode(NumInputs = 2, NumOutputs = 1, ID = 58, Res = true))

  //  %i.2 = phi i32 [ %i.164, %if.then ], [ %inc27, %if.else ], !UID !191
  val phii_259 = Module(new PhiFastNode(NumInputs = 2, NumOutputs = 1, ID = 59, Res = true))

  //  %indvars.iv.next = add nsw i64 %indvars.iv, 1, !dbg !192, !UID !193
  val binaryOp_indvars_iv_next60 = Module(new ComputeNode(NumOuts = 1, ID = 60, opCode = "add")(sign = false))

  //  %cmp16 = icmp slt i64 %indvars.iv, %12, !dbg !145, !UID !194
  val icmp_cmp1661 = Module(new IcmpNode(NumOuts = 1, ID = 61, opCode = "ult")(sign = false))

  //  br i1 %cmp16, label %for.body17, label %for.end30.loopexit, !dbg !148, !llvm.loop !195, !UID !197, !BB_UID !198
  val br_62 = Module(new CBranchNodeVariable(NumTrue = 1, NumFalse = 1, NumPredecessor = 0, ID = 62))

  //  br label %for.end30, !dbg !199
  val br_63 = Module(new UBranchNode(ID = 63))

  //  ret void, !dbg !199, !UID !200, !BB_UID !201
  val ret_64 = Module(new RetNode2(retTypes = List(), ID = 64))



  /* ================================================================== *
   *                   PRINTING CONSTANTS NODES                         *
   * ================================================================== */

  //i64 0
  val const0 = Module(new ConstFastNode(value = 0, ID = 0))

  //i64 2
  val const1 = Module(new ConstFastNode(value = 2, ID = 1))

  //i64 4
  val const2 = Module(new ConstFastNode(value = 4, ID = 2))

  //i32 1
  val const3 = Module(new ConstFastNode(value = 1, ID = 3))

  //i64 1
  val const4 = Module(new ConstFastNode(value = 1, ID = 4))

  //i64 0
  val const5 = Module(new ConstFastNode(value = 0, ID = 5))

  //i64 0
  val const6 = Module(new ConstFastNode(value = 0, ID = 6))

  //i64 0
  val const7 = Module(new ConstFastNode(value = 0, ID = 7))

  //i32 -1
  val const8 = Module(new ConstFastNode(value = -1, ID = 8))

  //i32 1
  val const9 = Module(new ConstFastNode(value = 1, ID = 9))

  //i64 1
  val const10 = Module(new ConstFastNode(value = 1, ID = 10))



  /* ================================================================== *
   *                   BASICBLOCK -> PREDICATE INSTRUCTION              *
   * ================================================================== */

  bb_entry0.io.predicateIn(0) <> InputSplitter.io.Out.enable

  bb_merge_label2_loopexit1.io.predicateIn(0) <> br_3.io.FalseOutput(0)

  bb_merge_label22.io.predicateIn(1) <> br_3.io.TrueOutput(0)

  bb_merge_label22.io.predicateIn(0) <> br_15.io.Out(0)

  bb_for_body5_lr_ph3.io.predicateIn(0) <> br_17.io.TrueOutput(0)

  bb_for_end146.io.predicateIn(1) <> br_17.io.FalseOutput(0)

  bb_for_end146.io.predicateIn(0) <> br_34.io.Out(0)

  bb_for_body17_lr_ph7.io.predicateIn(0) <> br_36.io.FalseOutput(0)

  bb_if_then9.io.predicateIn(0) <> br_51.io.TrueOutput(0)

  bb_if_else10.io.predicateIn(0) <> br_51.io.FalseOutput(0)

  bb_if_end11.io.predicateIn(1) <> br_54.io.Out(0)

  bb_if_end11.io.predicateIn(0) <> br_57.io.Out(0)

  bb_for_end3013.io.predicateIn(1) <> br_36.io.TrueOutput(0)

  bb_for_end3013.io.predicateIn(0) <> br_63.io.Out(0)



  /* ================================================================== *
   *                   BASICBLOCK -> PREDICATE LOOP                     *
   * ================================================================== */

  bb_for_body54.io.predicateIn(1) <> Loop_1.io.activate_loop_start

  bb_for_body54.io.predicateIn(0) <> Loop_1.io.activate_loop_back

  bb_for_end14_loopexit5.io.predicateIn(0) <> Loop_1.io.loopExit(0)

  bb_for_body178.io.predicateIn(1) <> Loop_0.io.activate_loop_start

  bb_for_body178.io.predicateIn(0) <> Loop_0.io.activate_loop_back

  bb_for_end30_loopexit12.io.predicateIn(0) <> Loop_0.io.loopExit(0)



  /* ================================================================== *
   *                   PRINTING PARALLEL CONNECTIONS                    *
   * ================================================================== */



  /* ================================================================== *
   *                   LOOP -> PREDICATE INSTRUCTION                    *
   * ================================================================== */

  Loop_0.io.enable <> br_39.io.Out(0)

  Loop_0.io.loopBack(0) <> br_62.io.TrueOutput(0)

  Loop_0.io.loopFinish(0) <> br_62.io.FalseOutput(0)

  Loop_1.io.enable <> br_22.io.Out(0)

  Loop_1.io.loopBack(0) <> br_33.io.TrueOutput(0)

  Loop_1.io.loopFinish(0) <> br_33.io.FalseOutput(0)



  /* ================================================================== *
   *                   ENDING INSTRUCTIONS                              *
   * ================================================================== */



  /* ================================================================== *
   *                   LOOP INPUT DATA DEPENDENCIES                     *
   * ================================================================== */

  Loop_0.io.InLiveIn(0) <> sext37.io.Out

  Loop_0.io.InLiveIn(1) <> InputSplitter.io.Out.data.elements("field1")(0)

  Loop_0.io.InLiveIn(2) <> InputSplitter.io.Out.data.elements("field3")(0)

  Loop_0.io.InLiveIn(3) <> alloca_temp0.io.Out(0)

  Loop_0.io.InLiveIn(4) <> InputSplitter.io.Out.data.elements("field0")(0)

  Loop_0.io.InLiveIn(5) <> sext38.io.Out

  Loop_1.io.InLiveIn(0) <> sext20.io.Out

  Loop_1.io.InLiveIn(1) <> InputSplitter.io.Out.data.elements("field0")(1)

  Loop_1.io.InLiveIn(2) <> binaryOp_add919.io.Out(0)

  Loop_1.io.InLiveIn(3) <> alloca_temp0.io.Out(1)

  Loop_1.io.InLiveIn(4) <> sext21.io.Out



  /* ================================================================== *
   *                   LOOP DATA LIVE-IN DEPENDENCIES                   *
   * ================================================================== */

  phiindvars_iv40.io.InData(1) <> Loop_0.io.OutLiveIn.elements("field0")(0)

  phii_16441.io.InData(1) <> Loop_0.io.OutLiveIn.elements("field1")(0)

  phij_16342.io.InData(1) <> Loop_0.io.OutLiveIn.elements("field2")(0)

  Gep_arrayidx1944.io.baseAddress <> Loop_0.io.OutLiveIn.elements("field3")(0)

  Gep_arrayidx2147.io.baseAddress <> Loop_0.io.OutLiveIn.elements("field3")(1)

  Gep_arrayidx2450.io.baseAddress <> Loop_0.io.OutLiveIn.elements("field4")(0)

  icmp_cmp1661.io.RightIO <> Loop_0.io.OutLiveIn.elements("field5")(0)

  phiindvars_iv69_in23.io.InData(1) <> Loop_1.io.OutLiveIn.elements("field0")(0)

  Gep_arrayidx725.io.baseAddress <> Loop_1.io.OutLiveIn.elements("field1")(0)

  binaryOp_sub28.io.LeftIO <> Loop_1.io.OutLiveIn.elements("field2")(0)

  Gep_arrayidx1130.io.baseAddress <> Loop_1.io.OutLiveIn.elements("field3")(0)

  icmp_cmp432.io.RightIO <> Loop_1.io.OutLiveIn.elements("field4")(0)



  /* ================================================================== *
   *                   LOOP DATA LIVE-OUT DEPENDENCIES                  *
   * ================================================================== */



  /* ================================================================== *
   *                   LOOP LIVE OUT DEPENDENCIES                       *
   * ================================================================== */



  /* ================================================================== *
   *                   LOOP CARRY DEPENDENCIES                          *
   * ================================================================== */

  Loop_0.io.CarryDepenIn(0) <> phij_258.io.Out(0)

  Loop_0.io.CarryDepenIn(1) <> binaryOp_indvars_iv_next60.io.Out(0)

  Loop_0.io.CarryDepenIn(2) <> phii_259.io.Out(0)

  Loop_1.io.CarryDepenIn(0) <> binaryOp_indvars_iv6924.io.Out(0)



  /* ================================================================== *
   *                   LOOP DATA CARRY DEPENDENCIES                     *
   * ================================================================== */

  phij_16342.io.InData(0) <> Loop_0.io.CarryDepenOut.elements("field0")(0)

  phiindvars_iv40.io.InData(0) <> Loop_0.io.CarryDepenOut.elements("field1")(0)

  phii_16441.io.InData(0) <> Loop_0.io.CarryDepenOut.elements("field2")(0)

  phiindvars_iv69_in23.io.InData(0) <> Loop_1.io.CarryDepenOut.elements("field0")(0)



  /* ================================================================== *
   *                   BASICBLOCK -> ENABLE INSTRUCTION                 *
   * ================================================================== */

  alloca_temp0.io.enable <> bb_entry0.io.Out(0)


  bitcast_1.io.enable <> bb_entry0.io.Out(1)


  icmp_cmp672.io.enable <> bb_entry0.io.Out(2)


  br_3.io.enable <> bb_entry0.io.Out(3)


  const0.io.enable <> bb_merge_label2_loopexit1.io.Out(0)

  const1.io.enable <> bb_merge_label2_loopexit1.io.Out(1)

  const2.io.enable <> bb_merge_label2_loopexit1.io.Out(2)

  sext4.io.enable <> bb_merge_label2_loopexit1.io.Out(3)


  Gep_scevgep5.io.enable <> bb_merge_label2_loopexit1.io.Out(4)


  bitcast_scevgep736.io.enable <> bb_merge_label2_loopexit1.io.Out(5)


  Gep_scevgep747.io.enable <> bb_merge_label2_loopexit1.io.Out(6)


  bitcast_scevgep74758.io.enable <> bb_merge_label2_loopexit1.io.Out(7)


  icmp_9.io.enable <> bb_merge_label2_loopexit1.io.Out(8)


  select_smax10.io.enable <> bb_merge_label2_loopexit1.io.Out(9)


  binaryOp_11.io.enable <> bb_merge_label2_loopexit1.io.Out(10)


  sext12.io.enable <> bb_merge_label2_loopexit1.io.Out(11)


  binaryOp_13.io.enable <> bb_merge_label2_loopexit1.io.Out(12)


  binaryOp_14.io.enable <> bb_merge_label2_loopexit1.io.Out(13)


  br_15.io.enable <> bb_merge_label2_loopexit1.io.Out(14)


  icmp_cmp46516.io.enable <> bb_merge_label22.io.Out(0)


  br_17.io.enable <> bb_merge_label22.io.Out(1)


  const3.io.enable <> bb_for_body5_lr_ph3.io.Out(0)

  binaryOp_add18.io.enable <> bb_for_body5_lr_ph3.io.Out(1)


  binaryOp_add919.io.enable <> bb_for_body5_lr_ph3.io.Out(2)


  sext20.io.enable <> bb_for_body5_lr_ph3.io.Out(3)


  sext21.io.enable <> bb_for_body5_lr_ph3.io.Out(4)


  br_22.io.enable <> bb_for_body5_lr_ph3.io.Out(5)


  const4.io.enable <> bb_for_body54.io.Out(0)

  const5.io.enable <> bb_for_body54.io.Out(1)

  phiindvars_iv69_in23.io.enable <> bb_for_body54.io.Out(2)


  binaryOp_indvars_iv6924.io.enable <> bb_for_body54.io.Out(3)


  Gep_arrayidx725.io.enable <> bb_for_body54.io.Out(4)


  ld_26.io.enable <> bb_for_body54.io.Out(5)


  trunc27.io.enable <> bb_for_body54.io.Out(6)


  binaryOp_sub28.io.enable <> bb_for_body54.io.Out(7)


  sextidxprom1029.io.enable <> bb_for_body54.io.Out(8)


  Gep_arrayidx1130.io.enable <> bb_for_body54.io.Out(9)


  st_31.io.enable <> bb_for_body54.io.Out(10)


  icmp_cmp432.io.enable <> bb_for_body54.io.Out(11)


  br_33.io.enable <> bb_for_body54.io.Out(12)


  br_34.io.enable <> bb_for_end14_loopexit5.io.Out(0)


  icmp_cmp166135.io.enable <> bb_for_end146.io.Out(0)


  br_36.io.enable <> bb_for_end146.io.Out(1)


  sext37.io.enable <> bb_for_body17_lr_ph7.io.Out(0)


  sext38.io.enable <> bb_for_body17_lr_ph7.io.Out(1)


  br_39.io.enable <> bb_for_body17_lr_ph7.io.Out(2)


  const6.io.enable <> bb_for_body178.io.Out(0)

  const7.io.enable <> bb_for_body178.io.Out(1)

  phiindvars_iv40.io.enable <> bb_for_body178.io.Out(2)


  phii_16441.io.enable <> bb_for_body178.io.Out(3)


  phij_16342.io.enable <> bb_for_body178.io.Out(4)


  sextidxprom1843.io.enable <> bb_for_body178.io.Out(5)


  Gep_arrayidx1944.io.enable <> bb_for_body178.io.Out(6)


  ld_45.io.enable <> bb_for_body178.io.Out(7)


  sextidxprom2046.io.enable <> bb_for_body178.io.Out(8)


  Gep_arrayidx2147.io.enable <> bb_for_body178.io.Out(9)


  ld_48.io.enable <> bb_for_body178.io.Out(10)


  icmp_cmp2249.io.enable <> bb_for_body178.io.Out(11)


  Gep_arrayidx2450.io.enable <> bb_for_body178.io.Out(12)


  br_51.io.enable <> bb_for_body178.io.Out(13)


  const8.io.enable <> bb_if_then9.io.Out(0)

  st_52.io.enable <> bb_if_then9.io.Out(1)


  binaryOp_dec53.io.enable <> bb_if_then9.io.Out(2)


  br_54.io.enable <> bb_if_then9.io.Out(3)


  const9.io.enable <> bb_if_else10.io.Out(0)

  st_55.io.enable <> bb_if_else10.io.Out(1)


  binaryOp_inc2756.io.enable <> bb_if_else10.io.Out(2)


  br_57.io.enable <> bb_if_else10.io.Out(3)


  const10.io.enable <> bb_if_end11.io.Out(0)

  phij_258.io.enable <> bb_if_end11.io.Out(1)


  phii_259.io.enable <> bb_if_end11.io.Out(2)


  binaryOp_indvars_iv_next60.io.enable <> bb_if_end11.io.Out(3)


  icmp_cmp1661.io.enable <> bb_if_end11.io.Out(4)


  br_62.io.enable <> bb_if_end11.io.Out(5)


  br_63.io.enable <> bb_for_end30_loopexit12.io.Out(0)


  ret_64.io.In.enable <> bb_for_end3013.io.Out(0)




  /* ================================================================== *
   *                   CONNECTING PHI NODES                             *
   * ================================================================== */

  phiindvars_iv69_in23.io.Mask <> bb_for_body54.io.MaskBB(0)

  phiindvars_iv40.io.Mask <> bb_for_body178.io.MaskBB(0)

  phii_16441.io.Mask <> bb_for_body178.io.MaskBB(1)

  phij_16342.io.Mask <> bb_for_body178.io.MaskBB(2)

  phij_258.io.Mask <> bb_if_end11.io.MaskBB(0)

  phii_259.io.Mask <> bb_if_end11.io.MaskBB(1)



  /* ================================================================== *
   *                   PRINT ALLOCA OFFSET                              *
   * ================================================================== */

  alloca_temp0.io.allocaInputIO.bits.size      := 1.U
  alloca_temp0.io.allocaInputIO.bits.numByte   := 8192.U
  alloca_temp0.io.allocaInputIO.bits.predicate := true.B
  alloca_temp0.io.allocaInputIO.bits.valid     := true.B
  alloca_temp0.io.allocaInputIO.valid          := true.B





  /* ================================================================== *
   *                   CONNECTING MEMORY CONNECTIONS                    *
   * ================================================================== */

  StackPointer.io.InData(0) <> alloca_temp0.io.allocaReqIO

  alloca_temp0.io.allocaRespIO <> StackPointer.io.OutData(0)

  MemCtrl.io.ReadIn(0) <> ld_26.io.memReq

  ld_26.io.memResp <> MemCtrl.io.ReadOut(0)

  MemCtrl.io.WriteIn(0) <> st_31.io.memReq

  st_31.io.memResp <> MemCtrl.io.WriteOut(0)

  MemCtrl.io.ReadIn(1) <> ld_45.io.memReq

  ld_45.io.memResp <> MemCtrl.io.ReadOut(1)

  MemCtrl.io.ReadIn(2) <> ld_48.io.memReq

  ld_48.io.memResp <> MemCtrl.io.ReadOut(2)

  MemCtrl.io.WriteIn(1) <> st_52.io.memReq

  st_52.io.memResp <> MemCtrl.io.WriteOut(1)

  MemCtrl.io.WriteIn(2) <> st_55.io.memReq

  st_55.io.memResp <> MemCtrl.io.WriteOut(2)



  /* ================================================================== *
   *                   PRINT SHARED CONNECTIONS                         *
   * ================================================================== */



  /* ================================================================== *
   *                   CONNECTING DATA DEPENDENCIES                     *
   * ================================================================== */

  Gep_scevgep5.io.idx(0) <> const0.io.Out

  binaryOp_13.io.RightIO <> const1.io.Out

  binaryOp_14.io.RightIO <> const2.io.Out

  binaryOp_add18.io.RightIO <> const3.io.Out

  binaryOp_indvars_iv6924.io.RightIO <> const4.io.Out

  Gep_arrayidx1130.io.idx(0) <> const5.io.Out

  Gep_arrayidx1944.io.idx(0) <> const6.io.Out

  Gep_arrayidx2147.io.idx(0) <> const7.io.Out

  binaryOp_dec53.io.RightIO <> const8.io.Out

  binaryOp_inc2756.io.RightIO <> const9.io.Out

  binaryOp_indvars_iv_next60.io.RightIO <> const10.io.Out

  bitcast_1.io.Input <> alloca_temp0.io.Out(2)

  Gep_scevgep5.io.baseAddress <> alloca_temp0.io.Out(3)

  br_3.io.CmpIO <> icmp_cmp672.io.Out(0)

  Gep_scevgep5.io.idx(1) <> sext4.io.Out

  Gep_scevgep747.io.idx(0) <> sext4.io.Out

  bitcast_scevgep736.io.Input <> Gep_scevgep5.io.Out(0)

  bitcast_scevgep74758.io.Input <> Gep_scevgep747.io.Out(0)

  select_smax10.io.Select <> icmp_9.io.Out(0)

  binaryOp_11.io.LeftIO <> select_smax10.io.Out(0)

  sext12.io.Input <> binaryOp_11.io.Out(0)

  binaryOp_13.io.LeftIO <> sext12.io.Out(0)

  binaryOp_14.io.LeftIO <> binaryOp_13.io.Out(0)

  br_17.io.CmpIO <> icmp_cmp46516.io.Out(0)

  binaryOp_add919.io.LeftIO <> binaryOp_add18.io.Out(0)

  binaryOp_indvars_iv6924.io.LeftIO <> phiindvars_iv69_in23.io.Out(0)

  Gep_arrayidx725.io.idx(0) <> binaryOp_indvars_iv6924.io.Out(1)

  trunc27.io.Input <> binaryOp_indvars_iv6924.io.Out(2)

  icmp_cmp432.io.LeftIO <> binaryOp_indvars_iv6924.io.Out(3)

  ld_26.io.GepAddr <> Gep_arrayidx725.io.Out(0)

  st_31.io.inData <> ld_26.io.Out(0)

  binaryOp_sub28.io.RightIO <> trunc27.io.Out(0)

  sextidxprom1029.io.Input <> binaryOp_sub28.io.Out(0)

  Gep_arrayidx1130.io.idx(1) <> sextidxprom1029.io.Out

  st_31.io.GepAddr <> Gep_arrayidx1130.io.Out(0)

  br_33.io.CmpIO <> icmp_cmp432.io.Out(0)

  br_36.io.CmpIO <> icmp_cmp166135.io.Out(0)

  Gep_arrayidx2450.io.idx(0) <> phiindvars_iv40.io.Out(0)

  binaryOp_indvars_iv_next60.io.LeftIO <> phiindvars_iv40.io.Out(1)

  icmp_cmp1661.io.LeftIO <> phiindvars_iv40.io.Out(2)

  sextidxprom2046.io.Input <> phii_16441.io.Out(0)

  binaryOp_inc2756.io.LeftIO <> phii_16441.io.Out(1)

  phii_259.io.InData(0) <> phii_16441.io.Out(2)

  sextidxprom1843.io.Input <> phij_16342.io.Out(0)

  binaryOp_dec53.io.LeftIO <> phij_16342.io.Out(1)

  phij_258.io.InData(1) <> phij_16342.io.Out(2)

  Gep_arrayidx1944.io.idx(1) <> sextidxprom1843.io.Out

  ld_45.io.GepAddr <> Gep_arrayidx1944.io.Out(0)

  icmp_cmp2249.io.LeftIO <> ld_45.io.Out(0)

  st_52.io.inData <> ld_45.io.Out(1)

  Gep_arrayidx2147.io.idx(1) <> sextidxprom2046.io.Out

  ld_48.io.GepAddr <> Gep_arrayidx2147.io.Out(0)

  icmp_cmp2249.io.RightIO <> ld_48.io.Out(0)

  st_55.io.inData <> ld_48.io.Out(1)

  br_51.io.CmpIO <> icmp_cmp2249.io.Out(0)

  st_52.io.GepAddr <> Gep_arrayidx2450.io.Out(0)

  st_55.io.GepAddr <> Gep_arrayidx2450.io.Out(1)

  phij_258.io.InData(0) <> binaryOp_dec53.io.Out(0)

  phii_259.io.InData(1) <> binaryOp_inc2756.io.Out(0)

  br_62.io.CmpIO <> icmp_cmp1661.io.Out(0)

  Gep_scevgep747.io.baseAddress <> InputSplitter.io.Out.data.elements("field0")(2)

  icmp_cmp672.io.LeftIO <> InputSplitter.io.Out.data.elements("field1")(1)

  sext4.io.Input <> InputSplitter.io.Out.data.elements("field1")(2)

  icmp_9.io.RightIO <> InputSplitter.io.Out.data.elements("field1")(3)

  select_smax10.io.InData2 <> InputSplitter.io.Out.data.elements("field1")(4)

  binaryOp_11.io.RightIO <> InputSplitter.io.Out.data.elements("field1")(5)

  icmp_cmp166135.io.LeftIO <> InputSplitter.io.Out.data.elements("field1")(6)

  sext37.io.Input <> InputSplitter.io.Out.data.elements("field1")(7)

  icmp_cmp672.io.RightIO <> InputSplitter.io.Out.data.elements("field2")(0)

  icmp_9.io.LeftIO <> InputSplitter.io.Out.data.elements("field2")(1)

  select_smax10.io.InData1 <> InputSplitter.io.Out.data.elements("field2")(2)

  icmp_cmp46516.io.LeftIO <> InputSplitter.io.Out.data.elements("field2")(3)

  binaryOp_add18.io.LeftIO <> InputSplitter.io.Out.data.elements("field2")(4)

  sext20.io.Input <> InputSplitter.io.Out.data.elements("field2")(5)

  icmp_cmp46516.io.RightIO <> InputSplitter.io.Out.data.elements("field3")(1)

  binaryOp_add919.io.RightIO <> InputSplitter.io.Out.data.elements("field3")(2)

  sext21.io.Input <> InputSplitter.io.Out.data.elements("field3")(3)

  icmp_cmp166135.io.RightIO <> InputSplitter.io.Out.data.elements("field3")(4)

  sext38.io.Input <> InputSplitter.io.Out.data.elements("field3")(5)

  st_31.io.Out(0).ready := true.B

  st_52.io.Out(0).ready := true.B

  st_55.io.Out(0).ready := true.B



  /* ================================================================== *
   *                   PRINTING OUTPUT INTERFACE                        *
   * ================================================================== */

  io.out <> ret_64.io.Out

}

import java.io.{File, FileWriter}

object mergeTop extends App {
  val dir = new File("RTL/mergeTop");
  dir.mkdirs
  implicit val p = config.Parameters.root((new MiniConfig).toInstance)
  val chirrtl = firrtl.Parser.parse(chisel3.Driver.emit(() => new mergeDF()))

  val verilogFile = new File(dir, s"${chirrtl.main}.v")
  val verilogWriter = new FileWriter(verilogFile)
  val compileResult = (new firrtl.VerilogCompiler).compileAndEmit(firrtl.CircuitState(chirrtl, firrtl.ChirrtlForm))
  val compiledStuff = compileResult.getEmittedCircuit
  verilogWriter.write(compiledStuff.value)
  verilogWriter.close()
}
