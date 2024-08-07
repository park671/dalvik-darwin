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
 * java.lang.reflect.AccessibleObject
 */
#include "Dalvik.h"
#include "native/InternalNativePriv.h"


/*
 * private static Object[] getClassSignatureAnnotation(Class clazz)
 *
 * Return the Signature annotation for the specified class.  Equivalent to
 * Class.getSignatureAnnotation(), but available to java.lang.reflect.
 */
static void Dalvik_java_lang_reflect_AccessibleObject_getClassSignatureAnnotation(
    const u8* args, JValue* pResult)
{
    ClassObject* clazz = (ClassObject*) args[0];
    ArrayObject* arr = dvmGetClassSignatureAnnotation(clazz);

    dvmReleaseTrackedAlloc((Object*) arr, NULL);
    RETURN_PTR(arr);
}

const DalvikNativeMethod dvm_java_lang_reflect_AccessibleObject[] = {
    { "getClassSignatureAnnotation", "(Ljava/lang/Class;)[Ljava/lang/Object;",
      Dalvik_java_lang_reflect_AccessibleObject_getClassSignatureAnnotation },
    { NULL, NULL, NULL },
};

