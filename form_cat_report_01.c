#include <string.h>
#include <math.h>
#include "form_cat_report_01.h"
#include "gtk_utils.h"




enum
{
	TVC_COL_ID_ID,
	TVC_COL_ID_PARENT_ID,
	TVC_COL_ID_CHECK,
	TVC_COL_ID_NAME,
	TVC_COL_ID_MONEY,
	TVC_N_COLUMNS
};

enum
{
	TVO_COL_ID_DATE,
	TVO_COL_ID_DESCRIPTION,
	TVO_COL_ID_MONEY,
	TVO_N_COLUMNS
};




void FormCatReport01_CalendarDate1DaySelected(GtkCalendar* calendar, FormCatReport01* fcr);
void FormCatReport01_CalendarDate2DaySelected(GtkCalendar* calendar, FormCatReport01* fcr);
GtkTreeViewColumn* FormCatReport01_ColumnNew(gint column_id, gchar* title);
void FormCatReport01_ComboBoxMonthChanged(GtkComboBox *widget, FormCatReport01* fcr);
void FormCatReport01_CreateTreeViewCategories(FormCatReport01* fcr);
void FormCatReport01_CreateTreeViewOperations(FormCatReport01* fcr);
void FormCatReport01_Destroy(GtkObject* object, FormCatReport01* fcr);
gboolean FormCatReport01_ExposeEvent(GtkWidget* widget, GdkEventExpose* event, FormCatReport01* fcr);
void FormCatReport01_Filter(FormCatReport01* fcr);
void FormCatReport01_SpinButtonYearValueChanged(GtkSpinButton *SpinButton, FormCatReport01* fcr);
void FormCatReport01_TreeViewCategoriesCursorChanged(GtkTreeView* treeview, FormCatReport01* fcr);
void FormCatReport01_TreeViewCategoriesUpdate(FormCatReport01* fcr);
void FormCatReport01_TreeViewCategoriesUpdate2(GtkTreeModel* model, GtkTreeIter* iter, CatTreeNode* ctn);
void FormCatReport01_TreeViewOperationsFill(GtkTreeModel* model, CatTreeNode* ctn);
void FormCatReport01_TreeViewOperationsFill2(GtkTreeModel* model, CatTreeNode* ctn);
void FormCatReport01_TreeViewOperationsFill3(GtkTreeModel* model, CatRepOperations* opers);
void FormCatReport01_UpdateDate(FormCatReport01* fcr);




void FormCatReport01_CalendarDate1DaySelected(GtkCalendar* calendar, FormCatReport01* fcr)
{
	if (fcr->CalendarDateLock)
		return ;
	
	FormCatReport01_Filter(fcr);
}

void FormCatReport01_CalendarDate2DaySelected(GtkCalendar* calendar, FormCatReport01* fcr)
{
	if (fcr->CalendarDateLock)
		return ;
	
	FormCatReport01_Filter(fcr);
}

GtkTreeViewColumn* FormCatReport01_ColumnNew(gint column_id, gchar* title)
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

void FormCatReport01_ComboBoxMonthChanged(GtkComboBox *widget, FormCatReport01* fcr)
{
	FormCatReport01_UpdateDate(fcr);
	FormCatReport01_Filter(fcr);
}

