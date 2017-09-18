#include "mock_data.h"
#include <linux/module.h>
#include <linux/random.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <asm/io.h>
MODULE_LICENSE("GPL");
#define total_pages (1024 * 1024)

#define expected_faults_per_page 51

static struct fault_per_page* fault_data;

// a very simple linear random number generator
static int get_random_variable (int target, int total) {
    static uint32_t x = 0x4567;
    uint32_t i = (0x1234 * x + 0x7890) % 0x9876;
    x = i;
    if (i % total <= target) 
        return 1;
    else 
        return 0;
}

/* Generate mock fault data, and store it in kernel space memory */
int generate_mock_data(void) {
    fault_data = vmalloc(sizeof(struct fault_per_page) * total_pages);
    if (!fault_data) {
        printk(KERN_ERR "vmalloc failed!");
        return -1;
    }

    int i, j;
    for (i = 0; i < total_pages; i++) {
        fault_data[i].len = 0;
        fault_data[i].fault_addresses 
            = kmalloc (expected_faults_per_page * 2, GFP_KERNEL);
        if (!fault_data[i].fault_addresses) {
            printk(KERN_ERR "malloc failed!");
            return -1;
        }
        uint64_t memsize = expected_faults_per_page * 2;
        
        for (j = 0; j < 4096; j += 8) { // granularity is 8 bytes
            // j is the offset within a page
            if (!get_random_variable(expected_faults_per_page, 512)) 
                continue;

            fault_data[i].len ++;
            if (memsize < fault_data[i].len * 2) {
                fault_data[i].fault_addresses 
                    = krealloc(fault_data[i].fault_addresses, memsize + 20, GFP_KERNEL);
                if (!fault_data[i].fault_addresses) {
                    printk(KERN_ERR "realloc failed!");
                    return -1;
                }
                memsize += 20;
            }
            fault_data[i].fault_addresses[fault_data[i].len - 1] = j;    

        }
        
    }
    return 0;
}

struct fault_per_page* lookup_fault_by_phys_page(uint64_t addr) {
    if (addr > total_pages * 4096) {
        return NULL;
    } else {
        return &fault_data[addr / 4096];
    }
}