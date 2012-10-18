#include <stdlib.h>
#include <string.h>
#include <gdk/gdkkeysyms.h>
#include <errno.h>
#include "gtk_utils.h"
#include "calc.h"
#include "form_account.h"
#include "utils.h"




enum RecordState_ {
	REC_STATE_NONE = 0,
	REC_STATE_EDIT,
	REC_STATE_ADD
};

typedef enum RecordState_ RecordState;

enum 
{
	COL_ID_RECORD_STATE,
	COL_ID_OPERATION_ID,
	COL_ID_DATE,
	COL_ID_DESCRIPTION,
	COL_ID_MONEY,
	COL_ID_CATEGORY_ID,
	COL_ID_CATEGORY_NAME,
	N_COLUMNS
};




void FormAccount_ButtonAddClicked(GtkButton* button, gpointer user_data);
void FormAccount_ButtonCommitClicked(GtkButton* button, gpointer user_data);
void FormAccount_CellMoneyDataFunc(GtkTreeViewColumn* column, GtkCellRenderer* renderer,
		GtkTreeModel* model, GtkTreeIter* iter, gpointer userdata);
void FormAccount_CursorChanged(GtkTreeView* treeview, gpointer user_data);
void FormAccount_CalendarDateDaySelected(GtkCalendar* calendar, gpointer user_data);
gboolean FormAccount_DescriptionKeyRelease(GtkWidget* widget, GdkEvent* event, gpointer user_data);
void FormAccount_Destroy(GtkObject* object, gpointer userdata);
gboolean FormAccount_GetCursor(FormAccount* fa, GtkTreeModel** model, GtkTreePath** path, GtkTreeIter* iter);
gboolean FormAccount_KeyRelease(GtkWidget* widget, GdkEvent* event, gpointer user_data);
gboolean FormAccount_MoneyKeyRelease(GtkWidget* widget, GdkEvent* event, gpointer user_data);
gboolean FormAccount_MoneyNowKeyRelease(GtkWidget* widget, GdkEvent* event, gpointer user_data);
void FormAccount_SetCursorOnLast(FormAccount* fa);
void FormAccount_SumMoneyTotal(FormAccount* fa, GtkTreeModel* model, double* MoneyTotal);
void FormAccount_TreeViewCategoriesRowActivated(unsigned int id, char* Desc, void* arg);
void FormAccount_TreeViewCategoriesKeyRelease(unsigned int updown, void* arg);
void FormAccount_UpdateModifiedRecordState(FormAccount* fa, GtkTreeModel** model, GtkTreeIter* iter);
void FormAccount_UpdateRemain(FormAccount* fa);
void FormAccount_UpDown(FormAccount* fa, unsigned int updown);




void FormAccount_ButtonAddClicked(GtkButton* button, gpointer user_data)
{
	FormAccount* fa = (FormAccount*) user_data;
	GtkTreeModel* model;
	GtkTreeIter iter;
	GtkTreePath* path;
	double Money;
	
	
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(fa->ListViewAccount));
	gtk_list_store_append(GTK_LIST_STORE(model), &iter);
	
	Money = 0.0;
	
	gtk_list_store_set(GTK_LIST_STORE(model), &iter,
			COL_ID_RECORD_STATE, REC_STATE_ADD,
			COL_ID_OPERATION_ID, "",
			COL_ID_DATE, "",
			COL_ID_DESCRIPTION, "",
			COL_ID_MONEY, Money,
			COL_ID_CATEGORY_ID, 0,
			COL_ID_CATEGORY_NAME, "",
			-1);
	
	path = gtk_tree_model_get_path(model, &iter);
	gtk_tree_view_set_cursor_on_cell(GTK_TREE_VIEW(fa->ListViewAccount),
			path, NULL, NULL, FALSE);
	gtk_tree_path_free(path);
	
	return;
	
}

