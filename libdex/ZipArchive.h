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
 * Read-only access to Zip archives, with minimal heap allocation.
 */
#ifndef _LIBDEX_ZIPARCHIVE
#define _LIBDEX_ZIPARCHIVE

#include "SysUtil.h"
#include "DexFile.h"            // need extern inline __attribute__((gnu_inline, always_inline))
#include "../Common.h"


/*
 * Trivial typedef to ensure that ZipEntry is not treated as a simple
 * integer.  We use NULL to indicate an invalid value.
 */
typedef void* ZipEntry;

/*
 * One entry in the hash table.
 */
typedef struct ZipHashEntry {
    const char*     name;
    unsigned short  nameLen;
    //unsigned int    hash;
} ZipHashEntry;

/*
 * Read-only Zip archive.
 *
 * We want "open" and "find entry by name" to be fast operations, and we
 * want to use as little memory as possible.  We memory-map the file,
 * and load a hash table with pointers to the filenames (which aren't
 * null-terminated).  The other fields are at a fixed offset from the
 * filename, so we don't need to extract those (but we do need to byte-read
 * and endian-swap them every time we want them).
 *
 * To speed comparisons when doing a lookup by name, we could make the mapping
 * "private" (copy-on-write) and null-terminate the filenames after verifying
 * the record structure.  However, this requires a private mapping of
 * every page that the Central Directory touches.  Easier to tuck a copy
 * of the string length into the hash table entry.
 */
typedef struct ZipArchive {
    /* open Zip archive */
    int         mFd;

    /* mapped file */
    MemMapping  mMap;

    /* number of entries in the Zip archive */
    int         mNumEntries;

    /*
     * We know how many entries are in the Zip archive, so we can have a
     * fixed-size hash table.  We probe on collisions.
     */
    int         mHashTableSize;
    ZipHashEntry* mHashTable;
} ZipArchive;

/* Zip compression methods we support */
enum {
    kCompressStored     = 0,        // no compression
    kCompressDeflated   = 8,        // standard deflate
};


/*
 * Open a Zip archive.
 *
 * On success, returns 0 and populates "pArchive".  Returns nonzero errno
 * value on failure.
 */
int dexZipOpenArchive(const char* fileName, ZipArchive* pArchive);

/*
 * Like dexZipOpenArchive, but takes a file descriptor open for reading
 * at the start of the file.  The descriptor must be mappable (this does
 * not allow access to a stream).
 *
 * "debugFileName" will appear in error messages, but is not otherwise used.
 */
int dexZipPrepArchive(int fd, const char* debugFileName, ZipArchive* pArchive);

/*
 * Close archive, releasing resources associated with it.
 *
 * Depending on the implementation this could unmap pages used by classes
 * stored in a Jar.  This should only be done after unloading classes.
 */
void dexZipCloseArchive(ZipArchive* pArchive);

/*
 * Return the archive's file descriptor.
 */
extern inline __attribute__((gnu_inline, always_inline)) int dexZipGetArchiveFd(const ZipArchive* pArchive) {
    return pArchive->mFd;
}

/*
 * Find an entry in the Zip archive, by name.  Returns NULL if the entry
 * was not found.
 */
ZipEntry dexZipFindEntry(const ZipArchive* pArchive,
    const char* entryName);

/*
 * Retrieve one or more of the "interesting" fields.  Non-NULL pointers
 * are filled in.
 */
bool dexZipGetEntryInfo(const ZipArchive* pArchive, ZipEntry entry,
    int* pMethod, long* pUncompLen, long* pCompLen, off_t* pOffset,
    long* pModWhen, long* pCrc32);

/*
 * Simple accessors.
 */
extern inline __attribute__((gnu_inline, always_inline)) long dexGetZipEntryOffset(const ZipArchive* pArchive,
    const ZipEntry entry)
{
    off_t val = 0;
    dexZipGetEntryInfo(pArchive, entry, NULL, NULL, NULL, &val, NULL, NULL);
    return (long) val;
}
extern inline __attribute__((gnu_inline, always_inline)) long dexGetZipEntryUncompLen(const ZipArchive* pArchive,
    const ZipEntry entry)
{
    long val = 0;
    dexZipGetEntryInfo(pArchive, entry, NULL, &val, NULL, NULL, NULL, NULL);
    return val;
}
extern inline __attribute__((gnu_inline, always_inline)) long dexGetZipEntryModTime(const ZipArchive* pArchive,
    const ZipEntry entry)
{
    long val = 0;
    dexZipGetEntryInfo(pArchive, entry, NULL, NULL, NULL, NULL, &val, NULL);
    return val;
}
extern inline __attribute__((gnu_inline, always_inline)) long dexGetZipEntryCrc32(const ZipArchive* pArchive,
    const ZipEntry entry)
{
    long val = 0;
    dexZipGetEntryInfo(pArchive, entry, NULL, NULL, NULL, NULL, NULL, &val);
    return val;
}

/*
 * Uncompress and write an entry to a file descriptor.
 */
bool dexZipExtractEntryToFile(const ZipArchive* pArchive,
    const ZipEntry entry, int fd);

/*
 * Utility function to compute a CRC-32.
 */
u4 dexInitCrc32(void);
u4 dexComputeCrc32(u4 crc, const void* buf, size_t len);

#endif /*_LIBDEX_ZIPARCHIVE*/
