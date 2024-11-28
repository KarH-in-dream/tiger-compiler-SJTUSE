; ModuleID = 'lightir'
source_filename = "lightir"
target triple = "x86_64-pc-linux-gnu"

%string = type { i32, i8* }

@tigermain_framesize_global = constant i64 32
@nop_framesize_global = constant i64 24
@0 = private unnamed_addr constant [1 x i8] zeroinitializer, align 1
@str = constant %string { i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @0, i32 0, i32 0) }
@init_framesize_global = constant i64 24
@bsearch_framesize_global = constant i64 48
@try_framesize_global = constant i64 40
@1 = private unnamed_addr constant [2 x i8] c"\0A\00", align 1
@str.1 = constant %string { i32 1, i8* getelementptr inbounds ([2 x i8], [2 x i8]* @1, i32 0, i32 0) }

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
  %3 = load i64, i64* @tigermain_framesize_global, align 4
  %4 = add i64 %3, -8
  %5 = add i64 %tigermain_sp, %4
  %6 = inttoptr i64 %5 to i32*
  store i32 16, i32* %6, align 4
  %7 = load i64, i64* @tigermain_framesize_global, align 4
  %8 = add i64 %7, -8
  %9 = add i64 %tigermain_sp, %8
  %tigermain_N_ptr = inttoptr i64 %9 to i32*
  %10 = load i32, i32* %tigermain_N_ptr, align 4
  %11 = call i64 @init_array(i32 %10, i64 0)
  %12 = inttoptr i64 %11 to i64*
  %13 = ptrtoint i64* %12 to i64
  %14 = load i64, i64* @tigermain_framesize_global, align 4
  %15 = add i64 %14, -16
  %16 = add i64 %tigermain_sp, %15
  %17 = inttoptr i64 %16 to i64*
  store i64 %13, i64* %17, align 4
  call void @try(i64 %tigermain_sp, i64 %tigermain_sp)
  ret i32 0
}

define void @nop(i64 %0, i64 %1) {
nop:
  %nop_local_framesize = load i64, i64* @nop_framesize_global, align 4
  %nop_sp = sub i64 %0, %nop_local_framesize
  %2 = load i64, i64* @nop_framesize_global, align 4
  %3 = add i64 %2, 8
  %4 = add i64 %nop_sp, %3
  %sl_ptr = inttoptr i64 %4 to i64*
  store i64 %1, i64* %sl_ptr, align 4
  call void @print(%string* @str)
  ret void
}

