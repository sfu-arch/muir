; ModuleID = 'test06.bc'
source_filename = "test06-load.c"
target datalayout = "e-m:e-p:32:32-f64:32:64-f80:32-n8:16:32-S128"
target triple = "i386-unknown-linux-gnu"

; Function Attrs: noinline nounwind
define i32 @foo(i32 %a, i32 %b) #0 !dbg !7 {
entry:
  %sum = alloca [2 x i32], align 4
  call void @llvm.dbg.value(metadata i32 %a, i64 0, metadata !12, metadata !13), !dbg !14
  call void @llvm.dbg.value(metadata i32 %b, i64 0, metadata !15, metadata !13), !dbg !16
  call void @llvm.dbg.declare(metadata [2 x i32]* %sum, metadata !17, metadata !13), !dbg !21
  %arrayidx = getelementptr inbounds [2 x i32], [2 x i32]* %sum, i32 0, i32 0, !dbg !22
  store i32 %a, i32* %arrayidx, align 4, !dbg !23
  %arrayidx1 = getelementptr inbounds [2 x i32], [2 x i32]* %sum, i32 0, i32 1, !dbg !24
  store i32 %b, i32* %arrayidx1, align 4, !dbg !25
  %arrayidx2 = getelementptr inbounds [2 x i32], [2 x i32]* %sum, i32 0, i32 0, !dbg !26
  %0 = load i32, i32* %arrayidx2, align 4, !dbg !26
  %arrayidx3 = getelementptr inbounds [2 x i32], [2 x i32]* %sum, i32 0, i32 1, !dbg !27
  %1 = load i32, i32* %arrayidx3, align 4, !dbg !27
  %add = add nsw i32 %0, %1, !dbg !28
  ret i32 %add, !dbg !29
}

; Function Attrs: nounwind readnone
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: noinline nounwind
define i32 @main() #0 !dbg !30 {
entry:
  call void @llvm.dbg.value(metadata i32 5, i64 0, metadata !33, metadata !13), !dbg !34
  call void @llvm.dbg.value(metadata i32 3, i64 0, metadata !35, metadata !13), !dbg !36
  %call = call i32 @foo(i32 5, i32 3), !dbg !37
  call void @llvm.dbg.value(metadata i32 %call, i64 0, metadata !38, metadata !13), !dbg !39
  ret i32 0, !dbg !40
}

; Function Attrs: nounwind readnone
declare void @llvm.dbg.value(metadata, i64, metadata, metadata) #1

attributes #0 = { noinline nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="pentium4" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!llvm.ident = !{!6}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 4.0.1 (tags/RELEASE_401/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2)
!1 = !DIFile(filename: "test06-load.c", directory: "/home/amiralis/git/xketch-generator/test/c")
!2 = !{}
!3 = !{i32 1, !"NumRegisterParameters", i32 0}
!4 = !{i32 2, !"Dwarf Version", i32 4}
!5 = !{i32 2, !"Debug Info Version", i32 3}
!6 = !{!"clang version 4.0.1 (tags/RELEASE_401/final)"}
!7 = distinct !DISubprogram(name: "foo", scope: !1, file: !1, line: 1, type: !8, isLocal: false, isDefinition: true, scopeLine: 1, flags: DIFlagPrototyped, isOptimized: false, unit: !0, variables: !2)
!8 = !DISubroutineType(types: !9)
!9 = !{!10, !11, !11}
!10 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!11 = !DIBasicType(name: "unsigned int", size: 32, encoding: DW_ATE_unsigned)
!12 = !DILocalVariable(name: "a", arg: 1, scope: !7, file: !1, line: 1, type: !11)
!13 = !DIExpression()
!14 = !DILocation(line: 1, column: 22, scope: !7)
!15 = !DILocalVariable(name: "b", arg: 2, scope: !7, file: !1, line: 1, type: !11)
!16 = !DILocation(line: 1, column: 38, scope: !7)
!17 = !DILocalVariable(name: "sum", scope: !7, file: !1, line: 2, type: !18)
!18 = !DICompositeType(tag: DW_TAG_array_type, baseType: !10, size: 64, elements: !19)
!19 = !{!20}
!20 = !DISubrange(count: 2)
!21 = !DILocation(line: 2, column: 9, scope: !7)
!22 = !DILocation(line: 3, column: 5, scope: !7)
!23 = !DILocation(line: 3, column: 12, scope: !7)
!24 = !DILocation(line: 4, column: 5, scope: !7)
!25 = !DILocation(line: 4, column: 12, scope: !7)
!26 = !DILocation(line: 5, column: 12, scope: !7)
!27 = !DILocation(line: 5, column: 21, scope: !7)
!28 = !DILocation(line: 5, column: 19, scope: !7)
!29 = !DILocation(line: 5, column: 5, scope: !7)
!30 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 8, type: !31, isLocal: false, isDefinition: true, scopeLine: 8, isOptimized: false, unit: !0, variables: !2)
!31 = !DISubroutineType(types: !32)
!32 = !{!10}
!33 = !DILocalVariable(name: "a", scope: !30, file: !1, line: 9, type: !11)
!34 = !DILocation(line: 9, column: 14, scope: !30)
!35 = !DILocalVariable(name: "b", scope: !30, file: !1, line: 10, type: !11)
!36 = !DILocation(line: 10, column: 14, scope: !30)
!37 = !DILocation(line: 11, column: 20, scope: !30)
!38 = !DILocalVariable(name: "sum", scope: !30, file: !1, line: 11, type: !11)
!39 = !DILocation(line: 11, column: 14, scope: !30)
!40 = !DILocation(line: 12, column: 5, scope: !30)
