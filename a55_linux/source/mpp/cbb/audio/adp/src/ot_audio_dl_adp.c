/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#include "ot_audio_dl_adp.h"
#include <stdio.h>
#ifndef __LITEOS__
#include <dlfcn.h>
#else
#include <los_ld_elflib.h>
#endif

td_s32 ot_audio_dlpath(td_char *lib_path)
{
#ifndef __LITEOS__
    (td_void)lib_path;
#else
    if (LOS_PathAdd(lib_path) != TD_SUCCESS) {
        printf("failed to add path %s!\n", lib_path);
        return TD_FAILURE;
    }
#endif

    return TD_SUCCESS;
}

td_s32 ot_audio_dlopen(td_void **lib_handle, td_char *lib_name)
{
    if ((lib_handle == TD_NULL) || (lib_name == TD_NULL)) {
        return TD_FAILURE;
    }

    *lib_handle = TD_NULL;
#ifndef __LITEOS__
    *lib_handle = dlopen(lib_name, RTLD_LAZY | RTLD_LOCAL);
#else
    *lib_handle = LOS_SoLoad(lib_name);
#endif

    if (*lib_handle == TD_NULL) {
        printf("failed to dlopen %s!\n", lib_name);
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

td_s32 ot_audio_dlsym(td_void **func_handle, td_void *lib_handle, td_char *func_name)
{
    if (lib_handle == TD_NULL) {
        printf("lib handle is empty!");
        return TD_FAILURE;
    }

    *func_handle = TD_NULL;
#ifndef __LITEOS__
    *func_handle = dlsym(lib_handle, func_name);
#else
    *func_handle = LOS_FindSymByName(lib_handle, func_name);
#endif

    if (*func_handle == TD_NULL) {
#ifndef __LITEOS__
        printf("dlsym %s failed with %s!\n", func_name, dlerror());
#else
        printf("failed to dlsym %s!\n", func_name);
#endif
        return TD_FAILURE;
    }

    return TD_SUCCESS;
}

td_s32 ot_audio_dlclose(td_void *lib_handle)
{
    if (lib_handle == TD_NULL) {
        printf("lib_handle is empty!");
        return TD_FAILURE;
    }

#ifndef __LITEOS__
    dlclose(lib_handle);
#else
    LOS_ModuleUnload(lib_handle);
#endif

    return TD_SUCCESS;
}
