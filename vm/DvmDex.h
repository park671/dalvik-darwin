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
 * The VM wraps some additional data structures around the DexFile.  These
 * are defined here.
 */
#ifndef _DALVIK_DVMDEX
#define _DALVIK_DVMDEX

#include "libdex/DexFile.h"

/* extern */
struct ClassObject;
struct HashTable;
struct InstField;
struct Method;
struct StringObject;


/*
 * Some additional VM data structures that are associated with the DEX file.
 */
typedef struct DvmDex {
    /* pointer to the DexFile we're associated with */
    DexFile*            pDexFile;

    /* clone of pDexFile->pHeader (it's used frequently enough) */
    const DexHeader*    pHeader;

    /* interned strings; parallel to "stringIds" */
    struct StringObject** pResStrings;

    /* resolved classes; parallel to "typeIds" */
    struct ClassObject** pResClasses;

    /* resolved methods; parallel to "methodIds" */
    struct Method**     pResMethods;

    /* resolved instance fields; parallel to "fieldIds" */
    /* (this holds both InstField and StaticField) */
    struct Field**      pResFields;

    /* interface method lookup cache */
    struct AtomicCache* pInterfaceCache;

    /* shared memory region with file contents */
    MemMapping          memMap;
} DvmDex;


/*
 * Given a file descriptor for an open "optimized" DEX file, map it into
 * memory and parse the contents.
 *
 * On success, returns 0 and sets "*ppDvmDex" to a newly-allocated DvmDex.
 * On failure, returns a meaningful error code [currently just -1].
 */
int dvmDexFileOpenFromFd(int fd, DvmDex** ppDvmDex);

/*
 * Open a partial DEX file.  Only useful as part of the optimization process.
 */
int dvmDexFileOpenPartial(const void* addr, int len, DvmDex** ppDvmDex);

/*
 * Free a DvmDex structure, along with any associated structures.
 */
void dvmDexFileFree(DvmDex* pDvmDex);


#if DVM_RESOLVER_CACHE == DVM_RC_DISABLED
/* 1:1 mapping */

/*
 * Return the requested item if it has been resolved, or NULL if it hasn't.
 */
INLINE struct StringObject* dvmDexGetResolvedString(const DvmDex* pDvmDex,
    u4 stringIdx)
{
    assert(stringIdx < pDvmDex->pHeader->stringIdsSize);
    return pDvmDex->pResStrings[stringIdx];
}
INLINE struct ClassObject* dvmDexGetResolvedClass(const DvmDex* pDvmDex,
    u4 classIdx)
{
    assert(classIdx < pDvmDex->pHeader->typeIdsSize);
    return pDvmDex->pResClasses[classIdx];
}
INLINE struct Method* dvmDexGetResolvedMethod(const DvmDex* pDvmDex,
    u4 methodIdx)
{
    assert(methodIdx < pDvmDex->pHeader->methodIdsSize);
    return pDvmDex->pResMethods[methodIdx];
}
INLINE struct Field* dvmDexGetResolvedField(const DvmDex* pDvmDex,
    u4 fieldIdx)
{
    assert(fieldIdx < pDvmDex->pHeader->fieldIdsSize);
    return pDvmDex->pResFields[fieldIdx];
}

/*
 * Update the resolved item table.  Resolution always produces the same
 * result, so we're not worried about atomicity here.
 */
INLINE void dvmDexSetResolvedString(DvmDex* pDvmDex, u4 stringIdx,
    struct StringObject* str)
{
    assert(stringIdx < pDvmDex->pHeader->stringIdsSize);
    pDvmDex->pResStrings[stringIdx] = str;
}
INLINE void dvmDexSetResolvedClass(DvmDex* pDvmDex, u4 classIdx,
    struct ClassObject* clazz)
{
    assert(classIdx < pDvmDex->pHeader->typeIdsSize);
    pDvmDex->pResClasses[classIdx] = clazz;
}
INLINE void dvmDexSetResolvedMethod(DvmDex* pDvmDex, u4 methodIdx,
    struct Method* method)
{
    assert(methodIdx < pDvmDex->pHeader->methodIdsSize);
    pDvmDex->pResMethods[methodIdx] = method;
}
INLINE void dvmDexSetResolvedField(DvmDex* pDvmDex, u4 fieldIdx,
    struct Field* field)
{
    assert(fieldIdx < pDvmDex->pHeader->fieldIdsSize);
    pDvmDex->pResFields[fieldIdx] = field;
}

#elif DVM_RESOLVER_CACHE == DVM_RC_REDUCING
/* reduce request to fit in a less-than-full-size cache table */

/*
 * Return the requested item if it has been resolved, or NULL if it hasn't.
 *
 * If we have a mapping table defined for this category, but there's no
 * entry for this index, we always return NULL.  Otherwise, we return the
 * entry.  (To regain some performance we may want to assume that the
 * table exists when compiled in this mode -- avoids a null check but
 * prevents us from switching back and forth without rebuilding the VM.)
 *
 * We could save an integer compare here by ensuring that map[kNoIndexMapping]
 * always evalutes to NULL (e.g. set kNoIndexMapping = 0).
 */
