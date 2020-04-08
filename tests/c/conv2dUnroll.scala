package dandelion.generator

import chipsalliance.rocketchip.config._
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


class conv2dUnrollDF(PtrsIn: Seq[Int] = List(32, 32, 32), ValsIn: Seq[Int] = List(32, 32, 32, 32), Returns: Seq[Int] = List())
			(implicit p: Parameters) extends DandelionAccelDCRModule(PtrsIn, ValsIn, Returns){


  /* ================================================================== *
   *                   PRINTING MEMORY MODULES                          *
   * ================================================================== */

  val MemCtrl = Module(new CacheMemoryEngine(ID = 0, NumRead = 10, NumWrite = 1))

  io.MemReq <> MemCtrl.io.cache.MemReq
  MemCtrl.io.cache.MemResp <> io.MemResp
  val ArgSplitter = Module(new SplitCallDCR(ptrsArgTypes = List(1, 1, 1), valsArgTypes = List(4, 1, 3, 1)))
  ArgSplitter.io.In <> io.in



  /* ================================================================== *
   *                   PRINTING LOOP HEADERS                            *
   * ================================================================== */

  val Loop_0 = Module(new LoopBlockNode(NumIns = List(1, 1, 5, 5, 1, 1), NumOuts = List(1), NumCarry = List(1, 1, 1, 1), NumExits = 1, ID = 0))

  val Loop_1 = Module(new LoopBlockNode(NumIns = List(1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1), NumOuts = List(), NumCarry = List(1), NumExits = 1, ID = 1))

  val Loop_2 = Module(new LoopBlockNode(NumIns = List(1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1), NumOuts = List(), NumCarry = List(1, 1, 1), NumExits = 1, ID = 2))



  /* ================================================================== *
   *                   PRINTING BASICBLOCK NODES                        *
   * ================================================================== */

  val bb_entry0 = Module(new BasicBlockNoMaskFastNode(NumInputs = 1, NumOuts = 5, BID = 0))

  val bb_for_body_lr_ph1 = Module(new BasicBlockNoMaskFastNode(NumInputs = 1, NumOuts = 12, BID = 1))

  val bb_for_cond_cleanup_loopexit2 = Module(new BasicBlockNoMaskFastNode(NumInputs = 1, NumOuts = 1, BID = 2))

  val bb_for_cond_cleanup3 = Module(new BasicBlockNoMaskFastNode(NumInputs = 2, NumOuts = 1, BID = 3))

  val bb_for_body4 = Module(new BasicBlockNode(NumInputs = 2, NumOuts = 6, NumPhi = 3, BID = 4))

  val bb_for_body5_preheader5 = Module(new BasicBlockNoMaskFastNode(NumInputs = 1, NumOuts = 1, BID = 5))

  val bb_for_cond_cleanup4_loopexit6 = Module(new BasicBlockNoMaskFastNode(NumInputs = 1, NumOuts = 1, BID = 6))

  val bb_for_cond_cleanup47 = Module(new BasicBlockNoMaskFastNode(NumInputs = 2, NumOuts = 6, BID = 7))

  val bb_for_body58 = Module(new BasicBlockNode(NumInputs = 2, NumOuts = 2, NumPhi = 1, BID = 8))

  val bb_for_body12_preheader9 = Module(new BasicBlockNoMaskFastNode(NumInputs = 1, NumOuts = 1, BID = 9))

  val bb_for_cond_cleanup11_loopexit10 = Module(new BasicBlockNoMaskFastNode(NumInputs = 1, NumOuts = 1, BID = 10))

  val bb_for_cond_cleanup1111 = Module(new BasicBlockNode(NumInputs = 2, NumOuts = 10, NumPhi = 1, BID = 11))

  val bb_for_body1212 = Module(new BasicBlockNode(NumInputs = 2, NumOuts = 61, NumPhi = 4, BID = 12))



  /* ================================================================== *
   *                   PRINTING INSTRUCTION NODES                       *
   * ================================================================== */

  //  %shr = ashr i32 %K, 1, !dbg !69, !UID !70
  val binaryOp_shr0 = Module(new ComputeNode(NumOuts = 7, ID = 0, opCode = "ashr")(sign = false, Debug = false))

  //  %sub = sub nsw i32 %H, %shr, !dbg !74, !UID !75
  val binaryOp_sub1 = Module(new ComputeNode(NumOuts = 2, ID = 1, opCode = "sub")(sign = false, Debug = false))

  //  %cmp124 = icmp slt i32 %shr, %sub, !dbg !76, !UID !77
  val icmp_cmp1242 = Module(new ComputeNode(NumOuts = 1, ID = 2, opCode = "slt")(sign = true, Debug = false))

  //  br i1 %cmp124, label %for.body.lr.ph, label %for.cond.cleanup, !dbg !78, !UID !79, !BB_UID !80
  val br_3 = Module(new CBranchNodeVariable(NumTrue = 1, NumFalse = 1, NumPredecessor = 0, ID = 3))

  //  %mul = mul nsw i32 %shr, %W, !dbg !81, !UID !82
  val binaryOp_mul4 = Module(new ComputeNode(NumOuts = 1, ID = 4, opCode = "mul")(sign = false, Debug = false))

  //  %sub2 = sub nsw i32 %W, %shr, !UID !83
  val binaryOp_sub25 = Module(new ComputeNode(NumOuts = 2, ID = 5, opCode = "sub")(sign = false, Debug = false))

  //  %cmp3122 = icmp slt i32 %shr, %sub2, !UID !84
  val icmp_cmp31226 = Module(new ComputeNode(NumOuts = 1, ID = 6, opCode = "slt")(sign = true, Debug = false))

  //  %cmp10117 = icmp slt i32 %K, 0, !UID !85
  val icmp_cmp101177 = Module(new ComputeNode(NumOuts = 1, ID = 7, opCode = "slt")(sign = true, Debug = false))

  //  %0 = sext i32 %W to i64, !dbg !78, !UID !86
  val sext8 = Module(new SextNode(NumOuts = 1))

  //  %1 = or i32 %K, 1, !dbg !78, !UID !87
  val binaryOp_9 = Module(new ComputeNode(NumOuts = 1, ID = 9, opCode = "or")(sign = false, Debug = false))

  //  %2 = sext i32 %shr to i64, !dbg !78, !UID !88
  val sext10 = Module(new SextNode(NumOuts = 1))

  //  %3 = sext i32 %mul to i64, !dbg !78, !UID !89
  val sext11 = Module(new SextNode(NumOuts = 1))

  //  %wide.trip.count = sext i32 %sub2 to i64, !UID !90
  val sextwide_trip_count12 = Module(new SextNode(NumOuts = 1))

  //  br label %for.body, !dbg !78, !UID !91, !BB_UID !92
  val br_13 = Module(new UBranchNode(ID = 13))

  //  br label %for.cond.cleanup, !dbg !93
  val br_14 = Module(new UBranchNode(ID = 14))

  //  ret void, !dbg !93, !UID !94, !BB_UID !95
  val ret_15 = Module(new RetNode2(retTypes = List(), ID = 15))

  //  %indvars.iv145 = phi i64 [ %3, %for.body.lr.ph ], [ %indvars.iv.next146, %for.cond.cleanup4 ], !UID !96
  val phiindvars_iv14516 = Module(new PhiFastNode(NumInputs = 2, NumOutputs = 2, ID = 16, Res = true))

  //  %indvars.iv132 = phi i32 [ 0, %for.body.lr.ph ], [ %indvars.iv.next133, %for.cond.cleanup4 ], !UID !97
  val phiindvars_iv13217 = Module(new PhiFastNode(NumInputs = 2, NumOutputs = 2, ID = 17, Res = true))

  //  %j.0127 = phi i32 [ %shr, %for.body.lr.ph ], [ %inc64, %for.cond.cleanup4 ], !UID !98
  val phij_012718 = Module(new PhiFastNode(NumInputs = 2, NumOutputs = 1, ID = 18, Res = true))

  //  %4 = sext i32 %indvars.iv132 to i64, !dbg !100, !UID !101
  val sext19 = Module(new SextNode(NumOuts = 1))

  //  br i1 %cmp3122, label %for.body5.preheader, label %for.cond.cleanup4, !dbg !100, !UID !102, !BB_UID !103
  val br_20 = Module(new CBranchNodeVariable(NumTrue = 1, NumFalse = 1, NumPredecessor = 0, ID = 20))

  //  br label %for.body5, !dbg !104, !UID !105, !BB_UID !106
  val br_21 = Module(new UBranchNode(ID = 21))

  //  br label %for.cond.cleanup4, !dbg !107
  val br_22 = Module(new UBranchNode(ID = 22))

  //  %indvars.iv.next146 = add i64 %indvars.iv145, %0, !dbg !107, !UID !108
  val binaryOp_indvars_iv_next14623 = Module(new ComputeNode(NumOuts = 1, ID = 23, opCode = "add")(sign = false, Debug = false))

  //  %inc64 = add nsw i32 %j.0127, 1, !dbg !109, !UID !110
  val binaryOp_inc6424 = Module(new ComputeNode(NumOuts = 2, ID = 24, opCode = "add")(sign = false, Debug = false))

  //  %indvars.iv.next133 = add i32 %indvars.iv132, %W, !dbg !78, !UID !111
  val binaryOp_indvars_iv_next13325 = Module(new ComputeNode(NumOuts = 1, ID = 25, opCode = "add")(sign = false, Debug = false))

  //  %exitcond147 = icmp eq i32 %inc64, %sub, !dbg !76, !UID !112
  val icmp_exitcond14726 = Module(new ComputeNode(NumOuts = 1, ID = 26, opCode = "eq")(sign = false, Debug = false))

  //  br i1 %exitcond147, label %for.cond.cleanup.loopexit, label %for.body, !dbg !78, !llvm.loop !113, !UID !115, !BB_UID !116
  val br_27 = Module(new CBranchNodeVariable(NumTrue = 1, NumFalse = 1, NumPredecessor = 0, ID = 27))

  //  %indvars.iv141 = phi i64 [ %indvars.iv.next142, %for.cond.cleanup11 ], [ %2, %for.body5.preheader ], !UID !117
  val phiindvars_iv14128 = Module(new PhiFastNode(NumInputs = 2, NumOutputs = 3, ID = 28, Res = false))

  //  br i1 %cmp10117, label %for.cond.cleanup11, label %for.body12.preheader, !dbg !104, !UID !121, !BB_UID !122
  val br_29 = Module(new CBranchNodeVariable(NumTrue = 1, NumFalse = 1, NumPredecessor = 0, ID = 29))

  //  br label %for.body12, !dbg !123, !UID !126, !BB_UID !127
  val br_30 = Module(new UBranchNode(ID = 30))

  //  br label %for.cond.cleanup11, !dbg !128
  val br_31 = Module(new UBranchNode(ID = 31))

  //  %val.0.lcssa = phi i32 [ 0, %for.body5 ], [ %add52, %for.cond.cleanup11.loopexit ], !UID !129
  val phival_0_lcssa32 = Module(new PhiFastNode(NumInputs = 2, NumOutputs = 1, ID = 32, Res = true))

  //  %shr55 = ashr i32 %val.0.lcssa, %scf, !dbg !128, !UID !130
  val binaryOp_shr5533 = Module(new ComputeNode(NumOuts = 1, ID = 33, opCode = "ashr")(sign = false, Debug = false))

  //  %5 = add nsw i64 %indvars.iv141, %indvars.iv145, !dbg !131, !UID !132
  val binaryOp_34 = Module(new ComputeNode(NumOuts = 1, ID = 34, opCode = "add")(sign = false, Debug = false))

  //  %arrayidx58 = getelementptr inbounds i32, i32* %res, i64 %5, !dbg !133, !UID !134
  val Gep_arrayidx5835 = Module(new GepNode(NumIns = 1, NumOuts = 1, ID = 35)(ElementSize = 8, ArraySize = List()))

  //  store i32 %shr55, i32* %arrayidx58, align 4, !dbg !135, !tbaa !136, !UID !140
  val st_36 = Module(new UnTypStoreCache(NumPredOps = 0, NumSuccOps = 1, ID = 36, RouteID = 10))

  //  %indvars.iv.next142 = add nsw i64 %indvars.iv141, 1, !dbg !141, !UID !142
  val binaryOp_indvars_iv_next14237 = Module(new ComputeNode(NumOuts = 2, ID = 37, opCode = "add")(sign = false, Debug = false))

  //  %exitcond144 = icmp eq i64 %indvars.iv.next142, %wide.trip.count, !dbg !143, !UID !144
  val icmp_exitcond14438 = Module(new ComputeNode(NumOuts = 1, ID = 38, opCode = "eq")(sign = false, Debug = false))

  //  br i1 %exitcond144, label %for.cond.cleanup4.loopexit, label %for.body5, !dbg !100, !llvm.loop !145, !UID !147, !BB_UID !148
  val br_39 = Module(new CBranchNodeVariable(NumTrue = 1, NumFalse = 1, NumPredecessor = 1, ID = 39))

  //  %indvars.iv134 = phi i64 [ %indvars.iv.next135, %for.body12 ], [ %4, %for.body12.preheader ], !UID !149
  val phiindvars_iv13440 = Module(new PhiFastNode(NumInputs = 2, NumOutputs = 2, ID = 40, Res = false))

  //  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body12 ], [ 0, %for.body12.preheader ], !UID !150
  val phiindvars_iv41 = Module(new PhiFastNode(NumInputs = 2, NumOutputs = 6, ID = 41, Res = false))

  //  %y.0121 = phi i32 [ %inc54, %for.body12 ], [ 0, %for.body12.preheader ], !UID !151
  val phiy_012142 = Module(new PhiFastNode(NumInputs = 2, NumOutputs = 1, ID = 42, Res = false))

  //  %val.0120 = phi i32 [ %add52, %for.body12 ], [ 0, %for.body12.preheader ], !UID !152
  val phival_012043 = Module(new PhiFastNode(NumInputs = 2, NumOutputs = 1, ID = 43, Res = false))

  //  %6 = add nuw nsw i64 %indvars.iv, 1, !dbg !123, !UID !154
  val binaryOp_44 = Module(new ComputeNode(NumOuts = 1, ID = 44, opCode = "add")(sign = false, Debug = false))

  //  %arrayidx = getelementptr inbounds i32, i32* %coeffs, i64 %indvars.iv, !dbg !155, !UID !156
  val Gep_arrayidx45 = Module(new GepNode(NumIns = 1, NumOuts = 1, ID = 45)(ElementSize = 8, ArraySize = List()))

  //  %7 = load i32, i32* %arrayidx, align 4, !dbg !155, !tbaa !136, !UID !157
  val ld_46 = Module(new UnTypLoadCache(NumPredOps = 0, NumSuccOps = 0, NumOuts = 1, ID = 46, RouteID = 0))

  //  %8 = add nsw i64 %indvars.iv134, %indvars.iv141, !dbg !158, !UID !159
  val binaryOp_47 = Module(new ComputeNode(NumOuts = 5, ID = 47, opCode = "add")(sign = false, Debug = false))

  //  %9 = add nsw i64 %8, -2, !dbg !160, !UID !161
  val binaryOp_48 = Module(new ComputeNode(NumOuts = 1, ID = 48, opCode = "add")(sign = false, Debug = false))

  //  %arrayidx15 = getelementptr inbounds i32, i32* %mat, i64 %9, !dbg !162, !UID !163
  val Gep_arrayidx1549 = Module(new GepNode(NumIns = 1, NumOuts = 1, ID = 49)(ElementSize = 8, ArraySize = List()))

  //  %10 = load i32, i32* %arrayidx15, align 4, !dbg !162, !tbaa !136, !UID !164
  val ld_50 = Module(new UnTypLoadCache(NumPredOps = 0, NumSuccOps = 0, NumOuts = 1, ID = 50, RouteID = 1))

  //  %mul16 = mul nsw i32 %10, %7, !dbg !165, !UID !166
  val binaryOp_mul1651 = Module(new ComputeNode(NumOuts = 1, ID = 51, opCode = "mul")(sign = false, Debug = false))

  //  %add17 = add nsw i32 %mul16, %val.0120, !dbg !167, !UID !168
  val binaryOp_add1752 = Module(new ComputeNode(NumOuts = 1, ID = 52, opCode = "add")(sign = false, Debug = false))

  //  %11 = add nuw nsw i64 %indvars.iv, 2, !dbg !169, !UID !170
  val binaryOp_53 = Module(new ComputeNode(NumOuts = 1, ID = 53, opCode = "add")(sign = false, Debug = false))

  //  %arrayidx20 = getelementptr inbounds i32, i32* %coeffs, i64 %6, !dbg !171, !UID !172
  val Gep_arrayidx2054 = Module(new GepNode(NumIns = 1, NumOuts = 1, ID = 54)(ElementSize = 8, ArraySize = List()))

  //  %12 = load i32, i32* %arrayidx20, align 4, !dbg !171, !tbaa !136, !UID !173
  val ld_55 = Module(new UnTypLoadCache(NumPredOps = 0, NumSuccOps = 0, NumOuts = 1, ID = 55, RouteID = 2))

  //  %13 = add nsw i64 %8, -1, !dbg !174, !UID !175
  val binaryOp_56 = Module(new ComputeNode(NumOuts = 1, ID = 56, opCode = "add")(sign = false, Debug = false))

  //  %arrayidx24 = getelementptr inbounds i32, i32* %mat, i64 %13, !dbg !176, !UID !177
  val Gep_arrayidx2457 = Module(new GepNode(NumIns = 1, NumOuts = 1, ID = 57)(ElementSize = 8, ArraySize = List()))

  //  %14 = load i32, i32* %arrayidx24, align 4, !dbg !176, !tbaa !136, !UID !178
  val ld_58 = Module(new UnTypLoadCache(NumPredOps = 0, NumSuccOps = 0, NumOuts = 1, ID = 58, RouteID = 3))

  //  %mul25 = mul nsw i32 %14, %12, !dbg !179, !UID !180
  val binaryOp_mul2559 = Module(new ComputeNode(NumOuts = 1, ID = 59, opCode = "mul")(sign = false, Debug = false))

  //  %add26 = add nsw i32 %add17, %mul25, !dbg !181, !UID !182
  val binaryOp_add2660 = Module(new ComputeNode(NumOuts = 1, ID = 60, opCode = "add")(sign = false, Debug = false))

  //  %15 = add nuw nsw i64 %indvars.iv, 3, !dbg !183, !UID !184
  val binaryOp_61 = Module(new ComputeNode(NumOuts = 1, ID = 61, opCode = "add")(sign = false, Debug = false))

  //  %arrayidx29 = getelementptr inbounds i32, i32* %coeffs, i64 %11, !dbg !185, !UID !186
  val Gep_arrayidx2962 = Module(new GepNode(NumIns = 1, NumOuts = 1, ID = 62)(ElementSize = 8, ArraySize = List()))

  //  %16 = load i32, i32* %arrayidx29, align 4, !dbg !185, !tbaa !136, !UID !187
  val ld_63 = Module(new UnTypLoadCache(NumPredOps = 0, NumSuccOps = 0, NumOuts = 1, ID = 63, RouteID = 4))

  //  %arrayidx32 = getelementptr inbounds i32, i32* %mat, i64 %8, !dbg !188, !UID !189
  val Gep_arrayidx3264 = Module(new GepNode(NumIns = 1, NumOuts = 1, ID = 64)(ElementSize = 8, ArraySize = List()))

  //  %17 = load i32, i32* %arrayidx32, align 4, !dbg !188, !tbaa !136, !UID !190
  val ld_65 = Module(new UnTypLoadCache(NumPredOps = 0, NumSuccOps = 0, NumOuts = 1, ID = 65, RouteID = 5))

  //  %mul33 = mul nsw i32 %17, %16, !dbg !191, !UID !192
  val binaryOp_mul3366 = Module(new ComputeNode(NumOuts = 1, ID = 66, opCode = "mul")(sign = false, Debug = false))

  //  %add34 = add nsw i32 %add26, %mul33, !dbg !193, !UID !194
  val binaryOp_add3467 = Module(new ComputeNode(NumOuts = 1, ID = 67, opCode = "add")(sign = false, Debug = false))

  //  %18 = add nuw nsw i64 %indvars.iv, 4, !dbg !195, !UID !196
  val binaryOp_68 = Module(new ComputeNode(NumOuts = 1, ID = 68, opCode = "add")(sign = false, Debug = false))

  //  %arrayidx37 = getelementptr inbounds i32, i32* %coeffs, i64 %15, !dbg !197, !UID !198
  val Gep_arrayidx3769 = Module(new GepNode(NumIns = 1, NumOuts = 1, ID = 69)(ElementSize = 8, ArraySize = List()))

  //  %19 = load i32, i32* %arrayidx37, align 4, !dbg !197, !tbaa !136, !UID !199
  val ld_70 = Module(new UnTypLoadCache(NumPredOps = 0, NumSuccOps = 0, NumOuts = 1, ID = 70, RouteID = 6))

  //  %20 = add nsw i64 %8, 1, !dbg !200, !UID !201
  val binaryOp_71 = Module(new ComputeNode(NumOuts = 1, ID = 71, opCode = "add")(sign = false, Debug = false))

  //  %arrayidx41 = getelementptr inbounds i32, i32* %mat, i64 %20, !dbg !202, !UID !203
  val Gep_arrayidx4172 = Module(new GepNode(NumIns = 1, NumOuts = 1, ID = 72)(ElementSize = 8, ArraySize = List()))

  //  %21 = load i32, i32* %arrayidx41, align 4, !dbg !202, !tbaa !136, !UID !204
  val ld_73 = Module(new UnTypLoadCache(NumPredOps = 0, NumSuccOps = 0, NumOuts = 1, ID = 73, RouteID = 7))

  //  %mul42 = mul nsw i32 %21, %19, !dbg !205, !UID !206
  val binaryOp_mul4274 = Module(new ComputeNode(NumOuts = 1, ID = 74, opCode = "mul")(sign = false, Debug = false))

  //  %add43 = add nsw i32 %add34, %mul42, !dbg !207, !UID !208
  val binaryOp_add4375 = Module(new ComputeNode(NumOuts = 1, ID = 75, opCode = "add")(sign = false, Debug = false))

  //  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 5, !dbg !209, !UID !210
  val binaryOp_indvars_iv_next76 = Module(new ComputeNode(NumOuts = 1, ID = 76, opCode = "add")(sign = false, Debug = false))

  //  %arrayidx46 = getelementptr inbounds i32, i32* %coeffs, i64 %18, !dbg !211, !UID !212
  val Gep_arrayidx4677 = Module(new GepNode(NumIns = 1, NumOuts = 1, ID = 77)(ElementSize = 8, ArraySize = List()))

  //  %22 = load i32, i32* %arrayidx46, align 4, !dbg !211, !tbaa !136, !UID !213
  val ld_78 = Module(new UnTypLoadCache(NumPredOps = 0, NumSuccOps = 0, NumOuts = 1, ID = 78, RouteID = 8))

  //  %23 = add nsw i64 %8, 2, !dbg !214, !UID !215
  val binaryOp_79 = Module(new ComputeNode(NumOuts = 1, ID = 79, opCode = "add")(sign = false, Debug = false))

  //  %arrayidx50 = getelementptr inbounds i32, i32* %mat, i64 %23, !dbg !216, !UID !217
  val Gep_arrayidx5080 = Module(new GepNode(NumIns = 1, NumOuts = 1, ID = 80)(ElementSize = 8, ArraySize = List()))

  //  %24 = load i32, i32* %arrayidx50, align 4, !dbg !216, !tbaa !136, !UID !218
  val ld_81 = Module(new UnTypLoadCache(NumPredOps = 0, NumSuccOps = 0, NumOuts = 1, ID = 81, RouteID = 9))

  //  %mul51 = mul nsw i32 %24, %22, !dbg !219, !UID !220
  val binaryOp_mul5182 = Module(new ComputeNode(NumOuts = 1, ID = 82, opCode = "mul")(sign = false, Debug = false))

  //  %add52 = add nsw i32 %add43, %mul51, !dbg !221, !UID !222
  val binaryOp_add5283 = Module(new ComputeNode(NumOuts = 2, ID = 83, opCode = "add")(sign = false, Debug = false))

  //  %indvars.iv.next135 = add i64 %indvars.iv134, %0, !dbg !223, !UID !224
  val binaryOp_indvars_iv_next13584 = Module(new ComputeNode(NumOuts = 1, ID = 84, opCode = "add")(sign = false, Debug = false))

  //  %inc54 = add nuw nsw i32 %y.0121, 1, !dbg !225, !UID !226
  val binaryOp_inc5485 = Module(new ComputeNode(NumOuts = 2, ID = 85, opCode = "add")(sign = false, Debug = false))

  //  %exitcond = icmp eq i32 %inc54, %1, !dbg !227, !UID !228
  val icmp_exitcond86 = Module(new ComputeNode(NumOuts = 1, ID = 86, opCode = "eq")(sign = false, Debug = false))

  //  br i1 %exitcond, label %for.cond.cleanup11.loopexit, label %for.body12, !dbg !104, !llvm.loop !229, !UID !231, !BB_UID !232
  val br_87 = Module(new CBranchNodeVariable(NumTrue = 1, NumFalse = 1, NumPredecessor = 0, ID = 87))



  /* ================================================================== *
   *                   PRINTING CONSTANTS NODES                         *
   * ================================================================== */

  //i32 1
  val const0 = Module(new ConstFastNode(value = 1, ID = 0))

  //i32 0
  val const1 = Module(new ConstFastNode(value = 0, ID = 1))

  //i32 1
  val const2 = Module(new ConstFastNode(value = 1, ID = 2))

  //i32 0
  val const3 = Module(new ConstFastNode(value = 0, ID = 3))

  //i32 1
  val const4 = Module(new ConstFastNode(value = 1, ID = 4))

  //i32 0
  val const5 = Module(new ConstFastNode(value = 0, ID = 5))

  //i64 1
  val const6 = Module(new ConstFastNode(value = 1, ID = 6))

  //i64 0
  val const7 = Module(new ConstFastNode(value = 0, ID = 7))

  //i32 0
  val const8 = Module(new ConstFastNode(value = 0, ID = 8))

  //i32 0
  val const9 = Module(new ConstFastNode(value = 0, ID = 9))

  //i64 1
  val const10 = Module(new ConstFastNode(value = 1, ID = 10))

  //i64 -2
  val const11 = Module(new ConstFastNode(value = -2, ID = 11))

  //i64 2
  val const12 = Module(new ConstFastNode(value = 2, ID = 12))

  //i64 -1
  val const13 = Module(new ConstFastNode(value = -1, ID = 13))

  //i64 3
  val const14 = Module(new ConstFastNode(value = 3, ID = 14))

  //i64 4
  val const15 = Module(new ConstFastNode(value = 4, ID = 15))

  //i64 1
  val const16 = Module(new ConstFastNode(value = 1, ID = 16))

  //i64 5
  val const17 = Module(new ConstFastNode(value = 5, ID = 17))

  //i64 2
  val const18 = Module(new ConstFastNode(value = 2, ID = 18))

  //i32 1
  val const19 = Module(new ConstFastNode(value = 1, ID = 19))



  /* ================================================================== *
   *                   BASICBLOCK -> PREDICATE INSTRUCTION              *
   * ================================================================== */

  bb_entry0.io.predicateIn(0) <> ArgSplitter.io.Out.enable

  bb_for_body_lr_ph1.io.predicateIn(0) <> br_3.io.TrueOutput(0)

  bb_for_cond_cleanup3.io.predicateIn(1) <> br_3.io.FalseOutput(0)

  bb_for_cond_cleanup3.io.predicateIn(0) <> br_14.io.Out(0)

  bb_for_body5_preheader5.io.predicateIn(0) <> br_20.io.TrueOutput(0)

  bb_for_cond_cleanup47.io.predicateIn(1) <> br_20.io.FalseOutput(0)

  bb_for_cond_cleanup47.io.predicateIn(0) <> br_22.io.Out(0)

  bb_for_body12_preheader9.io.predicateIn(0) <> br_29.io.FalseOutput(0)

  bb_for_cond_cleanup1111.io.predicateIn(1) <> br_29.io.TrueOutput(0)

  bb_for_cond_cleanup1111.io.predicateIn(0) <> br_31.io.Out(0)



  /* ================================================================== *
   *                   BASICBLOCK -> PREDICATE LOOP                     *
   * ================================================================== */

  bb_for_cond_cleanup_loopexit2.io.predicateIn(0) <> Loop_2.io.loopExit(0)

  bb_for_body4.io.predicateIn(1) <> Loop_2.io.activate_loop_start

  bb_for_body4.io.predicateIn(0) <> Loop_2.io.activate_loop_back

  bb_for_cond_cleanup4_loopexit6.io.predicateIn(0) <> Loop_1.io.loopExit(0)

  bb_for_body58.io.predicateIn(1) <> Loop_1.io.activate_loop_start

  bb_for_body58.io.predicateIn(0) <> Loop_1.io.activate_loop_back

  bb_for_cond_cleanup11_loopexit10.io.predicateIn(0) <> Loop_0.io.loopExit(0)

  bb_for_body1212.io.predicateIn(1) <> Loop_0.io.activate_loop_start

  bb_for_body1212.io.predicateIn(0) <> Loop_0.io.activate_loop_back



  /* ================================================================== *
   *                   PRINTING PARALLEL CONNECTIONS                    *
   * ================================================================== */



  /* ================================================================== *
   *                   LOOP -> PREDICATE INSTRUCTION                    *
   * ================================================================== */

  Loop_0.io.enable <> br_30.io.Out(0)

  Loop_0.io.loopBack(0) <> br_87.io.FalseOutput(0)

  Loop_0.io.loopFinish(0) <> br_87.io.TrueOutput(0)

  Loop_1.io.enable <> br_21.io.Out(0)

  Loop_1.io.loopBack(0) <> br_39.io.FalseOutput(0)

  Loop_1.io.loopFinish(0) <> br_39.io.TrueOutput(0)

  Loop_2.io.enable <> br_13.io.Out(0)

  Loop_2.io.loopBack(0) <> br_27.io.FalseOutput(0)

  Loop_2.io.loopFinish(0) <> br_27.io.TrueOutput(0)



  /* ================================================================== *
   *                   ENDING INSTRUCTIONS                              *
   * ================================================================== */



  /* ================================================================== *
   *                   LOOP INPUT DATA DEPENDENCIES                     *
   * ================================================================== */

  Loop_0.io.InLiveIn(0) <> phiindvars_iv14128.io.Out(0)

  Loop_0.io.InLiveIn(1) <> Loop_1.io.OutLiveIn.elements("field0")(0)

  Loop_0.io.InLiveIn(2) <> Loop_1.io.OutLiveIn.elements("field2")(0)

  Loop_0.io.InLiveIn(3) <> Loop_1.io.OutLiveIn.elements("field3")(0)

  Loop_0.io.InLiveIn(4) <> Loop_1.io.OutLiveIn.elements("field4")(0)

  Loop_0.io.InLiveIn(5) <> Loop_1.io.OutLiveIn.elements("field5")(0)

  Loop_1.io.InLiveIn(0) <> sext19.io.Out

  Loop_1.io.InLiveIn(1) <> phiindvars_iv14516.io.Out(0)

  Loop_1.io.InLiveIn(2) <> Loop_2.io.OutLiveIn.elements("field6")(0)

  Loop_1.io.InLiveIn(3) <> Loop_2.io.OutLiveIn.elements("field5")(0)

  Loop_1.io.InLiveIn(4) <> Loop_2.io.OutLiveIn.elements("field7")(0)

  Loop_1.io.InLiveIn(5) <> Loop_2.io.OutLiveIn.elements("field8")(0)

  Loop_1.io.InLiveIn(6) <> Loop_2.io.OutLiveIn.elements("field3")(0)

  Loop_1.io.InLiveIn(7) <> Loop_2.io.OutLiveIn.elements("field9")(0)

  Loop_1.io.InLiveIn(8) <> Loop_2.io.OutLiveIn.elements("field10")(0)

  Loop_1.io.InLiveIn(9) <> Loop_2.io.OutLiveIn.elements("field4")(0)

  Loop_1.io.InLiveIn(10) <> Loop_2.io.OutLiveIn.elements("field11")(0)

  Loop_2.io.InLiveIn(0) <> sext11.io.Out

  Loop_2.io.InLiveIn(1) <> binaryOp_shr0.io.Out(0)

  Loop_2.io.InLiveIn(2) <> icmp_cmp31226.io.Out(0)

  Loop_2.io.InLiveIn(3) <> sext10.io.Out

  Loop_2.io.InLiveIn(4) <> icmp_cmp101177.io.Out(0)

  Loop_2.io.InLiveIn(5) <> ArgSplitter.io.Out.dataPtrs.elements("field2")(0)

  Loop_2.io.InLiveIn(6) <> ArgSplitter.io.Out.dataPtrs.elements("field0")(0)

  Loop_2.io.InLiveIn(7) <> sext8.io.Out

  Loop_2.io.InLiveIn(8) <> binaryOp_9.io.Out(0)

  Loop_2.io.InLiveIn(9) <> ArgSplitter.io.Out.dataVals.elements("field3")(0)

  Loop_2.io.InLiveIn(10) <> ArgSplitter.io.Out.dataPtrs.elements("field1")(0)

  Loop_2.io.InLiveIn(11) <> sextwide_trip_count12.io.Out

  Loop_2.io.InLiveIn(12) <> ArgSplitter.io.Out.dataVals.elements("field0")(0)

  Loop_2.io.InLiveIn(13) <> binaryOp_sub1.io.Out(0)



  /* ================================================================== *
   *                   LOOP DATA LIVE-IN DEPENDENCIES                   *
   * ================================================================== */

  binaryOp_47.io.RightIO <> Loop_0.io.OutLiveIn.elements("field0")(0)

  phiindvars_iv13440.io.InData(1) <> Loop_0.io.OutLiveIn.elements("field1")(0)

  Gep_arrayidx1549.io.baseAddress <> Loop_0.io.OutLiveIn.elements("field2")(0)

  Gep_arrayidx2457.io.baseAddress <> Loop_0.io.OutLiveIn.elements("field2")(1)

  Gep_arrayidx3264.io.baseAddress <> Loop_0.io.OutLiveIn.elements("field2")(2)

  Gep_arrayidx4172.io.baseAddress <> Loop_0.io.OutLiveIn.elements("field2")(3)

  Gep_arrayidx5080.io.baseAddress <> Loop_0.io.OutLiveIn.elements("field2")(4)

  Gep_arrayidx45.io.baseAddress <> Loop_0.io.OutLiveIn.elements("field3")(0)

  Gep_arrayidx2054.io.baseAddress <> Loop_0.io.OutLiveIn.elements("field3")(1)

  Gep_arrayidx2962.io.baseAddress <> Loop_0.io.OutLiveIn.elements("field3")(2)

  Gep_arrayidx3769.io.baseAddress <> Loop_0.io.OutLiveIn.elements("field3")(3)

  Gep_arrayidx4677.io.baseAddress <> Loop_0.io.OutLiveIn.elements("field3")(4)

  binaryOp_indvars_iv_next13584.io.RightIO <> Loop_0.io.OutLiveIn.elements("field4")(0)

  icmp_exitcond86.io.RightIO <> Loop_0.io.OutLiveIn.elements("field5")(0)

  binaryOp_34.io.RightIO <> Loop_1.io.OutLiveIn.elements("field1")(0)

  phiindvars_iv14128.io.InData(1) <> Loop_1.io.OutLiveIn.elements("field6")(0)

  binaryOp_shr5533.io.RightIO <> Loop_1.io.OutLiveIn.elements("field7")(0)

  Gep_arrayidx5835.io.baseAddress <> Loop_1.io.OutLiveIn.elements("field8")(0)

  br_29.io.CmpIO <> Loop_1.io.OutLiveIn.elements("field9")(0)

  icmp_exitcond14438.io.RightIO <> Loop_1.io.OutLiveIn.elements("field10")(0)

  phiindvars_iv14516.io.InData(0) <> Loop_2.io.OutLiveIn.elements("field0")(0)

  phij_012718.io.InData(0) <> Loop_2.io.OutLiveIn.elements("field1")(0)

  br_20.io.CmpIO <> Loop_2.io.OutLiveIn.elements("field2")(0)

  binaryOp_indvars_iv_next14623.io.RightIO <> Loop_2.io.OutLiveIn.elements("field7")(1)

  binaryOp_indvars_iv_next13325.io.RightIO <> Loop_2.io.OutLiveIn.elements("field12")(0)

  icmp_exitcond14726.io.RightIO <> Loop_2.io.OutLiveIn.elements("field13")(0)



  /* ================================================================== *
   *                   LOOP DATA LIVE-OUT DEPENDENCIES                  *
   * ================================================================== */

  Loop_0.io.InLiveOut(0) <> binaryOp_add5283.io.Out(0)



  /* ================================================================== *
   *                   LOOP LIVE OUT DEPENDENCIES                       *
   * ================================================================== */

  phival_0_lcssa32.io.InData(1) <> Loop_0.io.OutLiveOut.elements("field0")(0)



  /* ================================================================== *
   *                   LOOP CARRY DEPENDENCIES                          *
   * ================================================================== */

  Loop_0.io.CarryDepenIn(0) <> binaryOp_add5283.io.Out(1)

  Loop_0.io.CarryDepenIn(1) <> binaryOp_inc5485.io.Out(0)

  Loop_0.io.CarryDepenIn(2) <> binaryOp_indvars_iv_next76.io.Out(0)

  Loop_0.io.CarryDepenIn(3) <> binaryOp_indvars_iv_next13584.io.Out(0)

  Loop_1.io.CarryDepenIn(0) <> binaryOp_indvars_iv_next14237.io.Out(0)

  Loop_2.io.CarryDepenIn(0) <> binaryOp_inc6424.io.Out(0)

  Loop_2.io.CarryDepenIn(1) <> binaryOp_indvars_iv_next14623.io.Out(0)

  Loop_2.io.CarryDepenIn(2) <> binaryOp_indvars_iv_next13325.io.Out(0)



  /* ================================================================== *
   *                   LOOP DATA CARRY DEPENDENCIES                     *
   * ================================================================== */

  phival_012043.io.InData(0) <> Loop_0.io.CarryDepenOut.elements("field0")(0)

  phiy_012142.io.InData(0) <> Loop_0.io.CarryDepenOut.elements("field1")(0)

  phiindvars_iv41.io.InData(0) <> Loop_0.io.CarryDepenOut.elements("field2")(0)

  phiindvars_iv13440.io.InData(0) <> Loop_0.io.CarryDepenOut.elements("field3")(0)

  phiindvars_iv14128.io.InData(0) <> Loop_1.io.CarryDepenOut.elements("field0")(0)

  phij_012718.io.InData(1) <> Loop_2.io.CarryDepenOut.elements("field0")(0)

  phiindvars_iv14516.io.InData(1) <> Loop_2.io.CarryDepenOut.elements("field1")(0)

  phiindvars_iv13217.io.InData(1) <> Loop_2.io.CarryDepenOut.elements("field2")(0)



  /* ================================================================== *
   *                   BASICBLOCK -> ENABLE INSTRUCTION                 *
   * ================================================================== */

  const0.io.enable <> bb_entry0.io.Out(0)

  binaryOp_shr0.io.enable <> bb_entry0.io.Out(1)


  binaryOp_sub1.io.enable <> bb_entry0.io.Out(2)


  icmp_cmp1242.io.enable <> bb_entry0.io.Out(3)


  br_3.io.enable <> bb_entry0.io.Out(4)


  const1.io.enable <> bb_for_body_lr_ph1.io.Out(0)

  const2.io.enable <> bb_for_body_lr_ph1.io.Out(1)

  binaryOp_mul4.io.enable <> bb_for_body_lr_ph1.io.Out(2)


  binaryOp_sub25.io.enable <> bb_for_body_lr_ph1.io.Out(3)


  icmp_cmp31226.io.enable <> bb_for_body_lr_ph1.io.Out(4)


  icmp_cmp101177.io.enable <> bb_for_body_lr_ph1.io.Out(5)


  sext8.io.enable <> bb_for_body_lr_ph1.io.Out(6)


  binaryOp_9.io.enable <> bb_for_body_lr_ph1.io.Out(7)


  sext10.io.enable <> bb_for_body_lr_ph1.io.Out(8)


  sext11.io.enable <> bb_for_body_lr_ph1.io.Out(9)


  sextwide_trip_count12.io.enable <> bb_for_body_lr_ph1.io.Out(10)


  br_13.io.enable <> bb_for_body_lr_ph1.io.Out(11)


  br_14.io.enable <> bb_for_cond_cleanup_loopexit2.io.Out(0)


  ret_15.io.In.enable <> bb_for_cond_cleanup3.io.Out(0)


  const3.io.enable <> bb_for_body4.io.Out(0)

  phiindvars_iv14516.io.enable <> bb_for_body4.io.Out(1)


  phiindvars_iv13217.io.enable <> bb_for_body4.io.Out(2)


  phij_012718.io.enable <> bb_for_body4.io.Out(3)


  sext19.io.enable <> bb_for_body4.io.Out(4)


  br_20.io.enable <> bb_for_body4.io.Out(5)


  br_21.io.enable <> bb_for_body5_preheader5.io.Out(0)


  br_22.io.enable <> bb_for_cond_cleanup4_loopexit6.io.Out(0)


  const4.io.enable <> bb_for_cond_cleanup47.io.Out(0)

  binaryOp_indvars_iv_next14623.io.enable <> bb_for_cond_cleanup47.io.Out(1)


  binaryOp_inc6424.io.enable <> bb_for_cond_cleanup47.io.Out(2)


  binaryOp_indvars_iv_next13325.io.enable <> bb_for_cond_cleanup47.io.Out(3)


  icmp_exitcond14726.io.enable <> bb_for_cond_cleanup47.io.Out(4)


  br_27.io.enable <> bb_for_cond_cleanup47.io.Out(5)


  phiindvars_iv14128.io.enable <> bb_for_body58.io.Out(0)


  br_29.io.enable <> bb_for_body58.io.Out(1)


  br_30.io.enable <> bb_for_body12_preheader9.io.Out(0)


  br_31.io.enable <> bb_for_cond_cleanup11_loopexit10.io.Out(0)


  const5.io.enable <> bb_for_cond_cleanup1111.io.Out(0)

  const6.io.enable <> bb_for_cond_cleanup1111.io.Out(1)

  phival_0_lcssa32.io.enable <> bb_for_cond_cleanup1111.io.Out(2)


  binaryOp_shr5533.io.enable <> bb_for_cond_cleanup1111.io.Out(3)


  binaryOp_34.io.enable <> bb_for_cond_cleanup1111.io.Out(4)


  Gep_arrayidx5835.io.enable <> bb_for_cond_cleanup1111.io.Out(5)


  st_36.io.enable <> bb_for_cond_cleanup1111.io.Out(6)


  binaryOp_indvars_iv_next14237.io.enable <> bb_for_cond_cleanup1111.io.Out(7)


  icmp_exitcond14438.io.enable <> bb_for_cond_cleanup1111.io.Out(8)


  br_39.io.enable <> bb_for_cond_cleanup1111.io.Out(9)


  const7.io.enable <> bb_for_body1212.io.Out(0)

  const8.io.enable <> bb_for_body1212.io.Out(1)

  const9.io.enable <> bb_for_body1212.io.Out(2)

  const10.io.enable <> bb_for_body1212.io.Out(3)

  const11.io.enable <> bb_for_body1212.io.Out(4)

  const12.io.enable <> bb_for_body1212.io.Out(5)

  const13.io.enable <> bb_for_body1212.io.Out(6)

  const14.io.enable <> bb_for_body1212.io.Out(7)

  const15.io.enable <> bb_for_body1212.io.Out(8)

  const16.io.enable <> bb_for_body1212.io.Out(9)

  const17.io.enable <> bb_for_body1212.io.Out(10)

  const18.io.enable <> bb_for_body1212.io.Out(11)

  const19.io.enable <> bb_for_body1212.io.Out(12)

  phiindvars_iv13440.io.enable <> bb_for_body1212.io.Out(13)


  phiindvars_iv41.io.enable <> bb_for_body1212.io.Out(14)


  phiy_012142.io.enable <> bb_for_body1212.io.Out(15)


  phival_012043.io.enable <> bb_for_body1212.io.Out(16)


  binaryOp_44.io.enable <> bb_for_body1212.io.Out(17)


  Gep_arrayidx45.io.enable <> bb_for_body1212.io.Out(18)


  ld_46.io.enable <> bb_for_body1212.io.Out(19)


  binaryOp_47.io.enable <> bb_for_body1212.io.Out(20)


  binaryOp_48.io.enable <> bb_for_body1212.io.Out(21)


  Gep_arrayidx1549.io.enable <> bb_for_body1212.io.Out(22)


  ld_50.io.enable <> bb_for_body1212.io.Out(23)


  binaryOp_mul1651.io.enable <> bb_for_body1212.io.Out(24)


  binaryOp_add1752.io.enable <> bb_for_body1212.io.Out(25)


  binaryOp_53.io.enable <> bb_for_body1212.io.Out(26)


  Gep_arrayidx2054.io.enable <> bb_for_body1212.io.Out(27)


  ld_55.io.enable <> bb_for_body1212.io.Out(28)


  binaryOp_56.io.enable <> bb_for_body1212.io.Out(29)


  Gep_arrayidx2457.io.enable <> bb_for_body1212.io.Out(30)


  ld_58.io.enable <> bb_for_body1212.io.Out(31)


  binaryOp_mul2559.io.enable <> bb_for_body1212.io.Out(32)


  binaryOp_add2660.io.enable <> bb_for_body1212.io.Out(33)


  binaryOp_61.io.enable <> bb_for_body1212.io.Out(34)


  Gep_arrayidx2962.io.enable <> bb_for_body1212.io.Out(35)


  ld_63.io.enable <> bb_for_body1212.io.Out(36)


  Gep_arrayidx3264.io.enable <> bb_for_body1212.io.Out(37)


  ld_65.io.enable <> bb_for_body1212.io.Out(38)


  binaryOp_mul3366.io.enable <> bb_for_body1212.io.Out(39)


  binaryOp_add3467.io.enable <> bb_for_body1212.io.Out(40)


  binaryOp_68.io.enable <> bb_for_body1212.io.Out(41)


  Gep_arrayidx3769.io.enable <> bb_for_body1212.io.Out(42)


  ld_70.io.enable <> bb_for_body1212.io.Out(43)


  binaryOp_71.io.enable <> bb_for_body1212.io.Out(44)


  Gep_arrayidx4172.io.enable <> bb_for_body1212.io.Out(45)


  ld_73.io.enable <> bb_for_body1212.io.Out(46)


  binaryOp_mul4274.io.enable <> bb_for_body1212.io.Out(47)


  binaryOp_add4375.io.enable <> bb_for_body1212.io.Out(48)


  binaryOp_indvars_iv_next76.io.enable <> bb_for_body1212.io.Out(49)


  Gep_arrayidx4677.io.enable <> bb_for_body1212.io.Out(50)


  ld_78.io.enable <> bb_for_body1212.io.Out(51)


  binaryOp_79.io.enable <> bb_for_body1212.io.Out(52)


  Gep_arrayidx5080.io.enable <> bb_for_body1212.io.Out(53)


  ld_81.io.enable <> bb_for_body1212.io.Out(54)


  binaryOp_mul5182.io.enable <> bb_for_body1212.io.Out(55)


  binaryOp_add5283.io.enable <> bb_for_body1212.io.Out(56)


  binaryOp_indvars_iv_next13584.io.enable <> bb_for_body1212.io.Out(57)


  binaryOp_inc5485.io.enable <> bb_for_body1212.io.Out(58)


  icmp_exitcond86.io.enable <> bb_for_body1212.io.Out(59)


  br_87.io.enable <> bb_for_body1212.io.Out(60)




  /* ================================================================== *
   *                   CONNECTING PHI NODES                             *
   * ================================================================== */

  phiindvars_iv14516.io.Mask <> bb_for_body4.io.MaskBB(0)

  phiindvars_iv13217.io.Mask <> bb_for_body4.io.MaskBB(1)

  phij_012718.io.Mask <> bb_for_body4.io.MaskBB(2)

  phiindvars_iv14128.io.Mask <> bb_for_body58.io.MaskBB(0)

  phival_0_lcssa32.io.Mask <> bb_for_cond_cleanup1111.io.MaskBB(0)

  phiindvars_iv13440.io.Mask <> bb_for_body1212.io.MaskBB(0)

  phiindvars_iv41.io.Mask <> bb_for_body1212.io.MaskBB(1)

  phiy_012142.io.Mask <> bb_for_body1212.io.MaskBB(2)

  phival_012043.io.Mask <> bb_for_body1212.io.MaskBB(3)



  /* ================================================================== *
   *                   PRINT ALLOCA OFFSET                              *
   * ================================================================== */



  /* ================================================================== *
   *                   CONNECTING MEMORY CONNECTIONS                    *
   * ================================================================== */

  MemCtrl.io.wr.mem(0).MemReq <> st_36.io.MemReq

  st_36.io.MemResp <> MemCtrl.io.wr.mem(0).MemResp

  MemCtrl.io.rd.mem(0).MemReq <> ld_46.io.MemReq

  ld_46.io.MemResp <> MemCtrl.io.rd.mem(0).MemResp

  MemCtrl.io.rd.mem(1).MemReq <> ld_50.io.MemReq

  ld_50.io.MemResp <> MemCtrl.io.rd.mem(1).MemResp

  MemCtrl.io.rd.mem(2).MemReq <> ld_55.io.MemReq

  ld_55.io.MemResp <> MemCtrl.io.rd.mem(2).MemResp

  MemCtrl.io.rd.mem(3).MemReq <> ld_58.io.MemReq

  ld_58.io.MemResp <> MemCtrl.io.rd.mem(3).MemResp

  MemCtrl.io.rd.mem(4).MemReq <> ld_63.io.MemReq

  ld_63.io.MemResp <> MemCtrl.io.rd.mem(4).MemResp

  MemCtrl.io.rd.mem(5).MemReq <> ld_65.io.MemReq

  ld_65.io.MemResp <> MemCtrl.io.rd.mem(5).MemResp

  MemCtrl.io.rd.mem(6).MemReq <> ld_70.io.MemReq

  ld_70.io.MemResp <> MemCtrl.io.rd.mem(6).MemResp

  MemCtrl.io.rd.mem(7).MemReq <> ld_73.io.MemReq

  ld_73.io.MemResp <> MemCtrl.io.rd.mem(7).MemResp

  MemCtrl.io.rd.mem(8).MemReq <> ld_78.io.MemReq

  ld_78.io.MemResp <> MemCtrl.io.rd.mem(8).MemResp

  MemCtrl.io.rd.mem(9).MemReq <> ld_81.io.MemReq

  ld_81.io.MemResp <> MemCtrl.io.rd.mem(9).MemResp



  /* ================================================================== *
   *                   PRINT SHARED CONNECTIONS                         *
   * ================================================================== */



  /* ================================================================== *
   *                   CONNECTING DATA DEPENDENCIES                     *
   * ================================================================== */

  binaryOp_shr0.io.RightIO <> const0.io.Out

  icmp_cmp101177.io.RightIO <> const1.io.Out

  binaryOp_9.io.RightIO <> const2.io.Out

  phiindvars_iv13217.io.InData(0) <> const3.io.Out

  binaryOp_inc6424.io.RightIO <> const4.io.Out

  phival_0_lcssa32.io.InData(0) <> const5.io.Out

  binaryOp_indvars_iv_next14237.io.RightIO <> const6.io.Out

  phiindvars_iv41.io.InData(1) <> const7.io.Out

  phiy_012142.io.InData(1) <> const8.io.Out

  phival_012043.io.InData(1) <> const9.io.Out

  binaryOp_44.io.RightIO <> const10.io.Out

  binaryOp_48.io.RightIO <> const11.io.Out

  binaryOp_53.io.RightIO <> const12.io.Out

  binaryOp_56.io.RightIO <> const13.io.Out

  binaryOp_61.io.RightIO <> const14.io.Out

  binaryOp_68.io.RightIO <> const15.io.Out

  binaryOp_71.io.RightIO <> const16.io.Out

  binaryOp_indvars_iv_next76.io.RightIO <> const17.io.Out

  binaryOp_79.io.RightIO <> const18.io.Out

  binaryOp_inc5485.io.RightIO <> const19.io.Out

  binaryOp_sub1.io.RightIO <> binaryOp_shr0.io.Out(1)

  icmp_cmp1242.io.LeftIO <> binaryOp_shr0.io.Out(2)

  binaryOp_mul4.io.LeftIO <> binaryOp_shr0.io.Out(3)

  binaryOp_sub25.io.RightIO <> binaryOp_shr0.io.Out(4)

  icmp_cmp31226.io.LeftIO <> binaryOp_shr0.io.Out(5)

  sext10.io.Input <> binaryOp_shr0.io.Out(6)

  icmp_cmp1242.io.RightIO <> binaryOp_sub1.io.Out(1)

  br_3.io.CmpIO <> icmp_cmp1242.io.Out(0)

  sext11.io.Input <> binaryOp_mul4.io.Out(0)

  icmp_cmp31226.io.RightIO <> binaryOp_sub25.io.Out(0)

  sextwide_trip_count12.io.Input <> binaryOp_sub25.io.Out(1)

  binaryOp_indvars_iv_next14623.io.LeftIO <> phiindvars_iv14516.io.Out(1)

  sext19.io.Input <> phiindvars_iv13217.io.Out(0)

  binaryOp_indvars_iv_next13325.io.LeftIO <> phiindvars_iv13217.io.Out(1)

  binaryOp_inc6424.io.LeftIO <> phij_012718.io.Out(0)

  icmp_exitcond14726.io.LeftIO <> binaryOp_inc6424.io.Out(1)

  br_27.io.CmpIO <> icmp_exitcond14726.io.Out(0)

  binaryOp_34.io.LeftIO <> phiindvars_iv14128.io.Out(1)

  binaryOp_indvars_iv_next14237.io.LeftIO <> phiindvars_iv14128.io.Out(2)

  binaryOp_shr5533.io.LeftIO <> phival_0_lcssa32.io.Out(0)

  st_36.io.inData <> binaryOp_shr5533.io.Out(0)

  Gep_arrayidx5835.io.idx(0) <> binaryOp_34.io.Out(0)

  st_36.io.GepAddr <> Gep_arrayidx5835.io.Out(0)

  icmp_exitcond14438.io.LeftIO <> binaryOp_indvars_iv_next14237.io.Out(1)

  br_39.io.CmpIO <> icmp_exitcond14438.io.Out(0)

  binaryOp_47.io.LeftIO <> phiindvars_iv13440.io.Out(0)

  binaryOp_indvars_iv_next13584.io.LeftIO <> phiindvars_iv13440.io.Out(1)

  binaryOp_44.io.LeftIO <> phiindvars_iv41.io.Out(0)

  Gep_arrayidx45.io.idx(0) <> phiindvars_iv41.io.Out(1)

  binaryOp_53.io.LeftIO <> phiindvars_iv41.io.Out(2)

  binaryOp_61.io.LeftIO <> phiindvars_iv41.io.Out(3)

  binaryOp_68.io.LeftIO <> phiindvars_iv41.io.Out(4)

  binaryOp_indvars_iv_next76.io.LeftIO <> phiindvars_iv41.io.Out(5)

  binaryOp_inc5485.io.LeftIO <> phiy_012142.io.Out(0)

  binaryOp_add1752.io.RightIO <> phival_012043.io.Out(0)

  Gep_arrayidx2054.io.idx(0) <> binaryOp_44.io.Out(0)

  ld_46.io.GepAddr <> Gep_arrayidx45.io.Out(0)

  binaryOp_mul1651.io.RightIO <> ld_46.io.Out(0)

  binaryOp_48.io.LeftIO <> binaryOp_47.io.Out(0)

  binaryOp_56.io.LeftIO <> binaryOp_47.io.Out(1)

  Gep_arrayidx3264.io.idx(0) <> binaryOp_47.io.Out(2)

  binaryOp_71.io.LeftIO <> binaryOp_47.io.Out(3)

  binaryOp_79.io.LeftIO <> binaryOp_47.io.Out(4)

  Gep_arrayidx1549.io.idx(0) <> binaryOp_48.io.Out(0)

  ld_50.io.GepAddr <> Gep_arrayidx1549.io.Out(0)

  binaryOp_mul1651.io.LeftIO <> ld_50.io.Out(0)

  binaryOp_add1752.io.LeftIO <> binaryOp_mul1651.io.Out(0)

  binaryOp_add2660.io.LeftIO <> binaryOp_add1752.io.Out(0)

  Gep_arrayidx2962.io.idx(0) <> binaryOp_53.io.Out(0)

  ld_55.io.GepAddr <> Gep_arrayidx2054.io.Out(0)

  binaryOp_mul2559.io.RightIO <> ld_55.io.Out(0)

  Gep_arrayidx2457.io.idx(0) <> binaryOp_56.io.Out(0)

  ld_58.io.GepAddr <> Gep_arrayidx2457.io.Out(0)

  binaryOp_mul2559.io.LeftIO <> ld_58.io.Out(0)

  binaryOp_add2660.io.RightIO <> binaryOp_mul2559.io.Out(0)

  binaryOp_add3467.io.LeftIO <> binaryOp_add2660.io.Out(0)

  Gep_arrayidx3769.io.idx(0) <> binaryOp_61.io.Out(0)

  ld_63.io.GepAddr <> Gep_arrayidx2962.io.Out(0)

  binaryOp_mul3366.io.RightIO <> ld_63.io.Out(0)

  ld_65.io.GepAddr <> Gep_arrayidx3264.io.Out(0)

  binaryOp_mul3366.io.LeftIO <> ld_65.io.Out(0)

  binaryOp_add3467.io.RightIO <> binaryOp_mul3366.io.Out(0)

  binaryOp_add4375.io.LeftIO <> binaryOp_add3467.io.Out(0)

  Gep_arrayidx4677.io.idx(0) <> binaryOp_68.io.Out(0)

  ld_70.io.GepAddr <> Gep_arrayidx3769.io.Out(0)

  binaryOp_mul4274.io.RightIO <> ld_70.io.Out(0)

  Gep_arrayidx4172.io.idx(0) <> binaryOp_71.io.Out(0)

  ld_73.io.GepAddr <> Gep_arrayidx4172.io.Out(0)

  binaryOp_mul4274.io.LeftIO <> ld_73.io.Out(0)

  binaryOp_add4375.io.RightIO <> binaryOp_mul4274.io.Out(0)

  binaryOp_add5283.io.LeftIO <> binaryOp_add4375.io.Out(0)

  ld_78.io.GepAddr <> Gep_arrayidx4677.io.Out(0)

  binaryOp_mul5182.io.RightIO <> ld_78.io.Out(0)

  Gep_arrayidx5080.io.idx(0) <> binaryOp_79.io.Out(0)

  ld_81.io.GepAddr <> Gep_arrayidx5080.io.Out(0)

  binaryOp_mul5182.io.LeftIO <> ld_81.io.Out(0)

  binaryOp_add5283.io.RightIO <> binaryOp_mul5182.io.Out(0)

  icmp_exitcond86.io.LeftIO <> binaryOp_inc5485.io.Out(1)

  br_87.io.CmpIO <> icmp_exitcond86.io.Out(0)

  binaryOp_mul4.io.RightIO <> ArgSplitter.io.Out.dataVals.elements("field0")(1)

  binaryOp_sub25.io.LeftIO <> ArgSplitter.io.Out.dataVals.elements("field0")(2)

  sext8.io.Input <> ArgSplitter.io.Out.dataVals.elements("field0")(3)

  binaryOp_sub1.io.LeftIO <> ArgSplitter.io.Out.dataVals.elements("field1")(0)

  binaryOp_shr0.io.LeftIO <> ArgSplitter.io.Out.dataVals.elements("field2")(0)

  icmp_cmp101177.io.LeftIO <> ArgSplitter.io.Out.dataVals.elements("field2")(1)

  binaryOp_9.io.LeftIO <> ArgSplitter.io.Out.dataVals.elements("field2")(2)

  st_36.io.Out(0).ready := true.B



  /* ================================================================== *
   *                   CONNECTING DATA DEPENDENCIES                     *
   * ================================================================== */

  br_39.io.PredOp(0) <> st_36.io.SuccOp(0)



  /* ================================================================== *
   *                   PRINTING OUTPUT INTERFACE                        *
   * ================================================================== */

  io.out <> ret_15.io.Out

}

