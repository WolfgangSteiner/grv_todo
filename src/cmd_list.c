#include "todo.h"

extern grv_str_t exe_name;

void cmd_list(grv_strarr_t args) {
    grv_str_t id_prefix = grv_str_ref("");
    bool list_all_issues = false;
    bool single_issue = false;

    while (args.size) {
        grv_str_t arg = grv_strarr_pop_front(&args);
        if (grv_str_eq_cstr(arg, "-a") || grv_str_eq_cstr(arg, "--all")) {
            list_all_issues = true;
        } else if (todo_id_valid(arg)) {
            id_prefix = arg;
            single_issue = true;
        } else if(grv_str_starts_with_char(arg, '-')) {
            grv_str_t error_msg = grv_str_format(grv_str_ref("{str} list: Invalid option {str}."), exe_name, arg);
            grv_log_error(error_msg);
            exit(1);
        } else {
            grv_str_t error_msg = grv_str_format(grv_str_ref("Usage: {str} list [--all|-a] [id]"), exe_name);
            grv_log_error(error_msg);
            exit(1);
        }
    }

    todoarr_t arr = todoarr_read(id_prefix);
    if (!list_all_issues) {
        arr = todoarr_select_by_status(arr, grv_str_ref("open"));
    }
    //grv_str_print(grv_str_ref("ID       Title"));
    if (single_issue && arr.size == 1) {
        todo_print(arr.arr[0]);
    } else {
        todoarr_list(arr);
    }
}