void FormAccount_ButtonCommitClicked(GtkButton* button, gpointer user_data)
{
	FormAccount* fa = (FormAccount*) user_data;
	GtkTreeModel* model;
	GtkTreeIter iter;
	gboolean isLast;
	guint rec_state;
	gchar* rec_state_str;
	gchar* OperationIDStr  = NULL;
	gchar* DateStr         = NULL;
	gchar* DescriptionStr  = NULL;
	gdouble Money;
	guint CategoryID;
	unsigned int DateYear;
	unsigned int DateMonth;
	unsigned int DateDay;
	
	MYSQL_STMT* stmt;
	
	MYSQL_STMT* stmt_insert = NULL;
	char* stmt_insert_str =
		"INSERT operations(date, description, money, account_id, category) VALUES (?,?,?,?,?)";
	MYSQL_BIND      param_insert[5];
	
	MYSQL_STMT* stmt_update = NULL;
	char* stmt_update_str =
		"UPDATE operations SET\n"
		"date = ?, description = ?, money = ?, category = ? WHERE id = ?;\n";
	MYSQL_BIND      param_update[5];
	
	unsigned int    row_account_id;
	int             row_id;
	MYSQL_TIME      row_datetime;
	char            row_description_buf[1005];
	unsigned long   row_description_length;
	double          row_money;
	unsigned int    row_category;
	
	
	
	stmt_insert = mysql_stmt_init(fa->MySQL);
	if (stmt_insert == NULL)
	{
		g_print("%s(): mysql_stmt_init(insert) error.\n", __func__);
		goto END;
	}
	
	if (mysql_stmt_prepare(stmt_insert, stmt_insert_str, strlen(stmt_insert_str)) != 0)
	{
		g_print("%s(): mysql_stmt_prepare(insert) error.\n", __func__);
		goto END;
	}
	
	memset((void*) param_insert, 0, sizeof(param_insert));
	
	param_insert[0].buffer_type  = MYSQL_TYPE_DATETIME;
	param_insert[0].buffer       = (void*) &row_datetime;
	
	param_insert[1].buffer_type    = MYSQL_TYPE_VAR_STRING;
	param_insert[1].buffer         = (void*) row_description_buf;
	param_insert[1].buffer_length  = sizeof(row_description_buf);
	param_insert[1].length         = &row_description_length;
	
	param_insert[2].buffer_type    = MYSQL_TYPE_DOUBLE;
	param_insert[2].buffer         = (void*) &row_money;
	
	param_insert[3].buffer_type    = MYSQL_TYPE_LONG;
	param_insert[3].buffer         = (void*) &row_account_id;
	param_insert[3].is_unsigned    = 1;
	
	param_insert[4].buffer_type    = MYSQL_TYPE_LONG;
	param_insert[4].buffer         = (void*) &row_category;
	param_insert[4].is_unsigned    = 1;
	
	if (mysql_stmt_bind_param(stmt_insert, param_insert))
	{
		g_print("%s(): mysql_stmt_bind_param(insert) error.\n", __func__);
		goto END;
	}
	
	stmt_update = mysql_stmt_init(fa->MySQL);
	if (stmt_update == NULL)
	{
		g_print("%s(): mysql_stmt_init(update) error.\n", __func__);
		goto END;
	}
	
	if (mysql_stmt_prepare(stmt_update, stmt_update_str, strlen(stmt_update_str)) != 0)
	{
		g_print("%s(): mysql_stmt_prepare(update) error.\n", __func__);
		goto END;
	}
	
	memset((void*) param_update, 0, sizeof(param_update));
	
	param_update[0].buffer_type  = MYSQL_TYPE_DATETIME;
	param_update[0].buffer       = (void*) &row_datetime;
	
	param_update[1].buffer_type    = MYSQL_TYPE_VAR_STRING;
	param_update[1].buffer         = (void*) row_description_buf;
	param_update[1].buffer_length  = sizeof(row_description_buf);
	param_update[1].length         = &row_description_length;
	
	param_update[2].buffer_type    = MYSQL_TYPE_DOUBLE;
	param_update[2].buffer         = (void*) &row_money;
	
	param_update[3].buffer_type    = MYSQL_TYPE_LONG;
	param_update[3].buffer         = (void*) &row_category;
	param_update[3].is_unsigned    = 1;
	
	param_update[4].buffer_type    = MYSQL_TYPE_LONG;
	param_update[4].buffer         = (void*) &row_id;
	param_update[4].is_unsigned    = 1;
	
	if (mysql_stmt_bind_param(stmt_update, param_update))
	{
		g_print("%s(): mysql_stmt_bind_param() error.\n", __func__);
		goto END;
	}
	
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(fa->ListViewAccount));
	
	for (isLast = gtk_tree_model_get_iter_first(model, &iter);
			isLast == TRUE;
			isLast = gtk_tree_model_iter_next(model, &iter))
	{
		gtk_tree_model_get(model, &iter, COL_ID_RECORD_STATE, &rec_state, -1);
		
		if (rec_state == REC_STATE_ADD || rec_state == REC_STATE_EDIT)
		{
			gtk_tree_model_get(model, &iter,
					COL_ID_OPERATION_ID, &OperationIDStr,
					COL_ID_DATE, &DateStr,
					COL_ID_DESCRIPTION, &DescriptionStr,
					COL_ID_MONEY, &Money,
					COL_ID_CATEGORY_ID, &CategoryID, -1);
			
			switch (rec_state)
			{
			case REC_STATE_ADD:
				rec_state_str = "ADD  |";
				break;
			case REC_STATE_EDIT:
				rec_state_str = "EDIT |";
				break;
			default:
				rec_state_str = " ??  |";
				break;
			}
			
			g_print("%s\t" "%s\t" "\"%s\"\t" "%.2f\n",
					rec_state_str, DateStr, DescriptionStr, Money);
			
			DateStrToDMY(DateStr, &DateYear, &DateMonth, &DateDay);
			row_datetime.year         = DateYear;
			row_datetime.month        = DateMonth;
			row_datetime.day          = DateDay;
			row_datetime.hour         = 0;
			row_datetime.minute       = 0;
			row_datetime.second       = 0;
			row_datetime.neg          = 0;
			row_datetime.second_part  = 0;
			
			strncpy(row_description_buf, DescriptionStr, sizeof(row_description_buf));
			row_description_buf[sizeof(row_description_buf)-1] = '\0';
			row_description_length = strlen(row_description_buf);
			
			row_money = Money;
			
			row_category = CategoryID;
			
			switch (rec_state)
			{
			case REC_STATE_ADD:
				row_account_id = fa->AccID;
				stmt = stmt_insert;
				break;
			case REC_STATE_EDIT:
				row_id = strtol(OperationIDStr, NULL, 10);
				stmt = stmt_update;
				break;
			default:
				g_print("%s(): wrong type.\n", __func__);
				goto END;
			}
			
			if (mysql_stmt_execute(stmt) != 0)
			{
				g_print("%s(): mysql_stmt_execute() error.\n", __func__);
				goto END;
			}
			
			gtk_list_store_set(GTK_LIST_STORE(model), &iter, COL_ID_RECORD_STATE, REC_STATE_NONE, -1);
			
			g_free(DateStr);         DateStr        = NULL;
			g_free(DescriptionStr);  DescriptionStr = NULL;
		}
	}
	
