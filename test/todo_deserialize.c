#include "grv/grv_test.h"
#include "../src/todo.h"
#include "../src/todo.c"

char* todo_sample = 
    "This is the title\n"
    "\n"
    "This is the \n"
    "description.\n"
    "[[type todo]]\n"
    "[[created 20240318T170523]]\n"
    "[[tags TAG1 TAG2 TAG3]]\n"
    "[[priority 2]]\n";

GRV_TEST_BEGIN_NEW(todo_deserialize)
    todo_t* todo = todo_deserialize(grv_str_ref(todo_sample));
    GRV_TEST_EQUAL_STR(todo->title, "This is the title");
    GRV_TEST_EQUAL_STR(todo->description, "This is the \ndescription.");
    GRV_TEST_EQUAL_STR(todo->type, "todo");
    GRV_TEST_EQUAL_STR(todo->created, "20240318T170523");
    GRV_TEST_EQUAL_INT(todo->tags.size, 3);
    GRV_TEST_EQUAL_STR(todo->tags.arr[0], "TAG1");
    GRV_TEST_EQUAL_STR(todo->tags.arr[1], "TAG2");
    GRV_TEST_EQUAL_STR(todo->tags.arr[2], "TAG3");
    GRV_TEST_EQUAL_F32(todo->priority, 2.0f);
GRV_TEST_END_NEW()
