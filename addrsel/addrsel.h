//
//  addrsel.h
//  addrsel
//
//  Created by isofew on 7/13/16.
//  Copyright (c) 2016 isofew. All rights reserved.
//

#ifndef addrsel_addrsel_h
#define addrsel_addrsel_h

#include "common.h"
#include "resolve.h"
#include "control.h"

#include <mach/mach_types.h>
#include <sys/systm.h>
#include <sys/param.h>

#include <net/if.h>
#include <netinet/ip.h>
#include <netinet6/in6_var.h>

kern_return_t addrsel_start(kmod_info_t * ki, void *d);
kern_return_t addrsel_stop(kmod_info_t *ki, void *d);

#endif
