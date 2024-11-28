; ModuleID = 'lightir'
source_filename = "lightir"
target triple = "x86_64-pc-linux-gnu"

%string = type { i32, i8* }

@tigermain_framesize_global = constant i64 32
@nop_framesize_global = constant i64 24
@0 = private unnamed_addr constant [1 x i8] zeroinitializer, align 1
@str = constant %string { i32 0, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @0, i32 0, i32 0) }
@init_framesize_global = constant i64 24
@quicksort_framesize_global = constant i64 56
@dosort_framesize_global = constant i64 40
@1 = private unnamed_addr constant [2 x i8] c" \00", align 1
@str.1 = constant %string { i32 1, i8* getelementptr inbounds ([2 x i8], [2 x i8]* @1, i32 0, i32 0) }
@2 = private unnamed_addr constant [2 x i8] c"\0A\00", align 1
@str.2 = constant %string { i32 1, i8* getelementptr inbounds ([2 x i8], [2 x i8]* @2, i32 0, i32 0) }

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
  call void @dosort(i64 %tigermain_sp, i64 %tigermain_sp)
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
  %37 = add i64 %36, 8
  %38 = add i64 %init_sp, %37
  %39 = inttoptr i64 %38 to i64*
  %40 = load i64, i64* %39, align 4
  %41 = load i64, i64* @tigermain_framesize_global, align 4
  %42 = add i64 %41, -8
  %43 = add i64 %40, %42
  %tigermain_N_ptr1 = inttoptr i64 %43 to i32*
  %44 = load i64, i64* @init_framesize_global, align 4
  %45 = add i64 %44, -8
  %46 = add i64 %init_sp, %45
  %init_i_ptr2 = inttoptr i64 %46 to i32*
  %47 = load i32, i32* %tigermain_N_ptr1, align 4
  %48 = load i32, i32* %init_i_ptr2, align 4
  %49 = sub i32 %47, %48
  store i32 %49, i32* %35, align 4
  %50 = load i64, i64* @init_framesize_global, align 4
  %51 = add i64 %50, 8
  %52 = add i64 %init_sp, %51
  %53 = inttoptr i64 %52 to i64*
  %54 = load i64, i64* %53, align 4
  call void @nop(i64 %init_sp, i64 %54)
  br label %for_incre

for_next:                                         ; preds = %for_incre
  ret void
}

