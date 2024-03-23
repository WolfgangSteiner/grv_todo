#include "grv/grv_test.h"
#include "grv/grv.h"
#include "../src/todo.h"
#include "../src/todo.c"

GRV_TEST_BEGIN_NEW(todo_serialize)
    todo_t todo = {0};
    todo.type = grv_str_ref("todo");
    todo.status = grv_str_ref("open");
    todo.title = grv_str_ref("This is the title.");
    todo.description = grv_str_ref("This is the\ndescription.");
    todo.created = grv_str_ref("20240318T210723");
    todo.tags = grv_str_split(grv_str_ref("tag_a tag_b tag_c"), grv_str_ref(" "));
    todo.due = grv_str_ref("20240319T090000");
    todo.priority = 2.1f;
    grv_str_t str = todo_serialize(&todo);
    grv_strarr_t arr = grv_str_split(str, grv_str_ref("\n"));
    GRV_TEST_EQUAL_STR(arr.arr[0], "This is the title.");
    GRV_TEST_EQUAL_STR(arr.arr[1], "");
    GRV_TEST_EQUAL_STR(arr.arr[2], "This is the");
    GRV_TEST_EQUAL_STR(arr.arr[3], "description.");
    GRV_TEST_EQUAL_STR(arr.arr[4], "");
    GRV_TEST_EQUAL_STR(arr.arr[5], "[[type todo]]");
    GRV_TEST_EQUAL_STR(arr.arr[6], "[[status open]]");
    GRV_TEST_EQUAL_STR(arr.arr[7], "[[created 20240318T210723]]");
    GRV_TEST_EQUAL_STR(arr.arr[8], "[[due 20240319T090000]]");
    GRV_TEST_EQUAL_STR(arr.arr[9], "[[tags tag_a tag_b tag_c]]");
    GRV_TEST_EQUAL_STR(arr.arr[10], "[[priority 2.10]]");
GRV_TEST_END_NEW()
