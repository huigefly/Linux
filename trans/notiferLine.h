#ifndef _LINE_NOTIFER_H__201901101132
#define _LINE_NOTIFER_H__201901101132

#include <stdio.h>
#include <stdlib.h>
#include "notifer.h"

class CNotiferLine : public CINotifer
{
public:
	virtual int notify(void *pParam);
	
	CNotiferLine();
	virtual ~CNotiferLine();
};

#endif // _LINE_NOTIFER_H__201901101132