define void @quicksort(i64 %0, i64 %1, i32 %2, i32 %3) {
quicksort:
  %quicksort_local_framesize = load i64, i64* @quicksort_framesize_global, align 4
  %quicksort_sp = sub i64 %0, %quicksort_local_framesize
  %4 = load i64, i64* @quicksort_framesize_global, align 4
  %5 = add i64 %4, 8
  %6 = add i64 %quicksort_sp, %5
  %sl_ptr = inttoptr i64 %6 to i64*
  store i64 %1, i64* %sl_ptr, align 4
  %7 = load i64, i64* @quicksort_framesize_global, align 4
  %8 = add i64 %7, 16
  %9 = add i64 %quicksort_sp, %8
  %10 = inttoptr i64 %9 to i32*
  store i32 %2, i32* %10, align 4
  %11 = load i64, i64* @quicksort_framesize_global, align 4
  %12 = add i64 %11, 24
  %13 = add i64 %quicksort_sp, %12
  %14 = inttoptr i64 %13 to i32*
  store i32 %3, i32* %14, align 4
  %15 = load i64, i64* @quicksort_framesize_global, align 4
  %16 = add i64 %15, 16
  %17 = add i64 %quicksort_sp, %16
  %quicksort_left_ptr = inttoptr i64 %17 to i32*
  %18 = load i32, i32* %quicksort_left_ptr, align 4
  %19 = load i64, i64* @quicksort_framesize_global, align 4
  %20 = add i64 %19, -8
  %21 = add i64 %quicksort_sp, %20
  %22 = inttoptr i64 %21 to i32*
  store i32 %18, i32* %22, align 4
  %23 = load i64, i64* @quicksort_framesize_global, align 4
  %24 = add i64 %23, 24
  %25 = add i64 %quicksort_sp, %24
  %quicksort_right_ptr = inttoptr i64 %25 to i32*
  %26 = load i32, i32* %quicksort_right_ptr, align 4
  %27 = load i64, i64* @quicksort_framesize_global, align 4
  %28 = add i64 %27, -16
  %29 = add i64 %quicksort_sp, %28
  %30 = inttoptr i64 %29 to i32*
  store i32 %26, i32* %30, align 4
  %31 = load i64, i64* @quicksort_framesize_global, align 4
  %32 = add i64 %31, 8
  %33 = add i64 %quicksort_sp, %32
  %34 = inttoptr i64 %33 to i64*
  %35 = load i64, i64* %34, align 4
  %36 = load i64, i64* @tigermain_framesize_global, align 4
  %37 = add i64 %36, -16
  %38 = add i64 %35, %37
  %tigermain_list_ptr = inttoptr i64 %38 to i32**
  %39 = load i64, i64* @quicksort_framesize_global, align 4
  %40 = add i64 %39, 16
  %41 = add i64 %quicksort_sp, %40
  %quicksort_left_ptr1 = inttoptr i64 %41 to i32*
  %42 = load i32*, i32** %tigermain_list_ptr, align 8
  %43 = load i32, i32* %quicksort_left_ptr1, align 4
  %44 = getelementptr i32, i32* %42, i32 %43
  %45 = load i32, i32* %44, align 4
  %46 = load i64, i64* @quicksort_framesize_global, align 4
  %47 = add i64 %46, -24
  %48 = add i64 %quicksort_sp, %47
  %49 = inttoptr i64 %48 to i32*
  store i32 %45, i32* %49, align 4
  br label %if_test

if_test:                                          ; preds = %quicksort
  %50 = load i64, i64* @quicksort_framesize_global, align 4
  %51 = add i64 %50, 16
  %52 = add i64 %quicksort_sp, %51
  %quicksort_left_ptr2 = inttoptr i64 %52 to i32*
  %53 = load i64, i64* @quicksort_framesize_global, align 4
  %54 = add i64 %53, 24
  %55 = add i64 %quicksort_sp, %54
  %quicksort_right_ptr3 = inttoptr i64 %55 to i32*
  %56 = load i32, i32* %quicksort_left_ptr2, align 4
  %57 = load i32, i32* %quicksort_right_ptr3, align 4
  %58 = icmp slt i32 %56, %57
  br i1 %58, label %if_then, label %if_next

if_then:                                          ; preds = %if_test
  br label %while_test

if_next:                                          ; preds = %while_next, %if_test
  ret void

while_test:                                       ; preds = %while_next19, %if_then
  %59 = load i64, i64* @quicksort_framesize_global, align 4
  %60 = add i64 %59, -8
  %61 = add i64 %quicksort_sp, %60
  %quicksort_i_ptr = inttoptr i64 %61 to i32*
  %62 = load i64, i64* @quicksort_framesize_global, align 4
  %63 = add i64 %62, -16
  %64 = add i64 %quicksort_sp, %63
  %quicksort_j_ptr = inttoptr i64 %64 to i32*
  %65 = load i32, i32* %quicksort_i_ptr, align 4
  %66 = load i32, i32* %quicksort_j_ptr, align 4
  %67 = icmp slt i32 %65, %66
  br i1 %67, label %while_body, label %while_next

while_body:                                       ; preds = %while_test
  br label %while_test4

while_next:                                       ; preds = %while_test
  %68 = load i64, i64* @quicksort_framesize_global, align 4
  %69 = add i64 %68, 8
  %70 = add i64 %quicksort_sp, %69
  %71 = inttoptr i64 %70 to i64*
  %72 = load i64, i64* %71, align 4
  %73 = load i64, i64* @tigermain_framesize_global, align 4
  %74 = add i64 %73, -16
  %75 = add i64 %72, %74
  %tigermain_list_ptr33 = inttoptr i64 %75 to i32**
  %76 = load i64, i64* @quicksort_framesize_global, align 4
  %77 = add i64 %76, -8
  %78 = add i64 %quicksort_sp, %77
  %quicksort_i_ptr34 = inttoptr i64 %78 to i32*
  %79 = load i32*, i32** %tigermain_list_ptr33, align 8
  %80 = load i32, i32* %quicksort_i_ptr34, align 4
  %81 = getelementptr i32, i32* %79, i32 %80
  %82 = load i64, i64* @quicksort_framesize_global, align 4
  %83 = add i64 %82, -24
  %84 = add i64 %quicksort_sp, %83
  %quicksort_key_ptr35 = inttoptr i64 %84 to i32*
  %85 = load i32, i32* %quicksort_key_ptr35, align 4
  store i32 %85, i32* %81, align 4
  %86 = load i64, i64* @quicksort_framesize_global, align 4
  %87 = add i64 %86, 16
  %88 = add i64 %quicksort_sp, %87
  %quicksort_left_ptr36 = inttoptr i64 %88 to i32*
  %actual_parm = load i32, i32* %quicksort_left_ptr36, align 4
  %89 = load i64, i64* @quicksort_framesize_global, align 4
  %90 = add i64 %89, -8
  %91 = add i64 %quicksort_sp, %90
  %quicksort_i_ptr37 = inttoptr i64 %91 to i32*
  %92 = load i32, i32* %quicksort_i_ptr37, align 4
  %93 = sub i32 %92, 1
  %94 = load i64, i64* @quicksort_framesize_global, align 4
  %95 = add i64 %94, 8
  %96 = add i64 %quicksort_sp, %95
  %97 = inttoptr i64 %96 to i64*
  %98 = load i64, i64* %97, align 4
  call void @quicksort(i64 %quicksort_sp, i64 %98, i32 %actual_parm, i32 %93)
  %99 = load i64, i64* @quicksort_framesize_global, align 4
  %100 = add i64 %99, -8
  %101 = add i64 %quicksort_sp, %100
  %quicksort_i_ptr38 = inttoptr i64 %101 to i32*
  %102 = load i32, i32* %quicksort_i_ptr38, align 4
  %103 = add i32 %102, 1
  %104 = load i64, i64* @quicksort_framesize_global, align 4
  %105 = add i64 %104, 24
  %106 = add i64 %quicksort_sp, %105
  %quicksort_right_ptr39 = inttoptr i64 %106 to i32*
  %actual_parm40 = load i32, i32* %quicksort_right_ptr39, align 4
  %107 = load i64, i64* @quicksort_framesize_global, align 4
  %108 = add i64 %107, 8
  %109 = add i64 %quicksort_sp, %108
  %110 = inttoptr i64 %109 to i64*
  %111 = load i64, i64* %110, align 4
  call void @quicksort(i64 %quicksort_sp, i64 %111, i32 %103, i32 %actual_parm40)
  br label %if_next

while_test4:                                      ; preds = %while_body5, %while_body
  %112 = load i64, i64* @quicksort_framesize_global, align 4
  %113 = add i64 %112, -8
  %114 = add i64 %quicksort_sp, %113
  %quicksort_i_ptr7 = inttoptr i64 %114 to i32*
  %115 = load i64, i64* @quicksort_framesize_global, align 4
  %116 = add i64 %115, -16
  %117 = add i64 %quicksort_sp, %116
  %quicksort_j_ptr8 = inttoptr i64 %117 to i32*
  %118 = load i32, i32* %quicksort_i_ptr7, align 4
  %119 = load i32, i32* %quicksort_j_ptr8, align 4
  %120 = icmp slt i32 %118, %119
  br i1 %120, label %opand_right_test, label %opand_next

while_body5:                                      ; preds = %opand_next
  %121 = load i64, i64* @quicksort_framesize_global, align 4
  %122 = add i64 %121, -16
  %123 = add i64 %quicksort_sp, %122
  %quicksort_j_ptr11 = inttoptr i64 %123 to i32*
  %124 = load i64, i64* @quicksort_framesize_global, align 4
  %125 = add i64 %124, -16
  %126 = add i64 %quicksort_sp, %125
  %quicksort_j_ptr12 = inttoptr i64 %126 to i32*
  %127 = load i32, i32* %quicksort_j_ptr12, align 4
  %128 = sub i32 %127, 1
  store i32 %128, i32* %quicksort_j_ptr11, align 4
  br label %while_test4

while_next6:                                      ; preds = %opand_next
  %129 = load i64, i64* @quicksort_framesize_global, align 4
  %130 = add i64 %129, 8
  %131 = add i64 %quicksort_sp, %130
  %132 = inttoptr i64 %131 to i64*
  %133 = load i64, i64* %132, align 4
  %134 = load i64, i64* @tigermain_framesize_global, align 4
  %135 = add i64 %134, -16
  %136 = add i64 %133, %135
  %tigermain_list_ptr13 = inttoptr i64 %136 to i32**
  %137 = load i64, i64* @quicksort_framesize_global, align 4
  %138 = add i64 %137, -8
  %139 = add i64 %quicksort_sp, %138
  %quicksort_i_ptr14 = inttoptr i64 %139 to i32*
  %140 = load i32*, i32** %tigermain_list_ptr13, align 8
  %141 = load i32, i32* %quicksort_i_ptr14, align 4
  %142 = getelementptr i32, i32* %140, i32 %141
  %143 = load i64, i64* @quicksort_framesize_global, align 4
  %144 = add i64 %143, 8
  %145 = add i64 %quicksort_sp, %144
  %146 = inttoptr i64 %145 to i64*
  %147 = load i64, i64* %146, align 4
  %148 = load i64, i64* @tigermain_framesize_global, align 4
  %149 = add i64 %148, -16
  %150 = add i64 %147, %149
  %tigermain_list_ptr15 = inttoptr i64 %150 to i32**
  %151 = load i64, i64* @quicksort_framesize_global, align 4
  %152 = add i64 %151, -16
  %153 = add i64 %quicksort_sp, %152
  %quicksort_j_ptr16 = inttoptr i64 %153 to i32*
  %154 = load i32*, i32** %tigermain_list_ptr15, align 8
  %155 = load i32, i32* %quicksort_j_ptr16, align 4
  %156 = getelementptr i32, i32* %154, i32 %155
  %157 = load i32, i32* %156, align 4
  store i32 %157, i32* %142, align 4
  br label %while_test17

opand_right_test:                                 ; preds = %while_test4
  %158 = load i64, i64* @quicksort_framesize_global, align 4
  %159 = add i64 %158, -24
  %160 = add i64 %quicksort_sp, %159
  %quicksort_key_ptr = inttoptr i64 %160 to i32*
  %161 = load i64, i64* @quicksort_framesize_global, align 4
  %162 = add i64 %161, 8
  %163 = add i64 %quicksort_sp, %162
  %164 = inttoptr i64 %163 to i64*
  %165 = load i64, i64* %164, align 4
  %166 = load i64, i64* @tigermain_framesize_global, align 4
  %167 = add i64 %166, -16
  %168 = add i64 %165, %167
  %tigermain_list_ptr9 = inttoptr i64 %168 to i32**
  %169 = load i64, i64* @quicksort_framesize_global, align 4
  %170 = add i64 %169, -16
  %171 = add i64 %quicksort_sp, %170
  %quicksort_j_ptr10 = inttoptr i64 %171 to i32*
  %172 = load i32*, i32** %tigermain_list_ptr9, align 8
  %173 = load i32, i32* %quicksort_j_ptr10, align 4
  %174 = getelementptr i32, i32* %172, i32 %173
  %175 = load i32, i32* %quicksort_key_ptr, align 4
  %176 = load i32, i32* %174, align 4
  %177 = icmp sle i32 %175, %176
  br label %opand_next

opand_next:                                       ; preds = %opand_right_test, %while_test4
  %178 = phi i1 [ false, %while_test4 ], [ %177, %opand_right_test ]
  br i1 %178, label %while_body5, label %while_next6

while_test17:                                     ; preds = %while_body18, %while_next6
  %179 = load i64, i64* @quicksort_framesize_global, align 4
  %180 = add i64 %179, -8
  %181 = add i64 %quicksort_sp, %180
  %quicksort_i_ptr20 = inttoptr i64 %181 to i32*
  %182 = load i64, i64* @quicksort_framesize_global, align 4
  %183 = add i64 %182, -16
  %184 = add i64 %quicksort_sp, %183
  %quicksort_j_ptr21 = inttoptr i64 %184 to i32*
  %185 = load i32, i32* %quicksort_i_ptr20, align 4
  %186 = load i32, i32* %quicksort_j_ptr21, align 4
  %187 = icmp slt i32 %185, %186
  br i1 %187, label %opand_right_test22, label %opand_next23

while_body18:                                     ; preds = %opand_next23
  %188 = load i64, i64* @quicksort_framesize_global, align 4
  %189 = add i64 %188, -8
  %190 = add i64 %quicksort_sp, %189
  %quicksort_i_ptr27 = inttoptr i64 %190 to i32*
  %191 = load i64, i64* @quicksort_framesize_global, align 4
  %192 = add i64 %191, -8
  %193 = add i64 %quicksort_sp, %192
  %quicksort_i_ptr28 = inttoptr i64 %193 to i32*
  %194 = load i32, i32* %quicksort_i_ptr28, align 4
  %195 = add i32 %194, 1
  store i32 %195, i32* %quicksort_i_ptr27, align 4
  br label %while_test17

while_next19:                                     ; preds = %opand_next23
  %196 = load i64, i64* @quicksort_framesize_global, align 4
  %197 = add i64 %196, 8
  %198 = add i64 %quicksort_sp, %197
  %199 = inttoptr i64 %198 to i64*
  %200 = load i64, i64* %199, align 4
  %201 = load i64, i64* @tigermain_framesize_global, align 4
  %202 = add i64 %201, -16
  %203 = add i64 %200, %202
  %tigermain_list_ptr29 = inttoptr i64 %203 to i32**
  %204 = load i64, i64* @quicksort_framesize_global, align 4
  %205 = add i64 %204, -16
  %206 = add i64 %quicksort_sp, %205
  %quicksort_j_ptr30 = inttoptr i64 %206 to i32*
  %207 = load i32*, i32** %tigermain_list_ptr29, align 8
  %208 = load i32, i32* %quicksort_j_ptr30, align 4
  %209 = getelementptr i32, i32* %207, i32 %208
  %210 = load i64, i64* @quicksort_framesize_global, align 4
  %211 = add i64 %210, 8
  %212 = add i64 %quicksort_sp, %211
  %213 = inttoptr i64 %212 to i64*
  %214 = load i64, i64* %213, align 4
  %215 = load i64, i64* @tigermain_framesize_global, align 4
  %216 = add i64 %215, -16
  %217 = add i64 %214, %216
  %tigermain_list_ptr31 = inttoptr i64 %217 to i32**
  %218 = load i64, i64* @quicksort_framesize_global, align 4
  %219 = add i64 %218, -8
  %220 = add i64 %quicksort_sp, %219
  %quicksort_i_ptr32 = inttoptr i64 %220 to i32*
  %221 = load i32*, i32** %tigermain_list_ptr31, align 8
  %222 = load i32, i32* %quicksort_i_ptr32, align 4
  %223 = getelementptr i32, i32* %221, i32 %222
  %224 = load i32, i32* %223, align 4
  store i32 %224, i32* %209, align 4
  br label %while_test

opand_right_test22:                               ; preds = %while_test17
  %225 = load i64, i64* @quicksort_framesize_global, align 4
  %226 = add i64 %225, -24
  %227 = add i64 %quicksort_sp, %226
  %quicksort_key_ptr24 = inttoptr i64 %227 to i32*
  %228 = load i64, i64* @quicksort_framesize_global, align 4
  %229 = add i64 %228, 8
  %230 = add i64 %quicksort_sp, %229
  %231 = inttoptr i64 %230 to i64*
  %232 = load i64, i64* %231, align 4
  %233 = load i64, i64* @tigermain_framesize_global, align 4
  %234 = add i64 %233, -16
  %235 = add i64 %232, %234
  %tigermain_list_ptr25 = inttoptr i64 %235 to i32**
  %236 = load i64, i64* @quicksort_framesize_global, align 4
  %237 = add i64 %236, -8
  %238 = add i64 %quicksort_sp, %237
  %quicksort_i_ptr26 = inttoptr i64 %238 to i32*
  %239 = load i32*, i32** %tigermain_list_ptr25, align 8
  %240 = load i32, i32* %quicksort_i_ptr26, align 4
  %241 = getelementptr i32, i32* %239, i32 %240
  %242 = load i32, i32* %quicksort_key_ptr24, align 4
  %243 = load i32, i32* %241, align 4
  %244 = icmp sge i32 %242, %243
  br label %opand_next23

opand_next23:                                     ; preds = %opand_right_test22, %while_test17
  %245 = phi i1 [ false, %while_test17 ], [ %244, %opand_right_test22 ]
  br i1 %245, label %while_body18, label %while_next19
}

