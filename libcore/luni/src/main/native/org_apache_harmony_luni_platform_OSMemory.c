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

#include "JNIHelp.h"
#include "AndroidSystemNatives.h"
#include "Misc.h"
#include "AndroidConfig.h"
#include <sys/mman.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#undef MMAP_READ_ONLY
#define MMAP_READ_ONLY 1L
#undef MMAP_READ_WRITE
#define MMAP_READ_WRITE 2L
#undef MMAP_WRITE_COPY
#define MMAP_WRITE_COPY 4L

/*
 * Class:     org_apache_harmony_luni_platform_OSMemory
 * Method:    littleEndian
 * Signature: ()Z
 */
static jboolean harmony_nio_littleEndian(JNIEnv *_env, jclass _this) {
    long l = 0x01020304;
    unsigned char *c = (unsigned char *) &l;
    return (*c == 0x04) ? JNI_TRUE : JNI_FALSE;
}

/*
 * Class:     org_apache_harmony_luni_platform_OSMemory
 * Method:    getPointerSizeImpl
 * Signature: ()I
 */
static jint harmony_nio_getPointerSizeImpl(JNIEnv *_env, jclass _this) {
    return sizeof(void *);
}

/*
 * Class:     org_apache_harmony_luni_platform_OSMemory
 * Method:    mallocImpl
 * Signature: (I)I
 */
static jlong harmony_nio_mallocImpl(JNIEnv *_env, jobject _this, jint size) {
    void *returnValue = malloc(size);
    if (returnValue == NULL) {
        jniThrowException(_env, "java.lang.OutOfMemoryError", "");
    }
    return (jlong) returnValue;
}

/*
 * Class:     org_apache_harmony_luni_platform_OSMemory
 * Method:    freeImpl
 * Signature: (I)V
 */
static void harmony_nio_freeImpl(JNIEnv *_env, jobject _this, jlong pointer) {
    free((void *) pointer);
}

/*
 * Class:     org_apache_harmony_luni_platform_OSMemory
 * Method:    memset
 * Signature: (IBJ)V
 */
static void harmony_nio_memset(JNIEnv *_env, jobject _this, jlong address,
                               jbyte value, jlong length) {
    memset ((void *) ((jlong) address), (jbyte) value, (jlong) length);
}

/*
 * Class:     org_apache_harmony_luni_platform_OSMemory
 * Method:    memmove
 * Signature: (IIJ)V
 */
static void harmony_nio_memmove(JNIEnv *_env, jobject _this, jlong destAddress,
                                jlong srcAddress, jlong length) {
    memmove ((void *) ((jlong) destAddress), (const void *) ((jlong) srcAddress),
             (jlong) length);
}

/*
 * Class:     org_apache_harmony_luni_platform_OSMemory
 * Method:    getByteImpl
 * Signature: (I)B
 */
static jbyte harmony_nio_getByteImpl(JNIEnv *_env, jobject _this,
                                     jlong pointer) {
    jbyte returnValue = *((jbyte *) pointer);
    return returnValue;
}

/*
 * Class:     org_apache_harmony_luni_platform_OSMemory
 * Method:    getBytesImpl
 * Signature: (I[BII)V
 */
static void harmony_nio_getBytesImpl(JNIEnv *_env, jobject _this, jlong pointer,
                                     jbyteArray dst, jint offset, jint length) {
    jbyte *dst_ = (jbyte *) (*_env)->GetPrimitiveArrayCritical(_env, dst, (jboolean *) 0);
    memcpy(dst_ + offset, (jbyte *) pointer, length);
    (*_env)->ReleasePrimitiveArrayCritical(_env, dst, dst_, 0);
}

/*
 * Class:     org_apache_harmony_luni_platform_OSMemory
 * Method:    putByteImpl
 * Signature: (IB)V
 */
static void harmony_nio_putByteImpl(JNIEnv *_env, jobject _this, jlong pointer,
                                    jbyte val) {
    *((jbyte *) pointer) = val;
}

/*
 * Class:     org_apache_harmony_luni_platform_OSMemory
 * Method:    putBytesImpl
 * Signature: (I[BII)V
 */
