; ModuleID = 'test_09_conv2d_a_ir_4.bc'
source_filename = "./09_conv2d_a_ir_4.bc"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux_gnu"

define void @test_09_conv2d_a_ir_4(i8* nocapture align 8 dereferenceable(8) %retval, i8* noalias nocapture readnone %run_options, i8** noalias nocapture readonly %params, i8** noalias nocapture readonly %temps, i64* noalias nocapture readnone %prof_counters) #0 {
convolution.loop_body.dim.1.lr.ph:
  %arg0.untyped = load i8*, i8** %params, align 8, !dereferenceable !0, !align !1
  %0 = bitcast i8* %arg0.untyped to [1 x [8 x [8 x [1 x float]]]]*
  %1 = getelementptr inbounds i8*, i8** %params, i64 1
  %arg1.untyped = load i8*, i8** %1, align 8, !dereferenceable !2, !align !1
  %2 = bitcast i8* %arg1.untyped to [3 x [3 x [1 x [2 x float]]]]*
  %3 = load i8*, i8** %temps, align 8, !dereferenceable !3, !align !4
  %convolution = bitcast i8* %3 to [1 x [8 x [8 x [2 x float]]]]*
  br label %convolution.loop_body.dim.2.lr.ph

convolution.loop_body.dim.2.lr.ph:                ; preds = %convolution.loop_exit.dim.2, %convolution.loop_body.dim.1.lr.ph
  %convolution.indvar.dim.126 = phi i64 [ 0, %convolution.loop_body.dim.1.lr.ph ], [ %invar.inc1, %convolution.loop_exit.dim.2 ]
  br label %convolution.loop_body.dim.3.lr.ph

convolution.loop_body.dim.3.lr.ph:                ; preds = %convolution.loop_exit.dim.3, %convolution.loop_body.dim.2.lr.ph
  %convolution.indvar.dim.223 = phi i64 [ 0, %convolution.loop_body.dim.2.lr.ph ], [ %invar.inc2, %convolution.loop_exit.dim.3 ]
  br label %convolution.inner.loop_body.k0.lr.ph

convolution.inner.loop_body.k0.lr.ph:             ; preds = %convolution.inner.loop_exit.k0, %convolution.loop_body.dim.3.lr.ph
  %convolution.indvar.dim.320 = phi i64 [ 0, %convolution.loop_body.dim.3.lr.ph ], [ %invar.inc3, %convolution.inner.loop_exit.k0 ]
  br label %convolution.inner.loop_body.k1.lr.ph

convolution.inner.loop_body.k1.lr.ph:             ; preds = %convolution.inner.loop_exit.k1, %convolution.inner.loop_body.k0.lr.ph
  %4 = phi float [ 0.000000e+00, %convolution.inner.loop_body.k0.lr.ph ], [ %24, %convolution.inner.loop_exit.k1 ]
  %5 = phi float [ 0.000000e+00, %convolution.inner.loop_body.k0.lr.ph ], [ %25, %convolution.inner.loop_exit.k1 ]
  %convolution.inner.indvar.k016 = phi i64 [ 0, %convolution.inner.loop_body.k0.lr.ph ], [ %invar.inc4, %convolution.inner.loop_exit.k1 ]
  %6 = add i64 %convolution.indvar.dim.126, -1
  %7 = add i64 %6, %convolution.inner.indvar.k016
  %8 = icmp ult i64 %7, 8
  %9 = add i64 %convolution.indvar.dim.223, -1
  br i1 %8, label %convolution.inner.loop_body.iz.lr.ph.us, label %convolution.inner.loop_exit.iz

convolution.inner.loop_exit.iz.us:                ; preds = %convolution.inner.loop_header.iz.convolution.inner.loop_exit.iz_crit_edge.us-lcssa.us.us, %convolution.inner.loop_body.iz.lr.ph.us
  %10 = phi float [ %22, %convolution.inner.loop_header.iz.convolution.inner.loop_exit.iz_crit_edge.us-lcssa.us.us ], [ %13, %convolution.inner.loop_body.iz.lr.ph.us ]
  %11 = phi float [ %22, %convolution.inner.loop_header.iz.convolution.inner.loop_exit.iz_crit_edge.us-lcssa.us.us ], [ %14, %convolution.inner.loop_body.iz.lr.ph.us ]
  %invar.inc5.us = add nuw nsw i64 %convolution.inner.indvar.k14.us, 1
  %12 = icmp ugt i64 %invar.inc5.us, 2
  br i1 %12, label %convolution.inner.loop_exit.k1, label %convolution.inner.loop_body.iz.lr.ph.us

