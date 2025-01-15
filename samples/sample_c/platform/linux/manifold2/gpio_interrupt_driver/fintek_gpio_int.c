/*
 *  Probe for F81216A LPC to 4 UART
 *
 *  Based on drivers/tty/serial/8250_pnp.c, by Russell King, et al
 *
 *  Copyright (C) 2014 Ricardo Ribalda, Qtechnology A/S
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License.
 */
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/pnp.h>
#include <linux/kernel.h>
//#include <linux/serial_core.h>
//#include <linux/serial_8250.h>
//#include <linux/serial_reg.h>
#include <linux/ioport.h>
#include <linux/version.h>
#include <linux/time.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/ioctl.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>

#include <linux/gpio.h>
#include <linux/interrupt.h>

#define DRVNAME "fintek-gpio-int"
#define DRVVER "v1.2-20240805"

#define MAX_IRQ_CNT 4
#define DISABLE_LOCK_IO 0

static int irq[MAX_IRQ_CNT];
module_param_array(irq, int, NULL, 0444);
MODULE_PARM_DESC(irq, "IRQ number, max 4 entry");

#define FINTEK_MAGIC 'S'
#define FINTEK_SET_IO _IOW(FINTEK_MAGIC, 0, struct sio_operation)
#define FINTEK_GET_IO _IOWR(FINTEK_MAGIC, 1, struct sio_operation)

#define FINTEK_SET_SIO _IOW(FINTEK_MAGIC, 2, struct sio_operation)
#define FINTEK_GET_SIO _IOWR(FINTEK_MAGIC, 3, struct sio_operation)

struct sio_operation {
	unsigned int port;

	unsigned char active_key;
	unsigned char reg;
	unsigned char data;
};

struct fintek_sio {
	u8 triggered[MAX_IRQ_CNT];
	spinlock_t lock;
	struct completion irq_comp[MAX_IRQ_CNT];
	struct device_attribute dev_irq[MAX_IRQ_CNT];
};

static struct fintek_sio m_sio_data;
static struct platform_device *gpio_dev;

#define DEVICE_NAME "fintek_io_device"

#if 0
static int fintek_sio_file_op_open(struct inode *inode, struct file *file)
{
	return 0;
}

static int fintek_sio_file_op_release(struct inode *inode, struct file *file)
{
	return 0;
}
#endif

static void dump_sio_op(const u8 *name, struct sio_operation *op)
{
	if (name)
		pr_info("%s\n", name);

	pr_info("port: %x\n", op->port);
	pr_info("reg: %x\n", op->reg);
	pr_info("data: %x\n", op->data);
}

static long fintek_ioctl_set_io(struct file *file, unsigned long arg)
{
	struct sio_operation op;

	if (copy_from_user(&op, (char __user *)arg, sizeof(op)))
		return -EFAULT;

#if !DISABLE_LOCK_IO
	if (!request_muxed_region(op.port, 1, DRVNAME)) {
		pr_err("%s: I/O address 0x%04x already in use\n", __func__,
		       op.port);
		return -EBUSY;
	}
#endif

	outb(op.data, op.port);
	pr_debug("%s: io: %x, write data: %x\n", __func__, op.port, op.data);

#if !DISABLE_LOCK_IO
	release_region(op.port, 1);
#endif

	return 0;
}

static long fintek_ioctl_get_io(struct file *file, unsigned long arg)
{
	struct sio_operation op;

	if (copy_from_user(&op, (char __user *)arg, sizeof(op)))
		return -EFAULT;

#if !DISABLE_LOCK_IO
	if (!request_muxed_region(op.port, 1, DRVNAME)) {
		pr_err("%s: I/O address 0x%04x already in use\n", __func__,
		       op.port);
		return -EBUSY;
	}
#endif

	op.data = inb(op.port);
	pr_debug("%s: io: %x, read data: %x\n", __func__, op.port, op.data);

#if !DISABLE_LOCK_IO
	release_region(op.port, 1);
#endif

	if (copy_to_user((char __user *)arg, &op, sizeof(op)))
		return -EFAULT;

	return 0;
}

