; ModuleID = 'alias.bc'
source_filename = "alias.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind uwtable
define void @bar(i32* %a) #0 !dbg !6 {
entry:
  call void @llvm.dbg.value(metadata i32* %a, i64 0, metadata !11, metadata !12), !dbg !13
  %0 = load i32, i32* %a, align 4, !dbg !14
  %add = add nsw i32 %0, 1, !dbg !14
  store i32 %add, i32* %a, align 4, !dbg !14
  ret void, !dbg !15
}

; Function Attrs: nounwind readnone
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: noinline nounwind uwtable
define void @foo(float** %in, float** %out, float %gain, i32 %nsamps) #0 !dbg !16 {
entry:
  %i = alloca i32, align 4
  call void @llvm.dbg.value(metadata float** %in, i64 0, metadata !22, metadata !12), !dbg !23
  call void @llvm.dbg.value(metadata float** %out, i64 0, metadata !24, metadata !12), !dbg !25
  call void @llvm.dbg.value(metadata float %gain, i64 0, metadata !26, metadata !12), !dbg !27
  call void @llvm.dbg.value(metadata i32 %nsamps, i64 0, metadata !28, metadata !12), !dbg !29
  call void @llvm.dbg.declare(metadata i32* %i, metadata !30, metadata !12), !dbg !31
  store i32 0, i32* %i, align 4, !dbg !32
  br label %for.cond, !dbg !34

for.cond:                                         ; preds = %for.inc19, %entry
  %0 = load i32, i32* %i, align 4, !dbg !35
  %cmp = icmp slt i32 %0, %nsamps, !dbg !38
  br i1 %cmp, label %for.body, label %for.end21, !dbg !39

for.body:                                         ; preds = %for.cond
  call void @llvm.dbg.value(metadata i32 0, i64 0, metadata !41, metadata !12), !dbg !42
  br label %for.cond1, !dbg !43

for.cond1:                                        ; preds = %for.inc, %for.body
  %j.0 = phi i32 [ 0, %for.body ], [ %inc, %for.inc ]
  call void @llvm.dbg.value(metadata i32 %j.0, i64 0, metadata !41, metadata !12), !dbg !42
  %cmp2 = icmp slt i32 %j.0, %nsamps, !dbg !46
  br i1 %cmp2, label %for.body3, label %for.end, !dbg !49

for.body3:                                        ; preds = %for.cond1
  %1 = load i32, i32* %i, align 4, !dbg !51
  %idxprom = sext i32 %1 to i64, !dbg !53
  %arrayidx = getelementptr inbounds float*, float** %in, i64 %idxprom, !dbg !53
  %2 = load float*, float** %arrayidx, align 8, !dbg !53
  %idxprom4 = sext i32 %j.0 to i64, !dbg !53
  %arrayidx5 = getelementptr inbounds float, float* %2, i64 %idxprom4, !dbg !53
  %3 = load float, float* %arrayidx5, align 4, !dbg !53
  %mul = fmul float %3, %gain, !dbg !54
  %4 = load i32, i32* %i, align 4, !dbg !55
  %idxprom6 = sext i32 %4 to i64, !dbg !56
  %arrayidx7 = getelementptr inbounds float*, float** %out, i64 %idxprom6, !dbg !56
  %5 = load float*, float** %arrayidx7, align 8, !dbg !56
  %idxprom8 = sext i32 %j.0 to i64, !dbg !56
  %arrayidx9 = getelementptr inbounds float, float* %5, i64 %idxprom8, !dbg !56
  store float %mul, float* %arrayidx9, align 4, !dbg !57
  %6 = load i32, i32* %i, align 4, !dbg !58
  %idxprom10 = sext i32 %6 to i64, !dbg !59
  %arrayidx11 = getelementptr inbounds float*, float** %in, i64 %idxprom10, !dbg !59
  %7 = load float*, float** %arrayidx11, align 8, !dbg !59
  %idxprom12 = sext i32 %j.0 to i64, !dbg !59
  %arrayidx13 = getelementptr inbounds float, float* %7, i64 %idxprom12, !dbg !59
  %8 = load float, float* %arrayidx13, align 4, !dbg !59
  %mul14 = fmul float %8, %gain, !dbg !60
  %9 = load i32, i32* %i, align 4, !dbg !61
  %idxprom15 = sext i32 %9 to i64, !dbg !62
  %arrayidx16 = getelementptr inbounds float*, float** %out, i64 %idxprom15, !dbg !62
  %10 = load float*, float** %arrayidx16, align 8, !dbg !62
  %idxprom17 = sext i32 %j.0 to i64, !dbg !62
  %arrayidx18 = getelementptr inbounds float, float* %10, i64 %idxprom17, !dbg !62
  store float %mul14, float* %arrayidx18, align 4, !dbg !63
  br label %for.inc, !dbg !64