END:
	
	if (stmt_insert != NULL)
	{
		mysql_stmt_close(stmt_insert);
	}
	if (stmt_update != NULL)
	{
		mysql_stmt_close(stmt_update);
	}
	if (DateStr != NULL)
	{
		g_free(DateStr);
	}
	if (DescriptionStr != NULL)
	{
		g_free(DescriptionStr);
	}
	
}

void FormAccount_CellMoneyDataFunc(GtkTreeViewColumn* column, GtkCellRenderer* renderer,
		GtkTreeModel* model, GtkTreeIter* iter, gpointer userdata)
{
	gdouble Money;
	char Buf[50];
	GdkColor color;
	
	gtk_tree_model_get(model, iter, COL_ID_MONEY, &Money, -1);
	sprintf(Buf, "%.2f", Money);
	
	if (Money >= 0.0)
	{
		color.red    = 0;
		color.green  = 256*140;
		color.blue   = 0;
	}
	else
	{
		color.red    = 256*220;
		color.green  = 0;
		color.blue   = 0;
	}
	
	g_object_set(renderer, "text", Buf, "foreground-gdk", &color, NULL);
}

void FormAccount_Create(FormAccount* fa, MYSQL* MySQL)
{
	GtkWidget *vbox, *scroll, *scroll2, *tbl, *label, *hbox;
	GtkTreeViewColumn* column;
	GtkCellRenderer* renderer;
	gint row1, row2;
	
	
	FORM_PRESENT(fa->WindowAccount)
	
	TreeViewCategories_Init(&fa->tvc);
	
	fa->MySQL = MySQL;
	
	fa->WindowAccount = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	vbox = gtk_vbox_new(FALSE, 0);
	scroll = gtk_scrolled_window_new(NULL, NULL);
	fa->ListStoreAccount = gtk_list_store_new(
			N_COLUMNS,
			G_TYPE_UINT,      /*  COL_ID_RECORD_STATE  */
			G_TYPE_STRING,    /*  COL_ID_OPERATION_ID  */
			G_TYPE_STRING,    /*  COL_ID_DATE          */
			G_TYPE_STRING,    /*  COL_ID_DESCRIPTION   */
			G_TYPE_DOUBLE,    /*  COL_ID_MONEY         */
			G_TYPE_UINT,      /*  COL_ID_CATEGORY_ID   */
			G_TYPE_STRING);   /*  COL_ID_CATEGORY_NAME */
	
	fa->ListViewAccount = gtk_tree_view_new_with_model(GTK_TREE_MODEL(fa->ListStoreAccount));
	g_object_unref(G_OBJECT(fa->ListStoreAccount));
	
	column = gtk_tree_view_column_new_with_attributes("Дата", gtk_cell_renderer_text_new(),
			"text", COL_ID_DATE, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(fa->ListViewAccount), column);
	
	column = gtk_tree_view_column_new_with_attributes("Описание", gtk_cell_renderer_text_new(),
			"text", COL_ID_DESCRIPTION, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_append_column(GTK_TREE_VIEW(fa->ListViewAccount), column);
	
	renderer = gtk_cell_renderer_text_new();
	g_object_set(renderer, "xalign", 1.0, NULL);
	column = gtk_tree_view_column_new_with_attributes("Деньги", renderer, "text", COL_ID_MONEY, NULL);
	gtk_tree_view_column_set_cell_data_func(column, renderer, FormAccount_CellMoneyDataFunc, NULL, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(fa->ListViewAccount), column);
	
	column = gtk_tree_view_column_new_with_attributes("Категория", gtk_cell_renderer_text_new(),
			"text", COL_ID_CATEGORY_NAME, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(fa->ListViewAccount), column);
	
	gtk_container_add(GTK_CONTAINER(scroll), fa->ListViewAccount);
	gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 0);
	
	tbl = gtk_table_new(7, 2, FALSE);
	row1=0; row2=1;
	
	label = gtk_label_new("Неизвестный расход:");
	gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
	gtk_widget_set_usize(label, -1, 50);
	gtk_table_attach(GTK_TABLE(tbl), label,             0, 1, row1, row2, GTK_FILL, GTK_FILL, 0, 0);
	
	fa->LabelRemain = gtk_label_new("REMAIN");
	gtk_misc_set_alignment(GTK_MISC(fa->LabelRemain), 0, 0.5);
	gtk_table_attach(GTK_TABLE(tbl), fa->LabelRemain,   1, 2, row1, row2, GTK_FILL | GTK_EXPAND, GTK_FILL, 0, 0);
	++row1; ++row2;
	
	
	label = gtk_label_new("Денег в настоящий момент");
	gtk_label_set_markup(GTK_LABEL(label), "Денег в настоящий момент");
	gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(tbl), label,           0, 1, row1, row2, GTK_FILL, GTK_FILL, 0, 0);
	fa->EntryMoneyNow = gtk_entry_new();
	gtk_table_attach(GTK_TABLE(tbl), fa->EntryMoneyNow, 1, 2, row1, row2, GTK_FILL | GTK_EXPAND, GTK_FILL, 0, 0);
	++row1; ++row2;
	
	label = gtk_label_new("Дата");
	gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(tbl), label,           0, 1, row1, row2, GTK_FILL, GTK_FILL, 0, 0);
	fa->CalendarDate = gtk_calendar_new();
	fa->CalendarDateOnSelectEnabled = TRUE;
	gtk_table_attach(GTK_TABLE(tbl), fa->CalendarDate, 1, 2, row1, row2, 0, 0, 0, 0);
	++row1; ++row2;
	
	label = gtk_label_new("Описание");
	gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(tbl), label,           0, 1, row1, row2, GTK_FILL, GTK_FILL, 0, 0);
	fa->EntryDescription = gtk_entry_new();
	gtk_table_attach(GTK_TABLE(tbl), fa->EntryDescription, 1, 2, row1, row2, GTK_FILL | GTK_EXPAND, GTK_FILL, 0, 0);
	++row1; ++row2;
	
	label = gtk_label_new("Деньги");
	gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(tbl), label,           0, 1, row1, row2, GTK_FILL, GTK_FILL, 0, 0);
	fa->EntryMoney = gtk_entry_new();
	gtk_table_attach(GTK_TABLE(tbl), fa->EntryMoney, 1, 2, row1, row2, GTK_FILL | GTK_EXPAND, GTK_FILL, 0, 0);
	++row1; ++row2;
	
	label = gtk_label_new("Категория");
	gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(tbl), label,           0, 1, row1, row2, GTK_FILL, GTK_FILL, 0, 0);
	
	scroll2 = gtk_scrolled_window_new(NULL, NULL);
	gtk_widget_set_usize(GTK_WIDGET(scroll2), -1, 170);
	TreeViewCategories_Create(&fa->tvc, fa->MySQL);
	gtk_container_add(GTK_CONTAINER(scroll2), fa->tvc.treeview);
	gtk_table_attach(GTK_TABLE(tbl), scroll2, 1, 2, row1, row2, GTK_FILL | GTK_EXPAND, GTK_FILL, 0, 0);
	++row1; ++row2;
	
	gtk_box_pack_start(GTK_BOX(vbox), tbl, FALSE, FALSE, 0);
	
	hbox = gtk_hbox_new(FALSE, 0);
	
	fa->ButtonAdd = gtk_button_new_with_label("Добавить");
	gtk_box_pack_start(GTK_BOX(hbox), fa->ButtonAdd, TRUE, TRUE, 0);
	
	fa->ButtonCommit = gtk_button_new_with_label("Обновить в базе");
	gtk_box_pack_start(GTK_BOX(hbox), fa->ButtonCommit, TRUE, TRUE, 0);
	
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
	gtk_container_add(GTK_CONTAINER(fa->WindowAccount), vbox);
	
	gtk_window_resize(GTK_WINDOW(fa->WindowAccount), 1200, 950);
	gtk_window_move(GTK_WINDOW(fa->WindowAccount), 350, 50);
	
	g_signal_connect(GTK_ENTRY(fa->EntryMoneyNow),    "key-release-event", G_CALLBACK(FormAccount_MoneyNowKeyRelease), (gpointer) fa);
	g_signal_connect(GTK_CALENDAR(fa->CalendarDate),  "day-selected",      G_CALLBACK(FormAccount_CalendarDateDaySelected), (gpointer) fa);
	g_signal_connect(GTK_CALENDAR(fa->CalendarDate),  "key-release-event", G_CALLBACK(FormAccount_KeyRelease), (gpointer) fa);
	g_signal_connect(GTK_ENTRY(fa->EntryDescription), "key-release-event", G_CALLBACK(FormAccount_DescriptionKeyRelease), (gpointer) fa);
	g_signal_connect(GTK_ENTRY(fa->EntryMoney),       "key-release-event", G_CALLBACK(FormAccount_MoneyKeyRelease), (gpointer) fa);
	TreeViewCategories_SetRowActivated(&fa->tvc, FormAccount_TreeViewCategoriesRowActivated, (void*) fa);
	TreeViewCategories_SetKeyRelease(&fa->tvc, FormAccount_TreeViewCategoriesKeyRelease, (void*) fa);
	
	g_signal_connect(GTK_TREE_VIEW(fa->ListViewAccount), "cursor-changed", G_CALLBACK(FormAccount_CursorChanged), (gpointer) fa);
	
	g_signal_connect(GTK_BUTTON(fa->ButtonAdd),    "clicked", G_CALLBACK(FormAccount_ButtonAddClicked), (gpointer) fa);
	g_signal_connect(GTK_BUTTON(fa->ButtonCommit), "clicked", G_CALLBACK(FormAccount_ButtonCommitClicked), (gpointer) fa);
	g_signal_connect(GTK_WINDOW(fa->WindowAccount), "destroy", G_CALLBACK(FormAccount_Destroy), (gpointer) fa);
	
	gtk_widget_show_all(fa->WindowAccount);
}

