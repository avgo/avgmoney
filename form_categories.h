#ifndef FORM_CATEGORIES_H
#define FORM_CATEGORIES_H

#include <gtk/gtk.h>
#include <mysql/mysql.h>
#include <mysql/mysqld_error.h>

#include "tree_view_categories.h"

struct FormCategories_
{
	GtkWidget* CheckButtonRoot;
	GtkWidget* EntryCategoryName;
	GtkWidget* WindowCategories;
	TreeViewCategories tvc;
};

typedef struct FormCategories_ FormCategories;




void FormCategories_Create(FormCategories* fc, MYSQL* MySQL);
void FormCategories_Init(FormCategories* fc);
void FormCategories_UpdateData(FormCategories* fc);

#endif