void FormCatReport01_Create(FormCatReport01* fcr, MYSQL* MySQL)
{
	GtkWidget* vbox;
	GtkWidget* hbox;
	GtkWidget* scroll;
	GtkWidget* table;
	GtkWidget* label;
	GtkWidget* paned;
	gint row1, row2;
	
	
	FORM_PRESENT(fcr->WindowCatReport);
	
	fcr->MySQL = MySQL;
	fcr->CalendarDateLock = FALSE;
	fcr->LastMarked = NULL;
	
	CatTree_Init(&fcr->ct);
	CatTree_Load(&fcr->ct, fcr->MySQL);
	
	fcr->WindowCatReport = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_resize(GTK_WINDOW(fcr->WindowCatReport), 1200, 950);
	gtk_window_move(GTK_WINDOW(fcr->WindowCatReport), 350, 50);
	gtk_window_set_title(GTK_WINDOW(fcr->WindowCatReport), "avgmoney - Отчёт по категориям");
	
	vbox = gtk_vbox_new(FALSE, 0);
	
	hbox = gtk_hbox_new(FALSE, 0);
	
	table = gtk_table_new(4, 2, FALSE); row1 = 0; row2 = 1;
	
	label = gtk_label_new("От:");
	gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(table), label, 0, 1, row1, row2, GTK_FILL | GTK_EXPAND, GTK_FILL, 0, 0);
	
	fcr->CalendarDate1 = gtk_calendar_new();
	gtk_table_attach(GTK_TABLE(table), fcr->CalendarDate1, 1, 2, row1, row2, GTK_FILL, GTK_FILL, 0, 0);
	++row1; ++row2;
	
	label = gtk_label_new("До:");
	gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(table), label, 0, 1, row1, row2, GTK_FILL | GTK_EXPAND, GTK_FILL, 0, 0);
	
	fcr->CalendarDate2 = gtk_calendar_new();
	gtk_table_attach(GTK_TABLE(table), fcr->CalendarDate2, 1, 2, row1, row2, GTK_FILL, GTK_FILL, 0, 0);
	++row1; ++row2;
	
	label = gtk_label_new("Месяц:");
	gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(table), label, 0, 1, row1, row2, GTK_FILL | GTK_EXPAND, GTK_FILL, 0, 0);
	
	fcr->ComboBoxMonth = gtk_combo_box_from_array(Months);
	gtk_table_attach(GTK_TABLE(table), fcr->ComboBoxMonth, 1, 2, row1, row2, GTK_FILL, GTK_FILL, 0, 0);
	++row1; ++row2;
	
	label = gtk_label_new("Год:");
	gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
	gtk_table_attach(GTK_TABLE(table), label, 0, 1, row1, row2, GTK_FILL | GTK_EXPAND, GTK_FILL, 0, 0);
	
	fcr->SpinButtonYear = gtk_spin_button_new_with_range(0, 1000000, 1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(fcr->SpinButtonYear), 2012);
	gtk_table_attach(GTK_TABLE(table), fcr->SpinButtonYear, 1, 2, row1, row2, GTK_FILL, GTK_FILL, 0, 0);
	++row1; ++row2;
	
	gtk_box_pack_start(GTK_BOX(vbox), table, FALSE, FALSE, 0);
	
	scroll = gtk_scrolled_window_new(NULL, NULL);
	gtk_widget_set_usize(GTK_WIDGET(scroll), 400, -1);
	FormCatReport01_CreateTreeViewCategories(fcr);
	gtk_container_add(GTK_CONTAINER(scroll), fcr->TreeViewCategories);
	
	gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 0);
	
	gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 0);
	
	paned = gtk_vpaned_new();
	gtk_paned_set_position(GTK_PANED(paned), 500);
	
	fcr->DrawingArea = gtk_drawing_area_new();
	gtk_paned_add1(GTK_PANED(paned), fcr->DrawingArea);
	
	scroll = gtk_scrolled_window_new(NULL, NULL);
	FormCatReport01_CreateTreeViewOperations(fcr);
	gtk_container_add(GTK_CONTAINER(scroll), fcr->TreeViewOperations);
	gtk_paned_add2(GTK_PANED(paned), scroll);
	
	gtk_box_pack_start(GTK_BOX(hbox), paned, TRUE, TRUE, 0);
	
	gtk_container_add(GTK_CONTAINER(fcr->WindowCatReport), hbox);
	
	g_signal_connect(GTK_CALENDAR(fcr->CalendarDate1), "day-selected",
				G_CALLBACK(FormCatReport01_CalendarDate1DaySelected), (gpointer) fcr);
	g_signal_connect(GTK_CALENDAR(fcr->CalendarDate2), "day-selected",
				G_CALLBACK(FormCatReport01_CalendarDate2DaySelected), (gpointer) fcr);
	g_signal_connect(GTK_COMBO_BOX(fcr->ComboBoxMonth), "changed",
				G_CALLBACK(FormCatReport01_ComboBoxMonthChanged), (gpointer) fcr);
	g_signal_connect(GTK_SPIN_BUTTON(fcr->SpinButtonYear), "value-changed",
				G_CALLBACK(FormCatReport01_SpinButtonYearValueChanged), (gpointer) fcr);
	g_signal_connect(GTK_WINDOW(fcr->WindowCatReport), "destroy", G_CALLBACK(FormCatReport01_Destroy), (gpointer) fcr);
	g_signal_connect(GTK_TREE_VIEW(fcr->TreeViewCategories), "cursor-changed",
				G_CALLBACK(FormCatReport01_TreeViewCategoriesCursorChanged), (gpointer) fcr);
	g_signal_connect(GTK_DRAWING_AREA(fcr->DrawingArea), "expose-event",
				G_CALLBACK(FormCatReport01_ExposeEvent), (gpointer) fcr);
	
	gtk_widget_show_all(fcr->WindowCatReport);
}

