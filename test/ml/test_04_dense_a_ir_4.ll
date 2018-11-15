; ModuleID = './test_04_dense_a_ir_4.bc'
source_filename = "./04_dense_a_ir_4.bc"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux_gnu"

define void @test_04_dense_a_ir_4(i8* nocapture align 8 dereferenceable(8) %retval, i8* noalias nocapture readnone %run_options, i8** noalias nocapture readonly %params, i8** noalias nocapture readonly %temps, i64* noalias nocapture readnone %prof_counters) #0 {
dot.loop_body.rhs.1.lr.ph:
  %0 = getelementptr inbounds i8*, i8** %params, i64 1
  %arg1.untyped = load i8*, i8** %0, align 8, !dereferenceable !0, !align !1
  %1 = bitcast i8* %arg1.untyped to [1 x [8 x float]]*
  %arg0.untyped = load i8*, i8** %params, align 8, !dereferenceable !2, !align !1
  %2 = getelementptr inbounds i8*, i8** %params, i64 2
  %arg2.untyped = load i8*, i8** %2, align 8, !dereferenceable !0, !align !1
  %3 = bitcast i8* %arg2.untyped to [8 x float]*
  %4 = getelementptr inbounds i8*, i8** %temps, i64 5
  %5 = load i8*, i8** %4, align 8, !dereferenceable !0, !align !1
  %dot = bitcast i8* %5 to [1 x [8 x float]]*
  br label %dot.loop_exit.reduction

dot.loop_exit.reduction:                          ; preds = %dot.loop_exit.reduction, %dot.loop_body.rhs.1.lr.ph
  %dot.indvar.rhs.16 = phi i64 [ 0, %dot.loop_body.rhs.1.lr.ph ], [ %invar.inc1, %dot.loop_exit.reduction ]
  %.phi.trans.insert = getelementptr inbounds [1 x [8 x float]], [1 x [8 x float]]* %1, i64 0, i64 0, i64 %dot.indvar.rhs.16
  %.pre = load float, float* %.phi.trans.insert, align 4
  %.phi.trans.insert9 = bitcast i8* %arg0.untyped to float*
  %.pre10 = load float, float* %.phi.trans.insert9, align 4
  %6 = fmul float %.pre10, %.pre
  %7 = fadd float %6, 0.000000e+00
  %8 = getelementptr inbounds [1 x [8 x float]], [1 x [8 x float]]* %dot, i64 0, i64 0, i64 %dot.indvar.rhs.16
  store float %7, float* %8, align 4
  %invar.inc1 = add nuw nsw i64 %dot.indvar.rhs.16, 1
  %9 = icmp ugt i64 %invar.inc1, 7
  br i1 %9, label %fusion.loop_body.dim.0.lr.ph, label %dot.loop_exit.reduction

fusion.loop_body.dim.0.lr.ph:                     ; preds = %dot.loop_exit.reduction
  %10 = load i8*, i8** %temps, align 8, !dereferenceable !0, !align !1
  %fusion = bitcast i8* %10 to [8 x float]*
  br label %fusion.loop_body.dim.0

fusion.loop_body.dim.0:                           ; preds = %fusion.loop_body.dim.0, %fusion.loop_body.dim.0.lr.ph
  %fusion.indvar.dim.02 = phi i64 [ 0, %fusion.loop_body.dim.0.lr.ph ], [ %invar.inc3, %fusion.loop_body.dim.0 ]
  %11 = getelementptr inbounds [8 x float], [8 x float]* %3, i64 0, i64 %fusion.indvar.dim.02
  %12 = load float, float* %11, align 4
  %13 = getelementptr inbounds [1 x [8 x float]], [1 x [8 x float]]* %dot, i64 0, i64 0, i64 %fusion.indvar.dim.02
  %14 = load float, float* %13, align 4
  %15 = fadd float %12, %14
  %16 = getelementptr inbounds [8 x float], [8 x float]* %fusion, i64 0, i64 %fusion.indvar.dim.02
  store float %15, float* %16, align 4
  %invar.inc3 = add nuw nsw i64 %fusion.indvar.dim.02, 1
  %17 = icmp ugt i64 %invar.inc3, 7
  br i1 %17, label %fusion.loop_exit.dim.0, label %fusion.loop_body.dim.0

fusion.loop_exit.dim.0:                           ; preds = %fusion.loop_body.dim.0
  %18 = bitcast i8* %retval to i8**
  store i8* %10, i8** %18, align 8
  ret void
}

attributes #0 = { "no-frame-pointer-elim"="false" }

!0 = !{i64 32}
!1 = !{i64 8}
!2 = !{i64 4}
