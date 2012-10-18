#include <gdk/gdkkeysyms.h>
#include <stdlib.h>
#include <string.h>

#include "tree_view_categories.h"
#include "gtk_utils.h"




enum RecordState_ {
	REC_STATE_NONE,
	REC_STATE_ADD,
	REC_STATE_EDIT,
	REC_STATE_DELETE,
	REC_STATE_MOVE
};

typedef enum RecordState_ RecordState;

enum {
	COL_ID_RECORD_STATE,
	COL_ID_ID,
	COL_ID_PARENT_ID,
	COL_ID_NAME,
	COL_ID_NAME2,
	N_COLUMNS
};

struct TreeViewCategoriesDelete_
{
	MYSQL_STMT* stmt;
	GtkTreeModel* model;
	unsigned int row_id;
};

typedef struct TreeViewCategoriesDelete_ TreeViewCategoriesDelete;




int TreeViewCategories_CategoryAddToDatabase(TreeViewCategories* c, unsigned int row_id, unsigned int row_parent_id, const char* row_cat_name);
int TreeViewCategories_CategoryDeleteFromDatabase(TreeViewCategories* c, GtkTreeModel* model, GtkTreeIter* iter, unsigned int row_id);
int TreeViewCategories_CategoryDeleteFromDatabase2(TreeViewCategoriesDelete* tvcd, GtkTreeIter* iter);
int TreeViewCategories_CategoryRenameAddToDatabase(TreeViewCategories* c, gchar* row_cat_name, unsigned int row_id);
GtkTreeViewColumn* TreeViewCategories_Column(gint colnum, gchar* title);
void TreeViewCategories_CursorChanged(GtkTreeView* treeview, gpointer user_data);
GtkTreeIter* TreeViewCategories_FindCategoryByID(GtkTreeModel* model, GtkTreeIter* iter1, GtkTreeIter* iter2, gint parent_id);
gchar* TreeViewCategories_GetName2(GtkTreeModel* model, GtkTreeIter* iter, const char* row);
gboolean TreeViewCategories_KeyRelease(GtkWidget* widget, GdkEvent* event, gpointer userdata);
void TreeViewCategories_RowActivated(GtkTreeView* treeview, GtkTreePath* treepath, GtkTreeViewColumn* column, gpointer userdata);




void TreeViewCategories_CategoryAdd(TreeViewCategories* c, const char* CatName, unsigned int IsRoot)
{
	GtkTreeModel* model;
	GtkTreePath* path;
	GtkTreeIter iter;
	GtkTreeIter iter_parent;
	GtkTreeIter* iter_parent_ptr;
	guint parent_id;
	gchar* CategoryName2Str;
	
	
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(c->treeview));
	
	if (IsRoot)
	{
		iter_parent_ptr = NULL;
		path = NULL;
		parent_id = 0;
	}
	else
	{
		gtk_tree_view_get_cursor(GTK_TREE_VIEW(c->treeview), &path, NULL);
		
		if (path == NULL)
		{
			iter_parent_ptr = NULL;
			parent_id = 0;
		}
		else
		{
			gtk_tree_model_get_iter(model, &iter_parent, path);
			iter_parent_ptr = &iter_parent;
			gtk_tree_model_get(model, iter_parent_ptr, COL_ID_ID, &parent_id, -1);
		}
	}
	
	CategoryName2Str = TreeViewCategories_GetName2(model, iter_parent_ptr, CatName);
	
	g_print("%s(): new category: id = %u, parent_id = %u, name = %s, name2 = %s.\n",
			__func__, c->new_cat_id, parent_id, CatName, CategoryName2Str);
	if (TreeViewCategories_CategoryAddToDatabase(c, c->new_cat_id, parent_id, CatName))
	{
		gtk_tree_store_append(GTK_TREE_STORE(model), &iter, iter_parent_ptr);
		gtk_tree_store_set(GTK_TREE_STORE(model), &iter,
				COL_ID_RECORD_STATE, REC_STATE_ADD,
				COL_ID_ID,           c->new_cat_id,
				COL_ID_PARENT_ID,    parent_id,
				COL_ID_NAME,         CatName,
				COL_ID_NAME2,        CategoryName2Str, -1);
		++c->new_cat_id;
	}
	if (path != NULL)
	{
		gtk_tree_view_expand_to_path(GTK_TREE_VIEW(c->treeview), path);
		gtk_tree_path_free(path);
	}
	
	g_free(CategoryName2Str);
}

