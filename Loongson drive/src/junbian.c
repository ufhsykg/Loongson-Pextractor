#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/io.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <asm/uaccess.h>
#include <linux/delay.h>

#define  LOONG_CHRGPIO_NMAE    "junbian"
#define  LOONG_CHRGPIO_COUNT   1
/*  寄存器物理地址  0x1FE10000 上偏移*/
#define  LXGPIO_BASE          (0x1fe10000)
#define  LXGPIO_OEN           (0x500)
#define  LXGPIO_O             (0x510)
#define  LXGPIO_I             (0x520)
#define  OFFSET               (0x04)

/*  停止所有电机指令*/
#define ALL_STOP            0x00 
/*  进行一次抽纸指令*/
#define START_DRAW          0x01
/*  检查红外传感指令*/
#define CHK_PEOPLE          0x08
/*  创建基地址偏移iomem指针*/
static void __iomem *LXGPIO_BASE_V ;

/*  龙芯gpio设备结构体 */
typedef struct loong_chrgpio_dev {
    struct cdev    loong_chrgpio_cdev;
    dev_t devid;
    struct class  *loong_chrclass;
    struct device *loong_chrdevice;
    int major;
    int minor;
} loong_chrgpio_t;


loong_chrgpio_t loong_chrgpio; 

void start_motor(void)
{
    u32 val = 0;
    val = readl(LXGPIO_BASE_V+LXGPIO_O+OFFSET);
    val |= (1 << 23);
    writel(val,LXGPIO_BASE_V+LXGPIO_O+OFFSET);
}
void stop_motor(void)
{
    u32 val = 0;
    val = readl(LXGPIO_BASE_V+LXGPIO_O+OFFSET);
    val &= ~(1 << 23);
    writel(val,LXGPIO_BASE_V+LXGPIO_O+OFFSET);
}
void start_electromagnet(void)
{
    u32 val = 0;
    val = readl(LXGPIO_BASE_V+LXGPIO_O+OFFSET);
    val |= (1 << 25);
    writel(val,LXGPIO_BASE_V+LXGPIO_O+OFFSET);
}
void stop_electromagnet(void)
{
    u32 val = 0;
    val = readl(LXGPIO_BASE_V+LXGPIO_O+OFFSET);
    val &= ~(1 << 25);
    writel(val,LXGPIO_BASE_V+LXGPIO_O+OFFSET);
}


void gpio_switch(u8 sta){
    u32 val = 0;
    if (sta == START_DRAW)
    {
        start_motor();
        msleep(10000);
        stop_motor();
        start_electromagnet();
        msleep(10000);
        stop_electromagnet();
    }
    else if (sta == ALL_STOP)
    {
        stop_motor();
        stop_electromagnet();
    }
}



static ssize_t loong_gpio_write(struct file *filp,const char __user *buf,size_t count,loff_t *ppos)
{
    int ret;
    u8  databuf[1];

    ret = copy_from_user(databuf,buf,count);
    if(ret < 0){
        printk("kernel write failed : gpio");
        return -1;
    }

    gpio_switch(databuf[0]);
    return 0;
}

static ssize_t loong_gpio_read(struct file *filp,char __user *buf,size_t count,loff_t *ppos)
{
    filp->private_data = &loong_chrgpio;
    u32 val = 0;
    u8 jun[1];
    val = readl(LXGPIO_BASE_V+LXGPIO_I+OFFSET);
    jun[0] =  (val>>28)&0x1;
    copy_to_user(buf,jun,sizeof(jun));
    return 0;
}

static int loong_gpio_open(struct inode *inode,struct file *filp)
{
    filp->private_data = &loong_chrgpio;
    return 0;
}

static int loong_gpio_release(struct inode *inode,struct file *filp)
{
    loong_chrgpio_t *dev = (loong_chrgpio_t *)filp->private_data;
    return 0;
}


static const struct file_operations loong_chrgpio_opera =
{
    .owner     = THIS_MODULE,
    .write     = loong_gpio_write,
    .open      = loong_gpio_open,
    .release   = loong_gpio_release,
    .read      = loong_gpio_read
};




