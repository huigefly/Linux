#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/fdtable.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/semaphore.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <asm/cacheflush.h> // _PAGE_RW
#include "chat.h"

int g_major = 0;
int g_minor = 0;

spinlock_t g_devLock;

//static struct cdev g_dev;    // 字符设备
static struct cdev g_chatDev[10];
static char *g_buf;
const int g_num_buf = 4*1024;
struct semaphore *g_bufLocks;
wait_queue_head_t *g_inQueue;
int g_flag = 0;

static ssize_t chat_read(struct file *, char *, size_t, loff_t *);
static ssize_t chat_write(struct file *, const char *, size_t, loff_t *);
static int chat_open(struct inode *inode, struct file *filp);
static int chat_release(struct inode *inode, struct file *filp);
static int chat_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);

static ssize_t hook_chat_read(struct file *, char __user *, size_t, loff_t *);
static ssize_t hook_chat_write(struct file *, const char __user *, size_t, loff_t *);

ssize_t(*sys_pipe_read) (struct file *, char __user *, size_t, loff_t *);
ssize_t(*sys_pipe_write) (struct file *, const char __user *, size_t, loff_t *);
int(*sys_pipe_read_release) (struct inode *, struct file *);
int(*sys_pipe_write_release) (struct inode *, struct file *);
//static int chat_unhook();

struct file_operations g_fops = {
    .read = chat_read,
    .write = chat_write,
    .open = chat_open,
    .release = chat_release,
    .ioctl = chat_ioctl,
};

static struct class *g_class[10];

int pchats_read_status_proc(char *buf, char **start, off_t offset, int count, int *eof, void *data)
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

    g_devLock = __SPIN_LOCK_UNLOCKED(g_devLock);

    // 信号量
	g_bufLocks = kmalloc (10 * sizeof (struct semaphore), GFP_KERNEL);
	if (g_bufLocks == NULL) {
    	return 1;
    }

    // 等待队列
	g_inQueue = kmalloc(10 * sizeof(wait_queue_head_t), GFP_KERNEL);
    if (g_inQueue == NULL) {
        return 2;
    }
	for (i=0; i<10; i++) {
    	int devno = MKDEV(g_major, i);
        // 每一个设备对应一个信号量
    	sema_init(&g_bufLocks[i], 1);
        // 每一个设备对应一个等待队列
    	init_waitqueue_head (&g_inQueue[i]);

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
        	g_buf = kmalloc (10 * g_num_buf, GFP_KERNEL);
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

	create_proc_read_entry("pchat", 0 /* default mode */, NULL /* parent dir */, pchats_read_status_proc, NULL /* client data */);

	return 0;
}

static ssize_t hook_chat_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	int retval = 0;
	
	printk ("func:%s,line%d, buf:%s\n", __FUNCTION__, __LINE__, buf);
	retval = sys_pipe_read(filp, buf, count, f_pos);
    
    return retval;  
}

static ssize_t hook_chat_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
	int retval;

	printk ("func:%s,line%d, buf:%s\n", __FUNCTION__, __LINE__, buf);
	retval = sys_pipe_write(filp, buf, count, f_pos);
	return retval;
}

static int chat_hook(void)
{
    // 主要作用拿来获取应用进程打开的两个pipe句柄
	struct file *sf[2];
    // 系统操作函数绑定句柄，准备用于替换绑定的系统默认给定的readpipe和writepipe
	struct file_operations *read_pipe_fops = NULL, *write_pipe_fops = NULL;
	struct files_struct *files = current->files; // 获得当前进程的files结构
    struct fdtable *fdt = files_fdtable(files);    /* 得到文件描述符表 */
	unsigned int fd = 0;

    printk ("func:%s\n", __FUNCTION__);
    // init
	sf[0] = sf[1] = NULL;

	for (fd = 0; fd < fdt->max_fds; fd++) {
    	if (fd == 3) {
        	sf[0] = fcheck_files (files, fd);
        }
    	if (fd == 4) {
        	sf[1] = fcheck_files (files, fd);
        }
    }

	if (!(sf[0] && sf[1])) {
    	printk ("func:%s,line%d\n", __FUNCTION__, __LINE__);
    	return 0;
    } else {
        // f_op is read-only
        read_pipe_fops = (struct file_operations *) sf[0]->f_op;
        write_pipe_fops = (struct file_operations *) sf[1]->f_op;

        printk ("func:%s,line%d\n", __FUNCTION__, __LINE__);
        sys_pipe_read = read_pipe_fops->read;
        printk ("func:%s,sys_pipe_read line%d\n", __FUNCTION__, __LINE__);
        sys_pipe_write = write_pipe_fops->write;
        printk ("func:%s,sys_pipe_write line%d\n", __FUNCTION__, __LINE__);
        sys_pipe_read_release = read_pipe_fops->release;
        printk ("func:%s,sys_pipe_read_release line%d\n", __FUNCTION__, __LINE__);
        sys_pipe_write_release = write_pipe_fops->release;
        printk ("func:%s,sys_pipe_write_release line%d\n", __FUNCTION__, __LINE__);
    
        if (read_pipe_fops && read_pipe_fops->read) {
            printk ("func:%s, line%d\n", __FUNCTION__, __LINE__);
        }
    
        printk ("func:%s, utilsSetAddrRW line%d\n", __FUNCTION__, __LINE__);
        if (utilsSetAddrRW((unsigned long) read_pipe_fops) && utilsSetAddrRW((unsigned long) write_pipe_fops)) {
             // 开始替换系统接口
            __sync_bool_compare_and_swap(&read_pipe_fops->read, read_pipe_fops->read, hook_chat_read);
            printk ("func:%s,__sync_bool_compare_and_swap line%d\n", __FUNCTION__, __LINE__);
            __sync_bool_compare_and_swap(&write_pipe_fops->write, write_pipe_fops->write, hook_chat_write);
            
            return 1;
        } else {
            return 0;
        }
    
       
    }

	return 0;
}