define void @init(i64 %0, i64 %1) {
init:
  %init_local_framesize = load i64, i64* @init_framesize_global, align 4
  %init_sp = sub i64 %0, %init_local_framesize
  %2 = load i64, i64* @init_framesize_global, align 4
  %3 = add i64 %2, 8
  %4 = add i64 %init_sp, %3
  %sl_ptr = inttoptr i64 %4 to i64*
  store i64 %1, i64* %sl_ptr, align 4
  %5 = load i64, i64* @init_framesize_global, align 4
  %6 = add i64 %5, 8
  %7 = add i64 %init_sp, %6
  %8 = inttoptr i64 %7 to i64*
  %9 = load i64, i64* %8, align 4
  %10 = load i64, i64* @tigermain_framesize_global, align 4
  %11 = add i64 %10, -8
  %12 = add i64 %9, %11
  %tigermain_N_ptr = inttoptr i64 %12 to i32*
  %13 = load i32, i32* %tigermain_N_ptr, align 4
  %14 = sub i32 %13, 1
  %15 = load i64, i64* @init_framesize_global, align 4
  %16 = add i64 %15, -8
  %17 = add i64 %init_sp, %16
  %18 = inttoptr i64 %17 to i32*
  store i32 0, i32* %18, align 4
  br label %for_body

for_incre:                                        ; preds = %for_body
  %19 = load i32, i32* %18, align 4
  %20 = add i32 1, %19
  store i32 %20, i32* %18, align 4
  %21 = icmp sle i32 %20, %14
  br i1 %21, label %for_body, label %for_next

for_body:                                         ; preds = %for_incre, %init
  %22 = load i64, i64* @init_framesize_global, align 4
  %23 = add i64 %22, 8
  %24 = add i64 %init_sp, %23
  %25 = inttoptr i64 %24 to i64*
  %26 = load i64, i64* %25, align 4
  %27 = load i64, i64* @tigermain_framesize_global, align 4
  %28 = add i64 %27, -16
  %29 = add i64 %26, %28
  %tigermain_list_ptr = inttoptr i64 %29 to i32**
  %30 = load i64, i64* @init_framesize_global, align 4
  %31 = add i64 %30, -8
  %32 = add i64 %init_sp, %31
  %init_i_ptr = inttoptr i64 %32 to i32*
  %33 = load i32*, i32** %tigermain_list_ptr, align 8
  %34 = load i32, i32* %init_i_ptr, align 4
  %35 = getelementptr i32, i32* %33, i32 %34
  %36 = load i64, i64* @init_framesize_global, align 4
  %37 = add i64 %36, -8
  %38 = add i64 %init_sp, %37
  %init_i_ptr1 = inttoptr i64 %38 to i32*
  %39 = load i32, i32* %init_i_ptr1, align 4
  %40 = mul i32 %39, 2
  %41 = add i32 %40, 1
  store i32 %41, i32* %35, align 4
  %42 = load i64, i64* @init_framesize_global, align 4
  %43 = add i64 %42, 8
  %44 = add i64 %init_sp, %43
  %45 = inttoptr i64 %44 to i64*
  %46 = load i64, i64* %45, align 4
  call void @nop(i64 %init_sp, i64 %46)
  br label %for_incre

for_next:                                         ; preds = %for_incre
  ret void
}

