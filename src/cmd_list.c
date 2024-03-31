#include "todo.h"

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
    todoarr_list(arr);
}
