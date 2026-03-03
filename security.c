#include "security.h"

const secure_func_handle_t secure_function_lookup_handle = 0;

int secure_function_register(void* fn, secure_func_handle_t* handle) {
    int h = secure_function_call(secure_function_lookup_handle, (uint32_t)fn, 0, 0, 0, 0);
    if(h < 0)
        return -1;

    *handle = h;
    return 0;
}