define i32 @bsearch(i64 %0, i64 %1, i32 %2, i32 %3, i32 %4) {
bsearch:
  %bsearch_local_framesize = load i64, i64* @bsearch_framesize_global, align 4
  %bsearch_sp = sub i64 %0, %bsearch_local_framesize
  %5 = load i64, i64* @bsearch_framesize_global, align 4
  %6 = add i64 %5, 8
  %7 = add i64 %bsearch_sp, %6
  %sl_ptr = inttoptr i64 %7 to i64*
  store i64 %1, i64* %sl_ptr, align 4
  %8 = load i64, i64* @bsearch_framesize_global, align 4
  %9 = add i64 %8, 16
  %10 = add i64 %bsearch_sp, %9
  %11 = inttoptr i64 %10 to i32*
  store i32 %2, i32* %11, align 4
  %12 = load i64, i64* @bsearch_framesize_global, align 4
  %13 = add i64 %12, 24
  %14 = add i64 %bsearch_sp, %13
  %15 = inttoptr i64 %14 to i32*
  store i32 %3, i32* %15, align 4
  %16 = load i64, i64* @bsearch_framesize_global, align 4
  %17 = add i64 %16, 32
  %18 = add i64 %bsearch_sp, %17
  %19 = inttoptr i64 %18 to i32*
  store i32 %4, i32* %19, align 4
  br label %if_test

if_test:                                          ; preds = %bsearch
  %20 = load i64, i64* @bsearch_framesize_global, align 4
  %21 = add i64 %20, 16
  %22 = add i64 %bsearch_sp, %21
  %bsearch_left_ptr = inttoptr i64 %22 to i32*
  %23 = load i64, i64* @bsearch_framesize_global, align 4
  %24 = add i64 %23, 24
  %25 = add i64 %bsearch_sp, %24
  %bsearch_right_ptr = inttoptr i64 %25 to i32*
  %26 = load i32, i32* %bsearch_left_ptr, align 4
  %27 = load i32, i32* %bsearch_right_ptr, align 4
  %28 = icmp eq i32 %26, %27
  br i1 %28, label %if_then, label %if_else

if_then:                                          ; preds = %if_test
  %29 = load i64, i64* @bsearch_framesize_global, align 4
  %30 = add i64 %29, 16
  %31 = add i64 %bsearch_sp, %30
  %bsearch_left_ptr1 = inttoptr i64 %31 to i32*
  %32 = load i32, i32* %bsearch_left_ptr1, align 4
  br label %if_next

if_else:                                          ; preds = %if_test
  %33 = load i64, i64* @bsearch_framesize_global, align 4
  %34 = add i64 %33, 16
  %35 = add i64 %bsearch_sp, %34
  %bsearch_left_ptr2 = inttoptr i64 %35 to i32*
  %36 = load i64, i64* @bsearch_framesize_global, align 4
  %37 = add i64 %36, 24
  %38 = add i64 %bsearch_sp, %37
  %bsearch_right_ptr3 = inttoptr i64 %38 to i32*
  %39 = load i32, i32* %bsearch_left_ptr2, align 4
  %40 = load i32, i32* %bsearch_right_ptr3, align 4
  %41 = add i32 %39, %40
  %42 = sdiv i32 %41, 2
  %43 = load i64, i64* @bsearch_framesize_global, align 4
  %44 = add i64 %43, -8
  %45 = add i64 %bsearch_sp, %44
  %46 = inttoptr i64 %45 to i32*
  store i32 %42, i32* %46, align 4
  br label %if_test4

if_next:                                          ; preds = %if_next7, %if_then
  %47 = phi i32 [ %32, %if_then ], [ %100, %if_next7 ]
  ret i32 %47

if_test4:                                         ; preds = %if_else
  %48 = load i64, i64* @bsearch_framesize_global, align 4
  %49 = add i64 %48, 8
  %50 = add i64 %bsearch_sp, %49
  %51 = inttoptr i64 %50 to i64*
  %52 = load i64, i64* %51, align 4
  %53 = load i64, i64* @tigermain_framesize_global, align 4
  %54 = add i64 %53, -16
  %55 = add i64 %52, %54
  %tigermain_list_ptr = inttoptr i64 %55 to i32**
  %56 = load i64, i64* @bsearch_framesize_global, align 4
  %57 = add i64 %56, -8
  %58 = add i64 %bsearch_sp, %57
  %bsearch_mid_ptr = inttoptr i64 %58 to i32*
  %59 = load i32*, i32** %tigermain_list_ptr, align 8
  %60 = load i32, i32* %bsearch_mid_ptr, align 4
  %61 = getelementptr i32, i32* %59, i32 %60
  %62 = load i64, i64* @bsearch_framesize_global, align 4
  %63 = add i64 %62, 32
  %64 = add i64 %bsearch_sp, %63
  %bsearch_c_ptr = inttoptr i64 %64 to i32*
  %65 = load i32, i32* %61, align 4
  %66 = load i32, i32* %bsearch_c_ptr, align 4
  %67 = icmp slt i32 %65, %66
  br i1 %67, label %if_then5, label %if_else6

if_then5:                                         ; preds = %if_test4
  %68 = load i64, i64* @bsearch_framesize_global, align 4
  %69 = add i64 %68, -8
  %70 = add i64 %bsearch_sp, %69
  %bsearch_mid_ptr8 = inttoptr i64 %70 to i32*
  %71 = load i32, i32* %bsearch_mid_ptr8, align 4
  %72 = add i32 %71, 1
  %73 = load i64, i64* @bsearch_framesize_global, align 4
  %74 = add i64 %73, 24
  %75 = add i64 %bsearch_sp, %74
  %bsearch_right_ptr9 = inttoptr i64 %75 to i32*
  %actual_parm = load i32, i32* %bsearch_right_ptr9, align 4
  %76 = load i64, i64* @bsearch_framesize_global, align 4
  %77 = add i64 %76, 32
  %78 = add i64 %bsearch_sp, %77
  %bsearch_c_ptr10 = inttoptr i64 %78 to i32*
  %actual_parm11 = load i32, i32* %bsearch_c_ptr10, align 4
  %79 = load i64, i64* @bsearch_framesize_global, align 4
  %80 = add i64 %79, 8
  %81 = add i64 %bsearch_sp, %80
  %82 = inttoptr i64 %81 to i64*
  %83 = load i64, i64* %82, align 4
  %84 = call i32 @bsearch(i64 %bsearch_sp, i64 %83, i32 %72, i32 %actual_parm, i32 %actual_parm11)
  br label %if_next7

if_else6:                                         ; preds = %if_test4
  %85 = load i64, i64* @bsearch_framesize_global, align 4
  %86 = add i64 %85, 16
  %87 = add i64 %bsearch_sp, %86
  %bsearch_left_ptr12 = inttoptr i64 %87 to i32*
  %actual_parm13 = load i32, i32* %bsearch_left_ptr12, align 4
  %88 = load i64, i64* @bsearch_framesize_global, align 4
  %89 = add i64 %88, -8
  %90 = add i64 %bsearch_sp, %89
  %bsearch_mid_ptr14 = inttoptr i64 %90 to i32*
  %actual_parm15 = load i32, i32* %bsearch_mid_ptr14, align 4
  %91 = load i64, i64* @bsearch_framesize_global, align 4
  %92 = add i64 %91, 32
  %93 = add i64 %bsearch_sp, %92
  %bsearch_c_ptr16 = inttoptr i64 %93 to i32*
  %actual_parm17 = load i32, i32* %bsearch_c_ptr16, align 4
  %94 = load i64, i64* @bsearch_framesize_global, align 4
  %95 = add i64 %94, 8
  %96 = add i64 %bsearch_sp, %95
  %97 = inttoptr i64 %96 to i64*
  %98 = load i64, i64* %97, align 4
  %99 = call i32 @bsearch(i64 %bsearch_sp, i64 %98, i32 %actual_parm13, i32 %actual_parm15, i32 %actual_parm17)
  br label %if_next7

if_next7:                                         ; preds = %if_else6, %if_then5
  %100 = phi i32 [ %84, %if_then5 ], [ %99, %if_else6 ]
  br label %if_next
}