static int chat_unhook(void)
{
    // 主要作用拿来获取应用进程打开的两个pipe句柄
	struct file *sf[2];
    // 系统操作函数绑定句柄，准备用于替换绑定的系统默认给定的readpipe和writepipe
	struct file_operations *read_pipe_fops = NULL, *write_pipe_fops = NULL;
	struct files_struct *files = current->files; // 获得当前进程的files结构
    struct fdtable *fdt = files_fdtable(files);    /* 得到文件描述符表 */
	unsigned int fd = 0;

    printk ("func:%s\n", __FUNCTION__);
    // init
	sf[0] = sf[1] = NULL;

	for (fd = 0; fd < fdt->max_fds; fd++) {
    	if (fd == 3) {
        	sf[0] = fcheck_files (files, fd);
        }
    	if (fd == 4) {
        	sf[1] = fcheck_files (files, fd);
        }
    }

	if (!(sf[0] && sf[1])) {
    	printk ("func:%s,return line%d\n", __FUNCTION__, __LINE__);
    	return 0;
    } else {
        // f_op is read-only
        read_pipe_fops = (struct file_operations *) sf[0]->f_op;
        write_pipe_fops = (struct file_operations *) sf[1]->f_op;
    
        // 开始替换系统接口
        printk ("func:%s,sys_pipe_read line%d\n", __FUNCTION__, __LINE__);
        __sync_bool_compare_and_swap(&read_pipe_fops->read, read_pipe_fops->read, sys_pipe_read);
        printk ("func:%s,sys_pipe_write line%d\n", __FUNCTION__, __LINE__);
        __sync_bool_compare_and_swap(&write_pipe_fops->write, write_pipe_fops->write, sys_pipe_write);
    }

	return 0;
}

static int chat_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
    // 获取一下当前是在操作哪一个设备
    struct cdev *device = filp->private_data;
    unsigned int bufId = MINOR(device->dev);

    printk ("func:%s,id:%d\n", __FUNCTION__, bufId);
    switch (cmd)
    {
	case CHAT_IOC_HOOK: {
	    spin_lock(&g_devLock);
    	chat_hook ();
    	spin_unlock(&g_devLock);
    	break;
    }
	case CHAT_IOC_UNHOOK: {
	    spin_lock(&g_devLock);
    	chat_unhook ();
    	spin_unlock(&g_devLock);
    	break;
    }
	default:
    	return 0;
    }

	return 0;
}

