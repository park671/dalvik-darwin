/* Copyright 2006 The Android Open Source Project */

/* A wrapper file for dlmalloc.c that compiles in the
 * mspace_*() functions, which provide an interface for
 * creating multiple heaps.
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/ioctl.h>

#include <ashmem.h>

/* It's a pain getting the mallinfo stuff to work
 * with Linux, OSX, and klibc, so just turn it off
 * for now.
 * TODO: make mallinfo work
 */
#define NO_MALLINFO 1

/* Allow setting the maximum heap footprint.
 */
#define USE_MAX_ALLOWED_FOOTPRINT 1

/* Don't try to trim memory.
 * TODO: support this.
 */
#define MORECORE_CANNOT_TRIM 1

/* Use mmap()d anonymous memory to guarantee
 * that an mspace is contiguous.
 *
 * create_mspace() won't work right if this is
 * defined, so hide the definition of it and
 * break any users at build time.
 */
#define USE_CONTIGUOUS_MSPACES 1
#if USE_CONTIGUOUS_MSPACES
/* This combination of settings forces sys_alloc()
 * to always use MORECORE().  It won't expect the
 * results to be contiguous, but we'll guarantee
 * that they are.
 */
#define HAVE_MMAP 1
#define HAVE_MORECORE 1
#define MORECORE_CONTIGUOUS 0
/* m is always the appropriate local when MORECORE() is called. */
#define MORECORE(S) contiguous_mspace_morecore(m, S)
#define create_mspace   HIDDEN_create_mspace_HIDDEN
#define destroy_mspace   HIDDEN_destroy_mspace_HIDDEN
typedef struct malloc_state *mstate0;

static void *contiguous_mspace_morecore(mstate0 m, ssize_t nb);
#endif

#define MSPACES 1
#define ONLY_MSPACES 1
#include "dlmalloc.c"
#include "Common.h"

#ifndef PAGESIZE
#define PAGESIZE  mparams.page_size
#endif

#define ALIGN_TO_PAGE_SIZE(x) (((u_int64_t)(x) + PAGESIZE - 1) & ~(PAGESIZE - 1))

/* A direct copy of dlmalloc_usable_size(),
 * which isn't compiled in when ONLY_MSPACES is set.
 * The mspace parameter isn't actually necessary,
 * but we include it to be consistent with the
 * rest of the mspace_*() functions.
 */
size_t mspace_usable_size(mspace _unused, const void *mem) {
    if (mem != 0) {
        const mchunkptr p = mem2chunk(mem);
        if (cinuse(p))
            return chunksize(p) - overhead_for(p);
    }
    return 0;
}

#if USE_CONTIGUOUS_MSPACES
#include <sys/mman.h>
#include <limits.h>

#define CONTIG_STATE_MAGIC  0xf00dd00d

struct mspace_contig_state {
    unsigned int magic;
    char *brk;
    char *top;
    mspace m;
};

static void *contiguous_mspace_morecore(mstate m, ssize_t nb) {
    struct mspace_contig_state *cs;
    char *oldbrk;
    const u8 pagesize = PAGESIZE;

    cs = (struct mspace_contig_state *) ((u_int64_t) m & ~(pagesize - 1));
    assert(cs->magic == CONTIG_STATE_MAGIC);
    assert(cs->m == m);
    assert(nb >= 0); //xxx deal with the trim case

    oldbrk = cs->brk;
    if (nb > 0) {
        /* Break to the first page boundary that satisfies the request.
         */
        char *newbrk = (char *) ALIGN_TO_PAGE_SIZE(oldbrk + nb);
        if (newbrk > cs->top)
            return CMFAIL;

        /* Update the protection on the underlying memory.
         * Pages we've given to dlmalloc are read/write, and
         * pages we haven't are not accessable (read or write
         * will cause a seg fault).
         */
        if (mprotect(cs, newbrk - (char *) cs, PROT_READ | PROT_WRITE) < 0)
            return CMFAIL;
        if (newbrk != cs->top) {
            if (mprotect(newbrk, cs->top - newbrk, PROT_NONE) < 0)
                return CMFAIL;
        }

        cs->brk = newbrk;

        /* Make sure that dlmalloc will merge this block with the
         * initial block that was passed to create_mspace_with_base().
         * We don't care about extern vs. non-extern, so just clear it.
         */
        m->seg.sflags &= ~EXTERN_BIT;
    }

    return oldbrk;
}