int TreeViewCategories_CategoryAddToDatabase(TreeViewCategories* c, unsigned int row_id, unsigned int row_parent_id, const char* row_cat_name)
{
	MYSQL_STMT* stmt = NULL;
	const char* stmt_insert_str = "INSERT INTO categories(id, parent_id, name) VALUES (?, ?, ?);";
	MYSQL_BIND param_insert[3];
	unsigned long row_cat_name_strlen;
	unsigned int ret = 1;
	char row_cat_name_buf[100];
	
	
	stmt = mysql_stmt_init(c->MySQL);
	if (stmt == NULL)
	{
		fprintf(stderr, "%s(): mysql_stmt_init() error.\n", __func__);
		ret = 0;
		goto END;
	}
	if (mysql_stmt_prepare(stmt, stmt_insert_str, strlen(stmt_insert_str)))
	{
		fprintf(stderr, "%s(): mysql_stmt_prepare() error.\n", __func__);
		ret = 0;
		goto END;
	}
	
	memset(param_insert, 0, sizeof(param_insert));
	
	param_insert[0].buffer_type     = MYSQL_TYPE_LONG;
	param_insert[0].buffer          = (void*) &row_id;
	param_insert[0].is_unsigned     = 1;
	
	param_insert[1].buffer_type     = MYSQL_TYPE_LONG;
	param_insert[1].buffer          = (void*) &row_parent_id;
	param_insert[1].is_unsigned     = 1;
	
	row_cat_name_strlen = strlen(row_cat_name);
	param_insert[2].buffer_type     = MYSQL_TYPE_VAR_STRING;
	param_insert[2].buffer          = (void*) row_cat_name;
	param_insert[2].buffer_length   = row_cat_name_strlen + 1;
	param_insert[2].length          = &row_cat_name_strlen;
	
	if (mysql_stmt_bind_param(stmt, param_insert))
	{
		fprintf(stderr, "%s(): mysql_stmt_bind_param() error.\n", __func__);
		ret = 0;
		goto END;
	}
	
	if (mysql_stmt_execute(stmt))
	{
		fprintf(stderr, "%s(): mysql_stmt_execute() error.\n", __func__);
		ret = 0;
		goto END;
	}
	
END:
	if (stmt != NULL)
		mysql_stmt_close(stmt);
	return ret;
}

void TreeViewCategories_CategoryDelete(TreeViewCategories* c)
{
	GtkTreePath* path = NULL;
	GtkTreeModel* model;
	GtkTreeIter iter;
	gint id;
	
	
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(c->treeview));
	gtk_tree_view_get_cursor(GTK_TREE_VIEW(c->treeview), &path, NULL);
	
	if (path == NULL)
		goto END;
	
	if (!gtk_tree_model_get_iter(model, &iter, path))
		goto END;
	
	gtk_tree_model_get(model, &iter, COL_ID_ID, &id, -1);
	
	if (!TreeViewCategories_CategoryDeleteFromDatabase(c, model, &iter, id))
		goto END;
	
END:
	if (path != NULL)
		gtk_tree_path_free(path);
}

int TreeViewCategories_CategoryDeleteFromDatabase(TreeViewCategories* c, GtkTreeModel* model, GtkTreeIter* iter, unsigned int row_id)
{
	TreeViewCategoriesDelete tvcd;
	MYSQL_BIND param_delete[1];
	const char* stmt_update_str = "DELETE FROM categories WHERE id = ?;\n";
	int ret = 1;
	GtkTreeIter iter_child;
	
	gchar* Name2;
	
	
	tvcd.stmt = mysql_stmt_init(c->MySQL);
	if (tvcd.stmt == NULL)
	{
		fprintf(stderr, "%s(): mysql_stmt_init() error.\n", __func__);
		ret = 0;
		goto END;
	}
	
	if (mysql_stmt_prepare(tvcd.stmt, stmt_update_str, strlen(stmt_update_str)))
	{
		fprintf(stderr, "%s(): mysql_stmt_prepare() error.\n", __func__);
		ret = 0;
		goto END;
	}
	
	memset(param_delete, 0, sizeof(param_delete));
	
	param_delete[0].buffer_type  = MYSQL_TYPE_LONG;
	param_delete[0].buffer       = (void*) &tvcd.row_id;
	param_delete[0].is_unsigned  = 1;
	
	tvcd.model = model;
	if (mysql_stmt_bind_param(tvcd.stmt, param_delete))
	{
		fprintf(stderr, "%s(): \n", __func__);
		ret = 0;
		goto END;
	}
	
	if (gtk_tree_model_iter_children(model, &iter_child, iter))
	{
		if (!TreeViewCategories_CategoryDeleteFromDatabase2(&tvcd, &iter_child))
		{
			ret = 0;
			goto END;
		}
	}
	
	gtk_tree_model_get(tvcd.model, iter,
			COL_ID_ID,    &tvcd.row_id,
			COL_ID_NAME2, &Name2,
			-1);
	g_print("%s(): %s\n", __func__, Name2);
	g_free(Name2);
	
	if (mysql_stmt_execute(tvcd.stmt))
	{
		fprintf(stderr, "%s(): \n", __func__);
		ret = 0;
		goto END;
	}
	
	gtk_tree_store_remove(GTK_TREE_STORE(tvcd.model), iter);
	
END:
	if (tvcd.stmt != NULL)
		mysql_stmt_close(tvcd.stmt);
	return ret;
}

