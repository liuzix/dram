#include <linux/module.h>  
#include <linux/kernel.h>  
#include <linux/init.h>  
#include <net/sock.h>  
#include <linux/socket.h>  
#include <linux/net.h>  
#include <asm/types.h>  
#include <linux/netlink.h>  
#include <linux/skbuff.h>  
#include <asm/pgtable.h>
#include <linux/mm.h>

MODULE_LICENSE("GPL");

#define NETLINK_USER 31  

static uint64_t translate_address(uint64_t vaddr, int pid);

struct sock *nl_sk = NULL; 


/* Handles lookup requests from use space  */
static void nl_recv_msg(struct sk_buff *skb) {
    struct nlmsghdr *nlh;
    int pid;
    struct sk_buff *skb_out;
    int msg_size;
    char *msg = "Hello from kernel";
    int res;

    msg_size = strlen(msg);

    nlh = (struct nlmsghdr *)skb->data;
    printk(KERN_INFO "Netlink received msg payload:0x%lx\n", *(uint64_t*)nlmsg_data(nlh));
    uint64_t _vaddr = *(uint64_t*)nlmsg_data(nlh);
    pid = nlh->nlmsg_pid; /*pid of sending process */

    uint64_t paddr = translate_address(_vaddr, pid);
    printk(KERN_INFO "Translating 0x%llx to 0x%llx", _vaddr, paddr);

    skb_out = nlmsg_new(msg_size, 0);
    if (!skb_out) {
        printk(KERN_ERR "Failed to allocate new skb\n");
        return;
    }

    nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, msg_size, 0);
    NETLINK_CB(skb_out).dst_group = 0; /* not in mcast group */
    strncpy(nlmsg_data(nlh), msg, msg_size);

    res = nlmsg_unicast(nl_sk, skb_out, pid);
    if (res < 0)
        printk(KERN_INFO "Error while sending bak to user\n");
}  

/* initializes Netlink mechanism to prepare to accept requests from user space */
int init_netlink(void) {
    struct netlink_kernel_cfg cfg = {
        .input = nl_recv_msg,
    };

    nl_sk = netlink_kernel_create(&init_net, NETLINK_USER, &cfg);
    if (!nl_sk) {
        printk(KERN_ALERT "Error creating socket.\n");
        return -1;
    }

    
}

int free_netlink(void) {
    if (nl_sk) {
        netlink_kernel_release(nl_sk);
    }
}

/* walks page table to translate process-specific virtual address to physical address */

static uint64_t translate_address(uint64_t vaddr, int pid) {
    pgd_t *pgd = NULL;
    pte_t *ptep = NULL, pte;
    pud_t *pud  = NULL;
    pmd_t *pmd  = NULL;


    /* Find the virtual memory structure for the requesting process */
    struct task_struct* task =  pid_task(find_get_pid(pid), PIDTYPE_PID);
    struct mm_struct* mm =  get_task_mm(task);


    struct page *page = NULL;
    pgd = pgd_offset(mm, vaddr);
    if (pgd_none(*pgd) || pgd_bad(*pgd))
        goto out;

    pud = pud_offset(pgd, vaddr);
    if (pud_none(*pud) || pud_bad(*pud))
        goto out;

    pmd = pmd_offset(pud, vaddr);
    if (pmd_none(*pmd) || pmd_bad(*pmd))
        goto out;

    ptep = pte_offset_map(pmd, vaddr);
    if (pte_none(*ptep)) {
        return -1;
    }
    pte = *ptep;

    page = pte_page(pte);
    if (page) {
        uint64_t page_addr = pte_val(pte) & PAGE_MASK;
        printk(KERN_INFO "physical address = %p", page_addr);
        printk(KERN_INFO "PTE = 0x%llx\n", pte_val(pte));
        return page_addr;
    }
out: 
    return 0;

}
