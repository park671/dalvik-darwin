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
 * sun.misc.Unsafe
 */
#include "Dalvik.h"
#include "native/InternalNativePriv.h"


/*
 * private static native long objectFieldOffset0(Field field);
 */
static void Dalvik_sun_misc_Unsafe_objectFieldOffset0(const u8* args,
    JValue* pResult)
{
    Object* fieldObject = (Object*) args[0];
    InstField* field = (InstField*) dvmGetFieldFromReflectObj(fieldObject);
    s8 result = ((s8) field->byteOffset);

    RETURN_LONG(result);
}

/*
 * private static native int arrayBaseOffset0(Class clazz);
 */
static void Dalvik_sun_misc_Unsafe_arrayBaseOffset0(const u8* args,
    JValue* pResult)
{
    // The base offset is not type-dependent in this vm.
    UNUSED_PARAMETER(args);
    RETURN_INT(offsetof(ArrayObject, contents));
}

/*
 * private static native int arrayIndexScale0(Class clazz);
 */
static void Dalvik_sun_misc_Unsafe_arrayIndexScale0(const u8* args,
    JValue* pResult)
{
    ClassObject* clazz = (ClassObject*) args[0];
    int result;

    if ((clazz == gDvm.classArrayBoolean) ||
            (clazz == gDvm.classArrayByte)) {
        result = sizeof(u1);
    } else if ((clazz == gDvm.classArrayChar) ||
            (clazz == gDvm.classArrayShort)) {
        result = sizeof(u2);
    } else if ((clazz == gDvm.classArrayLong) ||
            (clazz == gDvm.classArrayDouble)) {
        result = sizeof(u8);
    } else {
        result = sizeof(u4);
    }

    RETURN_INT(result);
}

/*
 * public native boolean compareAndSwapInt(Object obj, long offset,
 *         int expectedValue, int newValue);
 */
static void Dalvik_sun_misc_Unsafe_compareAndSwapInt(const u8* args,
    JValue* pResult)
{
    // We ignore the this pointer in args[0].
    Object* obj = (Object*) args[1];
    s8 offset = GET_ARG_LONG(args, 2);
    s4 expectedValue = args[4];
    s4 newValue = args[5];
    volatile int32_t* address = (volatile int32_t*) (((u1*) obj) + offset);

    // Note: android_atomic_cmpxchg() returns 0 on success, not failure.
    int result = android_atomic_cmpxchg(expectedValue, newValue, address);

    RETURN_BOOLEAN(result == 0);
}

/*
 * public native boolean compareAndSwapLong(Object obj, long offset,
 *         long expectedValue, long newValue);
 */
static void Dalvik_sun_misc_Unsafe_compareAndSwapLong(const u8* args,
    JValue* pResult)
{
    // We ignore the this pointer in args[0].
    Object* obj = (Object*) args[1];
    s8 offset = GET_ARG_LONG(args, 2);
    s8 expectedValue = GET_ARG_LONG(args, 4);
    s8 newValue = GET_ARG_LONG(args, 6);
    volatile int64_t* address = (volatile int64_t*) (((u1*) obj) + offset);

    // Note: android_atomic_cmpxchg() returns 0 on success, not failure.
    int result =
        android_quasiatomic_cmpxchg_64(expectedValue, newValue, address);

    RETURN_BOOLEAN(result == 0);
}

/*
 * public native boolean compareAndSwapObject(Object obj, long offset,
 *         Object expectedValue, Object newValue);
 */
static void Dalvik_sun_misc_Unsafe_compareAndSwapObject(const u8* args,
    JValue* pResult)
{
    // We ignore the this pointer in args[0].
    Object* obj = (Object*) args[1];
    s8 offset = GET_ARG_LONG(args, 2);
    Object* expectedValue = (Object*) args[4];
    Object* newValue = (Object*) args[5];
    s8* address = (s8*) (((u1*) obj) + offset);

    // Note: android_atomic_cmpxchg() returns 0 on success, not failure.
    int result = android_quasiatomic_cmpxchg_64((s8) expectedValue,
            (s8) newValue, address);
    
    RETURN_BOOLEAN(result == 0);
}

/*
 * public native int getIntVolatile(Object obj, long offset);
 */
