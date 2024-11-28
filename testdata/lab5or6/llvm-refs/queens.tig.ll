; ModuleID = 'lightir'
source_filename = "lightir"
target triple = "x86_64-pc-linux-gnu"

%string = type { i32, i8* }

@tigermain_framesize_global = constant i64 64
@printboard_framesize_global = constant i64 40
@0 = private unnamed_addr constant [3 x i8] c" O\00", align 1
@str = constant %string { i32 2, i8* getelementptr inbounds ([3 x i8], [3 x i8]* @0, i32 0, i32 0) }
@1 = private unnamed_addr constant [3 x i8] c" .\00", align 1
@str.1 = constant %string { i32 2, i8* getelementptr inbounds ([3 x i8], [3 x i8]* @1, i32 0, i32 0) }
@2 = private unnamed_addr constant [2 x i8] c"\0A\00", align 1
@str.2 = constant %string { i32 1, i8* getelementptr inbounds ([2 x i8], [2 x i8]* @2, i32 0, i32 0) }
@3 = private unnamed_addr constant [2 x i8] c"\0A\00", align 1
@str.3 = constant %string { i32 1, i8* getelementptr inbounds ([2 x i8], [2 x i8]* @3, i32 0, i32 0) }
@try_framesize_global = constant i64 32

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
  store i32 8, i32* %6, align 4
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
  %18 = load i64, i64* @tigermain_framesize_global, align 4
  %19 = add i64 %18, -8
  %20 = add i64 %tigermain_sp, %19
  %tigermain_N_ptr1 = inttoptr i64 %20 to i32*
  %21 = load i32, i32* %tigermain_N_ptr1, align 4
  %22 = call i64 @init_array(i32 %21, i64 0)
  %23 = inttoptr i64 %22 to i64*
  %24 = ptrtoint i64* %23 to i64
  %25 = load i64, i64* @tigermain_framesize_global, align 4
  %26 = add i64 %25, -24
  %27 = add i64 %tigermain_sp, %26
  %28 = inttoptr i64 %27 to i64*
  store i64 %24, i64* %28, align 4
  %29 = load i64, i64* @tigermain_framesize_global, align 4
  %30 = add i64 %29, -8
  %31 = add i64 %tigermain_sp, %30
  %tigermain_N_ptr2 = inttoptr i64 %31 to i32*
  %32 = load i64, i64* @tigermain_framesize_global, align 4
  %33 = add i64 %32, -8
  %34 = add i64 %tigermain_sp, %33
  %tigermain_N_ptr3 = inttoptr i64 %34 to i32*
  %35 = load i32, i32* %tigermain_N_ptr2, align 4
  %36 = load i32, i32* %tigermain_N_ptr3, align 4
  %37 = add i32 %35, %36
  %38 = sub i32 %37, 1
  %39 = call i64 @init_array(i32 %38, i64 0)
  %40 = inttoptr i64 %39 to i64*
  %41 = ptrtoint i64* %40 to i64
  %42 = load i64, i64* @tigermain_framesize_global, align 4
  %43 = add i64 %42, -32
  %44 = add i64 %tigermain_sp, %43
  %45 = inttoptr i64 %44 to i64*
  store i64 %41, i64* %45, align 4
  %46 = load i64, i64* @tigermain_framesize_global, align 4
  %47 = add i64 %46, -8
  %48 = add i64 %tigermain_sp, %47
  %tigermain_N_ptr4 = inttoptr i64 %48 to i32*
  %49 = load i64, i64* @tigermain_framesize_global, align 4
  %50 = add i64 %49, -8
  %51 = add i64 %tigermain_sp, %50
  %tigermain_N_ptr5 = inttoptr i64 %51 to i32*
  %52 = load i32, i32* %tigermain_N_ptr4, align 4
  %53 = load i32, i32* %tigermain_N_ptr5, align 4
  %54 = add i32 %52, %53
  %55 = sub i32 %54, 1
  %56 = call i64 @init_array(i32 %55, i64 0)
  %57 = inttoptr i64 %56 to i64*
  %58 = ptrtoint i64* %57 to i64
  %59 = load i64, i64* @tigermain_framesize_global, align 4
  %60 = add i64 %59, -40
  %61 = add i64 %tigermain_sp, %60
  %62 = inttoptr i64 %61 to i64*
  store i64 %58, i64* %62, align 4
  call void @try(i64 %tigermain_sp, i64 %tigermain_sp, i32 0)
  ret i32 0
}