define void @try(i64 %0, i64 %1) {
try:
  %try_local_framesize = load i64, i64* @try_framesize_global, align 4
  %try_sp = sub i64 %0, %try_local_framesize
  %2 = load i64, i64* @try_framesize_global, align 4
  %3 = add i64 %2, 8
  %4 = add i64 %try_sp, %3
  %sl_ptr = inttoptr i64 %4 to i64*
  store i64 %1, i64* %sl_ptr, align 4
  %5 = load i64, i64* @try_framesize_global, align 4
  %6 = add i64 %5, 8
  %7 = add i64 %try_sp, %6
  %8 = inttoptr i64 %7 to i64*
  %9 = load i64, i64* %8, align 4
  call void @init(i64 %try_sp, i64 %9)
  %10 = load i64, i64* @try_framesize_global, align 4
  %11 = add i64 %10, 8
  %12 = add i64 %try_sp, %11
  %13 = inttoptr i64 %12 to i64*
  %14 = load i64, i64* %13, align 4
  %15 = load i64, i64* @tigermain_framesize_global, align 4
  %16 = add i64 %15, -8
  %17 = add i64 %14, %16
  %tigermain_N_ptr = inttoptr i64 %17 to i32*
  %18 = load i32, i32* %tigermain_N_ptr, align 4
  %19 = sub i32 %18, 1
  %20 = load i64, i64* @try_framesize_global, align 4
  %21 = add i64 %20, 8
  %22 = add i64 %try_sp, %21
  %23 = inttoptr i64 %22 to i64*
  %24 = load i64, i64* %23, align 4
  %25 = call i32 @bsearch(i64 %try_sp, i64 %24, i32 0, i32 %19, i32 7)
  call void @printi(i32 %25)
  call void @print(%string* @str.1)
  ret void
}

; ===== Frame Information =====
; tigermain -16 32
; nop 0 24
; init -8 24
; bsearch -8 48
; try 0 40
