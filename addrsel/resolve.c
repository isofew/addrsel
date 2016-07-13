//
//  resolve.c
//  addrsel
//
//  cleaned up by @isofew
//

/*
 * original KernelResolver.c
 * by snare (snare@ho.ax)
 *
 * Mountain Lion port
 * by @_rc0r
 *
 *
 *
 * This is a simple example of how to resolve symbols in the kernel from within
 * a kernel extension. Symbols can be solved by using the kernel image from disk
 * (find_symbol_from_disk) and from memory (find_symbol).
 *
 * See the following URLs for more info:
 * http://ho.ax/posts/2012/02/resolving-kernel-symbols/
 * and
 * http://reverse.put.as/2013/05/08/there-is-an-error-in-my-syscan-slides/
 *
 */

#include "resolve.h"

#ifdef DEBUG
#define DLOG(args...)   printf(args)
#elif
#define DLOG(args...)   /* */
#endif

struct descriptor_idt
{
    uint16_t offset_low;
    uint16_t seg_selector;
    uint8_t reserved;
    uint8_t flag;
    uint16_t offset_middle;
    uint32_t offset_high;
    uint32_t reserved2;
};

/* Borrowed from kernel source. It doesn't exist in Kernel.framework. */
struct nlist_64 {
    union {
        uint32_t  n_strx;   /* index into the string table */
    } n_un;
    uint8_t n_type;         /* type flag, see below */
    uint8_t n_sect;         /* section number or NO_SECT */
    uint16_t n_desc;        /* see <mach-o/stab.h> */
    uint64_t n_value;       /* value of this symbol (or stab offset) */
};

static struct segment_command_64 *
find_segment_64(struct mach_header_64 *mh, const char *segname)
{
    struct load_command *lc;
    struct segment_command_64 *seg, *foundseg = NULL;
    
    /* First LC begins straight after the mach header */
    lc = (struct load_command *)((uint64_t)mh + sizeof(struct mach_header_64));
    while ((uint64_t)lc < (uint64_t)mh + (uint64_t)mh->sizeofcmds) {
        if (lc->cmd == LC_SEGMENT_64) {
            /* Check load command's segment name */
            seg = (struct segment_command_64 *)lc;
            if (strcmp(seg->segname, segname) == 0) {
                foundseg = seg;
                break;
            }
        }
        
        /* Next LC */
        lc = (struct load_command *)((uint64_t)lc + (uint64_t)lc->cmdsize);
    }
    
    /* Return the segment (NULL if we didn't find it) */
    return foundseg;
}

static struct load_command *
find_load_command(struct mach_header_64 *mh, uint32_t cmd)
{
    struct load_command *lc, *foundlc;
    
    /* First LC begins straight after the mach header */
    lc = (struct load_command *)((uint64_t)mh + sizeof(struct mach_header_64));
    while ((uint64_t)lc < (uint64_t)mh + (uint64_t)mh->sizeofcmds) {
        if (lc->cmd == cmd) {
            foundlc = (struct load_command *)lc;
            break;
        }
        
        /* Next LC */
        lc = (struct load_command *)((uint64_t)lc + (uint64_t)lc->cmdsize);
    }
    
    /* Return the load command (NULL if we didn't find it) */
    return foundlc;
}