int TreeViewCategories_CategoryDeleteFromDatabase2(TreeViewCategoriesDelete* tvcd, GtkTreeIter* iter)
{
	GtkTreeIter iter_child;
	int ret = 1;
	gchar* Name2;
	
	
	for (;;)
	{
		if (gtk_tree_model_iter_children(tvcd->model, &iter_child, iter))
		{
			if (!TreeViewCategories_CategoryDeleteFromDatabase2(tvcd, &iter_child))
			{
				return 0;
			}
		}
		
		gtk_tree_model_get(tvcd->model, iter,
				COL_ID_ID,    &tvcd->row_id,
				COL_ID_NAME2, &Name2,
				-1);
		g_print("%s(): %s\n", __func__, Name2);
		g_free(Name2);
		
		if (mysql_stmt_execute(tvcd->stmt))
		{
			fprintf(stderr, "%s(): \n", __func__);
			return 0;
		}
		
		if (!gtk_tree_store_remove(GTK_TREE_STORE(tvcd->model), iter))
			return 1;
	}
}

GtkTreeViewColumn* TreeViewCategories_Column(gint colnum, gchar* title)
{
	GtkTreeViewColumn* column;
	GtkCellRenderer* renderer;
	
	
	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, title);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_attributes(column, renderer, "text", colnum, NULL);
	gtk_tree_view_column_set_expand(column, FALSE);
	
	return column;
}

void TreeViewCategories_Create(TreeViewCategories* c, MYSQL* MySQL)
{
	GtkTreeStore* treestore;
	GtkTreeViewColumn* column;
	
	
	CatUpdate_Init(&c->cat_update);
	c->MySQL = MySQL;
	c->new_cat_id = 1;
	
	treestore = gtk_tree_store_new(N_COLUMNS,
			G_TYPE_UINT,      /*  COL_ID_RECORD_STATE   */
			G_TYPE_UINT,      /*  COL_ID_ID             */
			G_TYPE_UINT,      /*  COL_ID_PARENT_ID      */
			G_TYPE_STRING,    /*  COL_ID_NAME           */
			G_TYPE_STRING);   /*  COL_ID_NAME2          */
	
	c->treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(treestore));
	g_object_unref(treestore);
	
	column = TreeViewCategories_Column(COL_ID_NAME, "Категория");
	gtk_tree_view_append_column(GTK_TREE_VIEW(c->treeview), column);
}

void TreeViewCategories_CursorChanged(GtkTreeView* treeview, gpointer user_data)
{
	TreeViewCategories* c = (TreeViewCategories*) user_data;
	GtkTreeModel* model;
	GtkTreePath* path = NULL;
	GtkTreeIter iter;
	gchar* CatName = NULL;
	gint cat_id;
	
	
	if (c->CursorChangedHandler == NULL)
	{
		goto END;
	}
	
	gtk_tree_view_get_cursor(treeview, &path, NULL);
	if (path == NULL)
	{
		goto END;
	}
	
	model = gtk_tree_view_get_model(treeview);
	if (!gtk_tree_model_get_iter(model, &iter, path))
	{
		goto END;
	}
	
	gtk_tree_model_get(model, &iter,
			COL_ID_ID, &cat_id,
			COL_ID_NAME, &CatName, -1);
	c->CursorChangedHandler(cat_id, CatName, c->CursorChangedHandlerArg);

END:
	if (CatName != NULL)
		g_free(CatName);
	if (path != NULL)
		gtk_tree_path_free(path);
}