define void @printboard(i64 %0, i64 %1) {
printboard:
  %printboard_local_framesize = load i64, i64* @printboard_framesize_global, align 4
  %printboard_sp = sub i64 %0, %printboard_local_framesize
  %2 = load i64, i64* @printboard_framesize_global, align 4
  %3 = add i64 %2, 8
  %4 = add i64 %printboard_sp, %3
  %sl_ptr = inttoptr i64 %4 to i64*
  store i64 %1, i64* %sl_ptr, align 4
  %5 = load i64, i64* @printboard_framesize_global, align 4
  %6 = add i64 %5, 8
  %7 = add i64 %printboard_sp, %6
  %8 = inttoptr i64 %7 to i64*
  %9 = load i64, i64* %8, align 4
  %10 = load i64, i64* @tigermain_framesize_global, align 4
  %11 = add i64 %10, -8
  %12 = add i64 %9, %11
  %tigermain_N_ptr = inttoptr i64 %12 to i32*
  %13 = load i32, i32* %tigermain_N_ptr, align 4
  %14 = sub i32 %13, 1
  %15 = load i64, i64* @printboard_framesize_global, align 4
  %16 = add i64 %15, -8
  %17 = add i64 %printboard_sp, %16
  %18 = inttoptr i64 %17 to i32*
  store i32 0, i32* %18, align 4
  br label %for_body

for_incre:                                        ; preds = %for_next4
  %19 = load i32, i32* %18, align 4
  %20 = add i32 1, %19
  store i32 %20, i32* %18, align 4
  %21 = icmp sle i32 %20, %14
  br i1 %21, label %for_body, label %for_next

for_body:                                         ; preds = %for_incre, %printboard
  %22 = load i64, i64* @printboard_framesize_global, align 4
  %23 = add i64 %22, 8
  %24 = add i64 %printboard_sp, %23
  %25 = inttoptr i64 %24 to i64*
  %26 = load i64, i64* %25, align 4
  %27 = load i64, i64* @tigermain_framesize_global, align 4
  %28 = add i64 %27, -8
  %29 = add i64 %26, %28
  %tigermain_N_ptr1 = inttoptr i64 %29 to i32*
  %30 = load i32, i32* %tigermain_N_ptr1, align 4
  %31 = sub i32 %30, 1
  %32 = load i64, i64* @printboard_framesize_global, align 4
  %33 = add i64 %32, -16
  %34 = add i64 %printboard_sp, %33
  %35 = inttoptr i64 %34 to i32*
  store i32 0, i32* %35, align 4
  br label %for_body3

for_next:                                         ; preds = %for_incre
  call void @print(%string* @str.3)
  ret void

for_incre2:                                       ; preds = %if_next
  %36 = load i32, i32* %35, align 4
  %37 = add i32 1, %36
  store i32 %37, i32* %35, align 4
  %38 = icmp sle i32 %37, %31
  br i1 %38, label %for_body3, label %for_next4

for_body3:                                        ; preds = %for_incre2, %for_body
  br label %if_test

for_next4:                                        ; preds = %for_incre2
  call void @print(%string* @str.2)
  br label %for_incre

if_test:                                          ; preds = %for_body3
  %39 = load i64, i64* @printboard_framesize_global, align 4
  %40 = add i64 %39, 8
  %41 = add i64 %printboard_sp, %40
  %42 = inttoptr i64 %41 to i64*
  %43 = load i64, i64* %42, align 4
  %44 = load i64, i64* @tigermain_framesize_global, align 4
  %45 = add i64 %44, -24
  %46 = add i64 %43, %45
  %tigermain_col_ptr = inttoptr i64 %46 to i32**
  %47 = load i64, i64* @printboard_framesize_global, align 4
  %48 = add i64 %47, -8
  %49 = add i64 %printboard_sp, %48
  %printboard_i_ptr = inttoptr i64 %49 to i32*
  %50 = load i32*, i32** %tigermain_col_ptr, align 8
  %51 = load i32, i32* %printboard_i_ptr, align 4
  %52 = getelementptr i32, i32* %50, i32 %51
  %53 = load i64, i64* @printboard_framesize_global, align 4
  %54 = add i64 %53, -16
  %55 = add i64 %printboard_sp, %54
  %printboard_j_ptr = inttoptr i64 %55 to i32*
  %56 = load i32, i32* %52, align 4
  %57 = load i32, i32* %printboard_j_ptr, align 4
  %58 = icmp eq i32 %56, %57
  br i1 %58, label %if_then, label %if_else

if_then:                                          ; preds = %if_test
  br label %if_next

if_else:                                          ; preds = %if_test
  br label %if_next

if_next:                                          ; preds = %if_else, %if_then
  %59 = phi %string* [ @str, %if_then ], [ @str.1, %if_else ]
  call void @print(%string* %59)
  br label %for_incre2
}