static long fintek_ioctl_set_sio(struct file *file, unsigned long arg)
{
	struct sio_operation op;

	if (copy_from_user(&op, (char __user *)arg, sizeof(op)))
		return -EFAULT;

#if !DISABLE_LOCK_IO
	if (!request_muxed_region(op.port, 2, DRVNAME)) {
		pr_err("%s: I/O address 0x%04x already in use\n", __func__,
		       op.port);
		return -EBUSY;
	}
#endif

	outb(op.active_key, op.port + 0);
	outb(op.active_key, op.port + 0);

	outb(op.reg, op.port + 0);
	outb(op.data, op.port + 1);

	outb(0xaa, op.port + 0);

	pr_debug("%s: io: %x, reg: %x, write data: %x\n", __func__, op.port,
		 op.reg, op.data);

#if !DISABLE_LOCK_IO
	release_region(op.port, 2);
#endif

	return 0;
}

static long fintek_ioctl_get_sio(struct file *file, unsigned long arg)
{
	struct sio_operation op;

	if (copy_from_user(&op, (char __user *)arg, sizeof(op)))
		return -EFAULT;

#if !DISABLE_LOCK_IO
	if (!request_muxed_region(op.port, 2, DRVNAME)) {
		pr_err("%s: I/O address 0x%04x already in use\n", __func__,
		       op.port);
		return -EBUSY;
	}
#endif

	outb(op.active_key, op.port + 0);
	outb(op.active_key, op.port + 0);

	outb(op.reg, op.port + 0);
	op.data = inb(op.port + 1);

	outb(0xaa, op.port + 0);

	pr_debug("%s: io: %x, reg: %x, read data: %x\n", __func__, op.port,
		 op.reg, op.data);

#if !DISABLE_LOCK_IO
	release_region(op.port, 2);
#endif

	if (copy_to_user((char __user *)arg, &op, sizeof(op)))
		return -EFAULT;

	return 0;
}

static long fintek_sio_file_op_ioctl(struct file *file, unsigned int cmd,
				     unsigned long arg)
{
	switch (cmd) {
	case FINTEK_SET_IO:
		return fintek_ioctl_set_io(file, arg);

	case FINTEK_GET_IO:
		return fintek_ioctl_get_io(file, arg);

	case FINTEK_SET_SIO:
		return fintek_ioctl_set_sio(file, arg);

	case FINTEK_GET_SIO:
		return fintek_ioctl_get_sio(file, arg);
	}

	return -ENOTTY;
}

static struct file_operations fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = fintek_sio_file_op_ioctl,
	//.open = fintek_sio_file_op_open,
	//.release = fintek_sio_file_op_release,
};

static struct miscdevice misc_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = DEVICE_NAME,
	.fops = &fops,
};

static irqreturn_t sio_interrupt(int local_irq, void *dev_id)
{
	struct fintek_sio *sio = dev_get_platdata(dev_id);
	unsigned long flags;
	int i;

	//gettimeofday(&time, NULL);

	for (i = 0; i < MAX_IRQ_CNT; ++i) {
		if (local_irq == irq[i])
			break;
	}

	if (i >= MAX_IRQ_CNT) {
		pr_warn("%s: can't found match irq handler\n", __func__);
		return IRQ_NONE;
	}

	spin_lock_irqsave(&sio->lock, flags);
	sio->triggered[i] = 1;
	complete(&sio->irq_comp[i]);
	spin_unlock_irqrestore(&sio->lock, flags);

	//pr_info("%s: %ld %ld\n", __func__, time.tv_sec, time.tv_usec);
	return IRQ_HANDLED;
}

static ssize_t irq_sts_show(struct device *dev, struct device_attribute *attr,
			    char *buf)
{
	struct fintek_sio *sio = dev_get_platdata(dev);
	unsigned long flags;
	long status;
	int i;
	int found = 0;

	for (i = 0; i < MAX_IRQ_CNT; ++i) {
		if (!irq[i])
			break;

		if (!strcmp(sio->dev_irq[i].attr.name, attr->attr.name)) {
			found = 1;
			break;
		}
	}

	if (!found) {
		pr_warn("%s: can't found match dev_irq index\n", __func__);
		return -EINVAL;
	}

	while (1) {
		status = wait_for_completion_interruptible_timeout(
			&sio->irq_comp[i], msecs_to_jiffies(10));
		if (status < 0)
			return -EINTR;

		spin_lock_irqsave(&sio->lock, flags);
		if (sio->triggered[i]) {
			sio->triggered[i] = 0;
			spin_unlock_irqrestore(&sio->lock, flags);
			break;
		}
		spin_unlock_irqrestore(&sio->lock, flags);
	}

	return sprintf(buf, "1");
}

