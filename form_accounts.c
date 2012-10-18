#include <stdlib.h>
#include <string.h>
#include "form_accounts.h"
#include "mysqlutils.h"
#include "gtk_utils.h"

enum
{
	COL_ID_ACCOUNT_ID,
	COL_ID_NAME,
	COL_ID_MONEY,
	N_COLUMNS,
};

void FormAccounts_ButtonAccountAddClicked(GtkButton* button, FormAccounts* fa);
void FormAccounts_ButtonAccountRenameClicked(GtkButton* button, FormAccounts* fa);
void FormAccounts_ButtonCategoriesClicked(GtkButton* button, gpointer user_data);
void FormAccounts_ButtonCatReportClicked(GtkButton* button, FormAccounts* fa);
void FormAccounts_FillMoney(FormAccounts* fa);
void FormAccounts_RowActivated(GtkTreeView* treeview, GtkTreePath* treepath, GtkTreeViewColumn* column, gpointer userdata);




void FormAccounts_ButtonAccountAddClicked(GtkButton* button, FormAccounts* fa)
{
	MYSQL_STMT* stmt = NULL;
	MYSQL_BIND param_insert[1];
	gchar* Text = NULL;
	unsigned long TextStrLength;
	const char* mysql_stmt_str = "INSERT INTO accounts(name) VALUES (?);";
	
	
	gtk_input_dialog("Создать новый счёт", "Введите наименование нового счёта.", "", &Text);
	
	if (Text)
	{
		g_print("%s(): создать счёт '%s'\n", __func__, Text);
		
		stmt = mysql_stmt_init(fa->MySQL);
		if (stmt == NULL)
		{
			g_print("%s(): mysql_stmt_init() error.\n", __func__);
			goto END;
		}
		if (mysql_stmt_prepare(stmt, mysql_stmt_str, strlen(mysql_stmt_str)))
		{
			g_print("%s(): mysql_stmt_prepare() error.\n", __func__);
			goto END;
		}
		
		TextStrLength = strlen(Text);
		memset(param_insert, 0, sizeof(param_insert));
		
		param_insert[0].buffer_type   = MYSQL_TYPE_VAR_STRING;
		param_insert[0].buffer        = (void*) Text;
		param_insert[0].buffer_length = TextStrLength + 1;
		param_insert[0].length        = &TextStrLength;
		
		if (mysql_stmt_bind_param(stmt, param_insert))
		{
			g_print("%s(): mysql_stmt_bind() error.\n", __func__);
			goto END;
		}
		
		if (mysql_stmt_execute(stmt))
		{
			g_print("%s(): mysql_stmt_bind() error.\n", __func__);
			goto END;
		}
		
		FormAccounts_UpdateData(fa);
	}
	
END:
	if (Text)    g_free(Text);
	if (stmt)    mysql_stmt_close(stmt);
}

void FormAccounts_ButtonAccountRenameClicked(GtkButton* button, FormAccounts* fa)
{
	gchar* Text = NULL;
	gchar* TextOld = NULL;
	gchar* msg = NULL;
	GtkTreeModel* model;
	GtkTreePath* path = NULL;
	GtkTreeIter iter;
	gint* indices;
	gint depth;
	gint count;
	gint last_items_count = 1;
	gint last_account_item_index;
	gint cur_item_index;
	MYSQL_STMT* stmt = NULL;
	MYSQL_BIND param_update[2];
	unsigned long TextStrLength;
	const char* mysql_stmt_str = "UPDATE accounts SET name = ? WHERE id = ?;";
	gchar* id_str = NULL;
	unsigned int id;
	
	
	
	gtk_tree_view_get_cursor(GTK_TREE_VIEW(fa->ListViewAccounts), &path, NULL);
	
	if (path == NULL)
	{
		g_print("Ни один элемент не выбран.\n");
		goto END;
	}
	indices = gtk_tree_path_get_indices_with_depth(path, &depth);
	if (depth < 1)
		goto END;
	cur_item_index = indices[0];
	
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(fa->ListViewAccounts));
	count = gtk_tree_model_iter_n_children(model, NULL);
	if (count <= last_items_count)
		goto END;
	last_account_item_index = count - last_items_count - 1;
	if (cur_item_index > last_account_item_index)
		goto END;
	
	if (!gtk_tree_model_get_iter(model, &iter, path))
		goto END;
	gtk_tree_model_get(model, &iter,
			COL_ID_ACCOUNT_ID, &id_str,
			COL_ID_NAME,       &TextOld, -1);
	if (!id_str)
		goto END;
	id = strtoul(id_str, NULL, 10);
	
	msg = g_strdup_printf("Введите новое наименование для счёта «%s».", TextOld);
	gtk_input_dialog("Переименовать счёт", msg, TextOld, &Text);
	
	if (Text)
	{
		stmt = mysql_stmt_init(fa->MySQL);
		if (stmt == NULL)
		{
			g_print("%s(): mysql_stmt_init() error.\n", __func__);
			goto END;
		}
		
		if (mysql_stmt_prepare(stmt, mysql_stmt_str, strlen(mysql_stmt_str)))
		{
			g_print("%s(): mysql_stmt_prepare() error.\n", __func__);
			goto END;
		}
		
		TextStrLength = strlen(Text);
		memset(param_update, 0, sizeof(param_update));
		
		param_update[0].buffer_type   = MYSQL_TYPE_VAR_STRING;
		param_update[0].buffer        = (void*) Text;
		param_update[0].buffer_length = TextStrLength + 1;
		param_update[0].length        = &TextStrLength;
		
		param_update[1].buffer_type    = MYSQL_TYPE_LONG;
		param_update[1].buffer         = (void*) &id;
		param_update[1].is_unsigned    = 1;
		
		if (mysql_stmt_bind_param(stmt, param_update))
		{
			g_print("%s(): mysql_stmt_bind() error.\n", __func__);
			goto END;
		}
		
		if (mysql_stmt_execute(stmt))
		{
			g_print("%s(): mysql_stmt_bind() error.\n", __func__);
			goto END;
		}
		
		FormAccounts_UpdateData(fa);
	}
	
