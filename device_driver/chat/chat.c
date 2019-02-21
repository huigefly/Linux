#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/uaccess.h>

int g_major = 0;
int g_minor = 0;

static struct cdev g_dev;	// 字符设备
static char *g_buf;
const int num_buf = 4*1024;

static ssize_t chat_read(struct file *, char *, size_t, loff_t *);
static ssize_t chat_write(struct file *, const char *, size_t, loff_t *);
static int chat_open(struct inode *inode, struct file *filp);
static int chat_release(struct inode *inode, struct file *filp);

struct file_operations g_fops = {
	.read = chat_read,
	.write = chat_write,
	.open = chat_open,
	.release = chat_release,
};

static struct class *g_class;

static int chat_init(void)
{
	int nRtn = 0;
	int nErr = 0;
	
	// 申请设备号, 产生在/proc/devices
	dev_t dev = 0;
	if (g_major) {
		printk ("func:%s, static apply major\n", __FUNCTION__);
		// 已经知道主设备号 major
		dev = MKDEV (g_major, g_minor);
		nRtn = register_chrdev_region (dev, 1, "chat");
	} else {
		printk ("func:%s, alloc apply major\n", __FUNCTION__);
		// 未知主设备号，自动分配
		nRtn = alloc_chrdev_region (&dev, g_major, 1, "chat");
		// 获取设备号
		g_major = MAJOR (dev);
	}
	// 异常处理
	if (nRtn) {
		printk ("func:%s, nRtn:%d\n", __FUNCTION__, nRtn);
		return nRtn;
	}
	
	// 绑定fops 和 设备节点
	cdev_init (&g_dev, &g_fops);
	g_dev.owner = THIS_MODULE;
	
	// 内核中添加驱动
	nErr = cdev_add (&g_dev, dev, 1);
	if (nErr) {
		printk ("func:%s, error\n", __FUNCTION__);
	} else {
		printk ("dev register success!\n");
		// 其它初始化
		g_buf = kmalloc (num_buf, GFP_KERNEL);
		if (g_buf == NULL) {
			printk ("func:%s, kmalloc failed\n", __FUNCTION__);
			return -1;
		}
	}
	
	// 创建设备节点
	g_class = class_create (THIS_MODULE, "chatdev");
	device_create (g_class, NULL, dev, NULL, "chatdev");
	
	return 0;
}

static void chat_exit(void)
{
	printk ("func:%s\n", __FUNCTION__);
	
	// 获取设备号
	dev_t dev = MKDEV (g_major, g_minor);
	
	device_destroy(g_class, dev);
  class_destroy(g_class);
  cdev_del(&g_dev);
	unregister_chrdev_region (dev, 1);
}

static ssize_t chat_read(struct file *filp, char *buf, size_t len, loff_t *off)
{
	int i = 0;
	for (i=0; i<len; i++) {
		if((g_buf[i]>='a') && (g_buf[i]<='z'))  {
			g_buf[i] &= 0xdf;
		}
	}
	
	printk ("func:%s, buf:%s, g_buf:%s\n", __FUNCTION__, buf, g_buf);
	if (copy_to_user (buf, g_buf, len)) {
		return -1;
	}
	return 0;
}

static ssize_t chat_write(struct file *filp, const char *buf, size_t len, loff_t *off)
{
	printk ("func:%s, 2buf:#%s#\n", __FUNCTION__, buf);
	
	memset (g_buf, 0, num_buf);
	if (copy_from_user (g_buf, buf, len)) {
		return -1;
	}
	
	return strlen (buf);
}

static int chat_open(struct inode *inode, struct file *filp)
{
	printk ("func:%s\n", __FUNCTION__);
	
	return 0;
}

static int chat_release(struct inode *inode, struct file *filp)
{
	printk ("func:%s\n", __FUNCTION__);
	
	return 0;
}


module_init(chat_init);
module_exit(chat_exit);

MODULE_LICENSE("Dual BSD/GPL");