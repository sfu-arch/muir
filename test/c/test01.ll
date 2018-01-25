; ModuleID = 'test01.bc'
source_filename = "test01.c"
target datalayout = "e-m:e-p:32:32-f64:32:64-f80:32-n8:16:32-S128"
target triple = "i386-unknown-linux-gnu"

; Function Attrs: noinline nounwind
define i8* @foo(i32 %a, i32 %b) #0 !dbg !7 {
entry:
  %a.addr = alloca i32, align 4
  %b.addr = alloca i32, align 4
  store i32 %a, i32* %a.addr, align 4
  call void @llvm.dbg.declare(metadata i32* %a.addr, metadata !12, metadata !13), !dbg !14
  store i32 %b, i32* %b.addr, align 4
  call void @llvm.dbg.declare(metadata i32* %b.addr, metadata !15, metadata !13), !dbg !16
  %0 = bitcast i32* %a.addr to i8*, !dbg !17
  %1 = bitcast i32* %b.addr to i8*, !dbg !18
  %call = call i8* @Mat(i8* %0, i8* %1), !dbg !19
  ret i8* %call, !dbg !20
}

; Function Attrs: nounwind readnone
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

declare i8* @Mat(i8*, i8*) #2

; Function Attrs: noinline nounwind
define i32 @main() #0 !dbg !21 {
entry:
  call void @llvm.dbg.value(metadata i32 5, i64 0, metadata !24, metadata !13), !dbg !25
  call void @llvm.dbg.value(metadata i32 3, i64 0, metadata !26, metadata !13), !dbg !27
  %call = call i8* @foo(i32 5, i32 3), !dbg !28
  call void @llvm.dbg.value(metadata i8* %call, i64 0, metadata !29, metadata !13), !dbg !30
  ret i32 0, !dbg !31
}

; Function Attrs: nounwind readnone
declare void @llvm.dbg.value(metadata, i64, metadata, metadata) #1

attributes #0 = { noinline nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="pentium4" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="pentium4" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!llvm.ident = !{!6}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 4.0.1 (tags/RELEASE_401/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2)
!1 = !DIFile(filename: "test01.c", directory: "/home/amiralis/git/xketch-generator/test/c")
!2 = !{}
!3 = !{i32 1, !"NumRegisterParameters", i32 0}
!4 = !{i32 2, !"Dwarf Version", i32 4}
!5 = !{i32 2, !"Debug Info Version", i32 3}
!6 = !{!"clang version 4.0.1 (tags/RELEASE_401/final)"}
!7 = distinct !DISubprogram(name: "foo", scope: !1, file: !1, line: 3, type: !8, isLocal: false, isDefinition: true, scopeLine: 3, flags: DIFlagPrototyped, isOptimized: false, unit: !0, variables: !2)
!8 = !DISubroutineType(types: !9)
!9 = !{!10, !11, !11}
!10 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: null, size: 32)
!11 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!12 = !DILocalVariable(name: "a", arg: 1, scope: !7, file: !1, line: 3, type: !11)
!13 = !DIExpression()
!14 = !DILocation(line: 3, column: 15, scope: !7)
!15 = !DILocalVariable(name: "b", arg: 2, scope: !7, file: !1, line: 3, type: !11)
!16 = !DILocation(line: 3, column: 22, scope: !7)
!17 = !DILocation(line: 4, column: 17, scope: !7)
!18 = !DILocation(line: 4, column: 22, scope: !7)
!19 = !DILocation(line: 4, column: 13, scope: !7)
!20 = !DILocation(line: 4, column: 5, scope: !7)
!21 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 7, type: !22, isLocal: false, isDefinition: true, scopeLine: 7, isOptimized: false, unit: !0, variables: !2)
!22 = !DISubroutineType(types: !23)
!23 = !{!11}
!24 = !DILocalVariable(name: "a", scope: !21, file: !1, line: 8, type: !11)
!25 = !DILocation(line: 8, column: 9, scope: !21)
!26 = !DILocalVariable(name: "b", scope: !21, file: !1, line: 9, type: !11)
!27 = !DILocation(line: 9, column: 9, scope: !21)
!28 = !DILocation(line: 11, column: 17, scope: !21)
!29 = !DILocalVariable(name: "sum", scope: !21, file: !1, line: 11, type: !10)
!30 = !DILocation(line: 11, column: 11, scope: !21)
!31 = !DILocation(line: 13, column: 5, scope: !21)
