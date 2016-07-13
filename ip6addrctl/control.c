//
//  control.c
//  ip6addrctl
//

#include "control.h"

static void fatal(char* act, int ret)
{
    if (ret < 0) {
        printf("failed to %s: %d\n", act, ret);
        exit(ret);
    }
}

static int fd;

void control_init()
{
    fd = socket(PF_SYSTEM, SOCK_DGRAM, SYSPROTO_CONTROL);
    fatal("open socket", fd);
    
    struct ctl_info info = {
        0,
        PRODUCT_ID
    };
    fatal("get id", ioctl(fd, CTLIOCGINFO, &info));
    
    struct sockaddr_ctl addr = {
        sizeof(struct sockaddr_ctl),
        AF_SYSTEM,
        AF_SYS_CONTROL,
        info.ctl_id,
        0
    };
    memset(addr.sc_reserved, 0, sizeof(addr.sc_reserved));
    
    fatal("connect", connect(fd, (struct sockaddr*)&addr, sizeof(addr)));
}

errno_t control(int opt, void* data, size_t len)
{
    return setsockopt(fd, SYSPROTO_CONTROL, opt, data, len);
}