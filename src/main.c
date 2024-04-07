#include "grv/grv.h"
#include "grv/grv_base.h"
#include "grv/grv_str.h"
#include "grv/grv_fs.h"
#include "grv/grv_strarr.h"
#include "grv/grv_util.h"
#include "todo.h"

grv_str_t exe_name = {0};

void cmd_edit(grv_strarr_t args);
void cmd_list(grv_strarr_t args);
void cmd_resolve(grv_strarr_t args);

char* usage_update = "update <id> [--title=<new_title>] [--type=<new_type>]";

void print_usage(char* usage_cstr) {
    grv_str_t error_msg = grv_str_format(grv_str_ref("Usage: {str} "), exe_name);
    grv_str_append_cstr(&error_msg, usage_cstr);
    grv_log_error(error_msg);
}

todo_t* todo_get_with_id(grv_str_t id) {
    todoarr_t arr = todoarr_read(id);
    if (arr.size == 0) {
        grv_str_t error_msg = grv_str_ref("No todo found for id {str}.");
        grv_log_error(grv_str_format(error_msg, id));
        exit(1);
    } else if (arr.size > 1) {
        grv_str_t error_msg = grv_str_ref("Id {str} is not unique:");
        grv_log_error(grv_str_format(error_msg, id));
        todoarr_list(arr);
        exit(1);
    }

    todo_t* todo = arr.arr[0];
    grv_free(arr.arr);
    return todo;
}


void cmd_create(grv_strarr_t args) {
    if (args.size == 0) {
        printf("Usage: %s create <title>\n", grv_str_cstr(exe_name));
        exit(1);
    }

    if (grv_str_starts_with_cstr(args.arr[0], "--")) {
        printf("Invalid option %s\n", grv_str_cstr(args.arr[0]));
        exit(1);
    }

    grv_str_t title = args.arr[0];
    todo_t* todo = todo_create(title);
    grv_error_t result = todo_write(todo);
    if (result != GRV_ERROR_SUCCESS) {
        grv_log_error(grv_str_ref("Could not write new todo"));
        exit(1);
    }
    grv_str_t info_msg = todo_format_short(todo);
    grv_str_prepend_cstr(&info_msg, "Created issue: ");
    grv_log_info(info_msg);
}

void cmd_remove(grv_strarr_t args) {
    if (args.size == 0) {
        printf("Usage: %s remove <id>\n", grv_str_cstr(exe_name));
    }

    grv_str_t id_str = grv_strarr_pop_front(&args);
    todoarr_t arr = todoarr_read(id_str);
    if (arr.size == 0) {
        grv_str_t error_msg = grv_str_format(grv_str_ref("No issues found for id {str}."), id_str);
        grv_log_error(error_msg);
        exit(1);
    } else {
        grv_str_print(grv_str_ref("The following items will be removed permanently:"));
        todoarr_list(arr);
        char choice = grv_query_user(grv_str_ref("Delete items?"), grv_str_ref("Yn"));
        if (choice == 'y') {
            for (size_t i = 0; i < arr.size; ++i) {
                todo_remove_file(arr.arr[i]);
            }
        }
    }
}

void cmd_describe(grv_strarr_t args) {
    if (!grv_path_exists(grv_str_ref(".jj")))  {
        grv_log_error(grv_str_ref("Not a jujutsu repository, cannot jj describe."));
        exit(1);
    } else if (!grv_cmd_available(grv_str_ref("jj"))) {
        grv_log_error(grv_str_ref("jujutsu executable not found, cannot jj describe."));
        exit(1);
    } else if (args.size != 1) {
        grv_str_t error_msg = grv_str_format(grv_str_ref("Usage: {str} describe <id>"), exe_name);
        grv_log_error(error_msg);
        exit(1);
    }

    grv_str_t id_str = grv_strarr_pop_front(&args);
    todoarr_t arr = todoarr_read(id_str);
    todo_t* todo = arr.arr[0];
    grv_str_t cmd = grv_str_format(grv_str_ref("jj desc -m \"{str}\""), todo->title);
    grv_system(cmd);
}

typedef struct {
    grv_str_t key;
    grv_str_t value;
    grv_error_t status;
} parse_arg_return_t;

