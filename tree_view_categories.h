#ifndef TREE_VIEW_CATEGORIES
#define TREE_VIEW_CATEGORIES

#include <gtk/gtk.h>
#include <mysql/mysql.h>
#include <mysql/mysqld_error.h>

#include "cat_update.h"




#define  TVC_HANDLER_DECLARATION(handler) \
		TreeViewCategories ## handler ## Handler handler ## Handler;\
		void* handler ## HandlerArg
#define  TVC_HANDLER_PROC_DECLARATION(handler) void TreeViewCategories_Set ## handler(TreeViewCategories* c,\
		TreeViewCategories ## handler ## Handler handler ## Handler, void* arg)




typedef void (*TreeViewCategoriesCursorChangedHandler)(unsigned int id, char* Desc, void* arg);
typedef void (*TreeViewCategoriesKeyReleaseHandler)(unsigned int id, void* arg);
typedef void (*TreeViewCategoriesRowActivatedHandler)(unsigned int id, char* Desc, void* arg);




struct TreeViewCategories_
{
	GtkWidget* treeview;
	MYSQL* MySQL;
	
	TVC_HANDLER_DECLARATION(CursorChanged);
	TVC_HANDLER_DECLARATION(KeyRelease);
	TVC_HANDLER_DECLARATION(RowActivated);
	
	unsigned int new_cat_id;
	GtkTreePath* TreePathMoveSrc;
	CatUpdate cat_update;
};

typedef struct TreeViewCategories_ TreeViewCategories;




void TreeViewCategories_CategoryAdd(TreeViewCategories* c, const char* CatName, unsigned int IsRoot);
void TreeViewCategories_CategoryDelete(TreeViewCategories* c);
void TreeViewCategories_CategoryRename(TreeViewCategories* c, gchar* Text);
void TreeViewCategories_Create(TreeViewCategories* c, MYSQL* MySQL);
void TreeViewCategories_Cut(TreeViewCategories* c);
int TreeViewCategories_FindCategoryByID2(TreeViewCategories* tvc, unsigned int id,
		unsigned int* parent_id, char** Name, char** Path);
void TreeViewCategories_Init(TreeViewCategories* c);
void TreeViewCategories_Paste(TreeViewCategories* c, unsigned int IsRoot);

TVC_HANDLER_PROC_DECLARATION(CursorChanged);
TVC_HANDLER_PROC_DECLARATION(KeyRelease);
TVC_HANDLER_PROC_DECLARATION(RowActivated);

void TreeViewCategories_UpdateData(TreeViewCategories* c);

#endif
