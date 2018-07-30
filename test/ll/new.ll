; ModuleID = 'new.bc'
source_filename = "test03.c"
target datalayout = "e-m:e-p:32:32-f64:32:64-f80:32-n8:16:32-S128"
target triple = "i386-unknown-linux-gnu"

; Function Attrs: noinline nounwind
define i32 @foo(i32 %a, i32 %b, i32 %n) #0 {
entry:
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %sum.0 = phi i32 [ 0, %entry ], [ %mul, %for.inc ]
  %i.0 = phi i32 [ 0, %entry ], [ %inc, %for.inc ]
  %cmp = icmp ult i32 %i.0, %n
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %add = add i32 %sum.0, %a
  %mul = mul i32 %add, %b
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %inc = add i32 %i.0, 1
  br label %for.cond, !llvm.loop !4

for.end:                                          ; preds = %for.cond
  ret i32 %sum.0
}

; Function Attrs: nounwind readnone
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: noinline nounwind
define i32 @main() #0 {
entry:
  %call = call i32 @foo(i32 5, i32 3, i32 2)
  ret i32 0
}

; Function Attrs: nounwind readnone
declare void @llvm.dbg.value(metadata, i64, metadata, metadata) #1

attributes #0 = { noinline nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="pentium4" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }

!llvm.module.flags = !{!0, !1, !2}
!llvm.ident = !{!3}

!0 = !{i32 1, !"NumRegisterParameters", i32 0}
!1 = !{i32 2, !"Dwarf Version", i32 4}
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{!"clang version 4.0.1 (tags/RELEASE_401/final)"}
!4 = distinct !{!4, !5, !15}
!5 = !DILocation(line: 5, column: 5, scope: !6)
!6 = distinct !DILexicalBlock(scope: !8, file: !7, line: 5, column: 5)
!7 = !DIFile(filename: "test03.c", directory: "/home/amiralis/git/xketch-generator/test/c")
!8 = distinct !DISubprogram(name: "foo", scope: !7, file: !7, line: 1, type: !9, isLocal: false, isDefinition: true, scopeLine: 1, flags: DIFlagPrototyped, isOptimized: false, unit: !13, variables: !14)
!9 = !DISubroutineType(types: !10)
!10 = !{!11, !12, !12, !12}
!11 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!12 = !DIBasicType(name: "unsigned int", size: 32, encoding: DW_ATE_unsigned)
!13 = distinct !DICompileUnit(language: DW_LANG_C99, file: !7, producer: "clang version 4.0.1 (tags/RELEASE_401/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !14)
!14 = !{}
!15 = !DILocation(line: 6, column: 23, scope: !6)
