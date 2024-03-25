#include "grv/grv_base.h"
#include "grv/grv_arr.h"
#include "grv/grv_memory.h"
#include "grv/grv_fs.h"
#include "grv/grv_str.h"
#include "grv/grv_util.h"
#include "todo.h"

#define TODO_ID_TYPE_HEX 0
#define TODO_ID_TYPE_SIMPLE 1
#define TODO_ID_TYPE TODO_ID_TYPE_SIMPLE
#define TODO_ID_LENGTH 16

grv_str_t todo_create_id(void) {
    grv_str_t id_str = {0};
    if (TODO_ID_TYPE == TODO_ID_TYPE_SIMPLE) {
        u8 vowls[5] = {'a','e','i','o','u'}; 
        for (size_t i = 0; i < TODO_ID_LENGTH; i+=2) {
            char c1 = (grv_random_u8() % 21) + 'a';
            if (grv_str_contains_char(grv_str_ref("aeiou"), c1)) c1++;
            char c2 = vowls[grv_random_u8() % 5];
            grv_str_append_char(&id_str, c1);
            grv_str_append_char(&id_str, c2);
        }
    } else {
        u64 id = grv_random_u64();
        id_str = grv_str_format(grv_str_ref("{u64:hex}"), id);
        while (id_str.size < TODO_ID_LENGTH) {
            grv_str_prepend_char(&id_str, '0');
        }
    }
    return id_str;
}

todo_t* todo_create(grv_str_t title) {
    todo_t* todo = GRV_ALLOC_OBJECT_ZERO(todo_t);
    todo->id = todo_create_id();
    todo->title = grv_str_copy(title);
    todo->type = grv_str_ref("todo");
    todo->status = grv_str_ref("open");
    todo->created = grv_local_datetime_str();
    return todo;
}

grv_str_t todo_format_short(todo_t* todo) {
    grv_str_t format_str = 
        grv_str_eq_cstr(todo->status, "open")
            ? grv_str_ref("{str:color=yellow} {str}")
            : grv_str_ref("{str:color=green} {str}");
    grv_str_t id_str = grv_str_substr(todo->id, 0, 7);
    grv_str_t res = grv_str_format(format_str, id_str, todo->title);
    grv_str_free(&id_str);
    return res;
}

void append_field(grv_str_t* str, char* field_name, grv_str_t field_str) {
    if (!grv_str_empty(field_str)) {
        grv_str_append_cstr(str, "[[");
        grv_str_append_cstr(str, field_name);
        grv_str_append_space(str);
        grv_str_append_str(str, field_str);
        grv_str_append_cstr(str, "]]\n");
    }
}

grv_str_t todo_serialize(todo_t* todo) {
    grv_str_t res = {0};
    grv_str_append_str(&res, todo->title);
    grv_str_append_cstr(&res, "\n");
    if (!grv_str_empty(todo->description)) {
        grv_str_append_cstr(&res, "\n");
        grv_str_append_str(&res, todo->description);
        grv_str_append_cstr(&res, "\n\n");
    }
    append_field(&res, "type", todo->type);
    append_field(&res, "status", todo->status);
    append_field(&res, "created", todo->created);
    append_field(&res, "due", todo->due);
    if (todo->tags.size) {
        grv_str_t tags_str = grv_strarr_join(todo->tags, grv_str_ref(" "));
        append_field(&res, "tags", tags_str);
        grv_str_free(&tags_str);
    }
    if (todo->priority != 0.0f) {
        grv_str_t prio_str = grv_str_format(grv_str_ref("{f32:.2f}"), todo->priority);
        append_field(&res, "priority", prio_str);
        grv_str_free(&prio_str);
    }

    return res;
}        


enum {
    TODO_PARSE_ERROR,
};

int todo_parse_field(todo_t* todo, grv_str_t line) {
    if (!grv_str_starts_with_cstr(line, "[[") || !grv_str_ends_with_cstr(line, "]]")) {
        return TODO_PARSE_ERROR;
    }

    line = grv_str_substr(line, 2, -3);
    grv_strpair_t key_value = grv_str_split_head_front(line, grv_str_ref(" "));
    grv_str_t key = key_value.first;
    grv_str_t value = key_value.second;

    if (grv_str_eq_cstr(key, "type")) {
        todo->type = value;
    } else if (grv_str_eq_cstr(key, "status")) {
        todo->status = value;
    } else if (grv_str_eq_cstr(key, "created")) {
        todo->created = value;
    } else if (grv_str_eq_cstr(key, "due")) {
        todo->due = value;
    } else if (grv_str_eq_cstr(key, "tags")) {
        todo->tags = grv_strarr_copy(grv_str_split(value, grv_str_ref(" ")));
    } else if (grv_str_eq_cstr(key, "priority")) {
        if (!grv_str_is_float(value)) {
            grv_log_error(grv_str_ref("Invalid priority value"));
            return -1;
        }
        todo->priority = grv_str_to_f32(value);
    } else {
        grv_str_t msg = grv_str_cat("Unknown key: ", key);
        grv_log_error(msg);
        grv_str_free(&msg);
        return -1;
    }
    return 0;
}

