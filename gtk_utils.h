#ifndef GTK_UTILS
#define GTK_UTILS

#include <gtk/gtk.h>

#define FORM_PRESENT(fp_form)\
	if ((fp_form) != NULL)\
	{\
		if (GTK_IS_WINDOW((fp_form)))\
		{\
			gtk_window_present(GTK_WINDOW((fp_form)));\
			g_print("%s(): " #fp_form " != NULL, present.\n", __func__); \
		}\
		\
		return ;\
	}\
	\
	g_print("%s(): " #fp_form " = NULL\n", __func__);




GtkWidget* gtk_combo_box_from_array(gchar** items);
void gtk_input_dialog(gchar* title, gchar* message, gchar* text_old, gchar** text);
gchar* gtk_tree_path_indices_name_gen(GtkTreePath* path, const char* prefix, const char* separator);
void gtk_tree_store_check1(GtkTreeModel* model, GtkTreeIter* iter, guint column_id);
void gtk_tree_store_copy(GtkTreeStore* store, GtkTreeIter* iter_src, GtkTreeIter* iter_dst);
void gtk_tree_store_copy2(GtkTreeStore* store, GtkTreeIter* iter_src, GtkTreeIter* iter_dst);
void gtk_tree_store_move(GtkTreeStore* store, GtkTreeIter* iter_src, GtkTreeIter* iter_dst);
GtkTreeViewColumn* gtk_tree_view_checkbox_column_new(gint column_id, gchar* title);
GtkTreeViewColumn* gtk_tree_view_text_column_new(gint column_id, gchar* title);
GtkTreeViewColumn* gtk_tree_view_money_column_new(gint column_id, gchar* title);




extern gchar* Months[];




#endif
