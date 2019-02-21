#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/device.h>

int g_major = 0;
int g_minor = 0;

struct chat_dev {
	struct cdev m_dev;	// �ַ��豸
}g_chat_dev;

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
	
	// �����豸��, ������/proc/devices
	dev_t dev = 0;
	if (g_major) {
		printk ("func:%s, static apply major\n", __FUNCTION__);
		// �Ѿ�֪�����豸�� major
		dev = MKDEV (g_major, g_minor);
		nRtn = register_chrdev_region (dev, 1, "chat");
	} else {
		printk ("func:%s, alloc apply major\n", __FUNCTION__);
		// δ֪���豸�ţ��Զ�����
		nRtn = alloc_chrdev_region (&dev, g_major, 1, "chat");
		// ��ȡ�豸��
		g_major = MAJOR (dev);
	}
	// �쳣����
	if (nRtn) {
		printk ("func:%s, nRtn:%d\n", __FUNCTION__, nRtn);
		return nRtn;
	}
	
	// ��fops �� �豸�ڵ�
	cdev_init (&g_chat_dev.m_dev, &g_fops);
	g_chat_dev.m_dev.owner = THIS_MODULE;
	
	// �ں����������
	nErr = cdev_add (&g_chat_dev.m_dev, dev, 1);
	if (nErr) {
		printk ("func:%s, error\n", __FUNCTION__);
	} else {
		printk ("dev register success!\n");
		// ������ʼ��
	}
	
	// �����豸�ڵ�
	g_class = class_create (THIS_MODULE, "chatdev");
	device_create (g_class, NULL, dev, NULL, "chatdev");
	
	return 0;
}

static void chat_exit(void)
{
	printk ("func:%s\n", __FUNCTION__);
	
	// ��ȡ�豸��
	dev_t dev = MKDEV (g_major, g_minor);
	
	device_destroy(g_class, dev);
  class_destroy(g_class);
  cdev_del(&g_chat_dev.m_dev);
	unregister_chrdev_region (dev, 1);
}

static ssize_t chat_read(struct file *filp, char *buf, size_t len, loff_t *off)
{
	printk ("func:%s\n", __FUNCTION__);
	return 0;
}

static ssize_t chat_write(struct file *filp, const char *buf, size_t len, loff_t *off)
{
	printk ("func:%s, buf:%s\n", __FUNCTION__, buf);
	
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