void TreeViewCategories_Cut(TreeViewCategories* c)
{
	if (c->TreePathMoveSrc != NULL)
	{
		gtk_tree_path_free(c->TreePathMoveSrc);
	}
	
	gtk_tree_view_get_cursor(GTK_TREE_VIEW(c->treeview), &c->TreePathMoveSrc, NULL);
}

void TreeViewCategories_Enumerate(TreeViewCategories* c, GtkTreeStore* store, GtkTreeIter* iter, guint parent_id)
{
	gboolean IsNotEnd;
	GtkTreeIter iter2;
	GtkTreeIter iter_child;
	guint col_id;
	guint col_parent_id;
	gchar* col_name;
	CUListNode* NewNode;
	
	
	if (iter == NULL)
	{
		IsNotEnd = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store), &iter2);
		c->new_cat_id = 1;
	}
	else
	{
		IsNotEnd = TRUE;
		iter2 = *iter;
	}
	
	for ( ; IsNotEnd; IsNotEnd = gtk_tree_model_iter_next(GTK_TREE_MODEL(store), &iter2))
	{
		gtk_tree_model_get(GTK_TREE_MODEL(store), &iter2,
				COL_ID_ID,        &col_id,
				COL_ID_PARENT_ID, &col_parent_id,
				COL_ID_NAME,      &col_name,-1);
		
		NewNode = CUListNode_Alloc();
		
		NewNode->id            = c->new_cat_id;
		NewNode->parent_id     = parent_id;
		NewNode->old_id        = col_id;
		NewNode->old_parent_id = col_parent_id;
		
/*		g_print("id = %d, parent_id = %d, old_id = %d, old_parent_id = %d\n",
				NewNode->id,
				NewNode->parent_id,
				NewNode->old_id,
				NewNode->old_parent_id); */
		
		gtk_tree_store_set(store, &iter2,
				COL_ID_ID,        NewNode->id,
				COL_ID_PARENT_ID, NewNode->parent_id, -1);
		
		CatUpdate_AddAllocatedAndSort(&c->cat_update, NewNode);
		
		g_free(col_name);
		++c->new_cat_id;
		
		if (gtk_tree_model_iter_children(GTK_TREE_MODEL(store), &iter_child, &iter2))
		{
			TreeViewCategories_Enumerate(c, store, &iter_child, c->new_cat_id - 1);
		}
	}
}

GtkTreeIter* TreeViewCategories_FindCategoryByID(GtkTreeModel* model, GtkTreeIter* iter1, GtkTreeIter* iter2, gint parent_id)
{
	GtkTreeIter iter_child;
	gboolean isNotEnd;
	guint id;
	GtkTreeIter iter3;
	
	
	if (iter2 == NULL)
	{
		isNotEnd = gtk_tree_model_get_iter_first(model, &iter3);
	}
	else
	{
		isNotEnd = TRUE;
		iter3 = *iter2;
	}
	
	for ( ; isNotEnd; isNotEnd = gtk_tree_model_iter_next(model, &iter3))
	{
		gtk_tree_model_get(model, &iter3, COL_ID_ID, &id, -1);
		
		if (id == parent_id)
		{
			*iter1 = iter3;
			return iter1;
		}
		
		if (gtk_tree_model_iter_children(model, &iter_child, &iter3))
		{
			if (TreeViewCategories_FindCategoryByID(model, iter1, &iter_child, parent_id))
			{
				return iter1;
			}
		}
	}
	
	return NULL;
}

int TreeViewCategories_FindCategoryByID2(TreeViewCategories* tvc, unsigned int id, unsigned int* parent_id, char** Name, char** Path)
{
	GtkTreeModel* model;
	GtkTreeIter iter;
	GtkTreeIter* iter_ptr;
	
	unsigned int parent_id_tmp;
	char* NameTmp;
	char* PathTmp;
	
	
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(tvc->treeview));
	iter_ptr = TreeViewCategories_FindCategoryByID(model, &iter, NULL, id);
	
	if (iter_ptr == NULL)
	{
		parent_id_tmp = 0;
		NameTmp = NULL;
		PathTmp = NULL;
	}
	else
	{
		
		gtk_tree_model_get(model, iter_ptr,
				COL_ID_PARENT_ID, &parent_id_tmp,
				COL_ID_NAME,      &NameTmp,
				COL_ID_NAME2,     &PathTmp, -1);
	}
	
	if (parent_id != NULL)
		*parent_id = parent_id_tmp;
	
	if (Name != NULL)
		*Name = NameTmp;
	
	if (Path != NULL)
		*Path = PathTmp;
	
	return PathTmp ? 1 : 0;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Must be row != NULL !!
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

