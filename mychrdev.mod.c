#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0x972f93ab, "module_layout" },
	{ 0x6091b333, "unregister_chrdev_region" },
	{ 0xad39bb2b, "class_destroy" },
	{ 0x6ea4ef03, "class_unregister" },
	{ 0x37a0cba, "kfree" },
	{ 0xc8ab5383, "device_destroy" },
	{ 0xc959d152, "__stack_chk_fail" },
	{ 0x712605e5, "device_create" },
	{ 0xdc80a8de, "kmem_cache_alloc_trace" },
	{ 0xbdbfaccb, "kmalloc_caches" },
	{ 0xa89d7d2, "cdev_add" },
	{ 0xe3b72f06, "cdev_init" },
	{ 0x5b54da1d, "__class_create" },
	{ 0xe3ec2f2b, "alloc_chrdev_region" },
	{ 0x9c6febfc, "add_uevent_var" },
	{ 0x6b10bee1, "_copy_to_user" },
	{ 0x13c49cc2, "_copy_from_user" },
	{ 0x88db9f48, "__check_object_size" },
	{ 0x56470118, "__warn_printk" },
	{ 0xc5850110, "printk" },
	{ 0xbdfb6dbb, "__fentry__" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "CF468B09A4CC8C822DC0AA8");
