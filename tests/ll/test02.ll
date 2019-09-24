; ModuleID = 'test02.bc'
source_filename = "test02.c"
target datalayout = "e-m:e-p:32:32-f64:32:64-f80:32-n8:16:32-S128"
target triple = "i386-unknown-linux-gnu"

; Function Attrs: noinline nounwind
define i32 @foo(i32 %a, i32 %b) #0 !dbg !7 {
entry:
  call void @llvm.dbg.value(metadata i32 %a, i64 0, metadata !12, metadata !13), !dbg !14
  call void @llvm.dbg.value(metadata i32 %b, i64 0, metadata !15, metadata !13), !dbg !16
  call void @llvm.dbg.value(metadata i32 0, i64 0, metadata !17, metadata !13), !dbg !18
  %div = udiv i32 %a, 2, !dbg !19
  %cmp = icmp eq i32 %div, 4, !dbg !21
  br i1 %cmp, label %if.then, label %if.end, !dbg !22

if.then:                                          ; preds = %entry
  %add = add i32 %a, %b, !dbg !23
  call void @llvm.dbg.value(metadata i32 %add, i64 0, metadata !17, metadata !13), !dbg !18
  br label %if.end, !dbg !24

if.end:                                           ; preds = %if.then, %entry
  %sum.0 = phi i32 [ %add, %if.then ], [ 0, %entry ]
  call void @llvm.dbg.value(metadata i32 %sum.0, i64 0, metadata !17, metadata !13), !dbg !18
  ret i32 %sum.0, !dbg !25
}

; Function Attrs: nounwind readnone
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: noinline nounwind
define i32 @main() #0 !dbg !26 {
entry:
  call void @llvm.dbg.value(metadata i32 5, i64 0, metadata !29, metadata !13), !dbg !30
  call void @llvm.dbg.value(metadata i32 3, i64 0, metadata !31, metadata !13), !dbg !32
  %call = call i32 @foo(i32 5, i32 3), !dbg !33
  call void @llvm.dbg.value(metadata i32 %call, i64 0, metadata !34, metadata !13), !dbg !35
  ret i32 0, !dbg !36
}

; Function Attrs: nounwind readnone
declare void @llvm.dbg.value(metadata, i64, metadata, metadata) #1

attributes #0 = { noinline nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="pentium4" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!llvm.ident = !{!6}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 4.0.1 (tags/RELEASE_401/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2)
!1 = !DIFile(filename: "test02.c", directory: "/home/amiralis/git/xketch-generator/test/c")
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
!17 = !DILocalVariable(name: "sum", scope: !7, file: !1, line: 2, type: !11)
!18 = !DILocation(line: 2, column: 18, scope: !7)
!19 = !DILocation(line: 3, column: 10, scope: !20)
!20 = distinct !DILexicalBlock(scope: !7, file: !1, line: 3, column: 8)
!21 = !DILocation(line: 3, column: 14, scope: !20)
!22 = !DILocation(line: 3, column: 8, scope: !7)
!23 = !DILocation(line: 4, column: 17, scope: !20)
!24 = !DILocation(line: 4, column: 9, scope: !20)
!25 = !DILocation(line: 5, column: 5, scope: !7)
!26 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 8, type: !27, isLocal: false, isDefinition: true, scopeLine: 8, isOptimized: false, unit: !0, variables: !2)
!27 = !DISubroutineType(types: !28)
!28 = !{!10}
!29 = !DILocalVariable(name: "a", scope: !26, file: !1, line: 9, type: !11)
!30 = !DILocation(line: 9, column: 18, scope: !26)
!31 = !DILocalVariable(name: "b", scope: !26, file: !1, line: 10, type: !11)
!32 = !DILocation(line: 10, column: 18, scope: !26)
!33 = !DILocation(line: 11, column: 24, scope: !26)
!34 = !DILocalVariable(name: "sum", scope: !26, file: !1, line: 11, type: !11)
!35 = !DILocation(line: 11, column: 18, scope: !26)
!36 = !DILocation(line: 12, column: 5, scope: !26)
