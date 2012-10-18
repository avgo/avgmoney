#include "gtk_utils.h"




void gtk_tree_store_check2(GtkTreeModel* model, GtkTreeIter* iter, guint column_id, gboolean check);
void gtk_tree_view_money_column_cdf(GtkTreeViewColumn* column, GtkCellRenderer* renderer,
		GtkTreeModel* model, GtkTreeIter* iter, gpointer userdata);




gchar* Months[] = {
	"Январь",
	"Февраль",
	"Март",
	"Апрель",
	"Май",
	"Июнь",
	"Июль",
	"Август",
	"Сентябрь",
	"Октябрь",
	"Ноябрь",
	"Декабрь",
	NULL
};




GtkWidget* gtk_combo_box_from_array(gchar** items)
{
	GtkWidget* ComboBox;
	
	
	ComboBox = gtk_combo_box_text_new();
	
	for ( ; *items; ++items)
		gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(ComboBox), *items);
	gtk_combo_box_set_active(GTK_COMBO_BOX(ComboBox), 0);
	
	return ComboBox;
}

void gtk_input_dialog(gchar* title, gchar* message, gchar* text_old, gchar** text)
{
	GtkWidget* dialog;
	GtkWidget* content_area;
	GtkWidget* vbox;
	GtkWidget* entry;
	GtkWidget* label;
	gint dialog_result;
	gchar* text1;
	
	
	dialog = gtk_dialog_new_with_buttons(title,
			NULL,
			GTK_DIALOG_MODAL,
			GTK_STOCK_OK,
			GTK_RESPONSE_OK,
			GTK_STOCK_CANCEL,
			GTK_RESPONSE_CANCEL, NULL);
	content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
	vbox = gtk_vbox_new(FALSE, 0);
	
	label = gtk_label_new(message);
	gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
	gtk_misc_set_padding(GTK_MISC(label), 0, 30);
	gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);
	
	entry = gtk_entry_new();
	if (text_old)
		gtk_entry_set_text(GTK_ENTRY(entry), text_old);
	gtk_box_pack_start(GTK_BOX(vbox), entry, FALSE, FALSE, 0);
	
	gtk_container_add(GTK_CONTAINER(content_area), vbox);
	gtk_window_resize(GTK_WINDOW(dialog), 600, 100);
	gtk_widget_show_all(dialog);
	
	dialog_result = gtk_dialog_run(GTK_DIALOG(dialog));
	
	text1 = g_strdup(gtk_entry_get_text(GTK_ENTRY(entry)));
	
	gtk_widget_destroy(dialog);
	
	if (dialog_result == GTK_RESPONSE_OK)
	{
		*text = text1;
	}
	else
	{
		g_free(text1);
		*text = NULL;
	}
}

gchar* gtk_tree_path_indices_name_gen(GtkTreePath* path, const char* prefix, const char* separator)
{
	gint* indices;
	gint depth, i;
	GString* str;
	const char* sep;
	
	
	if (path == NULL)
	{
		return g_strdup(prefix);
	}
	
	str = g_string_new(prefix);
	
	indices = gtk_tree_path_get_indices_with_depth(path, &depth);
	for (i = 0; i < depth; ++i)
	{
		if (i == 0)
			sep = "";
		else
			sep = separator;
		g_string_append_printf(str, "%s%02d", sep, indices[i]+1);
	}
//	g_print("depth: %d, str: %s\n", depth, str->str);
	
	return g_string_free(str, FALSE);
}

void gtk_tree_store_check1(GtkTreeModel* model, GtkTreeIter* iter, guint column_id)
{
	GtkTreeIter iter_child;
	gboolean check;
	gboolean isValid;
	
	
	gtk_tree_model_get(model, iter, column_id, &check, -1);
	check = !check;
	gtk_tree_store_set(GTK_TREE_STORE(model), iter,
			column_id, check, -1);
	
	if (gtk_tree_model_iter_children(model, &iter_child, iter))
	{
		gtk_tree_store_check2(model, &iter_child, column_id, check);
	}
}

void gtk_tree_store_check2(GtkTreeModel* model, GtkTreeIter* iter, guint column_id, gboolean check)
{
	GtkTreeIter iter_child;
	gboolean isValid;
	
	
	for (isValid = TRUE; isValid; isValid = gtk_tree_model_iter_next(model, iter))
	{
		gtk_tree_store_set(GTK_TREE_STORE(model), iter,
				column_id, check, -1);
		
		if (gtk_tree_model_iter_children(model, &iter_child, iter))
		{
			gtk_tree_store_check2(model, &iter_child, column_id, check);
		}
	}
}

