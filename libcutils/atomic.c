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

#include "atomic.h"
#include <sched.h>

/*****************************************************************************/

#include <libkern/OSAtomic.h>

void android_atomic_write(int32_t value, volatile int32_t* addr) {
    int32_t oldValue;
    do {
        oldValue = *addr;
    } while (OSAtomicCompareAndSwap32Barrier(oldValue, value, (int32_t*)addr) == 0);
}

int32_t android_atomic_inc(volatile int32_t* addr) {
    return OSAtomicIncrement32Barrier((int32_t*)addr)-1;
}

int32_t android_atomic_dec(volatile int32_t* addr) {
    return OSAtomicDecrement32Barrier((int32_t*)addr)+1;
}

int32_t android_atomic_add(int32_t value, volatile int32_t* addr) {
    return OSAtomicAdd32Barrier(value, (int32_t*)addr)-value;
}

int32_t android_atomic_and(int32_t value, volatile int32_t* addr) {
    int32_t oldValue;
    do {
        oldValue = *addr;
    } while (OSAtomicCompareAndSwap32Barrier(oldValue, oldValue&value, (int32_t*)addr) == 0);
    return oldValue;
}

int32_t android_atomic_or(int32_t value, volatile int32_t* addr) {
    int32_t oldValue;
    do {
        oldValue = *addr;
    } while (OSAtomicCompareAndSwap32Barrier(oldValue, oldValue|value, (int32_t*)addr) == 0);
    return oldValue;
}

int32_t android_atomic_swap(int32_t value, volatile int32_t* addr) {
    int32_t oldValue;
    do {
        oldValue = *addr;
    } while (android_atomic_cmpxchg(oldValue, value, addr));
    return oldValue;
}

int android_atomic_cmpxchg(int32_t oldvalue, int32_t newvalue, volatile int32_t* addr) {
    return OSAtomicCompareAndSwap32Barrier(oldvalue, newvalue, (int32_t*)addr) == 0;
}

int android_quasiatomic_cmpxchg_64(int64_t oldvalue, int64_t newvalue,
        volatile int64_t* addr) {
    return OSAtomicCompareAndSwap64Barrier(oldvalue, newvalue,
            (int64_t*)addr) == 0;
}

int64_t android_quasiatomic_swap_64(int64_t value, volatile int64_t* addr) {
    int64_t oldValue;
    do {
        oldValue = *addr;
    } while (android_quasiatomic_cmpxchg_64(oldValue, value, addr));
    return oldValue;
}

int64_t android_quasiatomic_read_64(volatile int64_t* addr) {
    return OSAtomicAdd64Barrier(0, addr);
}
