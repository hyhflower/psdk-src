#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;
BUILD_LTO_INFO;

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
	{ 0xc8d01d53, "module_layout" },
	{ 0xae2ae519, "param_ops_int" },
	{ 0x453fa2e0, "param_array_ops" },
	{ 0x7d6dad01, "platform_device_unregister" },
	{ 0x9d6d692d, "platform_driver_unregister" },
	{ 0xa231a288, "platform_device_put" },
	{ 0xe919b5fc, "platform_device_add" },
	{ 0xe607b6ae, "platform_device_add_data" },
	{ 0x9e40e35a, "platform_device_alloc" },
	{ 0xf78aa7ac, "__platform_driver_register" },
	{ 0xa6257a2f, "complete" },
	{ 0xd0da656b, "__stack_chk_fail" },
	{ 0x837b7b09, "__dynamic_pr_debug" },
	{ 0x6b10bee1, "_copy_to_user" },
	{ 0x1035c7c2, "__release_region" },
	{ 0x85bd1608, "__request_region" },
	{ 0xdbdf6c92, "ioport_resource" },
	{ 0x13c49cc2, "_copy_from_user" },
	{ 0x92d5838e, "request_threaded_irq" },
	{ 0x27704737, "gpiod_to_irq" },
	{ 0x25e007c6, "gpio_to_desc" },
	{ 0x30d4ff22, "device_create_file" },
	{ 0x3c3ff9fd, "sprintf" },
	{ 0xc3fcadfd, "devm_kmalloc" },
	{ 0x608741b5, "__init_swait_queue_head" },
	{ 0xc2aad509, "misc_register" },
	{ 0xf02aa937, "wait_for_completion_interruptible_timeout" },
	{ 0xd35cce70, "_raw_spin_unlock_irqrestore" },
	{ 0x34db050b, "_raw_spin_lock_irqsave" },
	{ 0x92997ed8, "_printk" },
	{ 0xe2d5255a, "strcmp" },
	{ 0x87a21cb3, "__ubsan_handle_out_of_bounds" },
	{ 0x5b8239ca, "__x86_return_thunk" },
	{ 0x4bc81f50, "misc_deregister" },
	{ 0xc1514a3b, "free_irq" },
	{ 0x8fcd0aa3, "device_remove_file" },
	{ 0x93d6dd8c, "complete_all" },
	{ 0xbdfb6dbb, "__fentry__" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "0187DC8F7AAABD7D8421DC8");
