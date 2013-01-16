#include <stdlib.h>
#include <locale.h>
#include "avgmoney.h"




AvgMoney* AvgMoneyApp =  NULL;
static char* SQLCreateAccounts = 
	"CREATE TABLE `accounts` (\n"
	"  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,\n"
	"  `name` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,\n"
	"  PRIMARY KEY (`id`)\n"
	") ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;\n";

static char* SQLCreateCategories = 
	"CREATE TABLE `categories` (\n"
	"  `id` int(10) unsigned NOT NULL,\n"
	"  `parent_id` int(10) unsigned NOT NULL,\n"
	"  `name` varchar(100) COLLATE utf8_unicode_ci DEFAULT NULL,\n"
	"  PRIMARY KEY (`id`)\n"
	") ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;\n";

static char* SQLCreateOperations = 
	"CREATE TABLE `operations` (\n"
	"  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,\n"
	"  `date` datetime DEFAULT NULL,\n"
	"  `description` varchar(1000) COLLATE utf8_unicode_ci DEFAULT NULL,\n"
	"  `money` double DEFAULT NULL,\n"
	"  `category` int(10) unsigned DEFAULT '0',\n"
	"  `hb_cat` int(10) unsigned NOT NULL DEFAULT '0',\n"
	"  `account_id` int(10) unsigned NOT NULL DEFAULT '1',\n"
	"  PRIMARY KEY (`id`)\n"
	") ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;\n";




void AvgMoney_CheckCreateDB()
{
	GtkWidget* dialog;
	unsigned int mysqlerr;
	gint dlg_result;
	
	
	if (mysql_select_db(&AvgMoneyApp->MySQL, "avgmoney"))
	{
		mysqlerr = mysql_errno(&AvgMoneyApp->MySQL);
		if (mysqlerr == ER_BAD_DB_ERROR)
		{
			dialog = gtk_message_dialog_new(NULL,
					GTK_DIALOG_MODAL,
					GTK_MESSAGE_INFO,
					GTK_BUTTONS_OK_CANCEL,
					"База данных 'avgmoney' не обнаружена.\n"
					"Для работы с программой необходимо её создать.\n"
					"Создать базу данных 'avgmoney' ?\n");
			dlg_result = gtk_dialog_run(GTK_DIALOG(dialog));
			gtk_widget_destroy(dialog);
			if (dlg_result == GTK_RESPONSE_OK)
			{
				if (mysql_query(&AvgMoneyApp->MySQL, "CREATE DATABASE avgmoney;"))
				{
					g_print("mysql_query() error 1.\n");
					exit(1);
				}
				if (mysql_query(&AvgMoneyApp->MySQL, "USE avgmoney;"))
				{
					g_print("mysql_query() error 2.\n");
					exit(1);
				}
				if (mysql_query(&AvgMoneyApp->MySQL, SQLCreateAccounts))
				{
					g_print("mysql_query() error 3.\n");
					exit(1);
				}
				if (mysql_query(&AvgMoneyApp->MySQL, SQLCreateCategories))
				{
					g_print("mysql_query() error 4.\n");
					exit(1);
				}
				if (mysql_query(&AvgMoneyApp->MySQL, SQLCreateOperations))
				{
					g_print("mysql_query() error 5.\n");
					exit(1);
				}
			}
			else
				exit(1);
		}
		else
		{
			g_print("%s (%d)\n",
					mysql_error(&AvgMoneyApp->MySQL),
					mysql_errno(&AvgMoneyApp->MySQL));
			exit(1);
		}
	}
}

void AvgMoney_Create()
{
	if (AvgMoneyApp != NULL)
	{
		g_print("error\n");
		exit(1);
	}
	
	AvgMoneyApp = malloc(sizeof(AvgMoney));
	
	if (AvgMoneyApp == NULL)
	{
		g_print("error\n");
		exit(1);
	}
}

void AvgMoney_MySQLPrepare()
{
	if (mysql_init(&AvgMoneyApp->MySQL) == NULL)
	{
		g_print("AvgMoney_MySQLPrepare(): \n");
		exit(1);
	}
	mysql_options(&AvgMoneyApp->MySQL, MYSQL_SET_CHARSET_NAME, "utf8");
//	Реализовать диалог настроек подключения !
	
	if (mysql_real_connect(&AvgMoneyApp->MySQL, "", "", "", NULL,
			0, NULL, 0) == NULL)
	{
		g_print("%s() %s (%u).\n", __func__,
				mysql_error(&AvgMoneyApp->MySQL),
				mysql_errno(&AvgMoneyApp->MySQL));
		exit(1);
	}
	
	AvgMoney_CheckCreateDB();
}

int main(int argc, char* argv[])
{
	gtk_init(&argc, &argv);
	
	setlocale(LC_ALL, "");
	setlocale(LC_NUMERIC, "C");
	
	AvgMoney_Create();
	AvgMoney_MySQLPrepare();
	
	FormAccounts_Init(&AvgMoneyApp->form_accounts);
	FormAccounts_Create(&AvgMoneyApp->form_accounts, &AvgMoneyApp->MySQL);
	FormAccounts_UpdateData(&AvgMoneyApp->form_accounts);
	
	gtk_main();
	
	return 0;
}