static void harmony_nio_putBytesImpl(JNIEnv *_env, jobject _this,
                                     jlong pointer, jbyteArray src, jint offset, jint length) {
    jbyte *src_ = (jbyte *) (*_env)->GetPrimitiveArrayCritical(_env, src, (jboolean *) 0);
    memcpy((jbyte *) pointer, src_ + offset, length);
    (*_env)->ReleasePrimitiveArrayCritical(_env, src, src_, JNI_ABORT);
}

static void
swapShorts(jshort *shorts, int numBytes) {
    jbyte *src = (jbyte *) shorts;
    jbyte *dst = src;
    int i;

    for (i = 0; i < numBytes; i += 2) {
        jbyte b0 = *src++;
        jbyte b1 = *src++;
        *dst++ = b1;
        *dst++ = b0;
    }
}

static void
swapInts(jint *ints, int numBytes) {
    jbyte *src = (jbyte *) ints;
    jbyte *dst = src;
    int i;
    for (i = 0; i < numBytes; i += 4) {
        jbyte b0 = *src++;
        jbyte b1 = *src++;
        jbyte b2 = *src++;
        jbyte b3 = *src++;
        *dst++ = b3;
        *dst++ = b2;
        *dst++ = b1;
        *dst++ = b0;
    }
}

/*
 * Class:     org_apache_harmony_luni_platform_OSMemory
 * Method:    putShortsImpl
 * Signature: (I[SIIZ)V
 */
static void harmony_nio_putShortsImpl(JNIEnv *_env, jobject _this,
                                      jlong pointer, jshortArray src, jint offset, jint length, jboolean swap) {

    offset = offset << 1;
    length = length << 1;

    jshort *src_ =
            (jshort *) (*_env)->GetPrimitiveArrayCritical(_env, src, (jboolean *) 0);
    if (swap) {
        swapShorts(src_ + offset, length);
    }
    memcpy((jbyte *) pointer, (jbyte *) src_ + offset, length);
    if (swap) {
        swapShorts(src_ + offset, length);
    }
    (*_env)->ReleasePrimitiveArrayCritical(_env, src, src_, JNI_ABORT);
}

/*
 * Class:     org_apache_harmony_luni_platform_OSMemory
 * Method:    putIntsImpl
 * Signature: (I[IIIZ)V
 */
static void harmony_nio_putIntsImpl(JNIEnv *_env, jobject _this,
                                    jlong pointer, jintArray src, jint offset, jint length, jboolean swap) {

    offset = offset << 2;
    length = length << 2;

    jint *src_ =
            (jint *) (*_env)->GetPrimitiveArrayCritical(_env, src, (jboolean *) 0);
    if (swap) {
        swapInts(src_ + offset, length);
    }
    memcpy((jbyte *) pointer, (jbyte *) src_ + offset, length);
    if (swap) {
        swapInts(src_ + offset, length);
    }
    (*_env)->ReleasePrimitiveArrayCritical(_env, src, src_, JNI_ABORT);
}

/*
 * Class:     org_apache_harmony_luni_platform_OSMemory
 * Method:    getShortImpl
 * Signature: (I)S
 */
static jshort harmony_nio_getShortImpl(JNIEnv *_env, jobject _this,
                                       jlong pointer) {
    if ((pointer & 0x1) == 0) {
        jshort returnValue = *((jshort *) pointer);
        return returnValue;
    } else {
        // Handle unaligned memory access one byte at a time
        jshort s;
        unsigned char *src = (unsigned char *) pointer;
        unsigned char *dst = (unsigned char *) &s;
        dst[0] = src[0];
        dst[1] = src[1];
        return s;
    }
}

/*
 * Class:     org_apache_harmony_luni_platform_OSMemory
 * Method:    petShortImpl
 * Signature: (IS)V
 */