void FormAccount_CursorChanged(GtkTreeView* treeview, gpointer user_data)
{
	GtkTreePath* treepath;
	FormAccount* fa = (FormAccount*) user_data;
	GtkTreeModel* model;
	GtkTreeIter iter;
	gchar* OperationID;
	
	gchar* DateStr;
	guint DateYear;
	guint DateMonth;
	guint DateDay;
	gchar* DateStr1;
	gchar* DateStr2;
	
	gchar* Description;
	gdouble Money;
	guint Category;
	gchar Buf[50];
	
	
	
	model = gtk_tree_view_get_model(treeview);
	gtk_tree_view_get_cursor(GTK_TREE_VIEW(fa->ListViewAccount), &treepath, NULL);
	gtk_tree_model_get_iter(model, &iter, treepath);
	gtk_tree_model_get(model, &iter,
			COL_ID_OPERATION_ID, &OperationID,
			COL_ID_DATE, &DateStr,
			COL_ID_DESCRIPTION, &Description,
			COL_ID_MONEY, &Money,
			COL_ID_CATEGORY_ID, &Category, -1);
	
	
	if (DateStr != NULL && DateStrToDMY(DateStr, &DateYear, &DateMonth, &DateDay) != -1)
	{
		fa->CalendarDateOnSelectEnabled = FALSE;
		gtk_calendar_select_day(GTK_CALENDAR(fa->CalendarDate), 0);
		gtk_calendar_select_month(GTK_CALENDAR(fa->CalendarDate), DateMonth-1, DateYear);
		gtk_calendar_select_day(GTK_CALENDAR(fa->CalendarDate), DateDay);
		fa->CalendarDateOnSelectEnabled = TRUE;
	}
	
	gtk_entry_set_text(GTK_ENTRY(fa->EntryDescription), Description != NULL ? Description : "");
	
	sprintf(Buf, "%.2f", Money);
	gtk_entry_set_text(GTK_ENTRY(fa->EntryMoney), Buf);
	
	gtk_tree_path_free(treepath);
	
	if (OperationID != NULL) g_free(OperationID);
	if (DateStr != NULL) g_free(DateStr);
	if (Description != NULL) g_free(Description);
}

