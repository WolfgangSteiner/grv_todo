#ifndef TODO_H
#define TODO_H

#include "grv/grv.h"

typedef struct {
    grv_str_t id;
    grv_str_t title;
    grv_str_t description;
    grv_str_t type;
    grv_str_t status;
    grv_str_t created;
    grv_str_t resolved;
    grv_str_t due;
    grv_strarr_t tags;
    f32 priority;
} todo_t;

typedef struct {
    todo_t* todo;
    int error;
} todo_return_t;

typedef struct {
    todo_t** arr;
    size_t size;
    size_t capacity;
    grv_error_t error;
} todoarr_t;

grv_str_t todo_get_path(void);
grv_str_t todo_file_path(todo_t* todo);
grv_str_t todo_format_short(todo_t* todo);
grv_str_t todo_format_id(todo_t* todo);
grv_str_t todo_format_short_id(todo_t* todo);
todo_t* todo_create(grv_str_t title);
void todo_resolve(todo_t* todo);
void todo_remove_file(todo_t* todo);

grv_str_t todo_serialize(todo_t* todo);
void todo_print(todo_t* todo);
int todo_parse_field(todo_t* todo, grv_str_t line);
todo_t* todo_deserialize(grv_str_t str);
todoarr_t todoarr_read(grv_str_t id_prefix);
todoarr_t todoarr_select_by_status(todoarr_t arr, grv_str_t status);
void todoarr_list(todoarr_t arr);

todo_return_t todo_load(grv_str_t filename);
int todo_write(todo_t* todo);

bool todo_id_valid(grv_str_t id_str);

#endif
