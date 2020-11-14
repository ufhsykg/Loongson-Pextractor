#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x7c1fbeb5, __VMLINUX_SYMBOL_STR(module_layout) },
	{ 0x25f854b, __VMLINUX_SYMBOL_STR(class_destroy) },
	{ 0x1ee002dd, __VMLINUX_SYMBOL_STR(device_destroy) },
	{ 0x7485e15e, __VMLINUX_SYMBOL_STR(unregister_chrdev_region) },
	{ 0xb9033bd, __VMLINUX_SYMBOL_STR(cdev_del) },
	{ 0xd034bdd1, __VMLINUX_SYMBOL_STR(device_create) },
	{ 0xe1d1e0c4, __VMLINUX_SYMBOL_STR(__class_create) },
	{ 0x8094f56d, __VMLINUX_SYMBOL_STR(cdev_add) },
	{ 0xbb10d0a8, __VMLINUX_SYMBOL_STR(cdev_init) },
	{ 0x29537c9e, __VMLINUX_SYMBOL_STR(alloc_chrdev_region) },
	{ 0xdd3607f2, __VMLINUX_SYMBOL_STR(cpu_data) },
	{ 0x27e1a049, __VMLINUX_SYMBOL_STR(printk) },
	{ 0xf9a482f9, __VMLINUX_SYMBOL_STR(msleep) },
	{ 0x5397c01e, __VMLINUX_SYMBOL_STR(__copy_user) },
	{ 0xe89b68b6, __VMLINUX_SYMBOL_STR(_raw_spin_unlock_irqrestore) },
	{ 0x610090ee, __VMLINUX_SYMBOL_STR(_raw_spin_lock_irqsave) },
	{ 0x49dc1943, __VMLINUX_SYMBOL_STR(ls2k_io_lock) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

MODULE_INFO(rhelversion, "7.4");
