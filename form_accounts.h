#ifndef FORM_ACCOUNTS_H
#define FORM_ACCOUNTS_H

#include <gtk/gtk.h>
#include <mysql/mysql.h>
#include <mysql/mysqld_error.h>

#include "form_account.h"
#include "form_categories.h"
#include "form_cat_report_01.h"




struct FormAccounts_
{
	MYSQL* MySQL;
	GtkWidget* WindowAccounts;
	GtkWidget* ListViewAccounts;
	GtkListStore* ListStoreAccounts;
	FormAccount form_account;
	FormCategories form_categories;
	FormCatReport01 form_cat_report_01;
};

typedef struct FormAccounts_ FormAccounts;

void FormAccounts_Create(FormAccounts* fa, MYSQL* MySQL);
void FormAccounts_Init(FormAccounts* fa);
void FormAccounts_UpdateData(FormAccounts* fa);

#endif
