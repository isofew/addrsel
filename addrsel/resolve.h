//
//  resolve.h
//  addrsel
//

#ifndef __addrsel__resolve__
#define __addrsel__resolve__

#include <mach/mach_types.h>
#include <mach-o/loader.h>
#include <sys/fcntl.h>
#include <sys/systm.h>
#include <sys/types.h>
#include <sys/vnode.h>

void* find_base();
void* find_symbol(void* base, const char* name);

#endif /* defined(__addrsel__resolve__) */
