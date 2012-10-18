#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "mysqlutils.h"

int MySQLQuery(MYSQL* MySQL, char* Query, ...)
{
	char Buf[3000];
	va_list ap;
	
	va_start(ap, Query);
	vsnprintf(Buf, sizeof(Buf), Query, ap);
	va_end(ap);
	
	if (mysql_query(MySQL, Buf) > 0) {
		fprintf(stderr, "MySQLQuery(): mysql_query(): %s (%d)\n"
			"QUERY:\n"
			"%s\n", mysql_error(MySQL), mysql_errno(MySQL), Buf);
		return -1;
	}
	
	return 1;
}