void FormAccount_CalendarDateDaySelected(GtkCalendar* calendar, gpointer user_data)
{
	FormAccount* fa = (gpointer) user_data;
	GtkTreeModel* model;
	GtkTreePath* path;
	GtkTreeIter iter;
	guint year;
	guint month;
	guint day;
	gchar DateStr[100];
	
	
	if (fa->CalendarDateOnSelectEnabled == FALSE)
	{
		return ;
	}
	
	gtk_calendar_get_date(calendar, &year, &month, &day);
	++month;
	
	if (FormAccount_GetCursor(fa, &model, &path, &iter) == FALSE)
	{
		return ;
	}
	
	g_sprintf(DateStr, "%04u-%02u-%02u 00:00:00", year, month, day);
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, COL_ID_DATE, DateStr, -1);
	
	FormAccount_UpdateModifiedRecordState(fa, &model, &iter);
	
	gtk_tree_path_free(path);
}

gboolean FormAccount_DescriptionKeyRelease(GtkWidget* widget, GdkEvent* event, gpointer user_data)
{
	FormAccount* fa = (FormAccount*) user_data;
	GtkTreeModel* model;
	GtkTreePath* path;
	GtkTreeIter iter;
	gchar* DescriptionText;
	gchar* DescriptionTextOld;
	
	
	FormAccount_KeyRelease(widget, event, user_data);
	if (FormAccount_GetCursor(fa, &model, &path, &iter) == FALSE)
	{
		return FALSE;
	}
	
	DescriptionText = (gchar*) gtk_entry_get_text(GTK_ENTRY(fa->EntryDescription));
	gtk_tree_model_get(model, &iter, COL_ID_DESCRIPTION, &DescriptionTextOld, -1);
	
	if (strcmp(DescriptionText, DescriptionTextOld) != 0)
	{
		gtk_list_store_set(GTK_LIST_STORE(model), &iter, COL_ID_DESCRIPTION, DescriptionText, -1);
		FormAccount_UpdateModifiedRecordState(fa, &model, &iter);
	}
	
	gtk_tree_path_free(path);
	g_free(DescriptionTextOld);
	
	return FALSE;
}

