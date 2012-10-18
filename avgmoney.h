#ifndef AVGMONEY_H
#define AVGMONEY_H

#include <gtk/gtk.h>
#include <mysql/mysql.h>
#include <mysql/mysqld_error.h>

#include "form_account.h"
#include "form_accounts.h"

struct AvgMoney_ {
	MYSQL MySQL;
	FormAccounts form_accounts;
};

typedef struct AvgMoney_ AvgMoney;

#endif
