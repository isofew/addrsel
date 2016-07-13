//
//  control.h
//  ip6addrctl

#ifndef __addrsel__control__
#define __addrsel__control__

#include "common.h"

#include <sys/socket.h>
#include <sys/sys_domain.h>
#include <sys/kern_control.h>
#include <sys/ioctl.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void control_init();
errno_t control(int opt, void* data, size_t len);

#endif /* defined(__addrsel__control__) */
