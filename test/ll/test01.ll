; ModuleID = 'test01.bc'
source_filename = "test01.c"
target datalayout = "e-m:e-p:32:32-f64:32:64-f80:32-n8:16:32-S128"
target triple = "i386-unknown-linux-gnu"

; Function Attrs: noinline nounwind
define i32 @foo(i32 %a, i32 %b) #0 !dbg !7 {
entry:
  call void @llvm.dbg.value(metadata i32 %a, i64 0, metadata !11, metadata !12), !dbg !13
  call void @llvm.dbg.value(metadata i32 %b, i64 0, metadata !14, metadata !12), !dbg !15
  %add = add nsw i32 %a, %b, !dbg !16
  call void @llvm.dbg.value(metadata i32 %add, i64 0, metadata !17, metadata !12), !dbg !18
  ret i32 %add, !dbg !19
}

; Function Attrs: nounwind readnone
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: noinline nounwind
define i32 @main() #0 !dbg !20 {
entry:
  call void @llvm.dbg.value(metadata i32 5, i64 0, metadata !23, metadata !12), !dbg !24
  call void @llvm.dbg.value(metadata i32 3, i64 0, metadata !25, metadata !12), !dbg !26
  %call = call i32 @foo(i32 5, i32 3), !dbg !27
  call void @llvm.dbg.value(metadata i32 %call, i64 0, metadata !28, metadata !12), !dbg !29
  ret i32 0, !dbg !30
}

; Function Attrs: nounwind readnone
declare void @llvm.dbg.value(metadata, i64, metadata, metadata) #1

attributes #0 = { noinline nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="pentium4" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!llvm.ident = !{!6}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 4.0.1 (tags/RELEASE_401/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2)
!1 = !DIFile(filename: "test01.c", directory: "/home/amiralis/git/xketch-generator/test")
!2 = !{}
!3 = !{i32 1, !"NumRegisterParameters", i32 0}
!4 = !{i32 2, !"Dwarf Version", i32 4}
!5 = !{i32 2, !"Debug Info Version", i32 3}
!6 = !{!"clang version 4.0.1 (tags/RELEASE_401/final)"}
!7 = distinct !DISubprogram(name: "foo", scope: !1, file: !1, line: 1, type: !8, isLocal: false, isDefinition: true, scopeLine: 1, flags: DIFlagPrototyped, isOptimized: false, unit: !0, variables: !2)
!8 = !DISubroutineType(types: !9)
!9 = !{!10, !10, !10}
!10 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!11 = !DILocalVariable(name: "a", arg: 1, scope: !7, file: !1, line: 1, type: !10)
!12 = !DIExpression()
!13 = !DILocation(line: 1, column: 13, scope: !7)
!14 = !DILocalVariable(name: "b", arg: 2, scope: !7, file: !1, line: 1, type: !10)
!15 = !DILocation(line: 1, column: 20, scope: !7)
!16 = !DILocation(line: 3, column: 20, scope: !7)
!17 = !DILocalVariable(name: "sum", scope: !7, file: !1, line: 2, type: !10)
!18 = !DILocation(line: 2, column: 9, scope: !7)
!19 = !DILocation(line: 3, column: 5, scope: !7)
!20 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 6, type: !21, isLocal: false, isDefinition: true, scopeLine: 6, isOptimized: false, unit: !0, variables: !2)
!21 = !DISubroutineType(types: !22)
!22 = !{!10}
!23 = !DILocalVariable(name: "a", scope: !20, file: !1, line: 7, type: !10)
!24 = !DILocation(line: 7, column: 9, scope: !20)
!25 = !DILocalVariable(name: "b", scope: !20, file: !1, line: 8, type: !10)
!26 = !DILocation(line: 8, column: 9, scope: !20)
!27 = !DILocation(line: 9, column: 15, scope: !20)
!28 = !DILocalVariable(name: "sum", scope: !20, file: !1, line: 9, type: !10)
!29 = !DILocation(line: 9, column: 9, scope: !20)
!30 = !DILocation(line: 10, column: 5, scope: !20)