static void Dalvik_sun_misc_Unsafe_getIntVolatile(const u8* args,
    JValue* pResult)
{
    // We ignore the this pointer in args[0].
    Object* obj = (Object*) args[1];
    s8 offset = GET_ARG_LONG(args, 2);
    volatile s4* address = (volatile s4*) (((u1*) obj) + offset);

    RETURN_INT(*address);
}

/*
 * public native void putIntVolatile(Object obj, long offset, int newValue);
 */
static void Dalvik_sun_misc_Unsafe_putIntVolatile(const u8* args,
    JValue* pResult)
{
    // We ignore the this pointer in args[0].
    Object* obj = (Object*) args[1];
    s8 offset = GET_ARG_LONG(args, 2);
    s4 value = (s4) args[4];
    volatile s4* address = (volatile s4*) (((u1*) obj) + offset);

    *address = value;
    RETURN_VOID();
}

/*
 * public native long getLongVolatile(Object obj, long offset);
 */
static void Dalvik_sun_misc_Unsafe_getLongVolatile(const u8* args,
    JValue* pResult)
{
    // We ignore the this pointer in args[0].
    Object* obj = (Object*) args[1];
    s8 offset = GET_ARG_LONG(args, 2);
    volatile s8* address = (volatile s8*) (((u1*) obj) + offset);

    RETURN_LONG(android_quasiatomic_read_64(address));
}

/*
 * public native void putLongVolatile(Object obj, long offset, long newValue);
 */
static void Dalvik_sun_misc_Unsafe_putLongVolatile(const u8* args,
    JValue* pResult)
{
    // We ignore the this pointer in args[0].
    Object* obj = (Object*) args[1];
    s8 offset = GET_ARG_LONG(args, 2);
    s8 value = GET_ARG_LONG(args, 4);
    volatile s8* address = (volatile s8*) (((u1*) obj) + offset);

    android_quasiatomic_swap_64(value, address);
    RETURN_VOID();
}

/*
 * public native Object getObjectVolatile(Object obj, long offset);
 */
static void Dalvik_sun_misc_Unsafe_getObjectVolatile(const u8* args,
    JValue* pResult)
{
    // We ignore the this pointer in args[0].
    Object* obj = (Object*) args[1];
    s8 offset = GET_ARG_LONG(args, 2);
    volatile Object** address = (volatile Object**) (((u1*) obj) + offset);

    RETURN_PTR((void*) *address);
}

/*
 * public native void putObjectVolatile(Object obj, long offset,
 *         Object newValue);
 */
static void Dalvik_sun_misc_Unsafe_putObjectVolatile(const u8* args,
    JValue* pResult)
{
    // We ignore the this pointer in args[0].
    Object* obj = (Object*) args[1];
    s8 offset = GET_ARG_LONG(args, 2);
    Object* value = (Object*) args[4];
    volatile Object** address = (volatile Object**) (((u1*) obj) + offset);

    *address = value;
    RETURN_VOID();
}
            
/*
 * public native int getInt(Object obj, long offset);
 */
static void Dalvik_sun_misc_Unsafe_getInt(const u8* args, JValue* pResult)
{
    // We ignore the this pointer in args[0].
    Object* obj = (Object*) args[1];
    s8 offset = GET_ARG_LONG(args, 2);
    s4* address = (s4*) (((u1*) obj) + offset);

    RETURN_INT(*address);
}

/*
 * public native void putInt(Object obj, long offset, int newValue);
 */
static void Dalvik_sun_misc_Unsafe_putInt(const u8* args, JValue* pResult)
{
    // We ignore the this pointer in args[0].
    Object* obj = (Object*) args[1];
    s8 offset = GET_ARG_LONG(args, 2);
    s4 value = (s4) args[4];
    s4* address = (s4*) (((u1*) obj) + offset);

    *address = value;
    RETURN_VOID();
}

/*
 * public native long getLong(Object obj, long offset);
 */
static void Dalvik_sun_misc_Unsafe_getLong(const u8* args, JValue* pResult)
{
    // We ignore the this pointer in args[0].
    Object* obj = (Object*) args[1];
    s8 offset = GET_ARG_LONG(args, 2);
    s8* address = (s8*) (((u1*) obj) + offset);

    RETURN_LONG(*address);
}

