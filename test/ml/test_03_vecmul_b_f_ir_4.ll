; ModuleID = './test_03_vecmul_b_f_ir_4.bc'
source_filename = "./03_vecmul_b_f_ir_4.bc"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux_gnu"

define void @test_03_vecmul_b_f_ir_4(i8* nocapture align 8 dereferenceable(8) %retval, i8* noalias nocapture readnone %run_options, i8** noalias nocapture readonly %params, i8** noalias nocapture readonly %temps, i64* noalias nocapture readnone %prof_counters) #0 {
multiply.loop_body.dim.0.lr.ph:
  %0 = getelementptr inbounds i8*, i8** %params, i64 1
  %arg1.untyped = load i8*, i8** %0, align 8, !dereferenceable !0, !align !1
  %1 = bitcast i8* %arg1.untyped to [64 x float]*
  %arg0.untyped = load i8*, i8** %params, align 8, !dereferenceable !0, !align !1
  %2 = bitcast i8* %arg0.untyped to [64 x float]*
  %3 = load i8*, i8** %temps, align 8, !dereferenceable !0, !align !1
  %multiply = bitcast i8* %3 to [64 x float]*
  %4 = getelementptr inbounds [64 x float], [64 x float]* %2, i64 0, i64 0
  %5 = load float, float* %4, align 4
  %6 = getelementptr inbounds [64 x float], [64 x float]* %1, i64 0, i64 0
  %7 = load float, float* %6, align 4
  %8 = fmul float %5, %7
  %9 = getelementptr inbounds [64 x float], [64 x float]* %multiply, i64 0, i64 0
  store float %8, float* %9, align 4
  %10 = getelementptr inbounds [64 x float], [64 x float]* %2, i64 0, i64 1
  %11 = load float, float* %10, align 4
  %12 = getelementptr inbounds [64 x float], [64 x float]* %1, i64 0, i64 1
  %13 = load float, float* %12, align 4
  %14 = fmul float %11, %13
  %15 = getelementptr inbounds [64 x float], [64 x float]* %multiply, i64 0, i64 1
  store float %14, float* %15, align 4
  %16 = getelementptr inbounds [64 x float], [64 x float]* %2, i64 0, i64 2
  %17 = load float, float* %16, align 4
  %18 = getelementptr inbounds [64 x float], [64 x float]* %1, i64 0, i64 2
  %19 = load float, float* %18, align 4
  %20 = fmul float %17, %19
  %21 = getelementptr inbounds [64 x float], [64 x float]* %multiply, i64 0, i64 2
  store float %20, float* %21, align 4
  %22 = getelementptr inbounds [64 x float], [64 x float]* %2, i64 0, i64 3
  %23 = load float, float* %22, align 4
  %24 = getelementptr inbounds [64 x float], [64 x float]* %1, i64 0, i64 3
  %25 = load float, float* %24, align 4
  %26 = fmul float %23, %25
  %27 = getelementptr inbounds [64 x float], [64 x float]* %multiply, i64 0, i64 3
  store float %26, float* %27, align 4
  %28 = getelementptr inbounds [64 x float], [64 x float]* %2, i64 0, i64 4
  %29 = load float, float* %28, align 4
  %30 = getelementptr inbounds [64 x float], [64 x float]* %1, i64 0, i64 4
  %31 = load float, float* %30, align 4
  %32 = fmul float %29, %31
  %33 = getelementptr inbounds [64 x float], [64 x float]* %multiply, i64 0, i64 4
  store float %32, float* %33, align 4
  %34 = getelementptr inbounds [64 x float], [64 x float]* %2, i64 0, i64 5
  %35 = load float, float* %34, align 4
  %36 = getelementptr inbounds [64 x float], [64 x float]* %1, i64 0, i64 5
  %37 = load float, float* %36, align 4
  %38 = fmul float %35, %37
  %39 = getelementptr inbounds [64 x float], [64 x float]* %multiply, i64 0, i64 5
  store float %38, float* %39, align 4
  %40 = getelementptr inbounds [64 x float], [64 x float]* %2, i64 0, i64 6
  %41 = load float, float* %40, align 4
  %42 = getelementptr inbounds [64 x float], [64 x float]* %1, i64 0, i64 6
  %43 = load float, float* %42, align 4
  %44 = fmul float %41, %43
  %45 = getelementptr inbounds [64 x float], [64 x float]* %multiply, i64 0, i64 6
  store float %44, float* %45, align 4
  %46 = getelementptr inbounds [64 x float], [64 x float]* %2, i64 0, i64 7
  %47 = load float, float* %46, align 4
  %48 = getelementptr inbounds [64 x float], [64 x float]* %1, i64 0, i64 7
  %49 = load float, float* %48, align 4
  %50 = fmul float %47, %49
  %51 = getelementptr inbounds [64 x float], [64 x float]* %multiply, i64 0, i64 7
  store float %50, float* %51, align 4
  %52 = getelementptr inbounds [64 x float], [64 x float]* %2, i64 0, i64 8
  %53 = load float, float* %52, align 4
  %54 = getelementptr inbounds [64 x float], [64 x float]* %1, i64 0, i64 8
  %55 = load float, float* %54, align 4
  %56 = fmul float %53, %55
  %57 = getelementptr inbounds [64 x float], [64 x float]* %multiply, i64 0, i64 8
  store float %56, float* %57, align 4
  %58 = getelementptr inbounds [64 x float], [64 x float]* %2, i64 0, i64 9
  %59 = load float, float* %58, align 4
  %60 = getelementptr inbounds [64 x float], [64 x float]* %1, i64 0, i64 9
  %61 = load float, float* %60, align 4
  %62 = fmul float %59, %61
  %63 = getelementptr inbounds [64 x float], [64 x float]* %multiply, i64 0, i64 9
  store float %62, float* %63, align 4
  %64 = getelementptr inbounds [64 x float], [64 x float]* %2, i64 0, i64 10
  %65 = load float, float* %64, align 4
  %66 = getelementptr inbounds [64 x float], [64 x float]* %1, i64 0, i64 10
  %67 = load float, float* %66, align 4
  %68 = fmul float %65, %67
  %69 = getelementptr inbounds [64 x float], [64 x float]* %multiply, i64 0, i64 10
  store float %68, float* %69, align 4
  %70 = getelementptr inbounds [64 x float], [64 x float]* %2, i64 0, i64 11
  %71 = load float, float* %70, align 4
  %72 = getelementptr inbounds [64 x float], [64 x float]* %1, i64 0, i64 11
  %73 = load float, float* %72, align 4
  %74 = fmul float %71, %73
  %75 = getelementptr inbounds [64 x float], [64 x float]* %multiply, i64 0, i64 11
  store float %74, float* %75, align 4
  %76 = getelementptr inbounds [64 x float], [64 x float]* %2, i64 0, i64 12
  %77 = load float, float* %76, align 4
  %78 = getelementptr inbounds [64 x float], [64 x float]* %1, i64 0, i64 12
  %79 = load float, float* %78, align 4
  %80 = fmul float %77, %79
  %81 = getelementptr inbounds [64 x float], [64 x float]* %multiply, i64 0, i64 12
  store float %80, float* %81, align 4
  %82 = getelementptr inbounds [64 x float], [64 x float]* %2, i64 0, i64 13
  %83 = load float, float* %82, align 4
  %84 = getelementptr inbounds [64 x float], [64 x float]* %1, i64 0, i64 13
  %85 = load float, float* %84, align 4
  %86 = fmul float %83, %85
  %87 = getelementptr inbounds [64 x float], [64 x float]* %multiply, i64 0, i64 13
  store float %86, float* %87, align 4
  %88 = getelementptr inbounds [64 x float], [64 x float]* %2, i64 0, i64 14
  %89 = load float, float* %88, align 4
  %90 = getelementptr inbounds [64 x float], [64 x float]* %1, i64 0, i64 14
  %91 = load float, float* %90, align 4
  %92 = fmul float %89, %91
  %93 = getelementptr inbounds [64 x float], [64 x float]* %multiply, i64 0, i64 14
  store float %92, float* %93, align 4
  %94 = getelementptr inbounds [64 x float], [64 x float]* %2, i64 0, i64 15
  %95 = load float, float* %94, align 4
  %96 = getelementptr inbounds [64 x float], [64 x float]* %1, i64 0, i64 15
  %97 = load float, float* %96, align 4
  %98 = fmul float %95, %97
  %99 = getelementptr inbounds [64 x float], [64 x float]* %multiply, i64 0, i64 15
  store float %98, float* %99, align 4
  %100 = getelementptr inbounds [64 x float], [64 x float]* %2, i64 0, i64 16
  %101 = load float, float* %100, align 4
  %102 = getelementptr inbounds [64 x float], [64 x float]* %1, i64 0, i64 16
  %103 = load float, float* %102, align 4
  %104 = fmul float %101, %103
  %105 = getelementptr inbounds [64 x float], [64 x float]* %multiply, i64 0, i64 16
  store float %104, float* %105, align 4
  %106 = getelementptr inbounds [64 x float], [64 x float]* %2, i64 0, i64 17
  %107 = load float, float* %106, align 4
  %108 = getelementptr inbounds [64 x float], [64 x float]* %1, i64 0, i64 17
  %109 = load float, float* %108, align 4
  %110 = fmul float %107, %109
  %111 = getelementptr inbounds [64 x float], [64 x float]* %multiply, i64 0, i64 17
  store float %110, float* %111, align 4
  %112 = getelementptr inbounds [64 x float], [64 x float]* %2, i64 0, i64 18
  %113 = load float, float* %112, align 4
  %114 = getelementptr inbounds [64 x float], [64 x float]* %1, i64 0, i64 18
  %115 = load float, float* %114, align 4
  %116 = fmul float %113, %115
  %117 = getelementptr inbounds [64 x float], [64 x float]* %multiply, i64 0, i64 18
  store float %116, float* %117, align 4
  %118 = getelementptr inbounds [64 x float], [64 x float]* %2, i64 0, i64 19
  %119 = load float, float* %118, align 4
  %120 = getelementptr inbounds [64 x float], [64 x float]* %1, i64 0, i64 19
  %121 = load float, float* %120, align 4
  %122 = fmul float %119, %121
  %123 = getelementptr inbounds [64 x float], [64 x float]* %multiply, i64 0, i64 19
  store float %122, float* %123, align 4
  %124 = getelementptr inbounds [64 x float], [64 x float]* %2, i64 0, i64 20
  %125 = load float, float* %124, align 4
  %126 = getelementptr inbounds [64 x float], [64 x float]* %1, i64 0, i64 20
  %127 = load float, float* %126, align 4
  %128 = fmul float %125, %127
  %129 = getelementptr inbounds [64 x float], [64 x float]* %multiply, i64 0, i64 20
  store float %128, float* %129, align 4
  %130 = getelementptr inbounds [64 x float], [64 x float]* %2, i64 0, i64 21
  %131 = load float, float* %130, align 4
  %132 = getelementptr inbounds [64 x float], [64 x float]* %1, i64 0, i64 21
  %133 = load float, float* %132, align 4
  %134 = fmul float %131, %133
  %135 = getelementptr inbounds [64 x float], [64 x float]* %multiply, i64 0, i64 21
  store float %134, float* %135, align 4
  %136 = getelementptr inbounds [64 x float], [64 x float]* %2, i64 0, i64 22
  %137 = load float, float* %136, align 4
  %138 = getelementptr inbounds [64 x float], [64 x float]* %1, i64 0, i64 22
  %139 = load float, float* %138, align 4
  %140 = fmul float %137, %139
  %141 = getelementptr inbounds [64 x float], [64 x float]* %multiply, i64 0, i64 22
  store float %140, float* %141, align 4
  %142 = getelementptr inbounds [64 x float], [64 x float]* %2, i64 0, i64 23
  %143 = load float, float* %142, align 4
  %144 = getelementptr inbounds [64 x float], [64 x float]* %1, i64 0, i64 23
  %145 = load float, float* %144, align 4
  %146 = fmul float %143, %145
  %147 = getelementptr inbounds [64 x float], [64 x float]* %multiply, i64 0, i64 23
  store float %146, float* %147, align 4
  %148 = getelementptr inbounds [64 x float], [64 x float]* %2, i64 0, i64 24
  %149 = load float, float* %148, align 4
  %150 = getelementptr inbounds [64 x float], [64 x float]* %1, i64 0, i64 24
  %151 = load float, float* %150, align 4
  %152 = fmul float %149, %151
  %153 = getelementptr inbounds [64 x float], [64 x float]* %multiply, i64 0, i64 24
  store float %152, float* %153, align 4
  %154 = getelementptr inbounds [64 x float], [64 x float]* %2, i64 0, i64 25
  %155 = load float, float* %154, align 4
  %156 = getelementptr inbounds [64 x float], [64 x float]* %1, i64 0, i64 25
  %157 = load float, float* %156, align 4
  %158 = fmul float %155, %157
  %159 = getelementptr inbounds [64 x float], [64 x float]* %multiply, i64 0, i64 25
  store float %158, float* %159, align 4
  %160 = getelementptr inbounds [64 x float], [64 x float]* %2, i64 0, i64 26
  %161 = load float, float* %160, align 4
  %162 = getelementptr inbounds [64 x float], [64 x float]* %1, i64 0, i64 26
  %163 = load float, float* %162, align 4
  %164 = fmul float %161, %163
  %165 = getelementptr inbounds [64 x float], [64 x float]* %multiply, i64 0, i64 26
  store float %164, float* %165, align 4
  %166 = getelementptr inbounds [64 x float], [64 x float]* %2, i64 0, i64 27
  %167 = load float, float* %166, align 4
  %168 = getelementptr inbounds [64 x float], [64 x float]* %1, i64 0, i64 27
  %169 = load float, float* %168, align 4
  %170 = fmul float %167, %169
  %171 = getelementptr inbounds [64 x float], [64 x float]* %multiply, i64 0, i64 27
  store float %170, float* %171, align 4
  %172 = getelementptr inbounds [64 x float], [64 x float]* %2, i64 0, i64 28
  %173 = load float, float* %172, align 4
  %174 = getelementptr inbounds [64 x float], [64 x float]* %1, i64 0, i64 28
  %175 = load float, float* %174, align 4
  %176 = fmul float %173, %175
  %177 = getelementptr inbounds [64 x float], [64 x float]* %multiply, i64 0, i64 28
  store float %176, float* %177, align 4
  %178 = getelementptr inbounds [64 x float], [64 x float]* %2, i64 0, i64 29
  %179 = load float, float* %178, align 4
  %180 = getelementptr inbounds [64 x float], [64 x float]* %1, i64 0, i64 29
  %181 = load float, float* %180, align 4
  %182 = fmul float %179, %181
  %183 = getelementptr inbounds [64 x float], [64 x float]* %multiply, i64 0, i64 29
  store float %182, float* %183, align 4
  %184 = getelementptr inbounds [64 x float], [64 x float]* %2, i64 0, i64 30
  %185 = load float, float* %184, align 4
  %186 = getelementptr inbounds [64 x float], [64 x float]* %1, i64 0, i64 30
  %187 = load float, float* %186, align 4
  %188 = fmul float %185, %187
  %189 = getelementptr inbounds [64 x float], [64 x float]* %multiply, i64 0, i64 30
  store float %188, float* %189, align 4
  %190 = getelementptr inbounds [64 x float], [64 x float]* %2, i64 0, i64 31
  %191 = load float, float* %190, align 4
  %192 = getelementptr inbounds [64 x float], [64 x float]* %1, i64 0, i64 31
  %193 = load float, float* %192, align 4
  %194 = fmul float %191, %193
  %195 = getelementptr inbounds [64 x float], [64 x float]* %multiply, i64 0, i64 31
  store float %194, float* %195, align 4
  %196 = getelementptr inbounds [64 x float], [64 x float]* %2, i64 0, i64 32
  %197 = load float, float* %196, align 4
  %198 = getelementptr inbounds [64 x float], [64 x float]* %1, i64 0, i64 32
  %199 = load float, float* %198, align 4
  %200 = fmul float %197, %199
  %201 = getelementptr inbounds [64 x float], [64 x float]* %multiply, i64 0, i64 32
  store float %200, float* %201, align 4
  %202 = getelementptr inbounds [64 x float], [64 x float]* %2, i64 0, i64 33
  %203 = load float, float* %202, align 4
  %204 = getelementptr inbounds [64 x float], [64 x float]* %1, i64 0, i64 33
  %205 = load float, float* %204, align 4
  %206 = fmul float %203, %205
  %207 = getelementptr inbounds [64 x float], [64 x float]* %multiply, i64 0, i64 33
  store float %206, float* %207, align 4
  %208 = getelementptr inbounds [64 x float], [64 x float]* %2, i64 0, i64 34
  %209 = load float, float* %208, align 4
  %210 = getelementptr inbounds [64 x float], [64 x float]* %1, i64 0, i64 34
  %211 = load float, float* %210, align 4
  %212 = fmul float %209, %211
  %213 = getelementptr inbounds [64 x float], [64 x float]* %multiply, i64 0, i64 34
  store float %212, float* %213, align 4
  %214 = getelementptr inbounds [64 x float], [64 x float]* %2, i64 0, i64 35
  %215 = load float, float* %214, align 4
  %216 = getelementptr inbounds [64 x float], [64 x float]* %1, i64 0, i64 35
  %217 = load float, float* %216, align 4
  %218 = fmul float %215, %217
  %219 = getelementptr inbounds [64 x float], [64 x float]* %multiply, i64 0, i64 35
  store float %218, float* %219, align 4
  %220 = getelementptr inbounds [64 x float], [64 x float]* %2, i64 0, i64 36
  %221 = load float, float* %220, align 4
  %222 = getelementptr inbounds [64 x float], [64 x float]* %1, i64 0, i64 36
  %223 = load float, float* %222, align 4
  %224 = fmul float %221, %223
  %225 = getelementptr inbounds [64 x float], [64 x float]* %multiply, i64 0, i64 36
  store float %224, float* %225, align 4
  %226 = getelementptr inbounds [64 x float], [64 x float]* %2, i64 0, i64 37
  %227 = load float, float* %226, align 4
  %228 = getelementptr inbounds [64 x float], [64 x float]* %1, i64 0, i64 37
  %229 = load float, float* %228, align 4
  %230 = fmul float %227, %229
  %231 = getelementptr inbounds [64 x float], [64 x float]* %multiply, i64 0, i64 37
  store float %230, float* %231, align 4
  %232 = getelementptr inbounds [64 x float], [64 x float]* %2, i64 0, i64 38
  %233 = load float, float* %232, align 4
  %234 = getelementptr inbounds [64 x float], [64 x float]* %1, i64 0, i64 38
  %235 = load float, float* %234, align 4
  %236 = fmul float %233, %235
  %237 = getelementptr inbounds [64 x float], [64 x float]* %multiply, i64 0, i64 38
  store float %236, float* %237, align 4
  %238 = getelementptr inbounds [64 x float], [64 x float]* %2, i64 0, i64 39
  %239 = load float, float* %238, align 4
  %240 = getelementptr inbounds [64 x float], [64 x float]* %1, i64 0, i64 39
  %241 = load float, float* %240, align 4
  %242 = fmul float %239, %241
  %243 = getelementptr inbounds [64 x float], [64 x float]* %multiply, i64 0, i64 39
  store float %242, float* %243, align 4
  %244 = getelementptr inbounds [64 x float], [64 x float]* %2, i64 0, i64 40
  %245 = load float, float* %244, align 4
  %246 = getelementptr inbounds [64 x float], [64 x float]* %1, i64 0, i64 40
  %247 = load float, float* %246, align 4
  %248 = fmul float %245, %247
  %249 = getelementptr inbounds [64 x float], [64 x float]* %multiply, i64 0, i64 40
  store float %248, float* %249, align 4
  %250 = getelementptr inbounds [64 x float], [64 x float]* %2, i64 0, i64 41
  %251 = load float, float* %250, align 4
  %252 = getelementptr inbounds [64 x float], [64 x float]* %1, i64 0, i64 41
  %253 = load float, float* %252, align 4
  %254 = fmul float %251, %253
  %255 = getelementptr inbounds [64 x float], [64 x float]* %multiply, i64 0, i64 41
  store float %254, float* %255, align 4
  %256 = getelementptr inbounds [64 x float], [64 x float]* %2, i64 0, i64 42
  %257 = load float, float* %256, align 4
  %258 = getelementptr inbounds [64 x float], [64 x float]* %1, i64 0, i64 42
  %259 = load float, float* %258, align 4
  %260 = fmul float %257, %259
  %261 = getelementptr inbounds [64 x float], [64 x float]* %multiply, i64 0, i64 42
  store float %260, float* %261, align 4
  %262 = getelementptr inbounds [64 x float], [64 x float]* %2, i64 0, i64 43
  %263 = load float, float* %262, align 4
  %264 = getelementptr inbounds [64 x float], [64 x float]* %1, i64 0, i64 43
  %265 = load float, float* %264, align 4
  %266 = fmul float %263, %265
  %267 = getelementptr inbounds [64 x float], [64 x float]* %multiply, i64 0, i64 43
  store float %266, float* %267, align 4
  %268 = getelementptr inbounds [64 x float], [64 x float]* %2, i64 0, i64 44
  %269 = load float, float* %268, align 4
  %270 = getelementptr inbounds [64 x float], [64 x float]* %1, i64 0, i64 44
  %271 = load float, float* %270, align 4
  %272 = fmul float %269, %271
  %273 = getelementptr inbounds [64 x float], [64 x float]* %multiply, i64 0, i64 44
  store float %272, float* %273, align 4
  %274 = getelementptr inbounds [64 x float], [64 x float]* %2, i64 0, i64 45
  %275 = load float, float* %274, align 4
  %276 = getelementptr inbounds [64 x float], [64 x float]* %1, i64 0, i64 45
  %277 = load float, float* %276, align 4
  %278 = fmul float %275, %277
  %279 = getelementptr inbounds [64 x float], [64 x float]* %multiply, i64 0, i64 45
  store float %278, float* %279, align 4
  %280 = getelementptr inbounds [64 x float], [64 x float]* %2, i64 0, i64 46
  %281 = load float, float* %280, align 4
  %282 = getelementptr inbounds [64 x float], [64 x float]* %1, i64 0, i64 46
  %283 = load float, float* %282, align 4
  %284 = fmul float %281, %283
  %285 = getelementptr inbounds [64 x float], [64 x float]* %multiply, i64 0, i64 46
  store float %284, float* %285, align 4
  %286 = getelementptr inbounds [64 x float], [64 x float]* %2, i64 0, i64 47
  %287 = load float, float* %286, align 4
  %288 = getelementptr inbounds [64 x float], [64 x float]* %1, i64 0, i64 47
  %289 = load float, float* %288, align 4
  %290 = fmul float %287, %289
  %291 = getelementptr inbounds [64 x float], [64 x float]* %multiply, i64 0, i64 47
  store float %290, float* %291, align 4
  %292 = getelementptr inbounds [64 x float], [64 x float]* %2, i64 0, i64 48
  %293 = load float, float* %292, align 4
  %294 = getelementptr inbounds [64 x float], [64 x float]* %1, i64 0, i64 48
  %295 = load float, float* %294, align 4
  %296 = fmul float %293, %295
  %297 = getelementptr inbounds [64 x float], [64 x float]* %multiply, i64 0, i64 48
  store float %296, float* %297, align 4
  %298 = getelementptr inbounds [64 x float], [64 x float]* %2, i64 0, i64 49
  %299 = load float, float* %298, align 4
  %300 = getelementptr inbounds [64 x float], [64 x float]* %1, i64 0, i64 49
  %301 = load float, float* %300, align 4
  %302 = fmul float %299, %301
  %303 = getelementptr inbounds [64 x float], [64 x float]* %multiply, i64 0, i64 49
  store float %302, float* %303, align 4
  %304 = getelementptr inbounds [64 x float], [64 x float]* %2, i64 0, i64 50
  %305 = load float, float* %304, align 4
  %306 = getelementptr inbounds [64 x float], [64 x float]* %1, i64 0, i64 50
  %307 = load float, float* %306, align 4
  %308 = fmul float %305, %307
  %309 = getelementptr inbounds [64 x float], [64 x float]* %multiply, i64 0, i64 50
  store float %308, float* %309, align 4
  %310 = getelementptr inbounds [64 x float], [64 x float]* %2, i64 0, i64 51
  %311 = load float, float* %310, align 4
  %312 = getelementptr inbounds [64 x float], [64 x float]* %1, i64 0, i64 51
  %313 = load float, float* %312, align 4
  %314 = fmul float %311, %313
  %315 = getelementptr inbounds [64 x float], [64 x float]* %multiply, i64 0, i64 51
  store float %314, float* %315, align 4
  %316 = getelementptr inbounds [64 x float], [64 x float]* %2, i64 0, i64 52
  %317 = load float, float* %316, align 4
  %318 = getelementptr inbounds [64 x float], [64 x float]* %1, i64 0, i64 52
  %319 = load float, float* %318, align 4
  %320 = fmul float %317, %319
  %321 = getelementptr inbounds [64 x float], [64 x float]* %multiply, i64 0, i64 52
  store float %320, float* %321, align 4
  %322 = getelementptr inbounds [64 x float], [64 x float]* %2, i64 0, i64 53
  %323 = load float, float* %322, align 4
  %324 = getelementptr inbounds [64 x float], [64 x float]* %1, i64 0, i64 53
  %325 = load float, float* %324, align 4
  %326 = fmul float %323, %325
  %327 = getelementptr inbounds [64 x float], [64 x float]* %multiply, i64 0, i64 53
  store float %326, float* %327, align 4
  %328 = getelementptr inbounds [64 x float], [64 x float]* %2, i64 0, i64 54
  %329 = load float, float* %328, align 4
  %330 = getelementptr inbounds [64 x float], [64 x float]* %1, i64 0, i64 54
  %331 = load float, float* %330, align 4
  %332 = fmul float %329, %331
  %333 = getelementptr inbounds [64 x float], [64 x float]* %multiply, i64 0, i64 54
  store float %332, float* %333, align 4
  %334 = getelementptr inbounds [64 x float], [64 x float]* %2, i64 0, i64 55
  %335 = load float, float* %334, align 4
  %336 = getelementptr inbounds [64 x float], [64 x float]* %1, i64 0, i64 55
  %337 = load float, float* %336, align 4
  %338 = fmul float %335, %337
  %339 = getelementptr inbounds [64 x float], [64 x float]* %multiply, i64 0, i64 55
  store float %338, float* %339, align 4
  %340 = getelementptr inbounds [64 x float], [64 x float]* %2, i64 0, i64 56
  %341 = load float, float* %340, align 4
  %342 = getelementptr inbounds [64 x float], [64 x float]* %1, i64 0, i64 56
  %343 = load float, float* %342, align 4
  %344 = fmul float %341, %343
  %345 = getelementptr inbounds [64 x float], [64 x float]* %multiply, i64 0, i64 56
  store float %344, float* %345, align 4
  %346 = getelementptr inbounds [64 x float], [64 x float]* %2, i64 0, i64 57
  %347 = load float, float* %346, align 4
  %348 = getelementptr inbounds [64 x float], [64 x float]* %1, i64 0, i64 57
  %349 = load float, float* %348, align 4
  %350 = fmul float %347, %349
  %351 = getelementptr inbounds [64 x float], [64 x float]* %multiply, i64 0, i64 57
  store float %350, float* %351, align 4
  %352 = getelementptr inbounds [64 x float], [64 x float]* %2, i64 0, i64 58
  %353 = load float, float* %352, align 4
  %354 = getelementptr inbounds [64 x float], [64 x float]* %1, i64 0, i64 58
  %355 = load float, float* %354, align 4
  %356 = fmul float %353, %355
  %357 = getelementptr inbounds [64 x float], [64 x float]* %multiply, i64 0, i64 58
  store float %356, float* %357, align 4
  %358 = getelementptr inbounds [64 x float], [64 x float]* %2, i64 0, i64 59
  %359 = load float, float* %358, align 4
  %360 = getelementptr inbounds [64 x float], [64 x float]* %1, i64 0, i64 59
  %361 = load float, float* %360, align 4
  %362 = fmul float %359, %361
  %363 = getelementptr inbounds [64 x float], [64 x float]* %multiply, i64 0, i64 59
  store float %362, float* %363, align 4
  %364 = getelementptr inbounds [64 x float], [64 x float]* %2, i64 0, i64 60
  %365 = load float, float* %364, align 4
  %366 = getelementptr inbounds [64 x float], [64 x float]* %1, i64 0, i64 60
  %367 = load float, float* %366, align 4
  %368 = fmul float %365, %367
  %369 = getelementptr inbounds [64 x float], [64 x float]* %multiply, i64 0, i64 60
  store float %368, float* %369, align 4
  %370 = getelementptr inbounds [64 x float], [64 x float]* %2, i64 0, i64 61
  %371 = load float, float* %370, align 4
  %372 = getelementptr inbounds [64 x float], [64 x float]* %1, i64 0, i64 61
  %373 = load float, float* %372, align 4
  %374 = fmul float %371, %373
  %375 = getelementptr inbounds [64 x float], [64 x float]* %multiply, i64 0, i64 61
  store float %374, float* %375, align 4
  %376 = getelementptr inbounds [64 x float], [64 x float]* %2, i64 0, i64 62
  %377 = load float, float* %376, align 4
  %378 = getelementptr inbounds [64 x float], [64 x float]* %1, i64 0, i64 62
  %379 = load float, float* %378, align 4
  %380 = fmul float %377, %379
  %381 = getelementptr inbounds [64 x float], [64 x float]* %multiply, i64 0, i64 62
  store float %380, float* %381, align 4
  %382 = getelementptr inbounds [64 x float], [64 x float]* %2, i64 0, i64 63
  %383 = load float, float* %382, align 4
  %384 = getelementptr inbounds [64 x float], [64 x float]* %1, i64 0, i64 63
  %385 = load float, float* %384, align 4
  %386 = fmul float %383, %385
  %387 = getelementptr inbounds [64 x float], [64 x float]* %multiply, i64 0, i64 63
  store float %386, float* %387, align 4
  %388 = bitcast i8* %retval to i8**
  store i8* %3, i8** %388, align 8
  ret void
}

attributes #0 = { "no-frame-pointer-elim"="false" }

!0 = !{i64 256}
!1 = !{i64 8}
