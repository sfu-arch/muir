; ModuleID = 'bfs.c'
source_filename = "bfs.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.node_t_struct = type { i64, i64 }
%struct.edge_t_struct = type { i64 }

; Function Attrs: norecurse nounwind uwtable
define void @bfs(%struct.node_t_struct* nocapture readonly, %struct.edge_t_struct* nocapture readonly, i64, i64* nocapture, i64* nocapture) local_unnamed_addr #0 {
  %6 = getelementptr inbounds i64, i64* %3, i64 %2
  store i64 0, i64* %6, align 8, !tbaa !2
  store i64 1, i64* %4, align 8, !tbaa !2
  br label %7

; <label>:7:                                      ; preds = %10, %5
  %8 = phi i64 [ 0, %5 ], [ %11, %10 ]
  %9 = add nuw nsw i64 %8, 1
  br label %16

; <label>:10:                                     ; preds = %43
  %11 = add nuw nsw i64 %8, 1
  %12 = getelementptr inbounds i64, i64* %4, i64 %11
  store i64 %44, i64* %12, align 8, !tbaa !2
  %13 = icmp ne i64 %44, 0
  %14 = icmp ult i64 %8, 9
  %15 = and i1 %13, %14
  br i1 %15, label %7, label %47

; <label>:16:                                     ; preds = %43, %7
  %17 = phi i64 [ 0, %7 ], [ %44, %43 ]
  %18 = phi i64 [ 0, %7 ], [ %45, %43 ]
  %19 = getelementptr inbounds i64, i64* %3, i64 %18
  %20 = load i64, i64* %19, align 8, !tbaa !2
  %21 = icmp eq i64 %20, %8
  br i1 %21, label %22, label %43

; <label>:22:                                     ; preds = %16
  %23 = getelementptr inbounds %struct.node_t_struct, %struct.node_t_struct* %0, i64 %18, i32 0
  %24 = load i64, i64* %23, align 8, !tbaa !6
  %25 = getelementptr inbounds %struct.node_t_struct, %struct.node_t_struct* %0, i64 %18, i32 1
  %26 = load i64, i64* %25, align 8, !tbaa !8
  %27 = icmp ult i64 %24, %26
  br i1 %27, label %28, label %43

; <label>:28:                                     ; preds = %22
  br label %29

; <label>:29:                                     ; preds = %28, %39
  %30 = phi i64 [ %40, %39 ], [ %17, %28 ]
  %31 = phi i64 [ %41, %39 ], [ %24, %28 ]
  %32 = getelementptr inbounds %struct.edge_t_struct, %struct.edge_t_struct* %1, i64 %31, i32 0
  %33 = load i64, i64* %32, align 8, !tbaa !9
  %34 = getelementptr inbounds i64, i64* %3, i64 %33
  %35 = load i64, i64* %34, align 8, !tbaa !2
  %36 = icmp eq i64 %35, 127
  br i1 %36, label %37, label %39

; <label>:37:                                     ; preds = %29
  store i64 %9, i64* %34, align 8, !tbaa !2
  %38 = add i64 %30, 1
  br label %39

; <label>:39:                                     ; preds = %37, %29
  %40 = phi i64 [ %38, %37 ], [ %30, %29 ]
  %41 = add nuw i64 %31, 1
  %42 = icmp eq i64 %41, %26
  br i1 %42, label %43, label %29

; <label>:43:                                     ; preds = %39, %22, %16
  %44 = phi i64 [ %17, %16 ], [ %17, %22 ], [ %40, %39 ]
  %45 = add nuw nsw i64 %18, 1
  %46 = icmp eq i64 %45, 256
  br i1 %46, label %10, label %16

; <label>:47:                                     ; preds = %10
  ret void
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

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
