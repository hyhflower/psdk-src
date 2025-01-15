#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>

#define GPIO_PIN_PPS 4

static unsigned int g_irq_number;

static irqreturn_t gpio_int_handler(int irq,void *dev_id)
{
    printk(KERN_INFO"GPIO Interrput.GPIO PIN:%d\n",GPIO_PIN_PPS);
    return IRQ_HANDLED;
}

static int __init gpio_irq_init(void)
{
    int result;

    if(!gpio_is_valid(GPIO_PIN_PPS))
    {
        printk(KERN_INFO"Invalid GPIO PIN:%d\n",GPIO_PIN_PPS);
        return -ENODEV;
    }
    //gpio_request(GPIO_PIN_PPS,"gpiochip0");
    //gpio_direction_input(GPIO_PIN_PPS);
    //gpio_export(GPIO_PIN_PPS,false);

    g_irq_number=gpio_to_irq(GPIO_PIN_PPS);
    printk(KERN_INFO"GPIO mapped to IRQ:%d\n",g_irq_number);

    result=request_irq(g_irq_number,(irq_handler_t)gpio_int_handler,IRQF_TRIGGER_RISING,"gpio_irq_handler",NULL);
    if(result)
    {
        printk(KERN_INFO"GPIO IRQ request failed\n");
        return result;
    }

    printk(KERN_INFO"GPIO IRQ Module Loaded\n");
    return 0;
}

static void __exit gpio_irq_exit(void)
{
    free_irq(g_irq_number,NULL);
    gpio_unexport(GPIO_PIN_PPS);
    gpio_free(GPIO_PIN_PPS);
    printk(KERN_INFO"GPIO IRQ Module unloaded\n");
}

module_init(gpio_irq_init);
module_exit(gpio_irq_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("hhit-hyh");
MODULE_DESCRIPTION("pps pin IRQ Handler");
MODULE_VERSION("1.0");