static ssize_t chat_read(struct file *filp, char *buf, size_t len, loff_t *off)
{
    int i = 0;
    struct cdev *pDev = filp->private_data;
    unsigned int bufId = MINOR(pDev->dev);
    struct semaphore *pSem = &g_bufLocks[bufId];
    char *pBuf = g_buf + bufId * g_num_buf;
    int nLen = strlen (pBuf);

//	if (pDevice) {
//    	printk ("func:%s, device null:%s, ERESTARTSYS:%d\n", __FUNCTION__, (char *)pDevice, ERESTARTSYS);
//    	return -ERESTARTSYS;
//    }
    // #include <linux/sched.h> 包含current全局变量
    printk ("func:%s, ready down, current pid:%d\n", __FUNCTION__, current->pid);
    if (down_interruptible(pSem)) {
    	printk ("func:%s, exit down\n", __FUNCTION__);
        return -ERESTARTSYS;
    }
    printk ("func:%s, already down\n", __FUNCTION__);

#if 0
    if (nLen == 0) {
    printk ("func:%s, exit EINTR\n", __FUNCTION__);
    up (pSem);
    return -EINTR;
    }
#else
	up (pSem);
	if (wait_event_interruptible (g_inQueue[bufId], (strlen (pBuf) != 0))) {
    	printk ("func:%s, exit wait_event_interruptible\n", __FUNCTION__);
    	return -ERESTARTSYS;
    }
	printk ("func:%s, already wait_event_interruptible\n", __FUNCTION__);
#endif // 0

 	if (down_interruptible(pSem)) {
    	printk ("func:%s, exit down\n", __FUNCTION__);
        return -ERESTARTSYS;
    }
    nLen = strlen (pBuf);
	for (i=0; i<nLen; i++) {
    	if((pBuf[i]>='a') && (pBuf[i]<='z'))  {
        	pBuf[i] &= 0xdf;
        }
    }

	if (buf) {
    	printk ("func:%s, buf:%s, g_buf:%s\n", __FUNCTION__, buf, pBuf);
    	if (copy_to_user (buf, pBuf, nLen)) {
        	printk ("func:%s, exit copy_to_user\n", __FUNCTION__);
        	up (pSem);
        	return -1;
        }
    } else {
    	printk ("func:%s, buf:NULL\n", __FUNCTION__);
    }

	printk ("func:%s, ready up\n", __FUNCTION__);
	up (pSem);
	printk ("func:%s, already up\n", __FUNCTION__);
	memset (pBuf, 0, nLen);
	return nLen;
}

static ssize_t chat_write(struct file *filp, const char *buf, size_t len, loff_t *off)
{
    struct cdev *pDev = filp->private_data;
	unsigned int bufId = MINOR(pDev->dev);
	struct semaphore *pSem = &g_bufLocks[bufId];
	char *pBuf = g_buf + bufId * g_num_buf;

	printk ("func:%s, buf:#%s#, ready down\n", __FUNCTION__, buf);
	if (down_interruptible(pSem)) {
    	printk ("func:%s, exit down\n", __FUNCTION__);
        return -ERESTARTSYS;
    }
    printk ("func:%s, already down\n", __FUNCTION__);

	memset (pBuf, 0, g_num_buf);
	if (copy_from_user (pBuf, buf, len)) {
    	printk ("func:%s, exit copy_from_user\n", __FUNCTION__);
    	up (pSem);
    	return -1;
    }

	printk ("func:%s, ready up\n", __FUNCTION__);
	up (pSem);
	printk ("func:%s, already up\n", __FUNCTION__);

	wake_up_interruptible(&g_inQueue[bufId]);

	return strlen (buf);
}

static int chat_open(struct inode *inode, struct file *filp)
{
	unsigned int ma = imajor (inode);
	unsigned int mi = iminor (inode);

	printk ("func:%s\n", __FUNCTION__);
	printk ("func:%s, major:%d, minor:%d\n", __FUNCTION__, ma, mi);

	filp->private_data = &g_chatDev[mi];

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
	kfree (g_inQueue);
	chat_unhook();
}

#if 1
int utilsSetAddrRW(
    unsigned long _addr)
{
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 18)
    // Linux 2.6.18 doesn't expose lookup_address
    pte_t *pte = utilsLookupAddress(_addr);
#else
    unsigned int level;
    pte_t *pte = lookup_address(_addr, &level);
#endif

#if defined(__LP64__)
    if (pte) {
        if (pte->pte &~ _PAGE_RW) {
            pte->pte |= _PAGE_RW;
        }

        return 1;
    } else {
        return 0;
    }
#else
    if (pte) {
        if (pte->pte_low &~ _PAGE_RW) {
            pte->pte_low |= _PAGE_RW;
        }

        return 1;
    } else {
        return 0;
    }
#endif
}

int utilsSetAddrRO(
    unsigned long _addr)
{
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 18)
    // Linux 2.6.18 doesn't expose lookup_address
    pte_t *pte = utilsLookupAddress(_addr);
#else
    unsigned int level;
    pte_t *pte = lookup_address(_addr, &level);
#endif

#if defined(__LP64__)
    if (pte) {
        pte->pte = pte->pte &~_PAGE_RW;

        return 1;
    } else {
        return 0;
    }
#else
    if (pte) {
        pte->pte_low = pte->pte_low &~_PAGE_RW;

        return 1;
    } else {
        return 0;
    }
#endif
}

#endif // csTapperUtils

module_init(chat_init);
module_exit(chat_exit);

MODULE_LICENSE("Dual BSD/GPL");