END:
	if (msg)      g_free(msg);
	if (Text)     g_free(Text);
	if (TextOld)  g_free(TextOld);
	if (path)     gtk_tree_path_free(path);
	if (id_str)   g_free(id_str);
	if (stmt)     mysql_stmt_close(stmt);
}

void FormAccounts_ButtonCategoriesClicked(GtkButton* button, gpointer user_data)
{
	FormAccounts* fa = (FormAccounts*) user_data;
	
	
	FormCategories_Create(&fa->form_categories, fa->MySQL);
}

void FormAccounts_ButtonCatReportClicked(GtkButton* button, FormAccounts* fa)
{
	FormCatReport01_Create(&fa->form_cat_report_01, fa->MySQL);
}

void FormAccounts_Create(FormAccounts* fa, MYSQL* MySQL)
{
	GtkWidget *scroll;
	GtkTreeViewColumn* Column;
	GtkWidget* vbox;
	GtkWidget* button_categories;
	GtkWidget* button_cat_report_01;
	GtkWidget* button_account_add;
	GtkWidget* button_account_rename;
	
	
	fa->MySQL = MySQL;
	fa->WindowAccounts = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	vbox = gtk_vbox_new(FALSE, 0);
	
	scroll = gtk_scrolled_window_new(NULL, NULL);
	fa->ListStoreAccounts = gtk_list_store_new(N_COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_DOUBLE);
	
	fa->ListViewAccounts = gtk_tree_view_new_with_model(GTK_TREE_MODEL(fa->ListStoreAccounts));
	g_object_unref(G_OBJECT(fa->ListStoreAccounts));
	
	Column = gtk_tree_view_column_new_with_attributes("Название счёта", gtk_cell_renderer_text_new(),
			"text", COL_ID_NAME, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(fa->ListViewAccounts), Column);
	
	Column = gtk_tree_view_money_column_new(COL_ID_MONEY, "Остаток");
	gtk_tree_view_append_column(GTK_TREE_VIEW(fa->ListViewAccounts), Column);
	
	gtk_container_add(GTK_CONTAINER(scroll), fa->ListViewAccounts);
	gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 0);
	
	button_account_add = gtk_button_new_with_label("Создать счёт");
	gtk_box_pack_start(GTK_BOX(vbox), button_account_add, FALSE, FALSE, 0);
	
	button_account_rename = gtk_button_new_with_label("Переименовать счёт");
	gtk_box_pack_start(GTK_BOX(vbox), button_account_rename, FALSE, FALSE, 0);
	
	button_categories = gtk_button_new_with_label("Категории");
	gtk_box_pack_start(GTK_BOX(vbox), button_categories, FALSE, FALSE, 0);
	
	button_cat_report_01 = gtk_button_new_with_label("Отчёт");
	gtk_box_pack_start(GTK_BOX(vbox), button_cat_report_01, FALSE, FALSE, 0);
	
	gtk_container_add(GTK_CONTAINER(fa->WindowAccounts), vbox);
	
	gtk_window_resize(GTK_WINDOW(fa->WindowAccounts), 250, 500);
	gtk_window_move(GTK_WINDOW(fa->WindowAccounts), 90, 50);
	
	gtk_widget_show_all(fa->WindowAccounts);
	
	g_signal_connect_swapped(GTK_WINDOW(fa->WindowAccounts), "destroy", G_CALLBACK(gtk_main_quit), (gpointer) NULL);
	g_signal_connect(GTK_TREE_VIEW(fa->ListViewAccounts), "row-activated", G_CALLBACK(FormAccounts_RowActivated), (gpointer) fa);
	g_signal_connect(GTK_BUTTON(button_account_add), "clicked", G_CALLBACK(FormAccounts_ButtonAccountAddClicked), (gpointer) fa);
	g_signal_connect(GTK_BUTTON(button_account_rename), "clicked", G_CALLBACK(FormAccounts_ButtonAccountRenameClicked), (gpointer) fa);
	g_signal_connect(GTK_BUTTON(button_categories), "clicked", G_CALLBACK(FormAccounts_ButtonCategoriesClicked), (gpointer) fa);
	g_signal_connect(GTK_BUTTON(button_cat_report_01), "clicked", G_CALLBACK(FormAccounts_ButtonCatReportClicked), (gpointer) fa);
}