void FormAccount_Destroy(GtkObject* object, gpointer userdata)
{
	FormAccount* fa = (FormAccount*) userdata;
	
	
	fa->WindowAccount = NULL;
}

gboolean FormAccount_GetCursor(FormAccount* fa, GtkTreeModel** model, GtkTreePath** path, GtkTreeIter* iter)
{
	*model = gtk_tree_view_get_model(GTK_TREE_VIEW(fa->ListViewAccount));
	gtk_tree_view_get_cursor(GTK_TREE_VIEW(fa->ListViewAccount), path, NULL);
	
	if (*path == NULL)
	{
		return FALSE;
	}
	
	if (gtk_tree_model_get_iter(*model, iter, *path) == FALSE)
	{
		gtk_tree_path_free(*path);
		*path = NULL;
		return FALSE;
	}
	
	return TRUE;
}

void FormAccount_Init(FormAccount* fa)
{
	memset(fa, 0, sizeof(FormAccount));
}

gboolean FormAccount_KeyRelease(GtkWidget* widget, GdkEvent* event, gpointer user_data)
{
	FormAccount* fa = (FormAccount*) user_data;
	unsigned int updown;
	
	
	switch (event->key.keyval)
	{
	case GDK_KEY_Page_Down:
		updown = 2;
		break;
	case GDK_KEY_Page_Up:
		updown = 1;
		break;
	default:
		return FALSE;
	}
	
	FormAccount_UpDown(fa, updown);
	
	return TRUE;
}