mspace create_contiguous_mspace_with_name(size_t starting_capacity,
                                          size_t max_capacity, int locked, char const *name) {
    int fd, ret;
    struct mspace_contig_state *cs;
    char buf[ASHMEM_NAME_LEN] = "mspace";
    void *base;
    u8 pagesize;
    mstate m;

    if (starting_capacity > max_capacity) {
        return (mspace) 0;
    }


    init_mparams();
    pagesize = PAGESIZE;
    printf("[+] pagesize=%u\n", pagesize);

    /* Create the anonymous memory that will back the mspace.
     * This reserves all of the virtual address space we could
     * ever need.  Physical pages will be mapped as the memory
     * is touched.
     *
     * Align max_capacity to a whole page.
     */
    max_capacity = (size_t) ALIGN_TO_PAGE_SIZE(max_capacity);

    if (name)
        snprintf(buf, sizeof(buf), "mspace/%s", name);
    fd = ashmem_create_region(buf, max_capacity);
    printf("[+] create fp success:%d\n", fd);
    if (fd < 0) {
        return (mspace) 0;
    }

    base = mmap(NULL, max_capacity, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    printf("[+] mmap mem adr:%p\n", base);
    close(fd);
    if (base == MAP_FAILED) {
        return (mspace) 0;
    }

    /* Make sure that base is at the beginning of a page.
     */
    assert(((u_int64_t)base & (pagesize-1)) == 0);

    /* Reserve some space for the information that our MORECORE needs.
     */
    cs = base;

    /* Create the mspace, pointing to the memory we just reserved.
     */
    m = create_mspace_with_base(base + sizeof(*cs), starting_capacity, locked);
    if (m == (mspace) 0)
        goto error;

    /* Make sure that m is in the same page as cs.
     */
    assert(((u_int64_t)m & (u_int64_t)~(pagesize-1)) == (u_int64_t)base);

    /* Find out exactly how much of the memory the mspace
     * is using.
     */
    cs->brk = m->seg.base + m->seg.size;
    cs->top = (char *) base + max_capacity;
    assert((char *)base <= cs->brk);
    assert(cs->brk <= cs->top);

    /* Prevent access to the memory we haven't handed out yet.
     */
    if (cs->brk != cs->top) {
        /* mprotect() requires page-aligned arguments, but it's possible
         * for cs->brk not to be page-aligned at this point.
         */
        char *prot_brk = (char *) ALIGN_TO_PAGE_SIZE(cs->brk);
        if (mprotect(prot_brk, cs->top - prot_brk, PROT_NONE) < 0) {
            printf("mem:[%p, %p](%dKB), pagesize:%dKB, align up to:[%p, %p](%dKB), errno:%d\n",
                cs->brk,
                cs->top,
                (cs->top - cs->brk) / 1024,
                pagesize/1024,
                prot_brk,
                cs->top,
                (cs->top - prot_brk) / 1024,
                errno);
            goto error;
        }
    }

    cs->m = m;
    cs->magic = CONTIG_STATE_MAGIC;

    return (mspace) m;

error:
    munmap(base, max_capacity);
    return (mspace) 0;
}

mspace create_contiguous_mspace(size_t starting_capacity,
                                size_t max_capacity, int locked) {
    return create_contiguous_mspace_with_name(starting_capacity,
                                              max_capacity, locked, NULL);
}

size_t destroy_contiguous_mspace(mspace msp) {
    mstate ms = (mstate) msp;

    if (ok_magic(ms)) {
        struct mspace_contig_state *cs;
        size_t length;
        u8 pagesize = PAGESIZE;

        cs = (struct mspace_contig_state *) ((u_int64_t) ms & ~(pagesize - 1));
        assert(cs->magic == CONTIG_STATE_MAGIC);
        assert(cs->m == ms);

        length = cs->top - (char *) cs;
        if (munmap((char *) cs, length) != 0)
            return length;
    } else {
        USAGE_ERROR_ACTION(ms, ms);
    }
    return 0;
}
#endif
