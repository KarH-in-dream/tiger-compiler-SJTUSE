; ModuleID = 'lightir'
source_filename = "lightir"
target triple = "x86_64-pc-linux-gnu"

%string = type { i32, i8* }

@tigermain_framesize_global = constant i64 24
@a_framesize_global = constant i64 24
@b_framesize_global = constant i64 16

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
  %3 = call i32 @a(i64 %tigermain_sp, i64 %tigermain_sp, i32 2)
  call void @printi(i32 %3)
  ret i32 0
}

define i32 @a(i64 %0, i64 %1, i32 %2) {
a:
  %a_local_framesize = load i64, i64* @a_framesize_global, align 4
  %a_sp = sub i64 %0, %a_local_framesize
  %3 = load i64, i64* @a_framesize_global, align 4
  %4 = add i64 %3, 8
  %5 = add i64 %a_sp, %4
  %sl_ptr = inttoptr i64 %5 to i64*
  store i64 %1, i64* %sl_ptr, align 4
  %6 = load i64, i64* @a_framesize_global, align 4
  %7 = add i64 %6, 16
  %8 = add i64 %a_sp, %7
  %9 = inttoptr i64 %8 to i32*
  store i32 %2, i32* %9, align 4
  %10 = call i32 @b(i64 %a_sp, i64 %a_sp, i32 3)
  ret i32 %10
}

define i32 @b(i64 %0, i64 %1, i32 %2) {
b:
  %b_local_framesize = load i64, i64* @b_framesize_global, align 4
  %b_sp = sub i64 %0, %b_local_framesize
  %3 = load i64, i64* @b_framesize_global, align 4
  %4 = add i64 %3, 8
  %5 = add i64 %b_sp, %4
  %sl_ptr = inttoptr i64 %5 to i64*
  store i64 %1, i64* %sl_ptr, align 4
  %6 = load i64, i64* @b_framesize_global, align 4
  %7 = add i64 %6, 16
  %8 = add i64 %b_sp, %7
  %9 = inttoptr i64 %8 to i32*
  store i32 %2, i32* %9, align 4
  %10 = load i64, i64* @b_framesize_global, align 4
  %11 = add i64 %10, 8
  %12 = add i64 %b_sp, %11
  %13 = inttoptr i64 %12 to i64*
  %14 = load i64, i64* %13, align 4
  %15 = load i64, i64* @a_framesize_global, align 4
  %16 = add i64 %15, 16
  %17 = add i64 %14, %16
  %a_x_ptr = inttoptr i64 %17 to i32*
  %18 = load i64, i64* @b_framesize_global, align 4
  %19 = add i64 %18, 16
  %20 = add i64 %b_sp, %19
  %b_y_ptr = inttoptr i64 %20 to i32*
  %21 = load i32, i32* %a_x_ptr, align 4
  %22 = load i32, i32* %b_y_ptr, align 4
  %23 = add i32 %21, %22
  ret i32 %23
}

; ===== Frame Information =====
; tigermain 0 24
; a 0 24
; b 0 16
