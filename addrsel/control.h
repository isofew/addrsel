//
//  control.h
//  addrsel
//

#ifndef __addrsel__control__
#define __addrsel__control__

#include "common.h"

#include <mach/mach_types.h>
#include <sys/systm.h>
#include <sys/socket.h>
#include <sys/kern_control.h>

typedef errno_t (*Setter)(
                          int opt,
                          void* data,
                          size_t len
                         );

int control_register(Setter);

#endif /* defined(__addrsel__control__) */
