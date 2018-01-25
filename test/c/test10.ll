; ModuleID = 'test10.bc'
source_filename = "test10.c"
target datalayout = "e-m:e-p:32:32-f64:32:64-f80:32-n8:16:32-S128"
target triple = "i386-unknown-linux-gnu"

@main.a = private unnamed_addr constant [5 x i32] [i32 1, i32 2, i32 3, i32 4, i32 5], align 4
@main.b = private unnamed_addr constant [5 x i32] [i32 1, i32 2, i32 3, i32 4, i32 5], align 4
@.str = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1

; Function Attrs: noinline nounwind
define i32 @test10(i32* %a, i32* %b, i32* %c) #0 !dbg !7 {
entry:
  call void @llvm.dbg.value(metadata i32* %a, i64 0, metadata !12, metadata !13), !dbg !14
  call void @llvm.dbg.value(metadata i32* %b, i64 0, metadata !15, metadata !13), !dbg !16
  call void @llvm.dbg.value(metadata i32* %c, i64 0, metadata !17, metadata !13), !dbg !18
  call void @llvm.dbg.value(metadata i32 0, i64 0, metadata !19, metadata !13), !dbg !21
  br label %for.cond, !dbg !22

for.cond:                                         ; preds = %for.inc, %entry
  %i.0 = phi i32 [ 0, %entry ], [ %inc, %for.inc ]
  call void @llvm.dbg.value(metadata i32 %i.0, i64 0, metadata !19, metadata !13), !dbg !21
  %cmp = icmp ult i32 %i.0, 5, !dbg !23
  br i1 %cmp, label %for.body, label %for.end, !dbg !26

for.body:                                         ; preds = %for.cond
  %arrayidx = getelementptr inbounds i32, i32* %a, i32 %i.0, !dbg !28
  %0 = load i32, i32* %arrayidx, align 4, !dbg !28
  %arrayidx1 = getelementptr inbounds i32, i32* %b, i32 %i.0, !dbg !30
  %1 = load i32, i32* %arrayidx1, align 4, !dbg !30
  %add = add i32 %0, %1, !dbg !31
  %arrayidx2 = getelementptr inbounds i32, i32* %c, i32 %i.0, !dbg !32
  store i32 %add, i32* %arrayidx2, align 4, !dbg !33
  br label %for.inc, !dbg !34

for.inc:                                          ; preds = %for.body
  %inc = add i32 %i.0, 1, !dbg !35
  call void @llvm.dbg.value(metadata i32 %inc, i64 0, metadata !19, metadata !13), !dbg !21
  br label %for.cond, !dbg !37, !llvm.loop !38

for.end:                                          ; preds = %for.cond
  ret i32 1, !dbg !41
}

; Function Attrs: nounwind readnone
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: noinline nounwind
define i32 @main() #0 !dbg !42 {
entry:
  %a = alloca [5 x i32], align 4
  %b = alloca [5 x i32], align 4
  %c = alloca [5 x i32], align 4
  call void @llvm.dbg.declare(metadata [5 x i32]* %a, metadata !46, metadata !13), !dbg !50
  %0 = bitcast [5 x i32]* %a to i8*, !dbg !50
  call void @llvm.memcpy.p0i8.p0i8.i32(i8* %0, i8* bitcast ([5 x i32]* @main.a to i8*), i32 20, i32 4, i1 false), !dbg !50
  call void @llvm.dbg.declare(metadata [5 x i32]* %b, metadata !51, metadata !13), !dbg !52
  %1 = bitcast [5 x i32]* %b to i8*, !dbg !52
  call void @llvm.memcpy.p0i8.p0i8.i32(i8* %1, i8* bitcast ([5 x i32]* @main.b to i8*), i32 20, i32 4, i1 false), !dbg !52
  call void @llvm.dbg.declare(metadata [5 x i32]* %c, metadata !53, metadata !13), !dbg !54
  %2 = bitcast [5 x i32]* %c to i8*, !dbg !54
  call void @llvm.memset.p0i8.i32(i8* %2, i8 0, i32 20, i32 4, i1 false), !dbg !54
  %arraydecay = getelementptr inbounds [5 x i32], [5 x i32]* %a, i32 0, i32 0, !dbg !55
  %arraydecay1 = getelementptr inbounds [5 x i32], [5 x i32]* %b, i32 0, i32 0, !dbg !56
  %arraydecay2 = getelementptr inbounds [5 x i32], [5 x i32]* %c, i32 0, i32 0, !dbg !57
  %call = call i32 @test10(i32* %arraydecay, i32* %arraydecay1, i32* %arraydecay2), !dbg !58
  call void @llvm.dbg.value(metadata i32 0, i64 0, metadata !59, metadata !13), !dbg !60
  br label %for.cond, !dbg !61

