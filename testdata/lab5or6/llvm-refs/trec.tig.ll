; ModuleID = 'lightir'
source_filename = "lightir"
target triple = "x86_64-pc-linux-gnu"

%string = type { i32, i8* }
%MyStruct = type { i32, i32 }

@tigermain_framesize_global = constant i64 32

declare void @flush()

declare void @exit(i32)

declare %string* @chr(i32)

declare %string* @__wrap_getchar()

declare void @print(%string*)

declare void @printi(i32)

declare i32 @ord(%string*)

declare i32 @size(%string*)

declare %string* @concat(%string*, %string*)

declare %string* @substring(%string*, i32, i32)

declare i64 @alloc_record(i32)

declare i64 @init_array(i32, i64)

declare i1 @string_equal(%string*, %string*)

define i32 @tigermain(i64 %0, i64 %1) {
tigermain:
  %2 = load i64, i64* @tigermain_framesize_global, align 4
  %tigermain_sp = sub i64 %0, %2
  %3 = call i64 @alloc_record(i32 16)
  %4 = inttoptr i64 %3 to %MyStruct*
  %5 = getelementptr %MyStruct, %MyStruct* %4, i32 0, i32 0
  store i32 3, i32* %5, align 4
  %6 = getelementptr %MyStruct, %MyStruct* %4, i32 0, i32 1
  store i32 4, i32* %6, align 4
  %7 = ptrtoint %MyStruct* %4 to i64
  %8 = load i64, i64* @tigermain_framesize_global, align 4
  %9 = add i64 %8, -8
  %10 = add i64 %tigermain_sp, %9
  %11 = inttoptr i64 %10 to i64*
  store i64 %7, i64* %11, align 4
  %12 = load i64, i64* @tigermain_framesize_global, align 4
  %13 = add i64 %12, -8
  %14 = add i64 %tigermain_sp, %13
  %tigermain_x_ptr = inttoptr i64 %14 to %MyStruct**
  %15 = load %MyStruct*, %MyStruct** %tigermain_x_ptr, align 8
  %16 = getelementptr %MyStruct, %MyStruct* %15, i32 0, i32 0
  %actual_parm = load i32, i32* %16, align 4
  call void @printi(i32 %actual_parm)
  %17 = load i64, i64* @tigermain_framesize_global, align 4
  %18 = add i64 %17, -8
  %19 = add i64 %tigermain_sp, %18
  %tigermain_x_ptr1 = inttoptr i64 %19 to %MyStruct**
  %20 = load %MyStruct*, %MyStruct** %tigermain_x_ptr1, align 8
  %21 = getelementptr %MyStruct, %MyStruct* %20, i32 0, i32 1
  %actual_parm2 = load i32, i32* %21, align 4
  call void @printi(i32 %actual_parm2)
  ret i32 0
}

; ===== Frame Information =====
; tigermain -8 32