static void harmony_nio_putShortImpl(JNIEnv *_env, jobject _this, jlong pointer,
                                     jshort value) {
    if ((pointer & 0x1) == 0) {
        *((jshort *) pointer) = value;
    } else {
        // Handle unaligned memory access one byte at a time
        unsigned char *src = (unsigned char *) &value;
        unsigned char *dst = (unsigned char *) pointer;
        dst[0] = src[0];
        dst[1] = src[1];
    }
}

/*
 * Class:     org_apache_harmony_luni_platform_OSMemory
 * Method:    getIntImpl
 * Signature: (I)I
 */
static jint harmony_nio_getIntImpl(JNIEnv *_env, jobject _this, jlong pointer) {
    if ((pointer & 0x3) == 0) {
        jint returnValue = *((jint *) pointer);
        return returnValue;
    } else {
        // Handle unaligned memory access one byte at a time
        jint i;
        unsigned char *src = (unsigned char *) pointer;
        unsigned char *dst = (unsigned char *) &i;
        dst[0] = src[0];
        dst[1] = src[1];
        dst[2] = src[2];
        dst[3] = src[3];
        return i;
    }
}

/*
 * Class:     org_apache_harmony_luni_platform_OSMemory
 * Method:    putIntImpl
 * Signature: (II)V
 */
static void harmony_nio_putIntImpl(JNIEnv *_env, jobject _this, jlong pointer,
                                   jint value) {
    if ((pointer & 0x3) == 0) {
        *((jint *) pointer) = value;
    } else {
        // Handle unaligned memory access one byte at a time
        unsigned char *src = (unsigned char *) &value;
        unsigned char *dst = (unsigned char *) pointer;
        dst[0] = src[0];
        dst[1] = src[1];
        dst[2] = src[2];
        dst[3] = src[3];
    }
}

/*
 * Class:     org_apache_harmony_luni_platform_OSMemory
 * Method:    getLongImpl
 * Signature: (I)Ljava/lang/Long;
 */
static jlong harmony_nio_getLongImpl(JNIEnv *_env, jobject _this,
                                     jlong pointer) {
    if ((pointer & 0x7) == 0) {
        jlong returnValue = *((jlong *) pointer);
        return returnValue;
    } else {
        // Handle unaligned memory access one byte at a time
        jlong l;
        memcpy((void *) &l, (void *) pointer, sizeof(jlong));
        return l;
    }
}

/*
 * Class:     org_apache_harmony_luni_platform_OSMemory
 * Method:    putLongImpl
 * Signature: (IJ)V
 */
static void harmony_nio_putLongImpl(JNIEnv *_env, jobject _this, jlong pointer,
                                    jlong value) {
    if ((pointer & 0x7) == 0) {
        *((jlong *) pointer) = value;
    } else {
        // Handle unaligned memory access one byte at a time
        memcpy((void *) pointer, (void *) &value, sizeof(jlong));
    }
}

/*
 * Class:     org_apache_harmony_luni_platform_OSMemory
 * Method:    getFloatImpl
 * Signature: (I)F
 */
static jfloat harmony_nio_getFloatImpl(JNIEnv *_env, jobject _this,
                                       jlong pointer) {
    if ((pointer & 0x3) == 0) {
        jfloat returnValue = *((jfloat *) pointer);
        return returnValue;
    } else {
        // Handle unaligned memory access one byte at a time
        jfloat f;
        memcpy((void *) &f, (void *) pointer, sizeof(jfloat));
        return f;
    }
}

/*
 * Class:     org_apache_harmony_luni_platform_OSMemory
 * Method:    setFloatImpl
 * Signature: (IF)V
 */
static void harmony_nio_putFloatImpl(JNIEnv *_env, jobject _this, jlong pointer,
                                     jfloat value) {
    if ((pointer & 0x3) == 0) {
        *((jfloat *) pointer) = value;
    } else {
        // Handle unaligned memory access one byte at a time
        memcpy((void *) pointer, (void *) &value, sizeof(jfloat));
    }
}

/*
 * Class:     org_apache_harmony_luni_platform_OSMemory
 * Method:    getDoubleImpl
 * Signature: (I)D
 */
