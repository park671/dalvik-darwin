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
 * Functions for interpreting LEB128 (little endian base 128) values
 */

#ifndef LIBDEX_LEB128
#define LIBDEX_LEB128

#include "DexFile.h"
#include "../Common.h"

/*
 * Reads an unsigned LEB128 value, updating the given pointer to point
 * just past the end of the read value. This function tolerates
 * non-zero high-order bits in the fifth encoded byte.
 */
extern inline __attribute__((gnu_inline, always_inline)) int readUnsignedLeb128(const u1 **pStream) {
    const u1 *ptr = *pStream;
    int result = *(ptr++);

    if (result > 0x7f) {
        int cur = *(ptr++);
        result = (result & 0x7f) | ((cur & 0x7f) << 7);
        if (cur > 0x7f) {
            cur = *(ptr++);
            result |= (cur & 0x7f) << 14;
            if (cur > 0x7f) {
                cur = *(ptr++);
                result |= (cur & 0x7f) << 21;
                if (cur > 0x7f) {
                    /*
                     * Note: We don't check to see if cur is out of
                     * range here, meaning we tolerate garbage in the
                     * high four-order bits.
                     */
                    cur = *(ptr++);
                    result |= cur << 28;
                }
            }
        }
    }

    *pStream = ptr;
    return result;
}

/*
 * Reads a signed LEB128 value, updating the given pointer to point
 * just past the end of the read value. This function tolerates
 * non-zero high-order bits in the fifth encoded byte.
 */
extern inline __attribute__((gnu_inline, always_inline)) int readSignedLeb128(const u1 **pStream) {
    const u1 *ptr = *pStream;
    int result = *(ptr++);

    if (result <= 0x7f) {
        result = (result << 25) >> 25;
    } else {
        int cur = *(ptr++);
        result = (result & 0x7f) | ((cur & 0x7f) << 7);
        if (cur <= 0x7f) {
            result = (result << 18) >> 18;
        } else {
            cur = *(ptr++);
            result |= (cur & 0x7f) << 14;
            if (cur <= 0x7f) {
                result = (result << 11) >> 11;
            } else {
                cur = *(ptr++);
                result |= (cur & 0x7f) << 21;
                if (cur <= 0x7f) {
                    result = (result << 4) >> 4;
                } else {
                    /*
                     * Note: We don't check to see if cur is out of
                     * range here, meaning we tolerate garbage in the
                     * high four-order bits.
                     */
                    cur = *(ptr++);
                    result |= cur << 28;
                }
            }
        }
    }

    *pStream = ptr;
    return result;
}

/*
 * Reads an unsigned LEB128 value, updating the given pointer to point
 * just past the end of the read value and also indicating whether the
 * value was syntactically valid. The only syntactically *invalid*
 * values are ones that are five bytes long where the final byte has
 * any but the low-order four bits set. Additionally, if the limit is
 * passed as non-NULL and bytes would need to be read past the limit,
 * then the read is considered invalid.
 */
int readAndVerifyUnsignedLeb128(const u1 **pStream, const u1 *limit,
                                bool *okay);

/*
 * Reads a signed LEB128 value, updating the given pointer to point
 * just past the end of the read value and also indicating whether the
 * value was syntactically valid. The only syntactically *invalid*
 * values are ones that are five bytes long where the final byte has
 * any but the low-order four bits set. Additionally, if the limit is
 * passed as non-NULL and bytes would need to be read past the limit,
 * then the read is considered invalid.
 */
int readAndVerifySignedLeb128(const u1 **pStream, const u1 *limit, bool *okay);

#endif
