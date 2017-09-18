#ifndef INTERFACE_H
#define INTERFACE_H
int init_netlink(void);
int free_netlink(void);
uint64_t translate_address(uint64_t vaddr, int pid);
#endif