gboolean FormAccount_MoneyKeyRelease(GtkWidget* widget, GdkEvent* event, gpointer user_data)
{
	FormAccount* fa = (FormAccount*) user_data;
	GtkTreePath* path;
	GtkTreeModel* model;
	GtkTreeIter iter;
	double Money;
	double MoneyOld;
	gchar* Buf;
	uint rec_state;
	
	
	/* g_print("key = 0x%x\n", event->key.keyval);
	return FALSE; */
	
	FormAccount_KeyRelease(widget, event, user_data);
	
	if (FormAccount_GetCursor(fa, &model, &path, &iter) == FALSE)
	{
		return FALSE;
	}
	
	Buf = (gchar*) gtk_entry_get_text(GTK_ENTRY(fa->EntryMoney));
	Money = g_strtod(Buf, NULL);
	gtk_tree_model_get(model, &iter, COL_ID_MONEY, &MoneyOld, -1);
	
	if (Money != MoneyOld)
	{
		gtk_list_store_set(GTK_LIST_STORE(model), &iter, COL_ID_MONEY, Money, -1);
		
		FormAccount_UpdateModifiedRecordState(fa, &model, &iter);
		FormAccount_UpdateRemain(fa);
	}
	
	gtk_tree_path_free(path);
	
	return TRUE; // Не передавать сообщение следующему обработчику
}

gboolean FormAccount_MoneyNowKeyRelease(GtkWidget* widget, GdkEvent* event, gpointer user_data)
{
	FormAccount* fa = (FormAccount*) user_data;
	
	
	FormAccount_UpdateRemain(fa);
	
	return FALSE;
}

void FormAccount_SetCursorOnLast(FormAccount* fa)
{
	GtkTreeModel* model;
	GtkTreeIter iter;
	uint nlast;
	GtkTreePath* path;
	
	
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(fa->ListViewAccount));
	nlast = gtk_tree_model_iter_n_children(model, NULL);
	
	if (nlast == 0)
	{
		return ;
	}
	
	--nlast;
	
	if (gtk_tree_model_iter_nth_child(model, &iter, NULL, nlast) == FALSE)
	{
		return ;
	}
	
	path = gtk_tree_model_get_path(model, &iter);
	gtk_tree_view_set_cursor_on_cell(GTK_TREE_VIEW(fa->ListViewAccount),
			path, NULL, NULL, FALSE);
	gtk_tree_path_free(path);
}

void FormAccount_SumMoneyTotal(FormAccount* fa, GtkTreeModel* model, gdouble* MoneyTotal)
{
	GtkTreeIter Iter;
	gdouble MoneyCur;
	gboolean isLast;
	
	
	*MoneyTotal = 0.0;
	
	if (gtk_tree_model_get_iter_first(model, &Iter) == FALSE)
	{
		return ;
	}
	
	for (isLast = gtk_tree_model_get_iter_first(model, &Iter);
		isLast != FALSE;
		isLast = gtk_tree_model_iter_next(model, &Iter))
	{
		gtk_tree_model_get(model, &Iter,
				COL_ID_MONEY, &MoneyCur, -1);
		*MoneyTotal += MoneyCur;
	}
}

void FormAccount_TreeViewCategoriesKeyRelease(unsigned int updown,  void* arg)
{
	FormAccount* fa = (FormAccount*) arg;
	
	
	FormAccount_UpDown(fa, updown);
}

void FormAccount_TreeViewCategoriesRowActivated(unsigned int id, char* Desc, void* arg)
{
	FormAccount* fa = (FormAccount*) arg;
	GtkTreeModel* model;
	GtkTreePath* path;
	GtkTreeIter iter;
	
	
	if (!FormAccount_GetCursor(fa, &model, &path, &iter))
	{
		return ;
	}
	
	gtk_list_store_set(GTK_LIST_STORE(model), &iter,
			COL_ID_CATEGORY_ID, id,
			COL_ID_CATEGORY_NAME, Desc, -1);
	FormAccount_UpdateModifiedRecordState(fa, &model, &iter);
	
	gtk_tree_path_free(path);
}