parse_arg_return_t parse_arg(grv_str_t arg) {
    parse_arg_return_t res = {0};
    
    if (!grv_str_starts_with_char(arg, '-')) {
        res.status = GRV_ERROR_OTHER;
        return res;
    }

    if (grv_str_contains_char(arg, '=')) {
        grv_strpair_t pair = grv_str_split_head_front(arg, grv_str_ref("="));
        res.key = grv_str_lstrip_char(pair.first, '-');
        res.value = pair.second;
        res.status = GRV_ERROR_SUCCESS;
    } else {
        res.key = grv_str_lstrip_char(arg, '-');
        if (grv_str_starts_with_cstr(res.key, "no")) {
            res.key = grv_str_substr(res.key, 2, -1);
            res.value = grv_str_ref("false");
        } else {
            res.value = grv_str_ref("true");
        }
        res.status = GRV_ERROR_SUCCESS;
    }
    return res;
}

void invalid_argument(grv_str_t arg) {
    grv_str_t error_msg = grv_str_format(grv_str_ref("Invalid option {str}"), arg);
    grv_log_error(error_msg);
    exit(1);
}
 
void cmd_update(grv_strarr_t args) {
    grv_str_t new_title = {0};
    grv_str_t new_type = {0};
    grv_str_t id_str = {0};

    while (args.size) {
        grv_str_t arg = grv_strarr_pop_front(&args);
        if (grv_str_starts_with_char(arg, '-')) {
            parse_arg_return_t res = parse_arg(arg);
            if (res.status != GRV_ERROR_SUCCESS) {
                invalid_argument(arg);
            }
            if (grv_str_eq_cstr(res.key, "title")) {
                new_title = res.value;
            } else if (grv_str_eq_cstr(res.key, "type")) {
                new_type = res.value;
            } else {
                invalid_argument(arg);
            }
        } else if (id_str.size) {
            print_usage(usage_update);
        } else {
            id_str = arg;
        }
    }

    if (grv_str_empty(id_str)) {
        grv_log_error(grv_str_ref("Missing id."));
        print_usage(usage_update);
        exit(1);
    }

    bool todo_updated = false;

    todo_t* todo = todo_get_with_id(id_str);
    if (!grv_str_empty(new_title)) {
        todo->title = new_title;
        todo_updated = true;
    }
    if (!grv_str_empty(new_type)) {
        todo->type = new_type;
        todo_updated = true;
    }

    if (todo_updated) {
        todo_write(todo);
        grv_log_info(grv_str_ref("Todo updated successfully."));
        grv_str_print(todo_format_short(todo));
    } else {
        grv_log_info(grv_str_ref("Nothing to update."));
    }
}

void cmd_clean(grv_strarr_t args) {
    todoarr_t arr = todoarr_read(grv_str_ref(""));
    arr = todoarr_select_by_status(arr, grv_str_ref("resolved"));
    if (arr.size > 0) {
        grv_log_info(grv_str_ref("The following resolved issues will be permanently removed:"));
        todoarr_list(arr);
        char choice = grv_query_user(grv_str_ref("Remove resolved issues?"), grv_str_ref("Yn"));
        if (choice == 'y') {
            for (size_t i = 0; i < arr.size; ++i) {
                todo_remove_file(arr.arr[i]);
            }
        }
    }
}

int main(int argc, char** argv) {
    grv_strarr_t args = grv_strarr_new_from_cstrarr(argv, argc);
    exe_name = grv_fs_basename(grv_strarr_pop_front(&args));
    
    grv_str_t cmd = {0};
    if (args.size == 0 || grv_str_starts_with_char(*grv_strarr_front(args), '-')) {
        cmd = grv_str_ref("list");
    } else {
        cmd = grv_strarr_pop_front(&args);
    }

    if (grv_str_eq(cmd, "list")) {
        cmd_list(args);
    } else if (grv_str_eq(cmd, "create") || grv_str_eq(cmd, "c")) {
        cmd_create(args);
    } else if (grv_str_eq(cmd, "resolve") || grv_str_eq(cmd, "r")) {
        cmd_resolve(args);
    } else if (grv_str_eq(cmd, "remove") || grv_str_eq(cmd, "rm")) {
        cmd_remove(args);
    } else if (grv_str_eq(cmd, "describe") || grv_str_eq(cmd, "desc")) {
        cmd_describe(args);
    } else if (grv_str_eq(cmd, "update") || grv_str_eq(cmd, "u")) {
        cmd_update(args);
    } else if (grv_str_eq(cmd, "edit") || grv_str_eq(cmd, "e")) {
        cmd_edit(args);
    } else if (grv_str_eq(cmd, "clean")) {
        cmd_clean(args);
    }

    return 0;
}