gchar* TreeViewCategories_GetName2(GtkTreeModel* model, GtkTreeIter* iter, const char* row)
{
	GString* str;
	GtkTreeIter iter1;
	GtkTreeIter iter2;
	GSList* list;
	GSList* list_iter;
	gchar* Desc;
	
	
	str = g_string_new(NULL);
	
	if (iter != NULL)
	{
		
		iter1 = *iter;
		list = NULL;
		Desc = NULL;
		
		for (;;)
		{
			gtk_tree_model_get(model, &iter1, COL_ID_NAME, &Desc, -1);
			if (Desc == NULL)
			{
				Desc = "";
			}
			list = g_slist_prepend(list, Desc);
			if (gtk_tree_model_iter_parent(model, &iter2, &iter1))
			{
				iter1 = iter2;
			}
			else
			{
				break;
			}
		}
		
		list_iter = list;
		
		g_string_append(str, list_iter->data);
		g_free(list_iter->data);
		list_iter = g_slist_next(list_iter);
		
		for ( ; list_iter != NULL; list_iter = g_slist_next(list_iter))
		{
			g_string_append_printf(str, ":%s", (gchar*) list_iter->data);
			g_free(list_iter->data);
		}
		
		g_slist_free(list);
		g_string_append_printf(str, ":%s", row);
		
		return g_string_free(str, FALSE);
	}
	else
	{
		return g_strdup(row);
	}
}

void TreeViewCategories_Init(TreeViewCategories* c)
{
	memset(c, 0, sizeof(TreeViewCategories));
}

gboolean TreeViewCategories_KeyRelease(GtkWidget* widget, GdkEvent* event, gpointer userdata)
{
	TreeViewCategories* c = (TreeViewCategories*) userdata;
	int updown;
	
	
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
	
	if (c->KeyReleaseHandler == NULL)
	{
		return FALSE;
	}
	
	c->KeyReleaseHandler(updown, c->KeyReleaseHandlerArg);
	
	return TRUE;
}

void TreeViewCategories_Paste(TreeViewCategories* c, unsigned int IsRoot)
{
	GtkTreeModel* tree_model;
	GtkTreePath* TreePathMoveDst;
	gint* indices_src;
	gint depth_src;
	gint* indices_dst;
	gint depth_dst;
	gchar* NameSrc;
	gchar* NameDst;
	int i;
	GtkTreeIter iter_dst;
	GtkTreeIter* iter_dst_ptr;
	GtkTreeIter iter_src;
	
	
	if (c->TreePathMoveSrc == NULL)
	{
		return ;
	}
	
	tree_model = gtk_tree_view_get_model(GTK_TREE_VIEW(c->treeview));
	gtk_tree_model_get_iter(tree_model, &iter_src, c->TreePathMoveSrc);
	NameSrc = gtk_tree_path_indices_name_gen(c->TreePathMoveSrc, "cat-src://", "/");
	
	if (IsRoot)
	{
		TreePathMoveDst = NULL;
		iter_dst_ptr = NULL;
		NameDst = gtk_tree_path_indices_name_gen(TreePathMoveDst, "cat-dst://", "/");
		
		g_print("%s\n"
		        "%s.", NameSrc, NameDst);
		
		indices_src = gtk_tree_path_get_indices_with_depth(c->TreePathMoveSrc, &depth_src);
		
		if (depth_src == 1)
		{
			g_print(" Исходное и конечное совпадают.\n\n");
			goto PASTE_FAIL;
		}
		g_print("\n\n");
	}
	else
	{
		gtk_tree_view_get_cursor(GTK_TREE_VIEW(c->treeview), &TreePathMoveDst, NULL);
		
		if (TreePathMoveDst == NULL)
		{
			goto PASTE_FAIL;
		}
		
		gtk_tree_model_get_iter(tree_model, &iter_dst, TreePathMoveDst);
		iter_dst_ptr = &iter_dst;
		NameDst = gtk_tree_path_indices_name_gen(TreePathMoveDst, "cat-dst://", "/");
		
		g_print("%s\n"
		        "%s.", NameSrc, NameDst);
		
		indices_src = gtk_tree_path_get_indices_with_depth(c->TreePathMoveSrc, &depth_src);
		indices_dst = gtk_tree_path_get_indices_with_depth(TreePathMoveDst, &depth_dst);
		
		for (i = 0; i < depth_src && i < depth_dst; ++i)
		{
			if (indices_src[i] != indices_dst[i])
				break;
		}
		if (i == depth_src)
		{
			g_print(" В себя же нельзя.\n\n");
			goto PASTE_FAIL;
		}
		if (i + 1 == depth_src && i == depth_dst)
		{
			g_print(" Исходное и конечное совпадают.\n\n");
			goto PASTE_FAIL;
		}
		g_print("\n\n");
	}
	
	gtk_tree_store_move(GTK_TREE_STORE(tree_model), &iter_src, iter_dst_ptr);
	
	c->cat_update.new_cat_id = c->new_cat_id;
	c->cat_update.MySQL = c->MySQL;
	
	TreeViewCategories_Enumerate(c, GTK_TREE_STORE(tree_model), NULL, 0);
	CatUpdate_Execute(&c->cat_update);
	CatUpdate_Free(&c->cat_update);
	
	gtk_tree_path_free(c->TreePathMoveSrc);  c->TreePathMoveSrc = NULL;

PASTE_FAIL:

	if (TreePathMoveDst != NULL)
		gtk_tree_path_free(TreePathMoveDst);
	
	if (NameSrc != NULL)
		g_free(NameSrc);
	if (NameDst != NULL)
		g_free(NameDst);
}

