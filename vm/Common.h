/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/*
 * Common defines for all Dalvik code.
 */
#ifndef _DALVIK_COMMON
#define _DALVIK_COMMON

#ifndef LOG_TAG
# define LOG_TAG "dalvikvm"
#endif

#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include "../log/Log.h"
#include "logd.h"
#include <unistd.h>
#include <stdint.h>

#include "../safe-iop/safe_iop.h"

#if 0
# undef assert
# define assert(x) \
    ((x) ? ((void)0) : (LOGE("ASSERT FAILED (%s:%d): " #x "\n", \
        __FILE__, __LINE__), *(int*)39=39, 0) )
#endif


/*
 * If "very verbose" logging is enabled, make it equivalent to LOGV.
 * Otherwise, make it disappear.
 *
 * Define this above the #include "Dalvik.h" to enable for only a
 * single file.
 */
/* #define VERY_VERBOSE_LOG */
#if 1
# define LOGVV      LOGV
# define IF_LOGVV() IF_LOGV()
#else
# define LOGVV(...) ((void)0)
# define IF_LOGVV() if (false)
#endif

/*
 * These match the definitions in the VM specification.
 */
typedef uint8_t             u1;
typedef uint16_t            u2;
typedef uint32_t            u4;
typedef uint64_t            u8;
typedef int8_t              s1;
typedef int16_t             s2;
typedef int32_t             s4;
typedef int64_t             s8;

/*
 * Storage for primitive types and object references.
 *
 * Some parts of the code (notably object field access) assume that values
 * are "left aligned", i.e. given "JValue jv", "jv.i" and "*((s4*)&jv)"
 * yield the same result.  This seems to be guaranteed by gcc on big- and
 * little-endian systems.
 */
typedef union JValue {
    u1      z;
    s1      b;
    u2      c;
    s2      s;
    s4      i;
    s8      j;
    float   f;
    double  d;
    void*   l;
} JValue;

#define NELEM(x) ((int) (sizeof(x) / sizeof((x)[0])))

# define __LITTLE_ENDIAN 1234
#define __BYTE_ORDER __LITTLE_ENDIAN

#define LOGV(...)    LOG_PRI(2, 0, __VA_ARGS__)
#define LOGD(...)    LOG_PRI(3, 0, __VA_ARGS__)
#define LOGI(...)    LOG_PRI(4, 0, __VA_ARGS__)
#define LOGW(...)    LOG_PRI(5, 0, __VA_ARGS__)
#define LOGE(...)    LOG_PRI(6, 0, __VA_ARGS__)

#define LOG_PRI(priority, tag, ...) do {                    \
       fprintf(stdout, __VA_ARGS__);                        \
    } while(0)

#endif /*_DALVIK_COMMON*/
