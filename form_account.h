#ifndef FORM_ACCOUNT_H
#define FORM_ACCOUNT_H

#include <gtk/gtk.h>
#include <mysql/mysql.h>
#include <mysql/mysqld_error.h>

#include "tree_view_categories.h"

struct FormAccount_
{
	MYSQL* MySQL;
	GtkWidget* ButtonAdd;
	GtkWidget* ButtonCommit;
	GtkWidget* EntryMoneyNow;
	GtkWidget* CalendarDate;
	gboolean   CalendarDateOnSelectEnabled;
	GtkWidget* EntryDescription;
	GtkWidget* EntryMoney;
	TreeViewCategories tvc;
	GtkWidget* LabelRemain;
	GtkWidget* ListViewAccount;
	GtkWidget* WindowAccount;
	GtkListStore* ListStoreAccount;
	unsigned int AccID;
};

typedef struct FormAccount_ FormAccount;




void FormAccount_Create(FormAccount* fa, MYSQL* MySQL);
void FormAccount_Init(FormAccount* fa);
void FormAccount_UpdateData(FormAccount* fa, char* Acc);

#endif