static int __init fintek_gpio_device_add(const struct fintek_sio *sio)
{
	int err;

	err = platform_device_add_data(gpio_dev, sio, sizeof(*sio));
	if (err) {
		pr_err(DRVNAME "Platform data allocation failed\n");
		return err;
	}

	err = platform_device_add(gpio_dev);
	if (err) {
		pr_err(DRVNAME "Device addition failed\n");
		return err;
	}

	return 0;
}

static int fintek_sio_file_add(struct platform_device *pdev)
{
	return misc_register(&misc_device);
}

static int fintek_sio_file_remove(struct platform_device *pdev)
{
	misc_deregister(&misc_device);

	return 0;
}

static int fintek_gpio_probe(struct platform_device *pdev)
{
	struct fintek_sio *sio = dev_get_platdata(&pdev->dev);
	char *name;
	int status;
	int i;
	int iVal;

	status = fintek_sio_file_add(pdev);
	if (status)
		return status;

	for (i = 0; i < MAX_IRQ_CNT; ++i) {
		if (!irq[i])
			break;

		init_completion(&sio->irq_comp[i]);
		sio->triggered[i] = 0;

		name = devm_kzalloc(&pdev->dev, 8, GFP_KERNEL);
		if (!name)
			return -ENOMEM;

		sprintf(name, "irq-%d", irq[i]);

		sio->dev_irq[i].show = irq_sts_show;
		sio->dev_irq[i].attr.mode = 0444;
		sio->dev_irq[i].attr.name = name;

		status = device_create_file(&pdev->dev, &sio->dev_irq[i]);
		if (status)
			return -EPERM;

		iVal=gpio_to_irq(0x85);
		pr_info("hhit-gpio_to_irq(0x85)=%d\n",iVal);

		status = request_irq(irq[i], sio_interrupt, IRQF_TRIGGER_RISING|IRQF_ONESHOT,
				     DRVNAME, &pdev->dev);
		if (status) {
			device_remove_file(&pdev->dev, &sio->dev_irq[i]);

			printk(KERN_INFO "%s: request_irq failed: %02x\n",
			       __func__, status);
			return status;
		}
	}

	return 0;
}

static int fintek_gpio_remove(struct platform_device *pdev)
{
	struct fintek_sio *sio = dev_get_platdata(&pdev->dev);
	int i;

	for (i = 0; i < MAX_IRQ_CNT; ++i) {
		if (!irq[i])
			break;

		complete_all(&sio->irq_comp[i]);
		device_remove_file(&pdev->dev, &sio->dev_irq[i]);
		free_irq(irq[i], &pdev->dev);
	}

	fintek_sio_file_remove(pdev);

	return 0;
}

static struct platform_driver fintek_gpio_driver = {
	.driver = {
		.name	= DRVNAME,
	},

	.probe		= fintek_gpio_probe,
	.remove		= fintek_gpio_remove,
};

static int __init fintek_gpio_init(void)
{
	int status;
	int i;

	pr_info("Fintek GPIO interrupt handler " DRVVER "\n");

	for (i = 0; i < MAX_IRQ_CNT; ++i) {
		if (!irq[i])
			break;

		pr_info("handled irq : %x\n", irq[i]);
	}

	spin_lock_init(&m_sio_data.lock);

	status = platform_driver_register(&fintek_gpio_driver);
	if (status)
		return status;

	gpio_dev = platform_device_alloc(DRVNAME, -1);
	if (!gpio_dev)
		return -ENOMEM;

	status = fintek_gpio_device_add(&m_sio_data);
	if (status) {
		platform_device_put(gpio_dev);
		platform_driver_unregister(&fintek_gpio_driver);
		return status;
	}

	return 0;
}

static void __exit fintek_gpio_exit(void)
{
	platform_device_unregister(gpio_dev);
	platform_driver_unregister(&fintek_gpio_driver);
}

module_init(fintek_gpio_init);
module_exit(fintek_gpio_exit);

MODULE_LICENSE("GPL");
