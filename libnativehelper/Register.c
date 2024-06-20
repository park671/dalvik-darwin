/*
 * Copyright 2006 The Android Open Source Project
 *
 * JNI helper functions.
 */
#include <Bits.h>

#include "jni.h"
#include "AndroidSystemNatives.h"

#include <stdio.h>

/*
 * Register all methods for system classes.
 *
 * Remember to add the declarations to include/nativehelper/JavaSystemNatives.h.
 */
int jniRegisterSystemMethods(JNIEnv* env)
{
    int result = -1;

    (*env)->PushLocalFrame(env, 128);

    if (register_org_apache_harmony_dalvik_NativeTestTarget(env) != 0)
        goto bail;
    
    if (register_java_io_File(env) != 0)
        goto bail;
    if (register_java_io_FileDescriptor(env) != 0)
        goto bail;
    if (register_java_io_ObjectOutputStream(env) != 0)
        goto bail;
    if (register_java_io_ObjectInputStream(env) != 0)
        goto bail;
    if (register_java_io_ObjectStreamClass(env) != 0)
        goto bail;

    if (register_java_lang_Float(env) != 0)
        goto bail;
    if (register_java_lang_Double(env) != 0)
        goto bail;
    if (register_java_lang_Math(env) != 0)
        goto bail;
    if (register_java_lang_ProcessManager(env) != 0)
        goto bail;
    if (register_java_lang_StrictMath(env) != 0)
        goto bail;
    if (register_java_lang_System(env) != 0)
        goto bail;

    if (register_org_apache_harmony_luni_util_fltparse(env) != 0)
        goto bail;
    if (register_org_apache_harmony_luni_util_NumberConvert(env) != 0)
        goto bail;

    if (register_java_net_InetAddress(env) != 0)
        goto bail;
    if (register_java_net_NetworkInterface(env) != 0)
        goto bail;

    /*
     * Initialize the Android classes last, as they have dependencies
     * on the "corer" core classes.
     */

    if (register_dalvik_system_TouchDex(env) != 0)
        goto bail;
    
    result = 0;

bail:
    LOGE("reg native method error!");
    (*env)->PopLocalFrame(env, NULL);
    return result;
}