void TreeViewCategories_RowActivated(GtkTreeView* treeview, GtkTreePath* treepath, GtkTreeViewColumn* column, gpointer userdata)
{
	TreeViewCategories* c = (TreeViewCategories*) userdata;
	GtkTreeModel* model;
	GtkTreeIter iter;
	unsigned int id;
	gchar* Desc;
	
	
	if (c->RowActivatedHandler == NULL)
	{
		return ;
	}
	
	model = gtk_tree_view_get_model(treeview);
	gtk_tree_model_get_iter(model, &iter, treepath);
	gtk_tree_model_get(model, &iter,
			COL_ID_ID, &id,
			COL_ID_NAME2, &Desc, -1);
	
	c->RowActivatedHandler(id, Desc, (void*) c->RowActivatedHandlerArg);
	
	g_free(Desc);
}

TVC_HANDLER_PROC_DECLARATION(CursorChanged)
{
	if (c->CursorChangedHandler != NULL)
	{
		return ;
	}
	
	c->CursorChangedHandler = CursorChangedHandler;
	c->CursorChangedHandlerArg = arg;
	
	g_signal_connect(GTK_TREE_VIEW(c->treeview), "cursor-changed",
				G_CALLBACK(TreeViewCategories_CursorChanged), (gpointer) c);
}

TVC_HANDLER_PROC_DECLARATION(KeyRelease)
{
	if (c->KeyReleaseHandler != NULL)
	{
		return ;
	}
	
	c->KeyReleaseHandler = KeyReleaseHandler;
	c->KeyReleaseHandlerArg = arg;
	
	g_signal_connect(GTK_TREE_VIEW(c->treeview), "key-press-event",
				G_CALLBACK(TreeViewCategories_KeyRelease), (gpointer) c);
}

TVC_HANDLER_PROC_DECLARATION(RowActivated)
{
	if (c->RowActivatedHandler != NULL)
	{
		return ;
	}
	
	c->RowActivatedHandler = RowActivatedHandler;
	c->RowActivatedHandlerArg = arg;
	
	g_signal_connect(GTK_TREE_VIEW(c->treeview), "row-activated",
				G_CALLBACK(TreeViewCategories_RowActivated), (gpointer) c);
}

void TreeViewCategories_CategoryRename(TreeViewCategories* c, gchar* Text)
{
	GtkTreePath* path = NULL;
	GtkTreeModel* model;
	GtkTreeIter iter;
	gint id;
	
	
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(c->treeview));
	gtk_tree_view_get_cursor(GTK_TREE_VIEW(c->treeview), &path, NULL);
	
	if (path == NULL)
		goto END;
	
	if (!gtk_tree_model_get_iter(model, &iter, path))
		goto END;
	
	gtk_tree_model_get(model, &iter, COL_ID_ID, &id, -1);
	
	if (!TreeViewCategories_CategoryRenameAddToDatabase(c, Text, id))
		goto END;
	
	gtk_tree_store_set(GTK_TREE_STORE(model), &iter, COL_ID_NAME, Text, -1);
	
