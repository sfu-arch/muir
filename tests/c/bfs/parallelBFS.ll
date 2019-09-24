; ModuleID = 'cilk_bfs.c'
source_filename = "cilk_bfs.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.node_t_struct = type { i64, i64 }
%struct.edge_t_struct = type { i64 }

; Function Attrs: nounwind uwtable
define void @bfs(%struct.node_t_struct* nocapture readonly, %struct.edge_t_struct* nocapture readonly, i64, i64* nocapture, i64* nocapture) local_unnamed_addr #0 {
  %6 = alloca i64, align 8
  %7 = tail call token @llvm.syncregion.start()
  %8 = bitcast i64* %6 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %8)
  %9 = getelementptr inbounds i64, i64* %3, i64 %2
  store i64 0, i64* %9, align 8, !tbaa !2
  store i64 1, i64* %4, align 8, !tbaa !2
  br label %10

; <label>:10:                                     ; preds = %14, %5
  %11 = phi i64 [ 0, %5 ], [ %16, %14 ]
  store i64 0, i64* %6, align 8
  %12 = add nuw nsw i64 %11, 1
  br label %21

; <label>:13:                                     ; preds = %54
  sync within %7, label %14

; <label>:14:                                     ; preds = %13
  %15 = load i64, i64* %6, align 8
  %16 = add nuw nsw i64 %11, 1
  %17 = getelementptr inbounds i64, i64* %4, i64 %16
  store i64 %15, i64* %17, align 8, !tbaa !2
  %18 = icmp ne i64 %15, 0
  %19 = icmp ult i64 %11, 9
  %20 = and i1 %18, %19
  br i1 %20, label %10, label %57

; <label>:21:                                     ; preds = %54, %10
  %22 = phi i64 [ 0, %10 ], [ %55, %54 ]
  detach within %7, label %23, label %54

; <label>:23:                                     ; preds = %21
  %24 = tail call token @llvm.syncregion.start()
  %25 = getelementptr inbounds i64, i64* %3, i64 %22
  %26 = load i64, i64* %25, align 8, !tbaa !2
  %27 = icmp eq i64 %26, %11
  br i1 %27, label %28, label %53

; <label>:28:                                     ; preds = %23
  %29 = getelementptr inbounds %struct.node_t_struct, %struct.node_t_struct* %0, i64 %22, i32 0
  %30 = load i64, i64* %29, align 8, !tbaa !6
  %31 = getelementptr inbounds %struct.node_t_struct, %struct.node_t_struct* %0, i64 %22, i32 1
  %32 = load i64, i64* %31, align 8, !tbaa !8
  %33 = sub i64 %32, %30
  %34 = icmp eq i64 %33, 0
  br i1 %34, label %36, label %35

; <label>:35:                                     ; preds = %28
  br label %37

; <label>:36:                                     ; preds = %50, %28
  sync within %24, label %53

; <label>:37:                                     ; preds = %35, %50
  %38 = phi i64 [ %51, %50 ], [ 0, %35 ]
  detach within %24, label %39, label %50

; <label>:39:                                     ; preds = %37
  %40 = add i64 %38, %30
  %41 = getelementptr inbounds %struct.edge_t_struct, %struct.edge_t_struct* %1, i64 %40, i32 0
  %42 = load i64, i64* %41, align 8, !tbaa !9
  %43 = getelementptr inbounds i64, i64* %3, i64 %42
  %44 = load i64, i64* %43, align 8, !tbaa !2
  %45 = icmp eq i64 %44, 127
  br i1 %45, label %46, label %49

; <label>:46:                                     ; preds = %39
  store i64 %12, i64* %43, align 8, !tbaa !2
  %47 = load i64, i64* %6, align 8
  %48 = add i64 %47, 1
  store i64 %48, i64* %6, align 8
  br label %49

; <label>:49:                                     ; preds = %46, %39
  reattach within %24, label %50

; <label>:50:                                     ; preds = %49, %37
  %51 = add nuw i64 %38, 1
  %52 = icmp eq i64 %51, %33
  br i1 %52, label %36, label %37, !llvm.loop !11

; <label>:53:                                     ; preds = %36, %23
  reattach within %7, label %54

; <label>:54:                                     ; preds = %53, %21
  %55 = add nuw nsw i64 %22, 1
  %56 = icmp eq i64 %55, 256
  br i1 %56, label %13, label %21, !llvm.loop !13

; <label>:57:                                     ; preds = %14
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %8)
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare token @llvm.syncregion.start() #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 5.0.0 (https://github.com/wsmoses/Cilk-Clang 9e81b3be8a7749cb8feea3f6bad30df9b7ba1e75) (https://github.com/wsmoses/Parallel-IR 4ccec4566bf6d847c26c8e8d1af3ec378cc262f5)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"long", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!7, !3, i64 0}
!7 = !{!"node_t_struct", !3, i64 0, !3, i64 8}
!8 = !{!7, !3, i64 8}
!9 = !{!10, !3, i64 0}
!10 = !{!"edge_t_struct", !3, i64 0}
!11 = distinct !{!11, !12}
!12 = !{!"tapir.loop.spawn.strategy", i32 1}
!13 = distinct !{!13, !12}
