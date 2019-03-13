#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/semaphore.h>

int g_major = 0;
int g_minor = 0;

//static struct cdev g_dev;	// 字符设备
static struct cdev g_chatDev[10];
static char *g_buf;
const int g_num_buf = 4*1024;
struct semaphore *g_bufLocks;

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

static struct class *g_class[10];

int pchats_read_status_proc(char *buf, char **start, off_t offset,
    int count, int *eof, void *data)
{
	int len = 0;
	len += sprintf(buf+len,"pchat options:\n");

	return len;
}

static int chat_init(void)
{
	int nRtn = 0;
	int nErr = 0;
	int i = 0;
	char szDevName[64] = {0};
	
	// 申请设备号, 产生在/proc/devices
	dev_t dev = 0;
	if (g_major) {
		printk ("func:%s, static apply major\n", __FUNCTION__);
		// 已经知道主设备号 major
		dev = MKDEV (g_major, g_minor);
		nRtn = register_chrdev_region (dev, 10, "chats");
	} else {
		printk ("func:%s, alloc apply major\n", __FUNCTION__);
		// 未知主设备号，自动分配
		nRtn = alloc_chrdev_region (&dev, g_major, 10, "chats");
		// 获取设备号
		g_major = MAJOR (dev);
	}
	// 异常处理
	if (nRtn) {
		printk ("func:%s, nRtn:%d\n", __FUNCTION__, nRtn);
		return nRtn;
	}
	
	for (i=0; i<10; i++) {
		int devno = MKDEV(g_major, i);
		// 绑定fops 和 设备节点
		cdev_init (&g_chatDev[i], &g_fops);
		g_chatDev[i].owner = THIS_MODULE;
		g_chatDev[i].ops = &g_fops;
		// 内核中添加驱动
		nErr = cdev_add (&g_chatDev[i], devno, 1);
		if (nErr) {
			printk ("func:%s, error\n", __FUNCTION__);
		} else {
			printk ("dev register success!\n");
			// 其它初始化
			g_buf = kmalloc (g_num_buf, GFP_KERNEL);
			if (g_buf == NULL) {
				printk ("func:%s, kmalloc failed\n", __FUNCTION__);
				return -1;
			}
		}
		// 创建设备节点
		memset (szDevName, 0, 63);
		sprintf (szDevName, "chats_dev%d", i);
		g_class[i] = class_create (THIS_MODULE, szDevName);
		device_create (g_class[i], NULL, devno, NULL, szDevName);
		printk ("func:%s, device_create ok:%s\n", __FUNCTION__, szDevName);
	}
	
	g_bufLocks = kmalloc (sizeof (struct semaphore), GFP_KERNEL);
	sema_init(g_bufLocks, 1);
	create_proc_read_entry("pchat", 0 /* default mode */,
        NULL /* parent dir */,
        pchats_read_status_proc,
        NULL /* client data */);
	
	return 0;
}

static void chat_exit(void)
{
	//printk ("func:%s\n", __FUNCTION__);
	
  int i;
  for (i=0; i<10; i++) {
  	// 获取设备号
		dev_t dev = MKDEV (g_major, i);
  	device_destroy(g_class[i], dev);
  	class_destroy(g_class[i]);
  	cdev_del(&g_chatDev[i]);
  	unregister_chrdev_region (dev, 1);
  }
	remove_proc_entry("pchat", NULL /* parent dir */);
	kfree(g_bufLocks);
}

static ssize_t chat_read(struct file *filp, char *buf, size_t len, loff_t *off)
{
	int i = 0;
	struct dev *pDevice = filp->private_data;
//	if (pDevice) {
//		printk ("func:%s, device null:%s, ERESTARTSYS:%d\n", __FUNCTION__, (char *)pDevice, ERESTARTSYS);
//		return -ERESTARTSYS;
//	}
	printk ("func:%s, down_interruptible\n", __FUNCTION__);
	if (down_interruptible(g_bufLocks)) {
		printk ("func:%s, down_interruptible:ERESTARTSYS\n", __FUNCTION__);
    return -ERESTARTSYS;
  }

 	int nLen = strlen (g_buf);
 	if (nLen == 0) {
 		return -EINTR;
 	}
	for (i=0; i<nLen; i++) {
		if((g_buf[i]>='a') && (g_buf[i]<='z'))  {
			g_buf[i] &= 0xdf;
		}
	}
	
	if (buf) {
		printk ("func:%s, buf:%s, g_buf:%s\n", __FUNCTION__, buf, g_buf);
		if (copy_to_user (buf, g_buf, len)) {
			return -1;
		}
	} else {
		printk ("func:%s, buf:NULL\n", __FUNCTION__);
	}
	
	up (g_bufLocks);
	memset (g_buf, 0, nLen);
	return nLen;
}

static ssize_t chat_write(struct file *filp, const char *buf, size_t len, loff_t *off)
{
	printk ("func:%s, 2buf:#%s#\n", __FUNCTION__, buf);
	
	memset (g_buf, 0, g_num_buf);
	if (copy_from_user (g_buf, buf, len)) {
		return -1;
	}
	
	up(g_bufLocks);
	
	return strlen (buf);
}

static int chat_open(struct inode *inode, struct file *filp)
{
	printk ("func:%s\n", __FUNCTION__);
	unsigned int ma = imajor (inode);
	unsigned int mi = iminor (inode);
	printk ("func:%s, major:%d, minor:%d\n", __FUNCTION__, ma, mi);
	
	filp->private_data = "helloworld";
	
	if ((filp->f_flags & O_ACCMODE) == O_RDWR) {
		printk ("func:%s, f_flags:O_RDWR\n", __FUNCTION__);
	} else {
		printk ("func:%s, f_flags:%d, O_ACCMODE:%d,O_RDWR:%d\n", __FUNCTION__, filp->f_flags, O_ACCMODE, O_RDWR);
	}
	
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