END:
	if (path != NULL)
		gtk_tree_path_free(path);
}

int TreeViewCategories_CategoryRenameAddToDatabase(TreeViewCategories* c, gchar* row_cat_name, unsigned int row_id)
{
	MYSQL_STMT* stmt = NULL;
	MYSQL_BIND param_update[2];
	char* stmt_update_str = "UPDATE categories SET name = ? WHERE id = ?;\n";
	unsigned int ret = 1;
	unsigned long row_cat_name_strlen;
	
	
	stmt = mysql_stmt_init(c->MySQL);
	if (stmt == NULL)
	{
		fprintf(stderr, "%s(): mysql_stmt_init() error.\n", __func__);
		ret = 0;
		goto END;
	}
	if (mysql_stmt_prepare(stmt, stmt_update_str, strlen(stmt_update_str)))
	{
		fprintf(stderr, "%s(): mysql_stmt_prepare() error.\n", __func__);
		ret = 0;
		goto END;
	}
	
	memset(param_update, 0, sizeof(param_update));
	
	row_cat_name_strlen = strlen(row_cat_name);
	param_update[0].buffer_type     = MYSQL_TYPE_VAR_STRING;
	param_update[0].buffer          = (void*) row_cat_name;
	param_update[0].buffer_length   = row_cat_name_strlen + 1;
	param_update[0].length          = &row_cat_name_strlen;
	
	param_update[1].buffer_type     = MYSQL_TYPE_LONG;
	param_update[1].buffer          = (void*) &row_id;
	param_update[1].is_unsigned     = 1;
	
	if (mysql_stmt_bind_param(stmt, param_update))
	{
		fprintf(stderr, "%s(): mysql_stmt_bind_param() error.\n", __func__);
		ret = 0;
		goto END;
	}
	
	if (mysql_stmt_execute(stmt))
	{
		fprintf(stderr, "%s(): mysql_stmt_execute() error.\n", __func__);
		ret = 0;
		goto END;
	}
	
END:
	if (stmt != NULL)
		mysql_stmt_close(stmt);
	return ret;
}

void TreeViewCategories_UpdateData(TreeViewCategories* c)
{
	GtkTreeModel* model;
	GtkTreeIter iter;
	GtkTreeIter iter_parent;
	GtkTreeIter* iter_parent_ptr;
	MYSQL_RES* res_set;
	MYSQL_ROW row;
	gint id;
	gint parent_id;
	gchar* CategoryName2Str;
	
	
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(c->treeview));
	gtk_tree_store_clear(GTK_TREE_STORE(model));
	
	/* Вообще-то странно почему без "ORDER BY" работает, но на всякий случай поставим. */
	if (MySQLQuery(c->MySQL, "SELECT id, parent_id, name FROM categories ORDER BY id;") == -1)
	{
		g_print("%s(): \n", __func__);
		return ;
	}
	
	res_set = mysql_store_result(c->MySQL);
	if (res_set == NULL)
	{
		return ;
	}
	
	c->new_cat_id = 1;
	
	while (row = mysql_fetch_row(res_set))
	{
		id = strtol(row[0], NULL, 10);
		parent_id = strtol(row[1], NULL, 10);
		
		iter_parent_ptr = TreeViewCategories_FindCategoryByID(model, &iter_parent, NULL, parent_id);
		CategoryName2Str = TreeViewCategories_GetName2(model, iter_parent_ptr, row[2] ? row[2] : "");
		
		gtk_tree_store_append(GTK_TREE_STORE(model), &iter, iter_parent_ptr);
		gtk_tree_store_set(GTK_TREE_STORE(model), &iter,
				COL_ID_RECORD_STATE, REC_STATE_NONE,
				COL_ID_ID,           id,
				COL_ID_PARENT_ID,    parent_id,
				COL_ID_NAME,         row[2] ? row[2] : "",
				COL_ID_NAME2,        CategoryName2Str, -1);
		
		if (id >= c->new_cat_id)
			c->new_cat_id = id + 1;
		
		g_free(CategoryName2Str);
	}
}
