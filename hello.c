#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
MODULE_LICENSE("GPL");
#include "mock_data.h"
#include "interface.h"

static int __init hello_init(void)
{
	printk(KERN_INFO "Hello, world\n");
	if (0 == generate_mock_data()) {
		printk(KERN_INFO "Successfully generated mock data\n");
	}

	init_netlink();
	return 0;
}

static void __exit hello_exit(void)
{
	free_netlink();
	printk(KERN_INFO "Goodbye, world 2\n");
}

module_init(hello_init);
module_exit(hello_exit);