for.inc:                                          ; preds = %for.body3
  %inc = add nsw i32 %j.0, 1, !dbg !65
  call void @llvm.dbg.value(metadata i32 %inc, i64 0, metadata !41, metadata !12), !dbg !42
  br label %for.cond1, !dbg !67, !llvm.loop !68

for.end:                                          ; preds = %for.cond1
  br label %for.inc19, !dbg !71

for.inc19:                                        ; preds = %for.end
  %11 = load i32, i32* %i, align 4, !dbg !72
  %inc20 = add nsw i32 %11, 1, !dbg !72
  store i32 %inc20, i32* %i, align 4, !dbg !72
  br label %for.cond, !dbg !74, !llvm.loop !75

for.end21:                                        ; preds = %for.cond
  call void @bar(i32* %i), !dbg !78
  ret void, !dbg !79
}

; Function Attrs: noinline nounwind uwtable
define i32 @main() #0 !dbg !80 {
entry:
  call void @foo(float** undef, float** undef, float undef, i32 undef), !dbg !83
  ret i32 0, !dbg !84
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
!6 = distinct !DISubprogram(name: "bar", scope: !1, file: !1, line: 1, type: !7, isLocal: false, isDefinition: true, scopeLine: 1, flags: DIFlagPrototyped, isOptimized: false, unit: !0, variables: !2)
!7 = !DISubroutineType(types: !8)
!8 = !{null, !9}
!9 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !10, size: 64)
!10 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!11 = !DILocalVariable(name: "a", arg: 1, scope: !6, file: !1, line: 1, type: !9)
!12 = !DIExpression()
!13 = !DILocation(line: 1, column: 15, scope: !6)
!14 = !DILocation(line: 2, column: 7, scope: !6)
!15 = !DILocation(line: 3, column: 1, scope: !6)
!16 = distinct !DISubprogram(name: "foo", scope: !1, file: !1, line: 5, type: !17, isLocal: false, isDefinition: true, scopeLine: 6, flags: DIFlagPrototyped, isOptimized: false, unit: !0, variables: !2)
!17 = !DISubroutineType(types: !18)
!18 = !{null, !19, !19, !21, !10}
!19 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !20, size: 64)
!20 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !21, size: 64)
!21 = !DIBasicType(name: "float", size: 32, encoding: DW_ATE_float)
!22 = !DILocalVariable(name: "in", arg: 1, scope: !16, file: !1, line: 5, type: !19)
!23 = !DILocation(line: 5, column: 18, scope: !16)
!24 = !DILocalVariable(name: "out", arg: 2, scope: !16, file: !1, line: 5, type: !19)
!25 = !DILocation(line: 5, column: 30, scope: !16)
!26 = !DILocalVariable(name: "gain", arg: 3, scope: !16, file: !1, line: 5, type: !21)
!27 = !DILocation(line: 5, column: 41, scope: !16)
!28 = !DILocalVariable(name: "nsamps", arg: 4, scope: !16, file: !1, line: 5, type: !10)
!29 = !DILocation(line: 5, column: 51, scope: !16)
!30 = !DILocalVariable(name: "i", scope: !16, file: !1, line: 7, type: !10)
!31 = !DILocation(line: 7, column: 6, scope: !16)
!32 = !DILocation(line: 9, column: 9, scope: !33)
!33 = distinct !DILexicalBlock(scope: !16, file: !1, line: 9, column: 2)
!34 = !DILocation(line: 9, column: 7, scope: !33)
!35 = !DILocation(line: 9, column: 14, scope: !36)
!36 = !DILexicalBlockFile(scope: !37, file: !1, discriminator: 1)
!37 = distinct !DILexicalBlock(scope: !33, file: !1, line: 9, column: 2)
!38 = !DILocation(line: 9, column: 16, scope: !36)
!39 = !DILocation(line: 9, column: 2, scope: !40)
!40 = !DILexicalBlockFile(scope: !33, file: !1, discriminator: 1)
!41 = !DILocalVariable(name: "j", scope: !16, file: !1, line: 7, type: !10)
!42 = !DILocation(line: 7, column: 8, scope: !16)
!43 = !DILocation(line: 10, column: 13, scope: !44)
!44 = distinct !DILexicalBlock(scope: !45, file: !1, line: 10, column: 9)
!45 = distinct !DILexicalBlock(scope: !37, file: !1, line: 9, column: 31)
!46 = !DILocation(line: 10, column: 22, scope: !47)
!47 = !DILexicalBlockFile(scope: !48, file: !1, discriminator: 1)
!48 = distinct !DILexicalBlock(scope: !44, file: !1, line: 10, column: 9)
!49 = !DILocation(line: 10, column: 9, scope: !50)
!50 = !DILexicalBlockFile(scope: !44, file: !1, discriminator: 1)
!51 = !DILocation(line: 11, column: 22, scope: !52)
!52 = distinct !DILexicalBlock(scope: !48, file: !1, line: 10, column: 36)
!53 = !DILocation(line: 11, column: 19, scope: !52)
!54 = !DILocation(line: 11, column: 28, scope: !52)
!55 = !DILocation(line: 11, column: 11, scope: !52)
!56 = !DILocation(line: 11, column: 7, scope: !52)
!57 = !DILocation(line: 11, column: 17, scope: !52)
!58 = !DILocation(line: 12, column: 22, scope: !52)
!59 = !DILocation(line: 12, column: 19, scope: !52)
!60 = !DILocation(line: 12, column: 28, scope: !52)
!61 = !DILocation(line: 12, column: 11, scope: !52)
!62 = !DILocation(line: 12, column: 7, scope: !52)
!63 = !DILocation(line: 12, column: 17, scope: !52)
!64 = !DILocation(line: 13, column: 9, scope: !52)
!65 = !DILocation(line: 10, column: 33, scope: !66)
!66 = !DILexicalBlockFile(scope: !48, file: !1, discriminator: 2)
!67 = !DILocation(line: 10, column: 9, scope: !66)
!68 = distinct !{!68, !69, !70}
!69 = !DILocation(line: 10, column: 9, scope: !44)
!70 = !DILocation(line: 13, column: 9, scope: !44)
!71 = !DILocation(line: 14, column: 2, scope: !45)
!72 = !DILocation(line: 9, column: 27, scope: !73)
!73 = !DILexicalBlockFile(scope: !37, file: !1, discriminator: 2)
!74 = !DILocation(line: 9, column: 2, scope: !73)
!75 = distinct !{!75, !76, !77}
!76 = !DILocation(line: 9, column: 2, scope: !33)
!77 = !DILocation(line: 14, column: 2, scope: !33)
!78 = !DILocation(line: 16, column: 5, scope: !16)
!79 = !DILocation(line: 17, column: 1, scope: !16)
!80 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 19, type: !81, isLocal: false, isDefinition: true, scopeLine: 19, isOptimized: false, unit: !0, variables: !2)
!81 = !DISubroutineType(types: !82)
!82 = !{!10}
!83 = !DILocation(line: 23, column: 5, scope: !80)
!84 = !DILocation(line: 25, column: 5, scope: !80)
