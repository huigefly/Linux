#ifndef INOTIFER_H__20190110
#define INOTIFER_H__20190110

class CINotifer
{
public:
	virtual int notify(void *pParam) = 0;
	virtual ~CINotifer(){}
};

#endif // INOTIFER_H__20190110
