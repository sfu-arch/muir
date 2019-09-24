; ModuleID = './test_07_softmax_b_ir_4.bc'
source_filename = "./07_softmax_b_ir_4.bc"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux_gnu"

define void @test_07_softmax_b_ir_4(i8* nocapture align 8 dereferenceable(8) %retval, i8* noalias nocapture readnone %run_options, i8** noalias nocapture readonly %params, i8** noalias nocapture readonly %temps, i64* noalias nocapture readnone %prof_counters) #0 {
reduce.inner.loop_body.reduction_dim.1.lr.ph:
  %arg0.untyped = load i8*, i8** %params, align 8, !dereferenceable !0, !align !1
  %bitcast = bitcast i8* %arg0.untyped to [1 x [64 x float]]*
  %0 = load i8*, i8** %temps, align 8, !dereferenceable !0, !align !1
  br label %reduce.inner.loop_body.reduction_dim.1

reduce.inner.loop_body.reduction_dim.1:           ; preds = %reduce.inner.loop_body.reduction_dim.1, %reduce.inner.loop_body.reduction_dim.1.lr.ph
  %1 = phi float [ 0xFFF0000000000000, %reduce.inner.loop_body.reduction_dim.1.lr.ph ], [ %7, %reduce.inner.loop_body.reduction_dim.1 ]
  %reduce.inner.indvar.reduction_dim.112 = phi i64 [ 0, %reduce.inner.loop_body.reduction_dim.1.lr.ph ], [ %invar.inc1, %reduce.inner.loop_body.reduction_dim.1 ]
  %2 = getelementptr inbounds [1 x [64 x float]], [1 x [64 x float]]* %bitcast, i64 0, i64 0, i64 %reduce.inner.indvar.reduction_dim.112
  %3 = load float, float* %2, align 4
  %4 = fcmp oge float %1, %3
  %5 = fcmp uno float %1, 0.000000e+00
  %6 = or i1 %4, %5
  %7 = select i1 %6, float %1, float %3
  %invar.inc1 = add nuw nsw i64 %reduce.inner.indvar.reduction_dim.112, 1
  %8 = icmp ugt i64 %invar.inc1, 63
  br i1 %8, label %fusion.1.loop_body.dim.1.lr.ph, label %reduce.inner.loop_body.reduction_dim.1

fusion.1.loop_body.dim.1.lr.ph:                   ; preds = %reduce.inner.loop_body.reduction_dim.1
  %9 = bitcast i8* %0 to float*
  store float %7, float* %9, align 4
  %10 = getelementptr inbounds i8*, i8** %temps, i64 9
  %11 = load i8*, i8** %10, align 8, !dereferenceable !2, !align !1
  %fusion.1 = bitcast i8* %11 to [1 x [64 x float]]*
  br label %fusion.1.loop_body.dim.1

fusion.1.loop_body.dim.1:                         ; preds = %fusion.1.loop_body.dim.1, %fusion.1.loop_body.dim.1.lr.ph
  %fusion.1.indvar.dim.18 = phi i64 [ 0, %fusion.1.loop_body.dim.1.lr.ph ], [ %invar.inc3, %fusion.1.loop_body.dim.1 ]
  %12 = getelementptr inbounds [1 x [64 x float]], [1 x [64 x float]]* %bitcast, i64 0, i64 0, i64 %fusion.1.indvar.dim.18
  %13 = load float, float* %12, align 4
  %14 = load float, float* %9, align 4
  %15 = fsub float %13, %14
  %16 = call float @llvm.exp.f32(float %15)
  %17 = getelementptr inbounds [1 x [64 x float]], [1 x [64 x float]]* %fusion.1, i64 0, i64 0, i64 %fusion.1.indvar.dim.18
  store float %16, float* %17, align 4
  %invar.inc3 = add nuw nsw i64 %fusion.1.indvar.dim.18, 1
  %18 = icmp ugt i64 %invar.inc3, 63
  br i1 %18, label %reduce.1.inner.loop_body.reduction_dim.1.lr.ph, label %fusion.1.loop_body.dim.1

reduce.1.inner.loop_body.reduction_dim.1.lr.ph:   ; preds = %fusion.1.loop_body.dim.1
  %19 = getelementptr inbounds i8, i8* %11, i64 256
  br label %reduce.1.inner.loop_body.reduction_dim.1

reduce.1.inner.loop_body.reduction_dim.1:         ; preds = %reduce.1.inner.loop_body.reduction_dim.1, %reduce.1.inner.loop_body.reduction_dim.1.lr.ph
  %20 = phi float [ 0.000000e+00, %reduce.1.inner.loop_body.reduction_dim.1.lr.ph ], [ %23, %reduce.1.inner.loop_body.reduction_dim.1 ]
  %reduce.1.inner.indvar.reduction_dim.14 = phi i64 [ 0, %reduce.1.inner.loop_body.reduction_dim.1.lr.ph ], [ %invar.inc6, %reduce.1.inner.loop_body.reduction_dim.1 ]
  %21 = getelementptr inbounds [1 x [64 x float]], [1 x [64 x float]]* %fusion.1, i64 0, i64 0, i64 %reduce.1.inner.indvar.reduction_dim.14
  %22 = load float, float* %21, align 4
  %23 = fadd float %20, %22
  %invar.inc6 = add nuw nsw i64 %reduce.1.inner.indvar.reduction_dim.14, 1
  %24 = icmp ugt i64 %invar.inc6, 63
  br i1 %24, label %fusion.loop_body.dim.0.lr.ph, label %reduce.1.inner.loop_body.reduction_dim.1

fusion.loop_body.dim.0.lr.ph:                     ; preds = %reduce.1.inner.loop_body.reduction_dim.1
  %25 = bitcast i8* %19 to float*
  store float %23, float* %25, align 4
  %fusion = bitcast i8* %0 to [64 x float]*
  br label %fusion.loop_body.dim.0

fusion.loop_body.dim.0:                           ; preds = %fusion.loop_body.dim.0, %fusion.loop_body.dim.0.lr.ph
  %fusion.indvar.dim.02 = phi i64 [ 0, %fusion.loop_body.dim.0.lr.ph ], [ %invar.inc12, %fusion.loop_body.dim.0 ]
  %26 = getelementptr inbounds [1 x [64 x float]], [1 x [64 x float]]* %fusion.1, i64 0, i64 0, i64 %fusion.indvar.dim.02
  %27 = load float, float* %26, align 4
  %28 = load float, float* %25, align 4
  %29 = fdiv float %27, %28
  %30 = getelementptr inbounds [64 x float], [64 x float]* %fusion, i64 0, i64 %fusion.indvar.dim.02
  store float %29, float* %30, align 4
  %invar.inc12 = add nuw nsw i64 %fusion.indvar.dim.02, 1
  %31 = icmp ugt i64 %invar.inc12, 63
  br i1 %31, label %fusion.loop_exit.dim.0, label %fusion.loop_body.dim.0

fusion.loop_exit.dim.0:                           ; preds = %fusion.loop_body.dim.0
  %32 = bitcast i8* %retval to i8**
  store i8* %0, i8** %32, align 8
  ret void
}

; Function Attrs: nounwind readnone
declare float @llvm.exp.f32(float) #1

attributes #0 = { "no-frame-pointer-elim"="false" }
attributes #1 = { nounwind readnone }

!0 = !{i64 256}
!1 = !{i64 8}
!2 = !{i64 260}