void FormCatReport01_CreateTreeViewCategories(FormCatReport01* fcr)
{
	GtkTreeStore* store;
	GtkTreeViewColumn* column;
	
	
	store = gtk_tree_store_new(TVC_N_COLUMNS,
			G_TYPE_UINT,
			G_TYPE_UINT,
			G_TYPE_BOOLEAN,
			G_TYPE_STRING,
			G_TYPE_DOUBLE);
	fcr->TreeViewCategories = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	g_object_unref(G_OBJECT(store));
	
	column = gtk_tree_view_text_column_new(TVC_COL_ID_NAME, "Категория");
	gtk_tree_view_append_column(GTK_TREE_VIEW(fcr->TreeViewCategories), column);
	
	fcr->TreeViewColumnCheck = gtk_tree_view_checkbox_column_new(TVC_COL_ID_CHECK, "Сумм");
	gtk_tree_view_append_column(GTK_TREE_VIEW(fcr->TreeViewCategories), fcr->TreeViewColumnCheck);
	
	column = gtk_tree_view_money_column_new(TVC_COL_ID_MONEY, "Всего");
	gtk_tree_view_append_column(GTK_TREE_VIEW(fcr->TreeViewCategories), column);
}

void FormCatReport01_CreateTreeViewOperations(FormCatReport01* fcr)
{
	GtkListStore* store;
	GtkTreeViewColumn* column;
	
	
	store = gtk_list_store_new(TVO_N_COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_DOUBLE);
	fcr->TreeViewOperations = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	g_object_unref(G_OBJECT(store));
	
	column = FormCatReport01_ColumnNew(TVO_COL_ID_DATE, "Дата");
	gtk_tree_view_append_column(GTK_TREE_VIEW(fcr->TreeViewOperations), column);
	
	column = FormCatReport01_ColumnNew(TVO_COL_ID_DESCRIPTION, "Описание");
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_append_column(GTK_TREE_VIEW(fcr->TreeViewOperations), column);
	
	column = gtk_tree_view_money_column_new(TVO_COL_ID_MONEY, "Всего");
	gtk_tree_view_append_column(GTK_TREE_VIEW(fcr->TreeViewOperations), column);
}

void FormCatReport01_Destroy(GtkObject* object, FormCatReport01* fcr)
{
	CatTree_Close(&fcr->ct);
	fcr->WindowCatReport = NULL;
}

void FormCatReport01_Draw(FormCatReport01* fcr, cairo_t* c,
		CatTreeNode* ctn, GtkAllocation* allocation, gint radius, gint width, gdouble angle1, gdouble percent)
{
	double angle2;
	double cr, cg, cb;
	
	
	for ( ; ctn != NULL; ctn = ctn->next)
	{
		if (ctn->have_operations && ctn->sum < 0.0)
		{
			// g_print("%s(): \"%s\".\n", __func__, ctn->name);
			
			angle2 = 2*M_PI*percent*ctn->percent + angle1;
			
			// g_print("%s(): angle1 = %.2f, angle2 = %.2f.\n", __func__, angle1, angle2);
			cairo_arc(c,          allocation->width / 2, allocation->height / 2, radius+width, angle1, angle2);
			cairo_arc_negative(c, allocation->width / 2, allocation->height / 2, radius,       angle2, angle1);
			cairo_close_path(c);
			
			cr = 0.255;
			cg = 0.855;
			cb = 0.3;
			
			if (ctn->mark)
			{
				cr *= 0.80;
				cg *= 0.80;
				cb *= 0.80;
			}
			
			cairo_set_source_rgb(c, cr, cg, cb); 
			cairo_fill_preserve(c);
			cairo_set_source_rgb(c, 0, 0, 0);
			cairo_stroke(c);
			
			if (ctn->child)
				FormCatReport01_Draw(fcr, c, ctn->child, allocation,
						radius+width, width, angle1, percent*ctn->percent);
			
			angle1 = angle2;
		}
	}
	// g_print("\n");
}

