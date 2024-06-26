#include "lib/grv/grvbld.h"

int main(int argc, char** argv) {
    GRV_CHECK_AND_REBUILD();    
    grvbld_config_t* config =  grvbld_config_new(argc, argv);
    grvbld_strarr_push(&config->warnings, "-Wextra -Wpedantic");
    grvbld_config_add_include_directories(config, "lib/grv/include", NULL);

    grvbld_target_t* libgrv = grvbld_target_create("grv", GRVBLD_STATIC_LIBRARY);
    grvbld_target_add_src(libgrv, "lib/grv/src/grv.c");
    grvbld_build_target(config, libgrv);

    int result = grvbld_run_tests(config);
    if (config->tests_only) return result; 

    grvbld_target_t* todo = grvbld_target_create_executable("todo");
    grvbld_target_add_src_files(todo,
        "src/main.c",
        "src/todo.c",
        "src/cmd_edit.c",
        "src/cmd_list.c",
        "src/cmd_resolve.c",
        NULL);
    grvbld_target_link(todo, libgrv);
    todo->run_after_build = true;
    result = grvbld_build_target(config, todo);
    if (!result) system("cp build/todo ~/bin/todo");

    return 0;
}

