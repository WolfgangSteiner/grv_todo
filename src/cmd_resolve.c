#include "grv/grv.h"
#include "todo.h"

extern grv_str_t exe_name;

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
        todoarr_list(arr);
    } else {
        todo_t* todo = arr.arr[0];
        todo_resolve(todo);
        todo_write(todo);
        grv_str_t info_msg = todo_format_short(todo);
        grv_str_prepend_cstr(&info_msg, "Resolved issue: ");
        grv_log_info(info_msg);
    }
}