void gtk_tree_store_copy(GtkTreeStore* store, GtkTreeIter* iter_src, GtkTreeIter* iter_dst)
{
	gint col_index, n_columns;
	gpointer value;
	GType col_type;
	
	
	n_columns = gtk_tree_model_get_n_columns(GTK_TREE_MODEL(store));
	for (col_index = 0; col_index < n_columns; ++col_index)
	{
		col_type = gtk_tree_model_get_column_type(GTK_TREE_MODEL(store), col_index);
		switch (col_type)
		{
		case G_TYPE_STRING:
		case G_TYPE_UINT:
			break;
		default:
			g_print("%s(): %d type not supported.\n", __func__, col_type);
			continue ;
		}
		
		gtk_tree_model_get(GTK_TREE_MODEL(store), iter_src, col_index, &value, -1);
		gtk_tree_store_set(store, iter_dst, col_index, value, -1);
		
		switch (col_type)
		{
		case G_TYPE_STRING:
			g_free(value);
			break;
		default: ;
		}
	}
}

void gtk_tree_store_copy2(GtkTreeStore* store, GtkTreeIter* iter_src, GtkTreeIter* iter_dst)
{
	GtkTreeIter iter_src2;
	GtkTreeIter iter_dst2;
	gboolean valid;
	
	
	if (!gtk_tree_model_iter_children(GTK_TREE_MODEL(store), &iter_src2, iter_src))
	{
		return ;
	}
	
	for ( valid = TRUE; valid; valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(store), &iter_src2))
	{
		gtk_tree_store_append(store, &iter_dst2, iter_dst);
		gtk_tree_store_copy(store, &iter_src2, &iter_dst2);
		
		if (gtk_tree_model_iter_has_child(GTK_TREE_MODEL(store), &iter_src2))
		{
			gtk_tree_store_copy2(store, &iter_src2, &iter_dst2);
		}
	}
}

void gtk_tree_store_move(GtkTreeStore* store, GtkTreeIter* iter_src, GtkTreeIter* iter_dst)
{
	GtkTreeIter iter_dst2;
	
	
	gtk_tree_store_append(store, &iter_dst2, iter_dst);
	gtk_tree_store_copy(store, iter_src, &iter_dst2);
	gtk_tree_store_copy2(store, iter_src, &iter_dst2);
	gtk_tree_store_remove(store, iter_src);
}

GtkTreeViewColumn* gtk_tree_view_checkbox_column_new(gint column_id, gchar* title)
{
	GtkTreeViewColumn* column;
	GtkCellRenderer* renderer;
	
	
	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, title);
	renderer = gtk_cell_renderer_toggle_new();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_attributes(column, renderer, "active", column_id, NULL);
	
	return column;
}

GtkTreeViewColumn* gtk_tree_view_text_column_new(gint column_id, gchar* title)
{
	GtkTreeViewColumn* column;
	GtkCellRenderer* renderer;
	
	
	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, title);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_attributes(column, renderer, "text", column_id, NULL);
	
	return column;
}

void gtk_tree_view_money_column_cdf(GtkTreeViewColumn* column, GtkCellRenderer* renderer,
		GtkTreeModel* model, GtkTreeIter* iter, gpointer userdata)
{
	gdouble money;
	char Buf[50];
	GdkColor color;
	gint col_id;
	
	
	col_id = (gint) userdata;
	gtk_tree_model_get(model, iter, col_id, &money, -1);
	sprintf(Buf, "%.2f", money);
	
	if (money >= 0.0)
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

GtkTreeViewColumn* gtk_tree_view_money_column_new(gint column_id, gchar* title)
{
	GtkTreeViewColumn* column;
	GtkCellRenderer* renderer;
	
	
	column = gtk_tree_view_column_new();
	gtk_tree_view_column_set_title(column, title);
	renderer = gtk_cell_renderer_text_new();
	g_object_set(renderer, "xalign", 1.0, NULL);
	gtk_tree_view_column_pack_start(column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func(column, renderer, gtk_tree_view_money_column_cdf, (gpointer) column_id, NULL);
	
	return column;
}
