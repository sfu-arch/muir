; ModuleID = 'alias.bc'
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
  br label %5, !dbg !24

; <label>:5:                                      ; preds = %38, %4
  %.01 = phi i32 [ 0, %4 ], [ %39, %38 ]
  call void @llvm.dbg.value(metadata i32 %.01, i64 0, metadata !22, metadata !14), !dbg !23
  %6 = icmp slt i32 %.01, %3, !dbg !26
  br i1 %6, label %7, label %40, !dbg !29

; <label>:7:                                      ; preds = %5
  call void @llvm.dbg.value(metadata i32 0, i64 0, metadata !31, metadata !14), !dbg !32
  br label %8, !dbg !33

; <label>:8:                                      ; preds = %35, %7
  %.0 = phi i32 [ 0, %7 ], [ %36, %35 ]
  call void @llvm.dbg.value(metadata i32 %.0, i64 0, metadata !31, metadata !14), !dbg !32
  %9 = icmp slt i32 %.0, %3, !dbg !36
  br i1 %9, label %10, label %37, !dbg !39

; <label>:10:                                     ; preds = %8
  %11 = sext i32 %.01 to i64, !dbg !41
  %12 = getelementptr inbounds float*, float** %0, i64 %11, !dbg !41
  %13 = load float*, float** %12, align 8, !dbg !41
  %14 = sext i32 %.0 to i64, !dbg !41
  %15 = getelementptr inbounds float, float* %13, i64 %14, !dbg !41
  %16 = load float, float* %15, align 4, !dbg !41
  %17 = fmul float %16, %2, !dbg !43
  %18 = sext i32 %.01 to i64, !dbg !44
  %19 = getelementptr inbounds float*, float** %1, i64 %18, !dbg !44
  %20 = load float*, float** %19, align 8, !dbg !44
  %21 = sext i32 %.0 to i64, !dbg !44
  %22 = getelementptr inbounds float, float* %20, i64 %21, !dbg !44
  store float %17, float* %22, align 4, !dbg !45
  %23 = sext i32 %.01 to i64, !dbg !46
  %24 = getelementptr inbounds float*, float** %0, i64 %23, !dbg !46
  %25 = load float*, float** %24, align 8, !dbg !46
  %26 = sext i32 %.0 to i64, !dbg !46
  %27 = getelementptr inbounds float, float* %25, i64 %26, !dbg !46
  %28 = load float, float* %27, align 4, !dbg !46
  %29 = fmul float %28, %2, !dbg !47
  %30 = sext i32 %.01 to i64, !dbg !48
  %31 = getelementptr inbounds float*, float** %1, i64 %30, !dbg !48
  %32 = load float*, float** %31, align 8, !dbg !48
  %33 = sext i32 %.0 to i64, !dbg !48
  %34 = getelementptr inbounds float, float* %32, i64 %33, !dbg !48
  store float %29, float* %34, align 4, !dbg !49
  br label %35, !dbg !50

; <label>:35:                                     ; preds = %10
  %36 = add nsw i32 %.0, 1, !dbg !51
  call void @llvm.dbg.value(metadata i32 %36, i64 0, metadata !31, metadata !14), !dbg !32
  br label %8, !dbg !53, !llvm.loop !54

; <label>:37:                                     ; preds = %8
  br label %38, !dbg !57

; <label>:38:                                     ; preds = %37
  %39 = add nsw i32 %.01, 1, !dbg !58
  call void @llvm.dbg.value(metadata i32 %39, i64 0, metadata !22, metadata !14), !dbg !23
  br label %5, !dbg !60, !llvm.loop !61

; <label>:40:                                     ; preds = %5
  ret void, !dbg !64
}

; Function Attrs: nounwind readnone
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: noinline nounwind uwtable
define i32 @main() #0 !dbg !65 {
  call void @foo(float** undef, float** undef, float undef, i32 undef), !dbg !68
  ret i32 0, !dbg !69
}

; Function Attrs: nounwind readnone
declare void @llvm.dbg.value(metadata, i64, metadata, metadata) #1

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
!26 = !DILocation(line: 5, column: 16, scope: !27)
!27 = !DILexicalBlockFile(scope: !28, file: !1, discriminator: 1)
!28 = distinct !DILexicalBlock(scope: !25, file: !1, line: 5, column: 2)
!29 = !DILocation(line: 5, column: 2, scope: !30)
!30 = !DILexicalBlockFile(scope: !25, file: !1, discriminator: 1)
!31 = !DILocalVariable(name: "j", scope: !6, file: !1, line: 3, type: !12)
!32 = !DILocation(line: 3, column: 8, scope: !6)
!33 = !DILocation(line: 6, column: 13, scope: !34)
!34 = distinct !DILexicalBlock(scope: !35, file: !1, line: 6, column: 9)
!35 = distinct !DILexicalBlock(scope: !28, file: !1, line: 5, column: 31)
!36 = !DILocation(line: 6, column: 22, scope: !37)
!37 = !DILexicalBlockFile(scope: !38, file: !1, discriminator: 1)
!38 = distinct !DILexicalBlock(scope: !34, file: !1, line: 6, column: 9)
!39 = !DILocation(line: 6, column: 9, scope: !40)
!40 = !DILexicalBlockFile(scope: !34, file: !1, discriminator: 1)
!41 = !DILocation(line: 7, column: 19, scope: !42)
!42 = distinct !DILexicalBlock(scope: !38, file: !1, line: 6, column: 36)
!43 = !DILocation(line: 7, column: 28, scope: !42)
!44 = !DILocation(line: 7, column: 7, scope: !42)
!45 = !DILocation(line: 7, column: 17, scope: !42)
!46 = !DILocation(line: 8, column: 19, scope: !42)
!47 = !DILocation(line: 8, column: 28, scope: !42)
!48 = !DILocation(line: 8, column: 7, scope: !42)
!49 = !DILocation(line: 8, column: 17, scope: !42)
!50 = !DILocation(line: 9, column: 9, scope: !42)
!51 = !DILocation(line: 6, column: 33, scope: !52)
!52 = !DILexicalBlockFile(scope: !38, file: !1, discriminator: 2)
!53 = !DILocation(line: 6, column: 9, scope: !52)
!54 = distinct !{!54, !55, !56}
!55 = !DILocation(line: 6, column: 9, scope: !34)
!56 = !DILocation(line: 9, column: 9, scope: !34)
!57 = !DILocation(line: 10, column: 2, scope: !35)
!58 = !DILocation(line: 5, column: 27, scope: !59)
!59 = !DILexicalBlockFile(scope: !28, file: !1, discriminator: 2)
!60 = !DILocation(line: 5, column: 2, scope: !59)
!61 = distinct !{!61, !62, !63}
!62 = !DILocation(line: 5, column: 2, scope: !25)
!63 = !DILocation(line: 10, column: 2, scope: !25)
!64 = !DILocation(line: 11, column: 1, scope: !6)
!65 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 13, type: !66, isLocal: false, isDefinition: true, scopeLine: 13, isOptimized: false, unit: !0, variables: !2)
!66 = !DISubroutineType(types: !67)
!67 = !{!12}
!68 = !DILocation(line: 17, column: 5, scope: !65)
!69 = !DILocation(line: 19, column: 5, scope: !65)