INLINE struct StringObject* dvmDexGetResolvedString(const DvmDex* pDvmDex,
    u4 stringIdx)
{
    const DexIndexMap* pIndexMap = &pDvmDex->pDexFile->indexMap;

    assert(stringIdx < pDvmDex->pHeader->stringIdsSize);
    if (pIndexMap->stringReducedCount > 0) {
        stringIdx = pIndexMap->stringMap[stringIdx];
        if (stringIdx == kNoIndexMapping)
            return NULL;
    }
    return pDvmDex->pResStrings[stringIdx];
}
INLINE struct ClassObject* dvmDexGetResolvedClass(const DvmDex* pDvmDex,
    u4 classIdx)
{
    const DexIndexMap* pIndexMap = &pDvmDex->pDexFile->indexMap;

    assert(classIdx < pDvmDex->pHeader->typeIdsSize);
    if (pIndexMap->classReducedCount > 0) {
        classIdx = pIndexMap->classMap[classIdx];
        if (classIdx == kNoIndexMapping)
            return NULL;
    }
    return pDvmDex->pResClasses[classIdx];
}
INLINE struct Method* dvmDexGetResolvedMethod(const DvmDex* pDvmDex,
    u4 methodIdx)
{
    const DexIndexMap* pIndexMap = &pDvmDex->pDexFile->indexMap;

    assert(methodIdx < pDvmDex->pHeader->methodIdsSize);
    if (pIndexMap->methodReducedCount > 0) {
        methodIdx = pIndexMap->methodMap[methodIdx];
        if (methodIdx == kNoIndexMapping)
            return NULL;
    }
    return pDvmDex->pResMethods[methodIdx];
}
INLINE struct Field* dvmDexGetResolvedField(const DvmDex* pDvmDex,
    u4 fieldIdx)
{
    const DexIndexMap* pIndexMap = &pDvmDex->pDexFile->indexMap;

    assert(fieldIdx < pDvmDex->pHeader->fieldIdsSize);
    if (pIndexMap->fieldReducedCount > 0) {
        fieldIdx = pIndexMap->fieldMap[fieldIdx];
        if (fieldIdx == kNoIndexMapping)
            return NULL;
    }
    return pDvmDex->pResFields[fieldIdx];
}

/*
 * Update the resolved item table.  Resolution always produces the same
 * result, so we're not worried about atomicity here.
 */
INLINE void dvmDexSetResolvedString(DvmDex* pDvmDex, u4 stringIdx,
    struct StringObject* str)
{
    const DexIndexMap* pIndexMap = &pDvmDex->pDexFile->indexMap;
    u4 newIdx;

    assert(stringIdx < pDvmDex->pHeader->stringIdsSize);
    if (pIndexMap->stringReducedCount > 0) {
        newIdx = pIndexMap->stringMap[stringIdx];
        if (newIdx != kNoIndexMapping)
            pDvmDex->pResStrings[newIdx] = str;
    }
}
INLINE void dvmDexSetResolvedClass(DvmDex* pDvmDex, u4 classIdx,
    struct ClassObject* clazz)
{
    const DexIndexMap* pIndexMap = &pDvmDex->pDexFile->indexMap;
    u4 newIdx;

    assert(classIdx < pDvmDex->pHeader->typeIdsSize);
    if (pIndexMap->classReducedCount > 0) {
        newIdx = pIndexMap->classMap[classIdx];
        if (newIdx != kNoIndexMapping)
            pDvmDex->pResClasses[newIdx] = clazz;
    }
}
INLINE void dvmDexSetResolvedMethod(DvmDex* pDvmDex, u4 methodIdx,
    struct Method* method)
{
    const DexIndexMap* pIndexMap = &pDvmDex->pDexFile->indexMap;
    u4 newIdx;

    assert(methodIdx < pDvmDex->pHeader->methodIdsSize);
    if (pIndexMap->methodReducedCount > 0) {
        newIdx = pIndexMap->methodMap[methodIdx];
        if (newIdx != kNoIndexMapping)
            pDvmDex->pResMethods[newIdx] = method;
    }
}
INLINE void dvmDexSetResolvedField(DvmDex* pDvmDex, u4 fieldIdx,
    struct Field* field)
{
    const DexIndexMap* pIndexMap = &pDvmDex->pDexFile->indexMap;
    u4 newIdx;

    assert(fieldIdx < pDvmDex->pHeader->fieldIdsSize);
    if (pIndexMap->fieldReducedCount > 0) {
        newIdx = pIndexMap->fieldMap[fieldIdx];
        if (newIdx != kNoIndexMapping)
            pDvmDex->pResFields[newIdx] = field;
    }
}

#elif DVM_RESOLVER_CACHE == DVM_RC_EXPANDING

#error "not implemented"    /* TODO */

#elif DVM_RESOLVER_CACHE == DVM_RC_NO_CACHE

/*
 * There's no cache, so we always return NULL.
 */
INLINE struct StringObject* dvmDexGetResolvedString(const DvmDex* pDvmDex,
    u4 stringIdx)
{
    return NULL;
}
INLINE struct ClassObject* dvmDexGetResolvedClass(const DvmDex* pDvmDex,
    u4 classIdx)
{
    return NULL;
}
INLINE struct Method* dvmDexGetResolvedMethod(const DvmDex* pDvmDex,
    u4 methodIdx)
{
    return NULL;
}
INLINE struct Field* dvmDexGetResolvedField(const DvmDex* pDvmDex,
    u4 fieldIdx)
{
    return NULL;
}

/*
 * Update the resolved item table.  There is no table, so do nothing.
 */
INLINE void dvmDexSetResolvedString(DvmDex* pDvmDex, u4 stringIdx,
    struct StringObject* str)
{}
INLINE void dvmDexSetResolvedClass(DvmDex* pDvmDex, u4 classIdx,
    struct ClassObject* clazz)
{}
INLINE void dvmDexSetResolvedMethod(DvmDex* pDvmDex, u4 methodIdx,
    struct Method* method)
{}
INLINE void dvmDexSetResolvedField(DvmDex* pDvmDex, u4 fieldIdx,
    struct Field* field)
{}

#else
#error "huh?"
#endif /*DVM_RESOLVER_CACHE==N*/

#endif /*_DALVIK_DVMDEX*/