static int __init loong_chrgpio_init(void)
{
    int ret = 0;
    /* 初始化gpio */
    u32 val = 0 ;

    printk("init\r\n");

    /* 初始化gpio 地址映射 */
    LXGPIO_BASE_V = ioremap(LXGPIO_BASE,20);



    /* 设置GPIO 输出属性 */
    val = readl(LXGPIO_BASE_V+LXGPIO_OEN+OFFSET);
    //printk("LXGPIO_OEN%x\n",val);

    /*输出*/
    val &= ~(1 << 23);
    val &= ~(1 << 25);
    /*输入*/
    val |=     (1 << 28);

    //printk("LXGPIO_OEN%x\n",val);
    writel(val,LXGPIO_BASE_V+LXGPIO_OEN+OFFSET);
    //val = readl(LXGPIO_BASE_V+LXGPIO_OEN+OFFSET);
    //printk("LXGPIO_OEN%x\n",val);

/*******************************************************************************/

    /*  设置默认低电平 */
    val = readl(LXGPIO_BASE_V+LXGPIO_O+OFFSET);
   // printk("LXGPIO_O%x\n",val);
    /* 输出 */
    val &= ~(1 << 23);
    val &= ~(1 << 25);
    //printk("LXGPIO_O%x\n",val);
    writel(val,LXGPIO_BASE_V+LXGPIO_O+OFFSET);
    //val = readl(LXGPIO_BASE_V+LXGPIO_O+OFFSET);
    //printk("LXGPIO_O%x\n",val);

    loong_chrgpio.major = 0;

    /*  注册字符设备 */
    if(loong_chrgpio.major){
            loong_chrgpio.devid = MKDEV(loong_chrgpio.major,0);
        ret = register_chrdev_region(loong_chrgpio.devid,LOONG_CHRGPIO_COUNT,LOONG_CHRGPIO_NMAE);
    }else
    {
        ret = alloc_chrdev_region(&loong_chrgpio.devid,0,LOONG_CHRGPIO_COUNT,LOONG_CHRGPIO_NMAE);
        loong_chrgpio.major = MAJOR(loong_chrgpio.devid);
        loong_chrgpio.minor = MINOR(loong_chrgpio.devid);
    }
    if (ret < 0){
        printk ("register error!\r\n");
        return -1;
    }
    printk("major = %d,minor = %d\r\n",loong_chrgpio.major,loong_chrgpio.minor);
    
    loong_chrgpio.loong_chrgpio_cdev.owner = THIS_MODULE;
    cdev_init(&loong_chrgpio.loong_chrgpio_cdev,&loong_chrgpio_opera);

    ret = cdev_add(&loong_chrgpio.loong_chrgpio_cdev,loong_chrgpio.devid,LOONG_CHRGPIO_COUNT);
    if (ret < 0){
        printk ("cdev add error!\r\n");
        return -1;
    }


    /* 自动创建设备节点 */
    loong_chrgpio.loong_chrclass =class_create(THIS_MODULE,LOONG_CHRGPIO_NMAE);
    if(IS_ERR(loong_chrgpio.loong_chrclass))
        return PTR_ERR(loong_chrgpio.loong_chrclass);

    loong_chrgpio.loong_chrdevice = device_create(loong_chrgpio.loong_chrclass,NULL,loong_chrgpio.devid,
                                            NULL,LOONG_CHRGPIO_NMAE);
     if(IS_ERR(loong_chrgpio.loong_chrdevice))
        return PTR_ERR(loong_chrgpio.loong_chrdevice);
    

    return 0;
}

static void __exit loong_chrgpio_exit(void)
{
    printk("exit\r\n");
    iounmap(LXGPIO_BASE_V);

    /* 注销字符设备 */
    cdev_del(&loong_chrgpio.loong_chrgpio_cdev);

    unregister_chrdev_region(loong_chrgpio.devid,LOONG_CHRGPIO_COUNT);

    /* 摧毁设备*/
    device_destroy(loong_chrgpio.loong_chrclass,loong_chrgpio.devid);
    /* 摧毁类 */
    class_destroy(loong_chrgpio.loong_chrclass);
}

module_init(loong_chrgpio_init);
module_exit(loong_chrgpio_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("JunBian");