void *
find_symbol(void* base, const char *name)
{
    struct mach_header_64* mh = (struct mach_header_64*) base;

    struct symtab_command *msymtab = NULL;
    struct segment_command_64 *mlc = NULL;
    struct segment_command_64 *mlinkedit = NULL;
    void *mstrtab = NULL;
    
    struct nlist_64 *nl = NULL;
    char *str;
    uint64_t i;
    void *addr = NULL;
    
    /*
     * Check header
     */
    if (mh->magic != MH_MAGIC_64) {
        DLOG("FAIL: magic number doesn't match - 0x%x\n", mh->magic);
        return NULL;
    }
    
    /*
     * Find TEXT section
     */
    mlc = find_segment_64(mh, SEG_TEXT);
    if (!mlc) {
        DLOG("FAIL: couldn't find __TEXT\n");
        return NULL;
    }
    
    /*
     * Find the LINKEDIT and SYMTAB sections
     */
    mlinkedit = find_segment_64(mh, SEG_LINKEDIT);
    if (!mlinkedit) {
        DLOG("FAIL: couldn't find __LINKEDIT\n");
        return NULL;
    }
    
    msymtab = (struct symtab_command *)find_load_command(mh, LC_SYMTAB);
    if (!msymtab) {
        DLOG("FAIL: couldn't find SYMTAB\n");
        return NULL;
    }
    
    //DLOG( "[+] __TEXT.vmaddr      0x%016llX\n", mlc->vmaddr );
    //DLOG( "[+] __LINKEDIT.vmaddr  0x%016llX\n", mlinkedit->vmaddr );
    //DLOG( "[+] __LINKEDIT.vmsize  0x%08llX\n", mlinkedit->vmsize );
    //DLOG( "[+] __LINKEDIT.fileoff 0x%08llX\n", mlinkedit->fileoff );
    //DLOG( "[+] LC_SYMTAB.stroff   0x%08X\n", msymtab->stroff );
    //DLOG( "[+] LC_SYMTAB.strsize  0x%08X\n", msymtab->strsize );
    //DLOG( "[+] LC_SYMTAB.symoff   0x%08X\n", msymtab->symoff );
    //DLOG( "[+] LC_SYMTAB.nsyms    0x%08X\n", msymtab->nsyms );
    
    /*
     * Enumerate symbols until we find the one we're after
     *
     *  Be sure to use NEW calculation STRTAB in Mountain Lion!
     */
    mstrtab = (void *)((int64_t)mlinkedit->vmaddr + (msymtab->stroff - mlinkedit->fileoff));
    
    // First nlist_64 struct is NOW located @:
    for (i = 0, nl = (struct nlist_64 *)(mlinkedit->vmaddr + (msymtab->symoff - mlinkedit->fileoff));
         i < msymtab->nsyms;
         i++, nl = (struct nlist_64 *)((uint64_t)nl + sizeof(struct nlist_64)))
    {
        str = (char *)mstrtab + nl->n_un.n_strx;
        
        if (strcmp(str, name) == 0) {
            addr = (void *)nl->n_value;
        }
    }
    
    /* Return the address (NULL if we didn't find it) */
    return addr;
}

void* find_base( )
{
    uint8_t idtr[ 10 ];
    uint64_t idt = 0;
    
    __asm__ volatile ( "sidt %0": "=m" ( idtr ) );
    
    idt = *( ( uint64_t * ) &idtr[ 2 ] );
    struct descriptor_idt *int80_descriptor = NULL;
    uint64_t int80_address = 0;
    uint64_t high = 0;
    uint32_t middle = 0;
    
    int80_descriptor = _MALLOC( sizeof( struct descriptor_idt ), M_TEMP, M_WAITOK );
    bcopy( (void*)idt, int80_descriptor, sizeof( struct descriptor_idt ) );
    
    high = ( unsigned long ) int80_descriptor->offset_high << 32;
    middle = ( unsigned int ) int80_descriptor->offset_middle << 16;
    int80_address = ( uint64_t )( high + middle + int80_descriptor->offset_low );
    
    uint64_t temp_address = int80_address;
    uint8_t *temp_buffer = _MALLOC( 4, M_TEMP, M_WAITOK );
    
    while( temp_address > 0 )
    {
        bcopy( ( void * ) temp_address, temp_buffer, 4 );
        if ( *( uint32_t * )( temp_buffer ) == MH_MAGIC_64 )
        {
            return (void*)temp_address;
        }
        temp_address -= 1;
    }
    
    return 0;
}
