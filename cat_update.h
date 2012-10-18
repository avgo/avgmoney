#ifndef CAT_UPDATE_H
#define CAT_UPDATE_H

#include <mysql/mysql.h>
#include <mysql/mysqld_error.h>




struct CUList_;
struct CUListNode_;
struct CatUpdate_;




typedef struct CUList_ CUList;
typedef struct CUListNode_ CUListNode;
typedef struct CatUpdate_ CatUpdate;




struct CUList_
{
	CUListNode* first;
	CUListNode* last;
};

struct CatUpdate_
{
	CUList GreaterThan;
	CUList LessThan;
	unsigned int new_cat_id;
	MYSQL* MySQL;
};

struct CUListNode_
{
	unsigned int id;
	unsigned int parent_id;
	unsigned int old_id;
	unsigned int old_parent_id;
	unsigned int middle;
	CUListNode* next;
};




void CatUpdate_AddAllocatedAndSort(CatUpdate* cu, CUListNode* NewNode);
void CatUpdate_Execute(CatUpdate* cu);
void CatUpdate_Free(CatUpdate* cu);
void CatUpdate_Init(CatUpdate* cu);

CUListNode* CUListNode_Alloc();




#endif