for.cond:                                         ; preds = %for.inc, %entry
  %i.0 = phi i32 [ 0, %entry ], [ %inc, %for.inc ]
  call void @llvm.dbg.value(metadata i32 %i.0, i64 0, metadata !59, metadata !13), !dbg !60
  %cmp = icmp slt i32 %i.0, 5, !dbg !63
  br i1 %cmp, label %for.body, label %for.end, !dbg !66

for.body:                                         ; preds = %for.cond
  %arrayidx = getelementptr inbounds [5 x i32], [5 x i32]* %c, i32 0, i32 %i.0, !dbg !68
  %3 = load i32, i32* %arrayidx, align 4, !dbg !68
  %call3 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i32 0, i32 0), i32 %3), !dbg !70
  br label %for.inc, !dbg !71

for.inc:                                          ; preds = %for.body
  %inc = add nsw i32 %i.0, 1, !dbg !72
  call void @llvm.dbg.value(metadata i32 %inc, i64 0, metadata !59, metadata !13), !dbg !60
  br label %for.cond, !dbg !74, !llvm.loop !75

for.end:                                          ; preds = %for.cond
  ret i32 0, !dbg !78
}

; Function Attrs: argmemonly nounwind
declare void @llvm.memcpy.p0i8.p0i8.i32(i8* nocapture writeonly, i8* nocapture readonly, i32, i32, i1) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.memset.p0i8.i32(i8* nocapture writeonly, i8, i32, i32, i1) #2

declare i32 @printf(i8*, ...) #3

; Function Attrs: nounwind readnone
declare void @llvm.dbg.value(metadata, i64, metadata, metadata) #1

