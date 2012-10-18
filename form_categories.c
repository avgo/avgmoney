#include "form_categories.h"
#include "gtk_utils.h"




void FormCategories_ButtonAddClicked(GtkButton* button, gpointer user_data);
void FormCategories_ButtonCutClicked(GtkButton* button, gpointer user_data);
void FormCategories_ButtonDeleteClicked(GtkButton* button, gpointer user_data);
void FormCategories_ButtonPasteClicked(GtkButton* button, gpointer user_data);
void FormCategories_ButtonRenameClicked(GtkButton* button, gpointer user_data);
void FormCategories_Destroy(GtkObject* object, gpointer user_data);
void FormCategories_TreeViewCategoriesRowActivated(unsigned int id, char* Desc, void* arg);
void FormCategories_TreeViewCategoriesCursorChanged(unsigned int id, char* Desc, void* arg);




void FormCategories_ButtonAddClicked(GtkButton* button, gpointer user_data)
{
	FormCategories* fc = (FormCategories*) user_data;
	const char* CategoryName;
	gboolean IsRoot;
	
	
	CategoryName = gtk_entry_get_text(GTK_ENTRY(fc->EntryCategoryName));
	IsRoot = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(fc->CheckButtonRoot));
	TreeViewCategories_CategoryAdd(&fc->tvc, CategoryName, IsRoot);
}

void FormCategories_ButtonCutClicked(GtkButton* button, gpointer user_data)
{
	FormCategories* fc = (FormCategories*) user_data;
	
	
	TreeViewCategories_Cut(&fc->tvc);
}

void FormCategories_ButtonDeleteClicked(GtkButton* button, gpointer user_data)
{
	FormCategories* fc = (FormCategories*) user_data;
	
	
	TreeViewCategories_CategoryDelete(&fc->tvc);
}

void FormCategories_ButtonPasteClicked(GtkButton* button, gpointer user_data)
{
	FormCategories* fc = (FormCategories*) user_data;
	gboolean IsRoot;
	
	
	IsRoot = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(fc->CheckButtonRoot));
	TreeViewCategories_Paste(&fc->tvc, IsRoot);
}

void FormCategories_ButtonRenameClicked(GtkButton* button, gpointer user_data)
{
	FormCategories* fc = (FormCategories*) user_data;
	gchar* Text;
	
	
	Text = (gchar*) gtk_entry_get_text(GTK_ENTRY(fc->EntryCategoryName));
	if (Text == NULL)
		return ;
	TreeViewCategories_CategoryRename(&fc->tvc, Text);
}

void FormCategories_Create(FormCategories* fc, MYSQL* MySQL)
{
	GtkWidget* hbox;
	GtkWidget* vbox;
	GtkWidget* scrolled;
	GtkWidget* ButtonAdd;
	GtkWidget* ButtonRename;
	GtkWidget* ButtonDelete;
	GtkWidget* ButtonCut;
	GtkWidget* ButtonPaste;
	
	
	FORM_PRESENT(fc->WindowCategories);
	
	fc->WindowCategories = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(fc->WindowCategories), "Категории");
	
	vbox = gtk_vbox_new(FALSE, 0);
	
	scrolled = gtk_scrolled_window_new(NULL, NULL);
	
	TreeViewCategories_Create(&fc->tvc, MySQL);
	gtk_container_add(GTK_CONTAINER(scrolled), fc->tvc.treeview);
	gtk_box_pack_start(GTK_BOX(vbox), scrolled, TRUE, TRUE, 0);
	
	hbox = gtk_hbox_new(FALSE, 0);
	
	fc->CheckButtonRoot = gtk_check_button_new_with_label("В корень");
	gtk_box_pack_start(GTK_BOX(hbox), fc->CheckButtonRoot, FALSE, FALSE, 0);
	
	fc->EntryCategoryName = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(hbox), fc->EntryCategoryName, TRUE, TRUE, 0);
	
	ButtonAdd = gtk_button_new_with_label("Добавить");
	gtk_box_pack_start(GTK_BOX(hbox), ButtonAdd, FALSE, FALSE, 0);
	
	ButtonRename = gtk_button_new_with_label("Переименовать");
	gtk_box_pack_start(GTK_BOX(hbox), ButtonRename, FALSE, FALSE, 0);
	
	ButtonDelete = gtk_button_new_with_label("Удалить");
	gtk_box_pack_start(GTK_BOX(hbox), ButtonDelete, FALSE, FALSE, 0);
	
	ButtonCut = gtk_button_new_with_label("Вырезать");
	gtk_box_pack_start(GTK_BOX(hbox), ButtonCut, FALSE, FALSE, 0);
	
	ButtonPaste = gtk_button_new_with_label("Вставить");
	gtk_box_pack_start(GTK_BOX(hbox), ButtonPaste, FALSE, FALSE, 0);
	
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
	
	gtk_container_add(GTK_CONTAINER(fc->WindowCategories), vbox);
	
	gtk_window_resize(GTK_WINDOW(fc->WindowCategories), 650, 500);
	gtk_window_move(GTK_WINDOW(fc->WindowCategories), 450, 150);
	
	g_signal_connect(ButtonAdd,            "clicked", G_CALLBACK(FormCategories_ButtonAddClicked), (gpointer) fc);
	g_signal_connect(ButtonRename,         "clicked", G_CALLBACK(FormCategories_ButtonRenameClicked), (gpointer) fc);
	g_signal_connect(ButtonDelete,         "clicked", G_CALLBACK(FormCategories_ButtonDeleteClicked), (gpointer) fc);
	g_signal_connect(ButtonCut,            "clicked", G_CALLBACK(FormCategories_ButtonCutClicked), (gpointer) fc);
	g_signal_connect(ButtonPaste,          "clicked", G_CALLBACK(FormCategories_ButtonPasteClicked), (gpointer) fc);
	g_signal_connect(fc->WindowCategories, "destroy", G_CALLBACK(FormCategories_Destroy), fc);
	TreeViewCategories_SetCursorChanged(&fc->tvc, FormCategories_TreeViewCategoriesCursorChanged, (void*) fc);
	TreeViewCategories_SetRowActivated(&fc->tvc, FormCategories_TreeViewCategoriesRowActivated, (void*) fc);
	
	gtk_widget_show_all(fc->WindowCategories);
	
	FormCategories_UpdateData(fc);
}

void FormCategories_Destroy(GtkObject* object, gpointer user_data)
{
	FormCategories* fc = (FormCategories*) user_data;
	
	fc->WindowCategories = NULL;
}

void FormCategories_Init(FormCategories* fc)
{
	fc->WindowCategories = NULL;
	TreeViewCategories_Init(&fc->tvc);
}

void FormCategories_TreeViewCategoriesRowActivated(unsigned int id, char* Desc, void* arg)
{
}

void FormCategories_TreeViewCategoriesCursorChanged(unsigned int id, char* Desc, void* arg)
{
	FormCategories* fc = arg;
	
	
	if (Desc == NULL)
		return ;
	
	gtk_entry_set_text(GTK_ENTRY(fc->EntryCategoryName), Desc);
}

void FormCategories_UpdateData(FormCategories* fc)
{
	TreeViewCategories_UpdateData(&fc->tvc);
}
