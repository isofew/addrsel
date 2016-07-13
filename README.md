AddrSel
------
Address Selection Utility for Mac OS X

### Motivation

When running on a dual-stack network, the host needs to select which stack to use. Apple uses an improved version of Happy Eyeballs ([RFC6555](https://tools.ietf.org/html/rfc6555)) to address the issue. The algorithm dynamically makes decisions based on latency and other factors, providing a fast Internet experience.

While this is the favored behavior to most users, network administrators could be annoyed. They would like to configure the policy table (described in [RFC3484](https://tools.ietf.org/html/rfc3484)) statically, so they can have direct control for sure.

Therefore, I decide to bring the missing ADDRSEL section back. It was once enabled in the bsd part of xnu kernel, implementing several functions to configure the policy table. I copy that part of code into a kext and wrap it up. And here it is.

### How to use

1. Build the xcode project
2. `sudo chown root:wheel addrsel.kext`
3. `sudo kextload addrsel.kext` (you may need to disable SIP or kext signature check)
4. `sudo ./ip6addrctl `

### Stuff used to make this

 * [xnu/bsd/netinet6/in6_src.c](https://github.com/opensource-apple/xnu): configure policy table
 * [KernelResolver](https://github.com/rc0r/KernelResolver): resolve un-exported symbol (addrsel_policytab)
 * [Kernel Control API](https://developer.apple.com/library/prerelease/content/documentation/Darwin/Conceptual/NKEConceptual/control/control.html): communicate with userspace
 * [ip6addrctl.c](https://github.com/practicalswift/osx): command line interface
