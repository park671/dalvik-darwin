/*
 * Copyright (C) 2007 The Android Open Source Project
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
 * Registration functions for native methods in system classes.
 */
#ifndef _NATIVEHELPER_ANDROIDSYSTEMNATIVES
#define _NATIVEHELPER_ANDROIDSYSTEMNATIVES

#include "jni.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Call this to register the methods below.  Ideally, this is the only
 * symbol that needs to be exported from the library.
 */
int jniRegisterSystemJavaMethods(JNIEnv* env);


/*
 * Registration functions for native methods in libcore.
 */
int register_org_apache_harmony_dalvik_NativeTestTarget(JNIEnv* env);

int register_dalvik_system_TouchDex(JNIEnv* env);

int register_java_io_File(JNIEnv* env);
int register_java_io_FileDescriptor(JNIEnv* env);
int register_java_io_ObjectOutputStream(JNIEnv* env);
int register_java_io_ObjectInputStream(JNIEnv* env);
int register_java_io_ObjectStreamClass(JNIEnv* env);

int register_java_lang_Character(JNIEnv* env);
int register_java_lang_Double(JNIEnv* env);
int register_java_lang_Float(JNIEnv* env);
int register_java_lang_Math(JNIEnv* env);
int register_java_lang_ProcessManager(JNIEnv* env);
int register_java_lang_StrictMath(JNIEnv* env);
int register_java_lang_System(JNIEnv* env);

int register_java_net_InetAddress(JNIEnv* env);
int register_java_net_NetworkInterface(JNIEnv* env);

int register_org_apache_harmony_luni_util_fltparse(JNIEnv *env);
int register_org_apache_harmony_luni_util_NumberConvert(JNIEnv *env);

int register_org_apache_harmony_luni_platform_OSFileSystem(JNIEnv* env);
int register_org_apache_harmony_luni_platform_OSMemory(JNIEnv* env);
int register_org_apache_harmony_luni_platform_OSNetworkSystem(JNIEnv* env);

int register_sun_misc_Unsafe(JNIEnv* env);

#ifdef __cplusplus
}
#endif

#endif /*_NATIVEHELPER_ANDROIDSYSTEMNATIVES*/