/*
 * public native void putLong(Object obj, long offset, long newValue);
 */
static void Dalvik_sun_misc_Unsafe_putLong(const u8* args, JValue* pResult)
{
    // We ignore the this pointer in args[0].
    Object* obj = (Object*) args[1];
    s8 offset = GET_ARG_LONG(args, 2);
    s8 value = GET_ARG_LONG(args, 4);
    s8* address = (s8*) (((u1*) obj) + offset);

    *address = value;
    RETURN_VOID();
}

/*
 * public native Object getObject(Object obj, long offset);
 */
static void Dalvik_sun_misc_Unsafe_getObject(const u8* args, JValue* pResult)
{
    // We ignore the this pointer in args[0].
    Object* obj = (Object*) args[1];
    s8 offset = GET_ARG_LONG(args, 2);
    Object** address = (Object**) (((u1*) obj) + offset);

    RETURN_PTR(*address);
}

/*
 * public native void putObject(Object obj, long offset, Object newValue);
 */
static void Dalvik_sun_misc_Unsafe_putObject(const u8* args, JValue* pResult)
{
    // We ignore the this pointer in args[0].
    Object* obj = (Object*) args[1];
    s8 offset = GET_ARG_LONG(args, 2);
    Object* value = (Object*) args[4];
    Object** address = (Object**) (((u1*) obj) + offset);

    *address = value;
    RETURN_VOID();
}

const DalvikNativeMethod dvm_sun_misc_Unsafe[] = {
    { "objectFieldOffset0", "(Ljava/lang/reflect/Field;)J",
      Dalvik_sun_misc_Unsafe_objectFieldOffset0 },
    { "arrayBaseOffset0", "(Ljava/lang/Class;)I",
      Dalvik_sun_misc_Unsafe_arrayBaseOffset0 },
    { "arrayIndexScale0", "(Ljava/lang/Class;)I",
      Dalvik_sun_misc_Unsafe_arrayIndexScale0 },
    { "compareAndSwapInt", "(Ljava/lang/Object;JII)Z",
      Dalvik_sun_misc_Unsafe_compareAndSwapInt },
    { "compareAndSwapLong", "(Ljava/lang/Object;JJJ)Z",
      Dalvik_sun_misc_Unsafe_compareAndSwapLong },
    { "compareAndSwapObject",
      "(Ljava/lang/Object;JLjava/lang/Object;Ljava/lang/Object;)Z",
      Dalvik_sun_misc_Unsafe_compareAndSwapObject },
    { "getIntVolatile", "(Ljava/lang/Object;J)I",
      Dalvik_sun_misc_Unsafe_getIntVolatile },
    { "putIntVolatile", "(Ljava/lang/Object;JI)V",
      Dalvik_sun_misc_Unsafe_putIntVolatile },
    { "getLongVolatile", "(Ljava/lang/Object;J)J",
      Dalvik_sun_misc_Unsafe_getLongVolatile },
    { "putLongVolatile", "(Ljava/lang/Object;JJ)V",
      Dalvik_sun_misc_Unsafe_putLongVolatile },
    { "getObjectVolatile", "(Ljava/lang/Object;J)Ljava/lang/Object;",
      Dalvik_sun_misc_Unsafe_getObjectVolatile },
    { "putObjectVolatile", "(Ljava/lang/Object;JLjava/lang/Object;)V",
      Dalvik_sun_misc_Unsafe_putObjectVolatile },
    { "getInt", "(Ljava/lang/Object;J)I",
      Dalvik_sun_misc_Unsafe_getInt },
    { "putInt", "(Ljava/lang/Object;JI)V",
      Dalvik_sun_misc_Unsafe_putInt },
    { "getLong", "(Ljava/lang/Object;J)J",
      Dalvik_sun_misc_Unsafe_getLong },
    { "putLong", "(Ljava/lang/Object;JJ)V",
      Dalvik_sun_misc_Unsafe_putLong },
    { "getObject", "(Ljava/lang/Object;J)Ljava/lang/Object;",
      Dalvik_sun_misc_Unsafe_getObject },
    { "putObject", "(Ljava/lang/Object;JLjava/lang/Object;)V",
      Dalvik_sun_misc_Unsafe_putObject },
    { NULL, NULL, NULL },
};