convolution.inner.loop_body.iz.lr.ph.us:          ; preds = %convolution.inner.loop_exit.iz.us, %convolution.inner.loop_body.k1.lr.ph
  %13 = phi float [ %10, %convolution.inner.loop_exit.iz.us ], [ %4, %convolution.inner.loop_body.k1.lr.ph ]
  %14 = phi float [ %11, %convolution.inner.loop_exit.iz.us ], [ %5, %convolution.inner.loop_body.k1.lr.ph ]
  %convolution.inner.indvar.k14.us = phi i64 [ %invar.inc5.us, %convolution.inner.loop_exit.iz.us ], [ 0, %convolution.inner.loop_body.k1.lr.ph ]
  %15 = add i64 %9, %convolution.inner.indvar.k14.us
  %16 = icmp ult i64 %15, 8
  br i1 %16, label %convolution.inner.loop_header.iz.convolution.inner.loop_exit.iz_crit_edge.us-lcssa.us.us, label %convolution.inner.loop_exit.iz.us

convolution.inner.loop_header.iz.convolution.inner.loop_exit.iz_crit_edge.us-lcssa.us.us: ; preds = %convolution.inner.loop_body.iz.lr.ph.us
  %17 = getelementptr inbounds [3 x [3 x [1 x [2 x float]]]], [3 x [3 x [1 x [2 x float]]]]* %2, i64 0, i64 %convolution.inner.indvar.k016, i64 %convolution.inner.indvar.k14.us, i64 0, i64 %convolution.indvar.dim.320
  %18 = load float, float* %17, align 4
  %19 = getelementptr inbounds [1 x [8 x [8 x [1 x float]]]], [1 x [8 x [8 x [1 x float]]]]* %0, i64 0, i64 0, i64 %7, i64 %15, i64 0
  %20 = load float, float* %19, align 4
  %21 = fmul float %18, %20
  %22 = fadd float %13, %21
  br label %convolution.inner.loop_exit.iz.us

convolution.inner.loop_exit.iz:                   ; preds = %convolution.inner.loop_exit.iz, %convolution.inner.loop_body.k1.lr.ph
  %convolution.inner.indvar.k14 = phi i64 [ %invar.inc5, %convolution.inner.loop_exit.iz ], [ 0, %convolution.inner.loop_body.k1.lr.ph ]
  %invar.inc5 = add nuw nsw i64 %convolution.inner.indvar.k14, 1
  %23 = icmp ugt i64 %invar.inc5, 2
  br i1 %23, label %convolution.inner.loop_exit.k1, label %convolution.inner.loop_exit.iz

convolution.inner.loop_exit.k1:                   ; preds = %convolution.inner.loop_exit.iz, %convolution.inner.loop_exit.iz.us
  %24 = phi float [ %10, %convolution.inner.loop_exit.iz.us ], [ %4, %convolution.inner.loop_exit.iz ]
  %25 = phi float [ %11, %convolution.inner.loop_exit.iz.us ], [ %5, %convolution.inner.loop_exit.iz ]
  %invar.inc4 = add nuw nsw i64 %convolution.inner.indvar.k016, 1
  %26 = icmp ugt i64 %invar.inc4, 2
  br i1 %26, label %convolution.inner.loop_exit.k0, label %convolution.inner.loop_body.k1.lr.ph

convolution.inner.loop_exit.k0:                   ; preds = %convolution.inner.loop_exit.k1
  %27 = getelementptr inbounds [1 x [8 x [8 x [2 x float]]]], [1 x [8 x [8 x [2 x float]]]]* %convolution, i64 0, i64 0, i64 %convolution.indvar.dim.126, i64 %convolution.indvar.dim.223, i64 %convolution.indvar.dim.320
  store float %25, float* %27, align 4
  %invar.inc3 = add nuw nsw i64 %convolution.indvar.dim.320, 1
  %28 = icmp eq i64 %convolution.indvar.dim.320, 0
  br i1 %28, label %convolution.inner.loop_body.k0.lr.ph, label %convolution.loop_exit.dim.3

convolution.loop_exit.dim.3:                      ; preds = %convolution.inner.loop_exit.k0
  %invar.inc2 = add nuw nsw i64 %convolution.indvar.dim.223, 1
  %29 = icmp ugt i64 %invar.inc2, 7
  br i1 %29, label %convolution.loop_exit.dim.2, label %convolution.loop_body.dim.3.lr.ph

convolution.loop_exit.dim.2:                      ; preds = %convolution.loop_exit.dim.3
  %invar.inc1 = add nuw nsw i64 %convolution.indvar.dim.126, 1
  %30 = icmp ugt i64 %invar.inc1, 7
  br i1 %30, label %convolution.loop_exit.dim.0, label %convolution.loop_body.dim.2.lr.ph

convolution.loop_exit.dim.0:                      ; preds = %convolution.loop_exit.dim.2
  %31 = bitcast i8* %retval to i8**
  store i8* %3, i8** %31, align 8
  ret void
}

attributes #0 = { "no-frame-pointer-elim"="false" }

!0 = !{i64 256}
!1 = !{i64 8}
!2 = !{i64 72}
!3 = !{i64 512}
!4 = !{i64 16}