void FormAccounts_FillMoney(FormAccounts* fa)
{
	GtkTreeIter iter;
	gboolean isValid;
	gchar* id;
	gdouble Money;
	MYSQL_RES* ResSet;
	MYSQL_ROW row;
	gdouble Total = 0.0;
	
	
	for (isValid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(fa->ListStoreAccounts), &iter);
			isValid; isValid = gtk_tree_model_iter_next(GTK_TREE_MODEL(fa->ListStoreAccounts), &iter))
	{
		gtk_tree_model_get(GTK_TREE_MODEL(fa->ListStoreAccounts), &iter,
				COL_ID_ACCOUNT_ID, &id, -1);
		if (MySQLQuery(fa->MySQL,
			"SELECT ROUND(SUM(operations.money), 2)\n"
			"FROM operations WHERE account_id = %s;", id) == -1)
		{
			return ;
		}
		ResSet = mysql_store_result(fa->MySQL);
		if (ResSet == NULL)
		{
			return ;
		}
		row = mysql_fetch_row(ResSet);
		Money = row[0] ? strtod(row[0], NULL) : 0.0;
		Total += Money;
		gtk_list_store_set(fa->ListStoreAccounts, &iter, COL_ID_MONEY, Money, -1);
		
		mysql_free_result(ResSet);
		g_free(id);
	}
	
	gtk_list_store_append(fa->ListStoreAccounts, &iter);
	gtk_list_store_set(fa->ListStoreAccounts, &iter,
			COL_ID_NAME, "Всего:",
			COL_ID_MONEY, Total,
			-1);
}

void FormAccounts_Init(FormAccounts* fa)
{
	FormAccount_Init(&fa->form_account);
	FormCategories_Init(&fa->form_categories);
	FormCatReport01_Init(&fa->form_cat_report_01);
}

void FormAccounts_RowActivated(GtkTreeView* treeview, GtkTreePath* treepath, GtkTreeViewColumn* column, gpointer userdata)
{
	FormAccounts* fa = (FormAccounts*) userdata;
	GtkTreeModel* model;
	GtkTreeIter iter;
	gchar* AccID, *AccName;
	
	
	model = gtk_tree_view_get_model(treeview);
	if (gtk_tree_model_get_iter(model, &iter, treepath))
	{
		gtk_tree_model_get(model, &iter,
				COL_ID_ACCOUNT_ID, &AccID,
				COL_ID_NAME, &AccName, -1);
		
		if (AccID)
		{
			FormAccount_Create(&fa->form_account, fa->MySQL);
			FormAccount_UpdateData(&fa->form_account, AccID);
		}
		
		if (AccID != NULL)
			g_free(AccID);
		if (AccName != NULL)
			g_free(AccName);
	}
}

void FormAccounts_UpdateData(FormAccounts* fa)
{
	MYSQL_RES* ResSet;
	MYSQL_ROW Row;
	MYSQL_ROW Row1;
	GtkTreeIter Iter;
	
	
	gtk_list_store_clear(fa->ListStoreAccounts);
	if (mysql_query(fa->MySQL, "SELECT id, name FROM accounts;"))
	{
		return ;
	}
	
	ResSet = mysql_store_result(fa->MySQL);
	
	if (ResSet == NULL)
	{
		g_print("%s(): mysql_store_result() error.\n", __func__);
		return ;
	}
	
	while (Row = mysql_fetch_row(ResSet))
	{
		gtk_list_store_append(fa->ListStoreAccounts, &Iter);
		gtk_list_store_set(fa->ListStoreAccounts, &Iter,
				COL_ID_ACCOUNT_ID, Row[0],
				COL_ID_NAME, Row[1],
				COL_ID_MONEY, 0.0, -1);
	}
	
	mysql_free_result(ResSet);
	
	FormAccounts_FillMoney(fa);
}