void FormAccount_UpdateData(FormAccount* fa, char* AccID)
{
	MYSQL_RES* ResSet;
	MYSQL_ROW Row;
	GtkTreeIter Iter;
	gdouble Money;
	guint Category;
	char* CategoryPath;
	char* p;
	
	
	fa->AccID = strtol(AccID, &p, 10);
	if (p == AccID)
	{
		g_print("%s(): bad AccID.\n", __func__);
		return ;
	}
	
	TreeViewCategories_UpdateData(&fa->tvc);
	gtk_tree_view_expand_all(GTK_TREE_VIEW(fa->tvc.treeview));
	gtk_list_store_clear(fa->ListStoreAccount);
	
	if (MySQLQuery(fa->MySQL,
			"SELECT id, date, description, money, category\n"
			"FROM operations WHERE account_id = %u;", fa->AccID) == -1)
	{
		return ;
	}
	
	ResSet = mysql_store_result(fa->MySQL);
	
	if (ResSet == NULL)
	{
		g_print("%s():mysql_store_result() error.\n", __func__);
		return ;
	}
	
	while (Row = mysql_fetch_row(ResSet))
	{
		errno = 0;
		Money    = Row[3] ? g_strtod(Row[3], NULL) : 0;
		Category = Row[4] ? strtoul(Row[4], NULL, 10) : 0;
		
		if (!TreeViewCategories_FindCategoryByID2(&fa->tvc, Category, NULL, NULL, &CategoryPath))
		{
			CategoryPath = g_strdup("");
		}
		
		gtk_list_store_append(fa->ListStoreAccount, &Iter);
		gtk_list_store_set(fa->ListStoreAccount, &Iter,
				COL_ID_RECORD_STATE,   REC_STATE_NONE,
				COL_ID_OPERATION_ID,   Row[0],
				COL_ID_DATE,           Row[1],
				COL_ID_DESCRIPTION,    Row[2],
				COL_ID_MONEY,          Money,
				COL_ID_CATEGORY_ID,    Category,
				COL_ID_CATEGORY_NAME,  CategoryPath, -1);
		
		g_free(CategoryPath);
	}
	
	mysql_free_result(ResSet);
}

void FormAccount_UpdateModifiedRecordState(FormAccount* fa, GtkTreeModel** model, GtkTreeIter* iter)
{
	guint rec_state;
	
	
	gtk_tree_model_get(*model, iter, COL_ID_RECORD_STATE, &rec_state, -1);
	
	switch (rec_state)
	{
	case REC_STATE_NONE:
		gtk_list_store_set(GTK_LIST_STORE(*model), iter,
				COL_ID_RECORD_STATE, (guint) REC_STATE_EDIT, -1);
		break;
	default:
		return ;
	}
}

void FormAccount_UpdateRemain(FormAccount* fa)
{
	GtkTreeModel* model;
	gdouble MoneyNow;
	gdouble MoneyTotal;
	gdouble MoneyRemain;
	const gchar* Str;
	gchar* StrEnd;
	gchar Buf[100];
	
	
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(fa->ListViewAccount));
	Str = gtk_entry_get_text(GTK_ENTRY(fa->EntryMoneyNow));
	MoneyNow = g_strtod(Str, &StrEnd);
	
	if (Calc(Str, &MoneyNow) == -1)
	{
		g_snprintf(Buf, sizeof(Buf), "<span foreground=\"#FF0000\">inv!</span>");
	}
	else
	{
		FormAccount_SumMoneyTotal(fa, model, &MoneyTotal);
		MoneyRemain = MoneyNow - MoneyTotal;
		g_snprintf(Buf, sizeof(Buf), "<span foreground=\"%s\">%.2f</span>",
				MoneyRemain >= 0 ? "#00FF00" : "#FF0000",
				MoneyRemain);
	}
	
	gtk_label_set_markup(GTK_LABEL(fa->LabelRemain), Buf);
}

void FormAccount_UpDown(FormAccount* fa, unsigned int updown)
{
	GtkTreePath* path;
	GtkTreeModel* model;
	GtkTreeIter iter;
	
	
	gtk_tree_view_get_cursor(GTK_TREE_VIEW(fa->ListViewAccount), &path, NULL);
	
	if (path == NULL)
	{
		path = gtk_tree_path_new_first();
	}
	else
	{
		switch (updown)
		{
		case 2:
			gtk_tree_path_next(path);
			model = gtk_tree_view_get_model(GTK_TREE_VIEW(fa->ListViewAccount));
			
			if (gtk_tree_model_get_iter(model, &iter, path) == FALSE)
			{
				gtk_tree_path_prev(path);
			}
			
			break;
		case 1:
			gtk_tree_path_prev(path);
			break;
		}
	}
	
	gtk_tree_view_set_cursor_on_cell(GTK_TREE_VIEW(fa->ListViewAccount),
			path, NULL, NULL, FALSE);
	gtk_tree_path_free(path);
}
