#ifndef CAT_REPORT_01_H
#define CAT_REPORT_01_H

#include <gtk/gtk.h>
#include <mysql/mysql.h>
#include <mysql/mysqld_error.h>

#include "tree_view_categories.h"
#include "cat_report_tree.h"




struct FormCatReport01_
{
	MYSQL* MySQL;
	GtkWidget* CalendarDate1;
	GtkWidget* CalendarDate2;
	GtkWidget* ComboBoxMonth;
	GtkWidget* DrawingArea;
	GtkWidget* SpinButtonYear;
	GtkWidget* TreeViewCategories;
	GtkWidget* TreeViewOperations;
	GtkWidget* WindowCatReport;
	GtkTreeViewColumn* TreeViewColumnCheck;
	gboolean CalendarDateLock;
	CatTree ct;
	CatTreeNode* LastMarked;
	gint radius0;
	gint radius1;
};

typedef struct FormCatReport01_ FormCatReport01;




void FormCatReport01_Create(FormCatReport01* fcr, MYSQL* MySQL);
void FormCatReport01_Init(FormCatReport01* fcr);




#endif
