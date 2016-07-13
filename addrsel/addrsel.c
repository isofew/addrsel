//
//  addrsel.c
//  addrsel
//

#include "addrsel.h"

#define	ADDRSEL_LOCK()
#define	ADDRSEL_UNLOCK()

#define ADDR_LABEL_NOTAPP (-1)

struct addrsel_policyent {
    TAILQ_ENTRY(addrsel_policyent) ape_entry;
    struct in6_addrpolicy ape_policy;
};

TAILQ_HEAD(addrsel_policyhead, addrsel_policyent);

struct addrsel_policyhead* addrsel_policytab;

static int
add_addrsel_policyent(const struct in6_addrpolicy *newpolicy)
{
    struct addrsel_policyent *new, *pol;
    
    MALLOC(new, struct addrsel_policyent *, sizeof (*new), M_IFADDR,
           M_WAITOK);
    
    ADDRSEL_LOCK();
    
    /* duplication check */
    TAILQ_FOREACH(pol, addrsel_policytab, ape_entry) {
        if (IN6_ARE_ADDR_EQUAL(&newpolicy->addr.sin6_addr,
                               &pol->ape_policy.addr.sin6_addr) &&
            IN6_ARE_ADDR_EQUAL(&newpolicy->addrmask.sin6_addr,
                               &pol->ape_policy.addrmask.sin6_addr)) {
                ADDRSEL_UNLOCK();
                FREE(new, M_IFADDR);
                return (EEXIST);	/* or override it? */
            }
    }
    
    bzero(new, sizeof (*new));
    
    /* XXX: should validate entry */
    new->ape_policy = *newpolicy;
    
    TAILQ_INSERT_TAIL(addrsel_policytab, new, ape_entry);
    ADDRSEL_UNLOCK();
    
    return (0);
}

static int
delete_addrsel_policyent(const struct in6_addrpolicy *key)
{
    struct addrsel_policyent *pol;
    
    ADDRSEL_LOCK();
    
    /* search for the entry in the table */
    TAILQ_FOREACH(pol, addrsel_policytab, ape_entry) {
        if (IN6_ARE_ADDR_EQUAL(&key->addr.sin6_addr,
                               &pol->ape_policy.addr.sin6_addr) &&
            IN6_ARE_ADDR_EQUAL(&key->addrmask.sin6_addr,
                               &pol->ape_policy.addrmask.sin6_addr)) {
                break;
            }
    }
    if (pol == NULL) {
        ADDRSEL_UNLOCK();
        return (ESRCH);
    }
    
    TAILQ_REMOVE(addrsel_policytab, pol, ape_entry);
    FREE(pol, M_IFADDR);
    pol = NULL;
    ADDRSEL_UNLOCK();
    
    return (0);
}

typedef int (*Type)(struct in6_addr *, u_char *);
Type in6_mask2len;

errno_t
in6_src_ioctl(int cmd, void* _data, size_t len)
{
    caddr_t data = _data;

    int i;
    struct in6_addrpolicy ent0;
    
    if (cmd != ADD_POLICY && cmd != DELETE_POLICY)
        return (EOPNOTSUPP); /* check for safety */

    bcopy(data, &ent0, sizeof (ent0));
    
    if (ent0.label == ADDR_LABEL_NOTAPP)
        return (EINVAL);
    
    /* check if the prefix mask is consecutive. */
    if (in6_mask2len(&ent0.addrmask.sin6_addr, NULL) < 0)
        return (EINVAL);
    
    /* clear trailing garbages (if any) of the prefix address. */
    for (i = 0; i < 4; i++) {
        ent0.addr.sin6_addr.s6_addr32[i] &=
        ent0.addrmask.sin6_addr.s6_addr32[i];
    }
    ent0.use = 0;
    
    switch (cmd) {
        case ADD_POLICY:
            return (add_addrsel_policyent(&ent0));
        case DELETE_POLICY:
            return (delete_addrsel_policyent(&ent0));
    }
    
    return (0);		/* XXX: compromise compilers */
}


kern_return_t addrsel_start(kmod_info_t * ki, void *d)
{
    void* base = find_base();
    
    addrsel_policytab = find_symbol(base, "_addrsel_policytab");
    //printf("addrsel_policytab is at %lx\n", (size_t)&addrsel_policytab);
    
    in6_mask2len = find_symbol(base, "_in6_mask2len");
    //printf("in6_mask2len is at %lx\n", (size_t)in6_mask2len);
    
    control_register(in6_src_ioctl);
    
    return KERN_SUCCESS;
}

kern_return_t addrsel_stop(kmod_info_t *ki, void *d)
{
    return KERN_SUCCESS;
}
