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
 * Open an unoptimized DEX file.
 */
#include "Dalvik.h"

/*
 * Open an unoptimized DEX file.  This finds the optimized version in the
 * cache, constructing it if necessary.
 */
int dvmRamOdexFileOpen(
        void *baseAddr,
        size_t size,
        const char *odexOutputName,
        RamOdexFile **ppDexFile,
        bool isBootstrap) {
    DvmDex *pDvmDex = NULL;
    int result = -1;
    /*
     * Map the cached version.  This immediately rewinds the fd, so it
     * doesn't have to be seeked anywhere in particular.
     */
    if (dvmDexFileOpenFromArray(baseAddr, size, &pDvmDex) != 0) {
        LOGE("Unable to map ram odex in %p(length=%d)\n", baseAddr, size);
        goto bail;
    }

    LOGV("Successfully opened ram odex in '%p'(size=%d)\n", baseAddr, size);

    *ppDexFile = (RamOdexFile *) calloc(1, sizeof(RamOdexFile));
    (*ppDexFile)->baseAddr = baseAddr;
    (*ppDexFile)->size = size;
    (*ppDexFile)->pDvmDex = pDvmDex;
    result = 0;

    bail:
    return result;
}

/*
 * Close a RamOdexFile and free the struct.
 */
void dvmRamOdexFileFree(RamOdexFile *pRamOdexFile) {
    if (pRamOdexFile == NULL)
        return;

    dvmDexFileFree(pRamOdexFile->pDvmDex);
    free(pRamOdexFile);
}

