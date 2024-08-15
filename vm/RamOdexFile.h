/**
 * create by park.yu
 */
#ifndef _DALVIK_RAM_ODEX_FILE
#define _DALVIK_RAM_ODEX_FILE

/*
 * Structure representing a "raw" DEX file, in its unswapped unoptimized
 * state.
 */
typedef struct RamOdexFile {
    void *baseAddr;
    size_t size;
    DvmDex *pDvmDex;
} RamOdexFile;

/*
 * Open a raw ".dex" file, optimize it, and load it.
 *
 * On success, returns 0 and sets "*ppDexFile" to a newly-allocated DexFile.
 * On failure, returns a meaningful error code [currently just -1].
 */
int dvmRamOdexFileOpen(
        void *baseAddr,
        size_t size,
        const char *odexOutputName,
        RamOdexFile **ppDexFile,
        bool isBootstrap);

/*
 * Free a RamOdexFile structure, along with any associated structures.
 */
void dvmRamOdexFileFree(RamOdexFile *pRamOdexFile);

/*
 * Pry the DexFile out of a RamOdexFile.
 */
INLINE DvmDex *dvmGetRamOdexFileDex(RamOdexFile *pRamOdexFile) {
    return pRamOdexFile->pDvmDex;
}

#endif /*_DALVIK_RAM_ODEX_FILE*/