gboolean FormCatReport01_ExposeEvent(GtkWidget* widget, GdkEventExpose* event, FormCatReport01* fcr)
{
	cairo_t* c;
	GtkAllocation allocation;
	
	
	fcr->radius0 = 50;
	fcr->radius1 = 50;
	
	gtk_widget_get_allocation(fcr->DrawingArea, &allocation);
/*	g_print("%s(): x = %d, y = %d, width = %d, height = %d\n", __func__,
			allocation.x,
			allocation.y,
			allocation.width,
			allocation.height); */
	
	c = gdk_cairo_create(fcr->DrawingArea->window);
	
	cairo_set_line_width(c, 1);
	
	FormCatReport01_Draw(fcr, c, fcr->ct.First, &allocation, fcr->radius0, fcr->radius1, 0.0, 1.0);
	
	cairo_destroy(c);
}

void FormCatReport01_Filter(FormCatReport01* fcr)
{
	unsigned int d1, m1, y1;
	unsigned int d2, m2, y2;
	
	
	gtk_calendar_get_date(GTK_CALENDAR(fcr->CalendarDate1), &y1, &m1, &d1);
	gtk_calendar_get_date(GTK_CALENDAR(fcr->CalendarDate2), &y2, &m2, &d2);
	
	++m1;
	++m2;
	
	if (fcr->LastMarked)
	{
		fcr->LastMarked->mark = 0;
		fcr->LastMarked = NULL;
	}
	
	CatTree_Filter(&fcr->ct, d1, m1, y1, d2, m2, y2);
	FormCatReport01_TreeViewCategoriesUpdate(fcr);
	gtk_widget_queue_draw(fcr->DrawingArea);
}

void FormCatReport01_Init(FormCatReport01* fcr)
{
	memset(fcr, 0, sizeof(*fcr));
}

void FormCatReport01_SpinButtonYearValueChanged(GtkSpinButton *SpinButton, FormCatReport01* fcr)
{
	FormCatReport01_UpdateDate(fcr);
	FormCatReport01_Filter(fcr);
}

void FormCatReport01_TreeViewCategoriesCursorChanged(GtkTreeView* treeview, FormCatReport01* fcr)
{
	CatTreeNode* ctn;
	GtkTreeModel* tvc_model;
	GtkTreeModel* tvo_model;
	GtkTreePath* path = NULL;
	GtkTreeViewColumn* column;
	GtkTreeIter iter;
	guint id;
	gboolean check;
	gint column_id;
	
	
	gtk_tree_view_get_cursor(treeview, &path, &column);
	if (path == NULL)
		goto END;
	
	tvc_model = gtk_tree_view_get_model(GTK_TREE_VIEW(fcr->TreeViewCategories));
	if (!gtk_tree_model_get_iter(tvc_model, &iter, path))
		goto END;
	
	gtk_tree_model_get(tvc_model, &iter, TVC_COL_ID_ID, &id, -1);
	ctn = CatTreeNode_FindByID(fcr->ct.First, id);
	if (!ctn)
	{
		return ;
	}
	
	if (column != NULL)
	{
		if (column == fcr->TreeViewColumnCheck)
		{
			gtk_tree_model_get(tvc_model, &iter, TVC_COL_ID_CHECK, &check, -1);
			ctn->is_sum = !check;
			FormCatReport01_Filter(fcr);
		}
		else
		{
			if (fcr->LastMarked)
				fcr->LastMarked->mark = 0;
			ctn->mark = 1;
			fcr->LastMarked = ctn;
			tvo_model = gtk_tree_view_get_model(GTK_TREE_VIEW(fcr->TreeViewOperations));
			gtk_list_store_clear(GTK_LIST_STORE(tvo_model));
			FormCatReport01_TreeViewOperationsFill(tvo_model, ctn);
			gtk_widget_queue_draw(fcr->DrawingArea);
		}
	}
	
	
END:
	if (path != NULL)
		gtk_tree_path_free(path);
}

