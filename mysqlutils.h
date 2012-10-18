#ifndef MYSQLUTILS_H
#define MYSQLUTILS_H

#include <mysql/mysql.h>

int MySQLQuery(MYSQL* MySQL, char* Query, ...);

#endif
