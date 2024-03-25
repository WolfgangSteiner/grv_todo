#include "grv/grv.h"
#include "grv/grv_str.h"
#include "grv/grv_fs.h"
#include "grv/grv_strarr.h"
#include "grv/grv_util.h"
#include "todo.h"

static grv_str_t exe_name = {0};


void list_todos(todoarr_t arr) {
    for (size_t i = 0; i < arr.size; ++i) {
        todo_t* todo = arr.arr[i];
        grv_str_t display_str = todo_format_short(todo);
        grv_str_print(display_str);
        grv_str_free(&display_str);
    }
}

void cmd_list(grv_strarr_t args) {
    grv_str_t id_prefix = grv_str_ref("");
    bool list_all_issues = false;

    while (args.size) {
        grv_str_t arg = grv_strarr_pop_front(&args);
        if (grv_str_eq_cstr(arg, "-a")) {
            list_all_issues = true;
        } else {
            grv_str_t error_msg = grv_str_format(grv_str_ref("Invalid option {str}."), arg);
            grv_log_error(error_msg);
            exit(1);
        }
    }

    todoarr_t arr = todoarr_read(id_prefix);
    if (!list_all_issues) {
        arr = todoarr_select_by_status(arr, grv_str_ref("open"));
    }
    //grv_str_print(grv_str_ref("ID       Title"));
    list_todos(arr);
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
    printf("Created issue: ");
    
}

void cmd_resolve(grv_strarr_t args) {
    if (args.size == 0) {
        printf("Usage: %s resolve <id>\n", grv_str_cstr(exe_name));
        exit(1);
    }

    grv_str_t id_str = *grv_strarr_front(args);
    todoarr_t arr = todoarr_read(id_str);
    arr = todoarr_select_by_status(arr, grv_str_ref("open"));
    if (arr.size == 0) {
        grv_str_t error_msg = grv_str_format(grv_str_ref("No issues found for id {str}."), id_str);
        grv_log_error(error_msg);
        exit(1);
    } else if (arr.size > 1) {
        grv_str_t error_msg = grv_str_format(grv_str_ref("Multiple issues found for id {str}:"), id_str);
        grv_log_error(error_msg);
        list_todos(arr);
    } else {
        todo_t* todo = arr.arr[0];
        todo->status = grv_str_new("resolved");
        todo_write(todo);
        grv_str_t info_msg = todo_format_short(todo);
        grv_str_prepend_cstr(&info_msg, "Resolved issue: ");
        grv_log_info(info_msg);
    }
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
        list_todos(arr);
        char choice = grv_query_user(grv_str_ref("Delete items?"), grv_str_ref("Yn"));
        if (choice == 'y') {
            for (size_t i = 0; i < arr.size; ++i) {
                grv_str_t filepath = todo_file_path(arr.arr[i]);
                grv_remove_file(filepath);
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
    }

    return 0;
}