void FormCatReport01_TreeViewCategoriesUpdate(FormCatReport01* fcr)
{
	GtkTreeModel* model;
	CatTreeNode* ctn;
	CatRepOperation* oper;
	GtkTreeIter iter;
	
	
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(fcr->TreeViewCategories));
	
	gtk_tree_store_clear(GTK_TREE_STORE(model));
	FormCatReport01_TreeViewCategoriesUpdate2(model, NULL, fcr->ct.First);
	gtk_tree_store_append(GTK_TREE_STORE(model), &iter, NULL);
	gtk_tree_store_set(GTK_TREE_STORE(model), &iter,
			TVC_COL_ID_ID,         0,
			TVC_COL_ID_PARENT_ID,  0,
			TVC_COL_ID_CHECK,      (gboolean) TRUE,
			TVC_COL_ID_NAME,       "Итог:",
			TVC_COL_ID_MONEY,      (gdouble) fcr->ct.sum, -1);
}

void FormCatReport01_TreeViewCategoriesUpdate2(GtkTreeModel* model, GtkTreeIter* iter_parent, CatTreeNode* ctn)
{
	GtkTreeIter iter;
	GString* str;
	
	
	for ( ; ctn != NULL; ctn = ctn->next)
	{
		if (!ctn->have_operations)
			continue;
		str = g_string_new(NULL);
		g_string_sprintf(str, "%s (%.2f%%)", ctn->name, ctn->percent * 100.00);
		gtk_tree_store_append(GTK_TREE_STORE(model), &iter, iter_parent);
		gtk_tree_store_set(GTK_TREE_STORE(model), &iter,
				TVC_COL_ID_ID,         ctn->id,
				TVC_COL_ID_PARENT_ID,  ctn->parent_id,
				TVC_COL_ID_CHECK,      (gboolean) ctn->is_sum,
				TVC_COL_ID_NAME,       str->str,
				TVC_COL_ID_MONEY,      (gdouble) ctn->sum, -1);
		g_string_free(str, TRUE);
		
		if (ctn->child)
			FormCatReport01_TreeViewCategoriesUpdate2(model, &iter, ctn->child);
	}
	
}

void FormCatReport01_TreeViewOperationsFill(GtkTreeModel* model, CatTreeNode* ctn)
{
	FormCatReport01_TreeViewOperationsFill3(model, &ctn->opers);
	if (ctn->child)
		FormCatReport01_TreeViewOperationsFill2(model, ctn->child);
}

void FormCatReport01_TreeViewOperationsFill2(GtkTreeModel* model, CatTreeNode* ctn)
{
	for ( ; ctn != NULL; ctn = ctn->next)
	{
		FormCatReport01_TreeViewOperationsFill3(model, &ctn->opers);
		if (ctn->child)
			FormCatReport01_TreeViewOperationsFill2(model, ctn->child);
	}
}

void FormCatReport01_TreeViewOperationsFill3(GtkTreeModel* model, CatRepOperations* opers)
{
	CatRepOperation* oper;
	GtkTreeIter iter;
	gchar DateBuf[20];
	
	
	for (oper = opers->First; oper != NULL; oper = oper->next)
	{
		snprintf(DateBuf, sizeof(DateBuf), "%02u.%02u.%04u",
				oper->Day, oper->Month, oper->Year);
		
		gtk_list_store_append(GTK_LIST_STORE(model), &iter);
		gtk_list_store_set(GTK_LIST_STORE(model), &iter,
				TVO_COL_ID_DATE,         DateBuf,
				TVO_COL_ID_DESCRIPTION,  oper->Description,
				TVO_COL_ID_MONEY,        oper->Money, -1);
	}
}

void FormCatReport01_UpdateDate(FormCatReport01* fcr)
{
	guint Month, Year;
	guint8 DaysInMonth;
	
	
	Month = gtk_combo_box_get_active(GTK_COMBO_BOX(fcr->ComboBoxMonth)) + 1;
	Year = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(fcr->SpinButtonYear));
	DaysInMonth = g_date_get_days_in_month(Month, Year);
	
	fcr->CalendarDateLock = TRUE;
	
	gtk_calendar_select_day(GTK_CALENDAR(fcr->CalendarDate1), 0);
	gtk_calendar_select_month(GTK_CALENDAR(fcr->CalendarDate1), Month-1, Year);
	gtk_calendar_select_day(GTK_CALENDAR(fcr->CalendarDate1), 1);
	
	gtk_calendar_select_day(GTK_CALENDAR(fcr->CalendarDate2), 0);
	gtk_calendar_select_month(GTK_CALENDAR(fcr->CalendarDate2), Month-1, Year);
	gtk_calendar_select_day(GTK_CALENDAR(fcr->CalendarDate2), DaysInMonth);
	
	fcr->CalendarDateLock = FALSE;
}
