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
 * java.util.concurrent.atomic.AtomicLong
 */
#include "Dalvik.h"
#include "native/InternalNativePriv.h"


/*
 * private static native boolean VMSupportsCS8();
 */
static void Dalvik_java_util_concurrent_atomic_AtomicLong_VMSupportsCS8(
    const u8* args, JValue* pResult)
{
    UNUSED_PARAMETER(args);
    RETURN_BOOLEAN(1);
}

const DalvikNativeMethod dvm_java_util_concurrent_atomic_AtomicLong[] = {
    { "VMSupportsCS8", "()Z",
        Dalvik_java_util_concurrent_atomic_AtomicLong_VMSupportsCS8 },
    { NULL, NULL, NULL },
};