static jdouble harmony_nio_getDoubleImpl(JNIEnv *_env, jobject _this,
                                         jlong pointer) {
    if ((pointer & 0x7) == 0) {
        jdouble returnValue = *((jdouble *) pointer);
        return returnValue;
    } else {
        // Handle unaligned memory access one byte at a time
        jdouble d;
        memcpy((void *) &d, (void *) pointer, sizeof(jdouble));
        return d;
    }
}

/*
 * Class:     org_apache_harmony_luni_platform_OSMemory
 * Method:    putDoubleImpl
 * Signature: (ID)V
 */
static void harmony_nio_putDoubleImpl(JNIEnv *_env, jobject _this, jlong pointer,
                                      jdouble value) {
    if ((pointer & 0x7) == 0) {
        *((jdouble *) pointer) = value;
    } else {
        // Handle unaligned memory access one byte at a time
        memcpy((void *) pointer, (void *) &value, sizeof(jdouble));
    }
}

/*
 * Class:     org_apache_harmony_luni_platform_OSMemory
 * Method:    getAddress
 * Signature: (I)I
 */
static jlong harmony_nio_getAddress(JNIEnv *_env, jobject _this, jlong pointer) {
    return (jlong) *(u8 *) pointer;
}

/*
 * Class:     org_apache_harmony_luni_platform_OSMemory
 * Method:    setAddress
 * Signature: (II)V
 */
static void harmony_nio_setAddress(JNIEnv *_env, jobject _this, jlong pointer,
                                   jlong value) {
    *(u8 *) pointer = (u8) value;
}

/*
 * Class:     org_apache_harmony_luni_platform_OSMemory
 * Method:    mmapImpl
 * Signature: (IJJI)I
 */
static jlong harmony_nio_mmapImpl(JNIEnv *_env, jobject _this, jint fd,
                                 jlong alignment, jlong size, jint mmode) {
    void *mapAddress = NULL;
    int prot, flags;

    // Convert from Java mapping mode to port library mapping mode.
    switch (mmode) {
        case MMAP_READ_ONLY:
            prot = PROT_READ;
            flags = MAP_SHARED;
            break;
        case MMAP_READ_WRITE:
            prot = PROT_READ | PROT_WRITE;
            flags = MAP_SHARED;
            break;
        case MMAP_WRITE_COPY:
            prot = PROT_READ | PROT_WRITE;
            flags = MAP_PRIVATE;
            break;
        default:
            return -1;
    }

    mapAddress = mmap(0, (size_t) (size & 0x7fffffff), prot, flags, fd,
                      (off_t) (alignment & 0x7fffffff));
    if (mapAddress == MAP_FAILED) {
        return -1;
    }

    return (jlong) mapAddress;
}

/*
 * Class:     org_apache_harmony_luni_platform_OSMemory
 * Method:    unmapImpl
 * Signature: (IJ)V
 */
static void harmony_nio_unmapImpl(JNIEnv *_env, jobject _this, jlong address,
                                  jlong size) {
    munmap((void *) address, (size_t) size);
}

/*
 * Class:     org_apache_harmony_luni_platform_OSMemory
 * Method:    loadImpl
 * Signature: (IJ)I
 */
static jint harmony_nio_loadImpl(JNIEnv *_env, jobject _this, jlong address,
                                 jlong size) {

    if (mlock((void *) address, (size_t) size) != -1) {
        if (munlock((void *) address, (size_t) size) != -1) {
            return 0;  /* normally */
        }
    } else {
        /* according to linux sys call, only root can mlock memory. */
        if (errno == EPERM) {
            return 0;
        }
    }

    return -1;
}

int getPageSize() {
    static int page_size = 0;
    if (page_size == 0) {
        page_size = getpagesize();
    }
    return page_size;
}

/*
 * Class:     org_apache_harmony_luni_platform_OSMemory
 * Method:    isLoadedImpl
 * Signature: (IJ)Z
 */
