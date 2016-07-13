//
//  control.c
//  addrsel
//

#include "control.h"

static errno_t pass() {return 0;}

static Setter setter;
static errno_t on_set(
                      kern_ctl_ref ref,
                      u_int32_t unit,
                      void* userdata,
                      int opt,
                      void* data,
                      size_t len
                     )
{
    return setter(opt, data, (size_t)len);
}

int control_register(Setter s)
{
    setter = s;
    struct kern_ctl_reg entry = {
        PRODUCT_ID,
        0, 0, // id, unit
        CTL_FLAG_PRIVILEGED, // flags
        0, 0, // sendsize, recvsize
        pass, // connect
        pass, // disconnect
        pass, // send
        on_set, // setopt
        pass // getopt
    };
    kern_ctl_ref ref;
    return ctl_register(&entry, &ref);
}