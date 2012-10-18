#include <errno.h>
#include <stdlib.h>
#include "utils.h"




int DateStrToDMY(char* DateStr, unsigned int* Year, unsigned int* Month, unsigned int* Day)
{
	char* DateStr1;
	char* DateStr2;
	
	
	if (DateStr == NULL)
	{
		return -1;
	}
	
	DateStr1 = DateStr; errno = 0;
	*Year = strtoul(DateStr1, &DateStr2, 10);
	if (DateStr1 < DateStr2 && *DateStr2 == '-')
	{
		DateStr1 = DateStr2 + 1; errno = 0;
		*Month = strtoul(DateStr1, &DateStr2, 10);
		if (DateStr1 < DateStr2 && *DateStr2 == '-')
		{
			DateStr1 = DateStr2 + 1; errno = 0;
			*Day = strtoul(DateStr1, &DateStr2, 10);
			if (DateStr1 < DateStr2)
			{
				return 1;
			}
		}
	}
	
	return -1;
}

