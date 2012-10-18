#include <stdlib.h>
#include "calc.h"




int Calc(const char* Str, double* Result)
{
	char* cp1;
	char* cp2;
	double d1, d2, sign;
	
	
	*Result = 0;
	
	d2 = strtod(Str, &cp1);
	
	if (cp1 == Str)
	{
		return -1;
	}
	
	while (*cp1 == ' ')
	{
		++cp1;
	}
	
	while (*cp1 != '\0')
	{
		if (*cp1 == '+')
		{
			sign = 1;
			++cp1;
		}
		else
		if (*cp1 == '-')
		{
			sign = -1;
			++cp1;
		}
		
		while (*cp1 == ' ')
		{
			++cp1;
		}
		
		d1 = strtod(cp1, &cp2);
		
		if (cp1 == cp2)
		{
			return -1;
		}
		
		d2 = d2 + sign * d1;
		cp1 = cp2;
		
		while (*cp1 == ' ')
		{
			++cp1;
		}
	}
	
	*Result = d2;  return 1;
	
}
