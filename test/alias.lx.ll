; ModuleID = 'alias.lx.bc'
source_filename = "alias.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind uwtable
define void @foo(float**, float**, float, i32) #0 !dbg !6 {
  call void @llvm.dbg.value(metadata float** %0, i64 0, metadata !13, metadata !14), !dbg !15
  call void @llvm.dbg.value(metadata float** %1, i64 0, metadata !16, metadata !14), !dbg !17
  call void @llvm.dbg.value(metadata float %2, i64 0, metadata !18, metadata !14), !dbg !19
  call void @llvm.dbg.value(metadata i32 %3, i64 0, metadata !20, metadata !14), !dbg !21
  call void @llvm.dbg.value(metadata i32 0, i64 0, metadata !22, metadata !14), !dbg !23
  br label %codeRepl2, !dbg !24

codeRepl2:                                        ; preds = %4
  call void @__lx_alias.c_5(i32 %3, float** %0, float %2, float** %1)
  br label %5

; <label>:5:                                      ; preds = %codeRepl2
  br label %.split1, !dbg !26

.split1:                                          ; preds = %5
  ret void, !dbg !26
}

; Function Attrs: nounwind readnone
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: noinline nounwind uwtable
define i32 @main() #0 !dbg !27 {
  call void @foo(float** undef, float** undef, float undef, i32 undef), !dbg !30
  ret i32 0, !dbg !31
}

; Function Attrs: nounwind readnone
declare void @llvm.dbg.value(metadata, i64, metadata, metadata) #1

; Function Attrs: noinline nounwind uwtable
define internal void @__lx_alias.c_6(i32, i32 %.01, float**, float, float**) #0 {
newFuncRoot:
  br label %4

.exitStub:                                        ; preds = %4
  ret void

; <label>:4:                                      ; preds = %31, %newFuncRoot
  %.0 = phi i32 [ 0, %newFuncRoot ], [ %32, %31 ]
  %5 = icmp slt i32 %.0, %0
  br i1 %5, label %6, label %.exitStub

; <label>:6:                                      ; preds = %4
  %7 = sext i32 %.01 to i64
  %8 = getelementptr inbounds float*, float** %1, i64 %7
  %9 = load float*, float** %8, align 8
  %10 = sext i32 %.0 to i64
  %11 = getelementptr inbounds float, float* %9, i64 %10
  %12 = load float, float* %11, align 4
  %13 = fmul float %12, %2
  %14 = sext i32 %.01 to i64
  %15 = getelementptr inbounds float*, float** %3, i64 %14
  %16 = load float*, float** %15, align 8
  %17 = sext i32 %.0 to i64
  %18 = getelementptr inbounds float, float* %16, i64 %17
  store float %13, float* %18, align 4
  %19 = sext i32 %.01 to i64
  %20 = getelementptr inbounds float*, float** %1, i64 %19
  %21 = load float*, float** %20, align 8
  %22 = sext i32 %.0 to i64
  %23 = getelementptr inbounds float, float* %21, i64 %22
  %24 = load float, float* %23, align 4
  %25 = fmul float %24, %2
  %26 = sext i32 %.01 to i64
  %27 = getelementptr inbounds float*, float** %3, i64 %26
  %28 = load float*, float** %27, align 8
  %29 = sext i32 %.0 to i64
  %30 = getelementptr inbounds float, float* %28, i64 %29
  store float %25, float* %30, align 4
  br label %31

; <label>:31:                                     ; preds = %6
  %32 = add nsw i32 %.0, 1
  br label %4, !llvm.loop !32
}

; Function Attrs: noinline nounwind uwtable
define internal void @__lx_alias.c_5(i32, float**, float, float**) #0 {
newFuncRoot:
  br label %4

.exitStub:                                        ; preds = %4
  ret void

; <label>:4:                                      ; preds = %8, %newFuncRoot
  %.01 = phi i32 [ 0, %newFuncRoot ], [ %9, %8 ]
  %5 = icmp slt i32 %.01, %0
  br i1 %5, label %6, label %.exitStub

; <label>:6:                                      ; preds = %4
  br label %codeRepl

codeRepl:                                         ; preds = %6
  call void @__lx_alias.c_6(i32 %0, i32 %.01, float** %1, float %2, float** %3)
  br label %7

; <label>:7:                                      ; preds = %codeRepl
  br label %.split

.split:                                           ; preds = %7
  br label %8

; <label>:8:                                      ; preds = %.split
  %9 = add nsw i32 %.01, 1
  br label %4, !llvm.loop !38
}

attributes #0 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4}
!llvm.ident = !{!5}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 4.0.1 (tags/RELEASE_401/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2)
!1 = !DIFile(filename: "alias.c", directory: "/home/amiralis/git/xketch-generator/test")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{!"clang version 4.0.1 (tags/RELEASE_401/final)"}
!6 = distinct !DISubprogram(name: "foo", scope: !1, file: !1, line: 1, type: !7, isLocal: false, isDefinition: true, scopeLine: 2, flags: DIFlagPrototyped, isOptimized: false, unit: !0, variables: !2)
!7 = !DISubroutineType(types: !8)
!8 = !{null, !9, !9, !11, !12}
!9 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !10, size: 64)
!10 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !11, size: 64)
!11 = !DIBasicType(name: "float", size: 32, encoding: DW_ATE_float)
!12 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!13 = !DILocalVariable(name: "in", arg: 1, scope: !6, file: !1, line: 1, type: !9)
!14 = !DIExpression()
!15 = !DILocation(line: 1, column: 18, scope: !6)
!16 = !DILocalVariable(name: "out", arg: 2, scope: !6, file: !1, line: 1, type: !9)
!17 = !DILocation(line: 1, column: 30, scope: !6)
!18 = !DILocalVariable(name: "gain", arg: 3, scope: !6, file: !1, line: 1, type: !11)
!19 = !DILocation(line: 1, column: 41, scope: !6)
!20 = !DILocalVariable(name: "nsamps", arg: 4, scope: !6, file: !1, line: 1, type: !12)
!21 = !DILocation(line: 1, column: 51, scope: !6)
!22 = !DILocalVariable(name: "i", scope: !6, file: !1, line: 3, type: !12)
!23 = !DILocation(line: 3, column: 6, scope: !6)
!24 = !DILocation(line: 5, column: 7, scope: !25)
!25 = distinct !DILexicalBlock(scope: !6, file: !1, line: 5, column: 2)
!26 = !DILocation(line: 11, column: 1, scope: !6)
!27 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 13, type: !28, isLocal: false, isDefinition: true, scopeLine: 13, isOptimized: false, unit: !0, variables: !2)
!28 = !DISubroutineType(types: !29)
!29 = !{!12}
!30 = !DILocation(line: 17, column: 5, scope: !27)
!31 = !DILocation(line: 19, column: 5, scope: !27)
!32 = distinct !{!32, !33, !37}
!33 = !DILocation(line: 6, column: 9, scope: !34)
!34 = distinct !DILexicalBlock(scope: !35, file: !1, line: 6, column: 9)
!35 = distinct !DILexicalBlock(scope: !36, file: !1, line: 5, column: 31)
!36 = distinct !DILexicalBlock(scope: !25, file: !1, line: 5, column: 2)
!37 = !DILocation(line: 9, column: 9, scope: !34)
!38 = distinct !{!38, !39, !40}
!39 = !DILocation(line: 5, column: 2, scope: !25)
!40 = !DILocation(line: 10, column: 2, scope: !25)
