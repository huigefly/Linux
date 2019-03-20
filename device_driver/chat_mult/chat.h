#ifndef CHAT_H__
#define CHAT_H__

#include <linux/ioctl.h>

// ÓÃ×öioctl
#define CHAT_IOC_MAGIC	  'd'
#define CHAT_IOC_HOOK    	_IO(CHAT_IOC_MAGIC, 0)
#define CHAT_IOC_UNHOOK	  _IO(CHAT_IOC_MAGIC, 1)


void dbgPrintBufferHexSpace(char *, int);
void dbgPrintBufferCharSpace(char *, int);

char *utilsGetKtapVer(void);

unsigned int utilsStrValue(char *);
void utilsStrInitValues(unsigned int, char **, unsigned int *);
unsigned int utilsStrMatchByValue(unsigned int, unsigned int *, unsigned int);


#if defined(__linux)
void utilsSleepForSeconds(int);
int utilsSetAddrRW(unsigned long);
int utilsSetAddrRO(unsigned long);
#include <linux/fs.h>
#include <linux/version.h>
#include <linux/proc_fs.h> // read_proc_t
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0)
void utilsCreateProc(const char *, const struct file_operations  *);
#else
void utilsCreateProc(const char *, read_proc_t *);
#endif

void utilsRemoveProc(const char *);
#endif

#endif // CHAT_H__
