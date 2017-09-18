#ifndef MOCK_DATA_H
#define MOCK_DATA_H

#include <linux/types.h>

struct fault_per_page {
    uint16_t len;
    uint16_t* fault_addresses;      
};

int generate_mock_data(void);
struct fault_per_page* lookup_fault_by_phys_page(uint64_t addr);

#endif