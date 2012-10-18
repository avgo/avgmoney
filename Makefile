
BINS=avgmoney

PREPROC=tree_view_categories.out.c form_account.out.c

OBJECTS_avgmoney=\
	avgmoney.o \
	calc.o \
	cat_report_tree.o \
	cat_update.o \
	form_account.o \
	form_accounts.o \
	form_categories.o \
	form_cat_report_01.o \
	gtk_utils.o \
	mysqlutils.o \
	tree_view_categories.o \
	utils.o

INCLUDE=`pkg-config --cflags gtk+-2.0`
LIBS=`pkg-config --libs gtk+-2.0` -lmysqlclient

all: $(PREPROC) $(BINS)

avgmoney: $(OBJECTS_avgmoney)
	gcc -o avgmoney $(OBJECTS_avgmoney) $(LIBS)

avgmoney.o: avgmoney.c avgmoney.h
	gcc $(INCLUDE) -c avgmoney.c

calc.o: calc.c calc.h
	gcc $(INCLUDE) -c calc.c

cat_report_tree.o: cat_report_tree.c cat_report_tree.h
	gcc $(INCLUDE) -c cat_report_tree.c

cat_update.o: cat_update.c cat_update.h
	gcc $(INCLUDE) -c cat_update.c

form_cat_report_01.o: form_cat_report_01.c form_cat_report_01.h
	gcc $(INCLUDE) -c form_cat_report_01.c

form_account.o: form_account.c form_account.h
	gcc $(INCLUDE) -c form_account.c

form_account.out.c: form_account.c form_account.h
	cpp $(INCLUDE) form_account.c form_account.out.c

form_accounts.o: form_accounts.c form_accounts.h
	gcc $(INCLUDE) -c form_accounts.c

form_categories.o: form_categories.c form_categories.h
	gcc $(INCLUDE) -c form_categories.c

gtk_utils.o: gtk_utils.c gtk_utils.h
	gcc $(INCLUDE) -c gtk_utils.c

mysqlutils.o: mysqlutils.c mysqlutils.h
	gcc $(INCLUDE) -c mysqlutils.c

tree_view_categories.o: tree_view_categories.c tree_view_categories.h
	gcc $(INCLUDE) -c tree_view_categories.c

tree_view_categories.out.c: tree_view_categories.c tree_view_categories.h
	cpp $(INCLUDE) tree_view_categories.c tree_view_categories.out.c

utils.o: utils.c utils.h
	gcc $(INCLUDE) -c utils.c

clean:
	rm -vf $(PREPROC) $(BINS) $(OBJECTS_avgmoney)