define void @dosort(i64 %0, i64 %1) {
dosort:
  %dosort_local_framesize = load i64, i64* @dosort_framesize_global, align 4
  %dosort_sp = sub i64 %0, %dosort_local_framesize
  %2 = load i64, i64* @dosort_framesize_global, align 4
  %3 = add i64 %2, 8
  %4 = add i64 %dosort_sp, %3
  %sl_ptr = inttoptr i64 %4 to i64*
  store i64 %1, i64* %sl_ptr, align 4
  %5 = load i64, i64* @dosort_framesize_global, align 4
  %6 = add i64 %5, 8
  %7 = add i64 %dosort_sp, %6
  %8 = inttoptr i64 %7 to i64*
  %9 = load i64, i64* %8, align 4
  call void @init(i64 %dosort_sp, i64 %9)
  %10 = load i64, i64* @dosort_framesize_global, align 4
  %11 = add i64 %10, 8
  %12 = add i64 %dosort_sp, %11
  %13 = inttoptr i64 %12 to i64*
  %14 = load i64, i64* %13, align 4
  %15 = load i64, i64* @tigermain_framesize_global, align 4
  %16 = add i64 %15, -8
  %17 = add i64 %14, %16
  %tigermain_N_ptr = inttoptr i64 %17 to i32*
  %18 = load i32, i32* %tigermain_N_ptr, align 4
  %19 = sub i32 %18, 1
  %20 = load i64, i64* @dosort_framesize_global, align 4
  %21 = add i64 %20, 8
  %22 = add i64 %dosort_sp, %21
  %23 = inttoptr i64 %22 to i64*
  %24 = load i64, i64* %23, align 4
  call void @quicksort(i64 %dosort_sp, i64 %24, i32 0, i32 %19)
  %25 = load i64, i64* @dosort_framesize_global, align 4
  %26 = add i64 %25, 8
  %27 = add i64 %dosort_sp, %26
  %28 = inttoptr i64 %27 to i64*
  %29 = load i64, i64* %28, align 4
  %30 = load i64, i64* @tigermain_framesize_global, align 4
  %31 = add i64 %30, -8
  %32 = add i64 %29, %31
  %tigermain_N_ptr1 = inttoptr i64 %32 to i32*
  %33 = load i32, i32* %tigermain_N_ptr1, align 4
  %34 = sub i32 %33, 1
  %35 = load i64, i64* @dosort_framesize_global, align 4
  %36 = add i64 %35, -8
  %37 = add i64 %dosort_sp, %36
  %38 = inttoptr i64 %37 to i32*
  store i32 0, i32* %38, align 4
  br label %for_body

for_incre:                                        ; preds = %for_body
  %39 = load i32, i32* %38, align 4
  %40 = add i32 1, %39
  store i32 %40, i32* %38, align 4
  %41 = icmp sle i32 %40, %34
  br i1 %41, label %for_body, label %for_next

for_body:                                         ; preds = %for_incre, %dosort
  %42 = load i64, i64* @dosort_framesize_global, align 4
  %43 = add i64 %42, 8
  %44 = add i64 %dosort_sp, %43
  %45 = inttoptr i64 %44 to i64*
  %46 = load i64, i64* %45, align 4
  %47 = load i64, i64* @tigermain_framesize_global, align 4
  %48 = add i64 %47, -16
  %49 = add i64 %46, %48
  %tigermain_list_ptr = inttoptr i64 %49 to i32**
  %50 = load i64, i64* @dosort_framesize_global, align 4
  %51 = add i64 %50, -8
  %52 = add i64 %dosort_sp, %51
  %dosort_i_ptr = inttoptr i64 %52 to i32*
  %53 = load i32*, i32** %tigermain_list_ptr, align 8
  %54 = load i32, i32* %dosort_i_ptr, align 4
  %55 = getelementptr i32, i32* %53, i32 %54
  %actual_parm = load i32, i32* %55, align 4
  call void @printi(i32 %actual_parm)
  call void @print(%string* @str.1)
  br label %for_incre

for_next:                                         ; preds = %for_incre
  call void @print(%string* @str.2)
  ret void
}

; ===== Frame Information =====
; tigermain -16 32
; nop 0 24
; init -8 24
; quicksort -24 56
; dosort -8 40