static jboolean harmony_nio_isLoadedImpl(JNIEnv *_env, jobject _this,
                                         jlong address, jlong size) {

    jboolean result = 0;
    jlong m_addr = (jlong) address;
    int page_size = getPageSize();
    unsigned char *vec = NULL;
    jlong page_count = 0;

    jlong align_offset = m_addr % page_size;// addr should align with the boundary of a page.
    m_addr -= align_offset;
    size += align_offset;
    page_count = (size + page_size - 1) / page_size;

    vec = (unsigned char *) malloc(page_count * sizeof(char));

    if (mincore((void *) m_addr, size, (MINCORE_POINTER_TYPE) vec) == 0) {
        // or else there is error about the mincore and return false;
        int i;
        for (i = 0; i < page_count; i++) {
            if (vec[i] != 1) {
                break;
            }
        }
        if (i == page_count) {
            result = 1;
        }
    }

    free(vec);

    return result;
}

/*
 * Class:     org_apache_harmony_luni_platform_OSMemory
 * Method:    flushImpl
 * Signature: (IJ)I
 */
static jint harmony_nio_flushImpl(JNIEnv *_env, jobject _this, jlong address,
                                  jlong size) {
    return msync((void *) address, size, MS_SYNC);
}

/*
 * JNI registration
 */
static JNINativeMethod gMethods[] = {
        /* name, signature, funcPtr */
        {"isLittleEndianImpl", "()Z",       (void *) harmony_nio_littleEndian},
        {"getPointerSizeImpl", "()I",       (void *) harmony_nio_getPointerSizeImpl},
        {"malloc",             "(I)J",      (void *) harmony_nio_mallocImpl},
        {"free",               "(J)V",      (void *) harmony_nio_freeImpl},
        {"memset",             "(JBJ)V",    (void *) harmony_nio_memset},
        {"memmove",            "(JJJ)V",    (void *) harmony_nio_memmove},
        {"getByteArray",       "(J[BII)V",  (void *) harmony_nio_getBytesImpl},
        {"setByteArray",       "(J[BII)V",  (void *) harmony_nio_putBytesImpl},
        {"setShortArray",      "(J[SIIZ)V", (void *) harmony_nio_putShortsImpl},
        {"setIntArray",        "(J[IIIZ)V", (void *) harmony_nio_putIntsImpl},
        {"getByte",            "(J)B",      (void *) harmony_nio_getByteImpl},
        {"setByte",            "(JB)V",     (void *) harmony_nio_putByteImpl},
        {"getShort",           "(J)S",      (void *) harmony_nio_getShortImpl},
        {"setShort",           "(JS)V",     (void *) harmony_nio_putShortImpl},
        {"getInt",             "(J)I",      (void *) harmony_nio_getIntImpl},
        {"setInt",             "(JI)V",     (void *) harmony_nio_putIntImpl},
        {"getLong",            "(J)J",      (void *) harmony_nio_getLongImpl},
        {"setLong",            "(JJ)V",     (void *) harmony_nio_putLongImpl},
        {"getFloat",           "(J)F",      (void *) harmony_nio_getFloatImpl},
        {"setFloat",           "(JF)V",     (void *) harmony_nio_putFloatImpl},
        {"getDouble",          "(J)D",      (void *) harmony_nio_getDoubleImpl},
        {"setDouble",          "(JD)V",     (void *) harmony_nio_putDoubleImpl},
        {"getAddress",         "(J)J",      (void *) harmony_nio_getAddress},
        {"setAddress",         "(JJ)V",     (void *) harmony_nio_setAddress},
        {"mmapImpl",           "(IJJI)J",   (void *) harmony_nio_mmapImpl},
        {"unmapImpl",          "(JJ)V",     (void *) harmony_nio_unmapImpl},
        {"loadImpl",           "(JJ)I",     (void *) harmony_nio_loadImpl},
        {"isLoadedImpl",       "(JJ)Z",     (void *) harmony_nio_isLoadedImpl},
        {"flushImpl",          "(JJ)I",     (void *) harmony_nio_flushImpl}
};

int register_org_apache_harmony_luni_platform_OSMemory(JNIEnv *_env) {
    return jniRegisterNativeMethods(_env, "org/apache/harmony/luni/platform/OSMemory",
                                    gMethods, NELEM(gMethods));
}
