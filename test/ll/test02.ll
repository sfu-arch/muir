; ModuleID = 'test02.bc'
source_filename = "test02.c"
target datalayout = "e-m:e-p:32:32-f64:32:64-f80:32-n8:16:32-S128"
target triple = "i386-unknown-linux-gnu"

; Function Attrs: noinline nounwind
define i32 @foo(i32 %a, i32 %b) #0 !dbg !7 {
entry:
  call void @llvm.dbg.value(metadata i32 %a, i64 0, metadata !12, metadata !13), !dbg !14
  call void @llvm.dbg.value(metadata i32 %b, i64 0, metadata !15, metadata !13), !dbg !16
  %div = udiv i32 %a, 2, !dbg !17
  %cmp = icmp eq i32 %div, 4, !dbg !19
  br i1 %cmp, label %if.then, label %if.else, !dbg !20

if.then:                                          ; preds = %entry
  %add = add i32 %a, %b, !dbg !21
  call void @llvm.dbg.value(metadata i32 %add, i64 0, metadata !22, metadata !13), !dbg !23
  br label %if.end, !dbg !24

if.else:                                          ; preds = %entry
  %sub = sub i32 %a, %b, !dbg !25
  call void @llvm.dbg.value(metadata i32 %sub, i64 0, metadata !22, metadata !13), !dbg !23
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %sum.0 = phi i32 [ %add, %if.then ], [ %sub, %if.else ]
  call void @llvm.dbg.value(metadata i32 %sum.0, i64 0, metadata !22, metadata !13), !dbg !23
  ret i32 %sum.0, !dbg !26
}

; Function Attrs: nounwind readnone
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: noinline nounwind
define i32 @main() #0 !dbg !27 {
entry:
  call void @llvm.dbg.value(metadata i32 5, i64 0, metadata !30, metadata !13), !dbg !31
  call void @llvm.dbg.value(metadata i32 3, i64 0, metadata !32, metadata !13), !dbg !33
  %call = call i32 @foo(i32 5, i32 3), !dbg !34
  call void @llvm.dbg.value(metadata i32 %call, i64 0, metadata !35, metadata !13), !dbg !36
  ret i32 0, !dbg !37
}

; Function Attrs: nounwind readnone
declare void @llvm.dbg.value(metadata, i64, metadata, metadata) #1

attributes #0 = { noinline nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="pentium4" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!llvm.ident = !{!6}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 4.0.1 (tags/RELEASE_401/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2)
!1 = !DIFile(filename: "test02.c", directory: "/home/amiralis/git/xketch-generator/test")
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
!17 = !DILocation(line: 3, column: 10, scope: !18)
!18 = distinct !DILexicalBlock(scope: !7, file: !1, line: 3, column: 8)
!19 = !DILocation(line: 3, column: 14, scope: !18)
!20 = !DILocation(line: 3, column: 8, scope: !7)
!21 = !DILocation(line: 4, column: 17, scope: !18)
!22 = !DILocalVariable(name: "sum", scope: !7, file: !1, line: 2, type: !11)
!23 = !DILocation(line: 2, column: 18, scope: !7)
!24 = !DILocation(line: 4, column: 9, scope: !18)
!25 = !DILocation(line: 6, column: 17, scope: !18)
!26 = !DILocation(line: 7, column: 5, scope: !7)
!27 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 10, type: !28, isLocal: false, isDefinition: true, scopeLine: 10, isOptimized: false, unit: !0, variables: !2)
!28 = !DISubroutineType(types: !29)
!29 = !{!10}
!30 = !DILocalVariable(name: "a", scope: !27, file: !1, line: 11, type: !11)
!31 = !DILocation(line: 11, column: 18, scope: !27)
!32 = !DILocalVariable(name: "b", scope: !27, file: !1, line: 12, type: !11)
!33 = !DILocation(line: 12, column: 18, scope: !27)
!34 = !DILocation(line: 13, column: 24, scope: !27)
!35 = !DILocalVariable(name: "sum", scope: !27, file: !1, line: 13, type: !11)
!36 = !DILocation(line: 13, column: 18, scope: !27)
!37 = !DILocation(line: 14, column: 5, scope: !27)
