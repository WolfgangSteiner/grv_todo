#include "grv/grv_base.h"
#include "todo.h"
#include <stdlib.h>

extern grv_str_t exe_name;
char* fallback_editors[] = {"nvim", "vim", "emacs", "nano", NULL};

grv_str_return_t get_editor(void) {
    grv_str_return_t res = {0};
    char* val = getenv("EDITOR");
    if (val == NULL) {
        char** fallback = fallback_editors;
        while (*fallback != NULL) {
            grv_str_t editor = grv_str_ref(*fallback);
            if (grv_cmd_available(editor)) {
                grv_log_info(grv_str_format(grv_str_ref("EDITOR environment variable not set. Falling back to {str}."), editor));
                res.str = editor;
                return res;
            }
            fallback++;
        }
        res.error = GRV_ERROR_INVALID_KEY;
    }
    else {
        res.str = grv_str_new(val);
    }

    return res;
}

void cmd_edit(grv_strarr_t args) {
    grv_str_t error_fmt = grv_str_ref("Usage: {str} edit <id>");

    if (args.size != 1) {
        grv_str_t error_msg = grv_str_format(error_fmt, exe_name);
        grv_log_error(error_msg);
        exit(1);
    }
    
    grv_str_t id_str = args.arr[0];
    todoarr_t todo_arr = todoarr_read(id_str);
    if (todo_arr.size == 0) {
        grv_str_t error_msg = grv_str_format(grv_str_ref("No todo found for id {str}"), id_str);
        grv_log_error(error_msg);
        exit(1);
    } else if (todo_arr.size > 1) {
        grv_str_t error_msg = grv_str_format(grv_str_ref("Id {str} is not unique"), id_str);
        grv_log_error(error_msg);
        exit(1);
    }

    grv_str_return_t editor = get_editor();
    if (editor.error != GRV_ERROR_SUCCESS) {
        grv_str_t error_msg = grv_str_ref("EDITOR environment variable not set. Could not determine editor to launch.");
        grv_log_error(error_msg);
        exit(1);
    }
   
    todo_t* todo = todo_arr.arr[0];
    grv_str_t todo_path = todo_file_path(todo);
    grv_str_t edit_cmd = grv_str_format(grv_str_ref("{str} {str}"), editor.str, todo_path);
    int result = grv_system(edit_cmd);
}