define void @try(i64 %0, i64 %1, i32 %2) {
try:
  %try_local_framesize = load i64, i64* @try_framesize_global, align 4
  %try_sp = sub i64 %0, %try_local_framesize
  %3 = load i64, i64* @try_framesize_global, align 4
  %4 = add i64 %3, 8
  %5 = add i64 %try_sp, %4
  %sl_ptr = inttoptr i64 %5 to i64*
  store i64 %1, i64* %sl_ptr, align 4
  %6 = load i64, i64* @try_framesize_global, align 4
  %7 = add i64 %6, 16
  %8 = add i64 %try_sp, %7
  %9 = inttoptr i64 %8 to i32*
  store i32 %2, i32* %9, align 4
  br label %if_test

if_test:                                          ; preds = %try
  %10 = load i64, i64* @try_framesize_global, align 4
  %11 = add i64 %10, 16
  %12 = add i64 %try_sp, %11
  %try_c_ptr = inttoptr i64 %12 to i32*
  %13 = load i64, i64* @try_framesize_global, align 4
  %14 = add i64 %13, 8
  %15 = add i64 %try_sp, %14
  %16 = inttoptr i64 %15 to i64*
  %17 = load i64, i64* %16, align 4
  %18 = load i64, i64* @tigermain_framesize_global, align 4
  %19 = add i64 %18, -8
  %20 = add i64 %17, %19
  %tigermain_N_ptr = inttoptr i64 %20 to i32*
  %21 = load i32, i32* %try_c_ptr, align 4
  %22 = load i32, i32* %tigermain_N_ptr, align 4
  %23 = icmp eq i32 %21, %22
  br i1 %23, label %if_then, label %if_else

if_then:                                          ; preds = %if_test
  %24 = load i64, i64* @try_framesize_global, align 4
  %25 = add i64 %24, 8
  %26 = add i64 %try_sp, %25
  %27 = inttoptr i64 %26 to i64*
  %28 = load i64, i64* %27, align 4
  call void @printboard(i64 %try_sp, i64 %28)
  br label %if_next

if_else:                                          ; preds = %if_test
  %29 = load i64, i64* @try_framesize_global, align 4
  %30 = add i64 %29, 8
  %31 = add i64 %try_sp, %30
  %32 = inttoptr i64 %31 to i64*
  %33 = load i64, i64* %32, align 4
  %34 = load i64, i64* @tigermain_framesize_global, align 4
  %35 = add i64 %34, -8
  %36 = add i64 %33, %35
  %tigermain_N_ptr1 = inttoptr i64 %36 to i32*
  %37 = load i32, i32* %tigermain_N_ptr1, align 4
  %38 = sub i32 %37, 1
  %39 = load i64, i64* @try_framesize_global, align 4
  %40 = add i64 %39, -8
  %41 = add i64 %try_sp, %40
  %42 = inttoptr i64 %41 to i32*
  store i32 0, i32* %42, align 4
  br label %for_body

if_next:                                          ; preds = %for_next, %if_then
  ret void

for_incre:                                        ; preds = %if_next4
  %43 = load i32, i32* %42, align 4
  %44 = add i32 1, %43
  store i32 %44, i32* %42, align 4
  %45 = icmp sle i32 %44, %38
  br i1 %45, label %for_body, label %for_next

for_body:                                         ; preds = %for_incre, %if_else
  br label %if_test2

for_next:                                         ; preds = %for_incre
  br label %if_next

if_test2:                                         ; preds = %for_body
  %46 = load i64, i64* @try_framesize_global, align 4
  %47 = add i64 %46, 8
  %48 = add i64 %try_sp, %47
  %49 = inttoptr i64 %48 to i64*
  %50 = load i64, i64* %49, align 4
  %51 = load i64, i64* @tigermain_framesize_global, align 4
  %52 = add i64 %51, -16
  %53 = add i64 %50, %52
  %tigermain_row_ptr = inttoptr i64 %53 to i32**
  %54 = load i64, i64* @try_framesize_global, align 4
  %55 = add i64 %54, -8
  %56 = add i64 %try_sp, %55
  %try_r_ptr = inttoptr i64 %56 to i32*
  %57 = load i32*, i32** %tigermain_row_ptr, align 8
  %58 = load i32, i32* %try_r_ptr, align 4
  %59 = getelementptr i32, i32* %57, i32 %58
  %60 = load i32, i32* %59, align 4
  %61 = icmp eq i32 %60, 0
  br i1 %61, label %opand_right_test, label %opand_next

if_then3:                                         ; preds = %opand_next8
  %62 = load i64, i64* @try_framesize_global, align 4
  %63 = add i64 %62, 8
  %64 = add i64 %try_sp, %63
  %65 = inttoptr i64 %64 to i64*
  %66 = load i64, i64* %65, align 4
  %67 = load i64, i64* @tigermain_framesize_global, align 4
  %68 = add i64 %67, -16
  %69 = add i64 %66, %68
  %tigermain_row_ptr11 = inttoptr i64 %69 to i32**
  %70 = load i64, i64* @try_framesize_global, align 4
  %71 = add i64 %70, -8
  %72 = add i64 %try_sp, %71
  %try_r_ptr12 = inttoptr i64 %72 to i32*
  %73 = load i32*, i32** %tigermain_row_ptr11, align 8
  %74 = load i32, i32* %try_r_ptr12, align 4
  %75 = getelementptr i32, i32* %73, i32 %74
  store i32 1, i32* %75, align 4
  %76 = load i64, i64* @try_framesize_global, align 4
  %77 = add i64 %76, 8
  %78 = add i64 %try_sp, %77
  %79 = inttoptr i64 %78 to i64*
  %80 = load i64, i64* %79, align 4
  %81 = load i64, i64* @tigermain_framesize_global, align 4
  %82 = add i64 %81, -32
  %83 = add i64 %80, %82
  %tigermain_diag1_ptr13 = inttoptr i64 %83 to i32**
  %84 = load i64, i64* @try_framesize_global, align 4
  %85 = add i64 %84, -8
  %86 = add i64 %try_sp, %85
  %try_r_ptr14 = inttoptr i64 %86 to i32*
  %87 = load i64, i64* @try_framesize_global, align 4
  %88 = add i64 %87, 16
  %89 = add i64 %try_sp, %88
  %try_c_ptr15 = inttoptr i64 %89 to i32*
  %90 = load i32, i32* %try_r_ptr14, align 4
  %91 = load i32, i32* %try_c_ptr15, align 4
  %92 = add i32 %90, %91
  %93 = load i32*, i32** %tigermain_diag1_ptr13, align 8
  %94 = getelementptr i32, i32* %93, i32 %92
  store i32 1, i32* %94, align 4
  %95 = load i64, i64* @try_framesize_global, align 4
  %96 = add i64 %95, 8
  %97 = add i64 %try_sp, %96
  %98 = inttoptr i64 %97 to i64*
  %99 = load i64, i64* %98, align 4
  %100 = load i64, i64* @tigermain_framesize_global, align 4
  %101 = add i64 %100, -40
  %102 = add i64 %99, %101
  %tigermain_diag2_ptr16 = inttoptr i64 %102 to i32**
  %103 = load i64, i64* @try_framesize_global, align 4
  %104 = add i64 %103, -8
  %105 = add i64 %try_sp, %104
  %try_r_ptr17 = inttoptr i64 %105 to i32*
  %106 = load i32, i32* %try_r_ptr17, align 4
  %107 = add i32 %106, 7
  %108 = load i64, i64* @try_framesize_global, align 4
  %109 = add i64 %108, 16
  %110 = add i64 %try_sp, %109
  %try_c_ptr18 = inttoptr i64 %110 to i32*
  %111 = load i32, i32* %try_c_ptr18, align 4
  %112 = sub i32 %107, %111
  %113 = load i32*, i32** %tigermain_diag2_ptr16, align 8
  %114 = getelementptr i32, i32* %113, i32 %112
  store i32 1, i32* %114, align 4
  %115 = load i64, i64* @try_framesize_global, align 4
  %116 = add i64 %115, 8
  %117 = add i64 %try_sp, %116
  %118 = inttoptr i64 %117 to i64*
  %119 = load i64, i64* %118, align 4
  %120 = load i64, i64* @tigermain_framesize_global, align 4
  %121 = add i64 %120, -24
  %122 = add i64 %119, %121
  %tigermain_col_ptr = inttoptr i64 %122 to i32**
  %123 = load i64, i64* @try_framesize_global, align 4
  %124 = add i64 %123, 16
  %125 = add i64 %try_sp, %124
  %try_c_ptr19 = inttoptr i64 %125 to i32*
  %126 = load i32*, i32** %tigermain_col_ptr, align 8
  %127 = load i32, i32* %try_c_ptr19, align 4
  %128 = getelementptr i32, i32* %126, i32 %127
  %129 = load i64, i64* @try_framesize_global, align 4
  %130 = add i64 %129, -8
  %131 = add i64 %try_sp, %130
  %try_r_ptr20 = inttoptr i64 %131 to i32*
  %132 = load i32, i32* %try_r_ptr20, align 4
  store i32 %132, i32* %128, align 4
  %133 = load i64, i64* @try_framesize_global, align 4
  %134 = add i64 %133, 16
  %135 = add i64 %try_sp, %134
  %try_c_ptr21 = inttoptr i64 %135 to i32*
  %136 = load i32, i32* %try_c_ptr21, align 4
  %137 = add i32 %136, 1
  %138 = load i64, i64* @try_framesize_global, align 4
  %139 = add i64 %138, 8
  %140 = add i64 %try_sp, %139
  %141 = inttoptr i64 %140 to i64*
  %142 = load i64, i64* %141, align 4
  call void @try(i64 %try_sp, i64 %142, i32 %137)
  %143 = load i64, i64* @try_framesize_global, align 4
  %144 = add i64 %143, 8
  %145 = add i64 %try_sp, %144
  %146 = inttoptr i64 %145 to i64*
  %147 = load i64, i64* %146, align 4
  %148 = load i64, i64* @tigermain_framesize_global, align 4
  %149 = add i64 %148, -16
  %150 = add i64 %147, %149
  %tigermain_row_ptr22 = inttoptr i64 %150 to i32**
  %151 = load i64, i64* @try_framesize_global, align 4
  %152 = add i64 %151, -8
  %153 = add i64 %try_sp, %152
  %try_r_ptr23 = inttoptr i64 %153 to i32*
  %154 = load i32*, i32** %tigermain_row_ptr22, align 8
  %155 = load i32, i32* %try_r_ptr23, align 4
  %156 = getelementptr i32, i32* %154, i32 %155
  store i32 0, i32* %156, align 4
  %157 = load i64, i64* @try_framesize_global, align 4
  %158 = add i64 %157, 8
  %159 = add i64 %try_sp, %158
  %160 = inttoptr i64 %159 to i64*
  %161 = load i64, i64* %160, align 4
  %162 = load i64, i64* @tigermain_framesize_global, align 4
  %163 = add i64 %162, -32
  %164 = add i64 %161, %163
  %tigermain_diag1_ptr24 = inttoptr i64 %164 to i32**
  %165 = load i64, i64* @try_framesize_global, align 4
  %166 = add i64 %165, -8
  %167 = add i64 %try_sp, %166
  %try_r_ptr25 = inttoptr i64 %167 to i32*
  %168 = load i64, i64* @try_framesize_global, align 4
  %169 = add i64 %168, 16
  %170 = add i64 %try_sp, %169
  %try_c_ptr26 = inttoptr i64 %170 to i32*
  %171 = load i32, i32* %try_r_ptr25, align 4
  %172 = load i32, i32* %try_c_ptr26, align 4
  %173 = add i32 %171, %172
  %174 = load i32*, i32** %tigermain_diag1_ptr24, align 8
  %175 = getelementptr i32, i32* %174, i32 %173
  store i32 0, i32* %175, align 4
  %176 = load i64, i64* @try_framesize_global, align 4
  %177 = add i64 %176, 8
  %178 = add i64 %try_sp, %177
  %179 = inttoptr i64 %178 to i64*
  %180 = load i64, i64* %179, align 4
  %181 = load i64, i64* @tigermain_framesize_global, align 4
  %182 = add i64 %181, -40
  %183 = add i64 %180, %182
  %tigermain_diag2_ptr27 = inttoptr i64 %183 to i32**
  %184 = load i64, i64* @try_framesize_global, align 4
  %185 = add i64 %184, -8
  %186 = add i64 %try_sp, %185
  %try_r_ptr28 = inttoptr i64 %186 to i32*
  %187 = load i32, i32* %try_r_ptr28, align 4
  %188 = add i32 %187, 7
  %189 = load i64, i64* @try_framesize_global, align 4
  %190 = add i64 %189, 16
  %191 = add i64 %try_sp, %190
  %try_c_ptr29 = inttoptr i64 %191 to i32*
  %192 = load i32, i32* %try_c_ptr29, align 4
  %193 = sub i32 %188, %192
  %194 = load i32*, i32** %tigermain_diag2_ptr27, align 8
  %195 = getelementptr i32, i32* %194, i32 %193
  store i32 0, i32* %195, align 4
  br label %if_next4

if_next4:                                         ; preds = %if_then3, %opand_next8
  br label %for_incre

opand_right_test:                                 ; preds = %if_test2
  %196 = load i64, i64* @try_framesize_global, align 4
  %197 = add i64 %196, 8
  %198 = add i64 %try_sp, %197
  %199 = inttoptr i64 %198 to i64*
  %200 = load i64, i64* %199, align 4
  %201 = load i64, i64* @tigermain_framesize_global, align 4
  %202 = add i64 %201, -32
  %203 = add i64 %200, %202
  %tigermain_diag1_ptr = inttoptr i64 %203 to i32**
  %204 = load i64, i64* @try_framesize_global, align 4
  %205 = add i64 %204, -8
  %206 = add i64 %try_sp, %205
  %try_r_ptr5 = inttoptr i64 %206 to i32*
  %207 = load i64, i64* @try_framesize_global, align 4
  %208 = add i64 %207, 16
  %209 = add i64 %try_sp, %208
  %try_c_ptr6 = inttoptr i64 %209 to i32*
  %210 = load i32, i32* %try_r_ptr5, align 4
  %211 = load i32, i32* %try_c_ptr6, align 4
  %212 = add i32 %210, %211
  %213 = load i32*, i32** %tigermain_diag1_ptr, align 8
  %214 = getelementptr i32, i32* %213, i32 %212
  %215 = load i32, i32* %214, align 4
  %216 = icmp eq i32 %215, 0
  br label %opand_next

opand_next:                                       ; preds = %opand_right_test, %if_test2
  %217 = phi i1 [ false, %if_test2 ], [ %216, %opand_right_test ]
  br i1 %217, label %opand_right_test7, label %opand_next8

opand_right_test7:                                ; preds = %opand_next
  %218 = load i64, i64* @try_framesize_global, align 4
  %219 = add i64 %218, 8
  %220 = add i64 %try_sp, %219
  %221 = inttoptr i64 %220 to i64*
  %222 = load i64, i64* %221, align 4
  %223 = load i64, i64* @tigermain_framesize_global, align 4
  %224 = add i64 %223, -40
  %225 = add i64 %222, %224
  %tigermain_diag2_ptr = inttoptr i64 %225 to i32**
  %226 = load i64, i64* @try_framesize_global, align 4
  %227 = add i64 %226, -8
  %228 = add i64 %try_sp, %227
  %try_r_ptr9 = inttoptr i64 %228 to i32*
  %229 = load i32, i32* %try_r_ptr9, align 4
  %230 = add i32 %229, 7
  %231 = load i64, i64* @try_framesize_global, align 4
  %232 = add i64 %231, 16
  %233 = add i64 %try_sp, %232
  %try_c_ptr10 = inttoptr i64 %233 to i32*
  %234 = load i32, i32* %try_c_ptr10, align 4
  %235 = sub i32 %230, %234
  %236 = load i32*, i32** %tigermain_diag2_ptr, align 8
  %237 = getelementptr i32, i32* %236, i32 %235
  %238 = load i32, i32* %237, align 4
  %239 = icmp eq i32 %238, 0
  br label %opand_next8

opand_next8:                                      ; preds = %opand_right_test7, %opand_next
  %240 = phi i1 [ false, %opand_next ], [ %239, %opand_right_test7 ]
  br i1 %240, label %if_then3, label %if_next4
}

; ===== Frame Information =====
; tigermain -40 64
; printboard -16 40
; try -8 32