todo_t* todo_deserialize(grv_str_t str) {
    todo_t* todo = GRV_ALLOC_OBJECT_ZERO(todo_t);
    grv_str_iter_t iter = grv_str_iter_begin(&str);
    grv_str_t title = grv_str_iter_get_line(&iter);
    todo->title = grv_str_copy(title);
    bool finished_description = false;
    while (!grv_str_iter_is_end(&iter)) {
        grv_str_t line = grv_str_iter_get_line(&iter);
        if (grv_str_empty(line)) {
            continue;
        } else if (grv_str_starts_with_cstr(line, "[[")) {
            finished_description = true;
            int result = todo_parse_field(todo, line);
            if (result) {
                grv_log_error(grv_str_ref("Error parsing field."));
            }
        } else if (!finished_description) {
            if (!grv_str_empty(todo->description)) {
                grv_str_append_newline(&todo->description);
            }
            grv_str_append(&todo->description, line);
        } else {
            grv_log_error(grv_str_ref("Unexpected input:"));
            grv_str_print(line);
        }
    }
    return todo;
}

grv_str_t todo_id_from_filename(grv_str_t filename) {
    grv_str_t id_str = grv_fs_stem(grv_fs_basename(filename));
    return id_str;
}

todo_return_t todo_load(grv_str_t filename) {
    todo_return_t result = {0};
    grv_str_return_t str_return = grv_read_file(filename);
    if (str_return.error != GRV_ERROR_SUCCESS) {
        result.error = str_return.error;
        return result;
    }
    result.todo = todo_deserialize(str_return.str);
    result.todo->id = grv_str_copy(todo_id_from_filename(filename));
    return result;
}

grv_str_t todo_get_path(void) {
    grv_str_t todo_path = grv_str_ref(".todo");
    grv_error_t result = grv_make_path(todo_path);
    if (result != GRV_ERROR_SUCCESS) {
        grv_log_error(grv_str_ref("Could not create .todo directory"));
        exit(1);
    }
    return todo_path;
}

grv_str_t todo_file_path(todo_t* todo) {
    grv_str_t res = todo->id;
    grv_str_append(&res, ".todo");
    grv_path_prepend(&res, todo_get_path());
    return res;
}

int todo_write(todo_t* todo) {
    grv_str_t todo_path = todo_get_path();
    grv_str_t file_path = grv_str_format(grv_str_ref("{str}.todo"), todo->id);
    grv_path_prepend(&file_path, todo_path);
    grv_log_info(grv_str_format(grv_str_ref("Writing todo to file {str}"), file_path));
    grv_str_t str = todo_serialize(todo);
    grv_error_t result = grv_str_write_to_file(str, file_path);
    grv_str_free(&todo_path);
    grv_str_free(&file_path);
    grv_str_free(&str);
    return result;
}

bool todo_matches_template(todo_t* todo, todo_t* template) {
    bool result = true;
    if (!grv_str_empty(template->id)) {
        result &= grv_str_starts_with_str(todo->id, template->id);
    }
    if (!grv_str_empty(template->status)) {
        result &= grv_str_eq_str(todo->status, template->status);
    }
    if (!grv_str_empty(template->type)) {
        result &= grv_str_eq_str(todo->type, template->type);
    }
    return result;
}

todoarr_t todoarr_select_by_status(todoarr_t arr, grv_str_t status) {
    todoarr_t res = {0};
    for (size_t i = 0; i < arr.size; ++i) {
        todo_t* todo = arr.arr[i];
        if (grv_str_eq(todo->status, status)) {
            grv_arr_push(&res, todo);
        }
    }
    return res;
}

todoarr_t todoarr_select_by_template(todoarr_t arr, todo_t template) {
    todoarr_t res = {0};
    for (size_t i = 0; i < arr.size; ++i) {
         todo_t* todo = arr.arr[i];
         if (todo_matches_template(todo, &template)) {
            grv_arr_push(&res, todo);
         }
    }
    return res;
}

todoarr_t todoarr_read(grv_str_t id_prefix) {
    todoarr_t arr = {0};
    grv_read_directory_return_t dirlist = grv_read_directory(todo_get_path());
    if (dirlist.error != GRV_ERROR_SUCCESS) {
        arr.error = dirlist.error;
        return arr;
    }

    for (size_t i = 0; i < dirlist.entries.size; i++) {
        grv_str_t filename = dirlist.entries.arr[i];
        grv_str_t id_str = todo_id_from_filename(filename);
        if (grv_str_empty(id_prefix) || grv_str_starts_with_str(id_str, id_prefix)) {
            todo_return_t todo_res = todo_load(filename);
            if (todo_res.error == GRV_ERROR_SUCCESS) {
                grv_arr_push(&arr, todo_res.todo);
            } else {
                grv_str_t error_msg = grv_str_format(grv_str_ref("Could not parse todo {str}."), filename);
                grv_log_error(error_msg);
                grv_str_free(&error_msg);
            }
        }
    }

    return arr;
}