attributes #0 = { noinline nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="pentium4" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }
attributes #2 = { argmemonly nounwind }
attributes #3 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="pentium4" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!llvm.ident = !{!6}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 4.0.1 (tags/RELEASE_401/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2)
!1 = !DIFile(filename: "test10.c", directory: "/home/amiralis/git/xketch-generator/test/c")
!2 = !{}
!3 = !{i32 1, !"NumRegisterParameters", i32 0}
!4 = !{i32 2, !"Dwarf Version", i32 4}
!5 = !{i32 2, !"Debug Info Version", i32 3}
!6 = !{!"clang version 4.0.1 (tags/RELEASE_401/final)"}
!7 = distinct !DISubprogram(name: "test10", scope: !1, file: !1, line: 3, type: !8, isLocal: false, isDefinition: true, scopeLine: 3, flags: DIFlagPrototyped, isOptimized: false, unit: !0, variables: !2)
!8 = !DISubroutineType(types: !9)
!9 = !{!10, !11, !11, !11}
!10 = !DIBasicType(name: "unsigned int", size: 32, encoding: DW_ATE_unsigned)
!11 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !10, size: 32)
!12 = !DILocalVariable(name: "a", arg: 1, scope: !7, file: !1, line: 3, type: !11)
!13 = !DIExpression()
!14 = !DILocation(line: 3, column: 27, scope: !7)
!15 = !DILocalVariable(name: "b", arg: 2, scope: !7, file: !1, line: 3, type: !11)
!16 = !DILocation(line: 3, column: 40, scope: !7)
!17 = !DILocalVariable(name: "c", arg: 3, scope: !7, file: !1, line: 3, type: !11)
!18 = !DILocation(line: 3, column: 53, scope: !7)
!19 = !DILocalVariable(name: "i", scope: !20, file: !1, line: 4, type: !10)
!20 = distinct !DILexicalBlock(scope: !7, file: !1, line: 4, column: 3)
!21 = !DILocation(line: 4, column: 17, scope: !20)
!22 = !DILocation(line: 4, column: 8, scope: !20)
!23 = !DILocation(line: 4, column: 26, scope: !24)
!24 = !DILexicalBlockFile(scope: !25, file: !1, discriminator: 1)
!25 = distinct !DILexicalBlock(scope: !20, file: !1, line: 4, column: 3)
!26 = !DILocation(line: 4, column: 3, scope: !27)
!27 = !DILexicalBlockFile(scope: !20, file: !1, discriminator: 1)
!28 = !DILocation(line: 5, column: 10, scope: !29)
!29 = distinct !DILexicalBlock(scope: !25, file: !1, line: 4, column: 36)
!30 = !DILocation(line: 5, column: 15, scope: !29)
!31 = !DILocation(line: 5, column: 14, scope: !29)
!32 = !DILocation(line: 5, column: 5, scope: !29)
!33 = !DILocation(line: 5, column: 9, scope: !29)
!34 = !DILocation(line: 6, column: 3, scope: !29)
!35 = !DILocation(line: 4, column: 31, scope: !36)
!36 = !DILexicalBlockFile(scope: !25, file: !1, discriminator: 2)
!37 = !DILocation(line: 4, column: 3, scope: !36)
!38 = distinct !{!38, !39, !40}
!39 = !DILocation(line: 4, column: 3, scope: !20)
!40 = !DILocation(line: 6, column: 3, scope: !20)
!41 = !DILocation(line: 7, column: 3, scope: !7)
!42 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 10, type: !43, isLocal: false, isDefinition: true, scopeLine: 10, isOptimized: false, unit: !0, variables: !2)
!43 = !DISubroutineType(types: !44)
!44 = !{!45}
!45 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!46 = !DILocalVariable(name: "a", scope: !42, file: !1, line: 12, type: !47)
!47 = !DICompositeType(tag: DW_TAG_array_type, baseType: !10, size: 160, elements: !48)
!48 = !{!49}
!49 = !DISubrange(count: 5)
!50 = !DILocation(line: 12, column: 12, scope: !42)
!51 = !DILocalVariable(name: "b", scope: !42, file: !1, line: 13, type: !47)
!52 = !DILocation(line: 13, column: 12, scope: !42)
!53 = !DILocalVariable(name: "c", scope: !42, file: !1, line: 14, type: !47)
!54 = !DILocation(line: 14, column: 12, scope: !42)
!55 = !DILocation(line: 15, column: 10, scope: !42)
!56 = !DILocation(line: 15, column: 12, scope: !42)
!57 = !DILocation(line: 15, column: 14, scope: !42)
!58 = !DILocation(line: 15, column: 3, scope: !42)
!59 = !DILocalVariable(name: "i", scope: !42, file: !1, line: 11, type: !45)
!60 = !DILocation(line: 11, column: 7, scope: !42)
!61 = !DILocation(line: 16, column: 7, scope: !62)
!62 = distinct !DILexicalBlock(scope: !42, file: !1, line: 16, column: 3)
!63 = !DILocation(line: 16, column: 12, scope: !64)
!64 = !DILexicalBlockFile(scope: !65, file: !1, discriminator: 1)
!65 = distinct !DILexicalBlock(scope: !62, file: !1, line: 16, column: 3)
!66 = !DILocation(line: 16, column: 3, scope: !67)
!67 = !DILexicalBlockFile(scope: !62, file: !1, discriminator: 1)
!68 = !DILocation(line: 17, column: 20, scope: !69)
!69 = distinct !DILexicalBlock(scope: !65, file: !1, line: 16, column: 20)
!70 = !DILocation(line: 17, column: 5, scope: !69)
!71 = !DILocation(line: 18, column: 3, scope: !69)
!72 = !DILocation(line: 16, column: 16, scope: !73)
!73 = !DILexicalBlockFile(scope: !65, file: !1, discriminator: 2)
!74 = !DILocation(line: 16, column: 3, scope: !73)
!75 = distinct !{!75, !76, !77}
!76 = !DILocation(line: 16, column: 3, scope: !62)
!77 = !DILocation(line: 18, column: 3, scope: !62)
!78 = !DILocation(line: 20, column: 1, scope: !42)
