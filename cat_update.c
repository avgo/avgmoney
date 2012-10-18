#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cat_update.h"




struct CatUpdateStmt_
{
	MYSQL* MySQL;
	MYSQL_STMT* stmt;
	char* stmt_update1_str;
	char* stmt_update2_str;
	MYSQL_BIND param_update[3];
	
	unsigned int row_id;
	unsigned int row_parent_id;
	unsigned int row_old_id;
};

typedef struct CatUpdateStmt_ CatUpdateStmt;

enum CatUpdateSorting_
{
	CAT_UPDATE_SORT_ASCENDANT,
	CAT_UPDATE_SORT_DESCENDANT
};

typedef enum CatUpdateSorting_ CatUpdateSorting;




void CatUpdate_Print(CatUpdate* cu);

void CUList_AddAllocatedAndSort(CUList* list, CUListNode* NewNode, CatUpdateSorting us);
int CUList_ForEach(CUList* list, int (*f)(CUListNode*, void*), void* arg);
void CUList_Free(CUList* list);
void CUList_Init(CUList* list);
void CUList_Print(CUList* list);

int CUListNode_Print1(CUListNode* ListNode, void* arg);
int CUListNode_Print2(CUListNode* ListNode, void* arg);
int CUListNode_Print3(CUListNode* ListNode, void* arg);
int CUListNode_StmtExecUpdateCategories1(CUListNode* ListNode, void* arg);
int CUListNode_StmtExecUpdateCategories2(CUListNode* ListNode, void* arg);
int CUListNode_StmtExecUpdateCategories3(CUListNode* ListNode, void* arg);




void CatUpdate_AddAllocatedAndSort(CatUpdate* cu, CUListNode* NewNode)
{
	if (NewNode->id == NewNode->old_id && NewNode->parent_id == NewNode->old_parent_id)
	{
		free(NewNode);
		return ;
	}
	
	if (NewNode->id < NewNode->old_id)  /* ??? */
	{
		NewNode->middle = cu->new_cat_id; ++cu->new_cat_id;
		CUList_AddAllocatedAndSort(&cu->LessThan, NewNode, CAT_UPDATE_SORT_DESCENDANT);
	}
	else
	{
		NewNode->middle = 0;
		CUList_AddAllocatedAndSort(&cu->GreaterThan, NewNode, CAT_UPDATE_SORT_DESCENDANT);
	}
}

void CatUpdate_Execute(CatUpdate* cu)
{
	CatUpdateStmt Stmt = {
		.MySQL = cu->MySQL,
		.stmt = NULL,
		.stmt_update1_str = "UPDATE categories SET id = ?, parent_id = ? WHERE id = ?;\n",
		.stmt_update2_str = "UPDATE operations SET category = ? WHERE category = ?;\n"
/*		.param_update[3] = ...
		.row_id = ...
		.row_parent_id = ...
		.row_old_id ... */
	};
	
	
	Stmt.stmt = mysql_stmt_init(cu->MySQL);
	if (Stmt.stmt == NULL)
	{
		fprintf(stderr, "%s(): mysql_stmt_init() error.\n", __func__);
		goto END;
	}
	
	if (mysql_stmt_prepare(Stmt.stmt, Stmt.stmt_update1_str, strlen(Stmt.stmt_update1_str)))
	{
		fprintf(stderr, "%s(): mysql_stmt_prepare() error.\n", __func__);
		goto END;
	}
	
	memset(Stmt.param_update, 0, sizeof(Stmt.param_update));
	
	Stmt.param_update[0].buffer_type    = MYSQL_TYPE_LONG;
	Stmt.param_update[0].buffer         = (void*) &Stmt.row_id;
	Stmt.param_update[0].is_unsigned    = 1;
	
	Stmt.param_update[1].buffer_type    = MYSQL_TYPE_LONG;
	Stmt.param_update[1].buffer         = (void*) &Stmt.row_parent_id;
	Stmt.param_update[1].is_unsigned    = 1;
	
	Stmt.param_update[2].buffer_type    = MYSQL_TYPE_LONG;
	Stmt.param_update[2].buffer         = (void*) &Stmt.row_old_id;
	Stmt.param_update[2].is_unsigned    = 1;
	
	if (mysql_stmt_bind_param(Stmt.stmt, Stmt.param_update))
	{
		fprintf(stderr, "%s(): mysql_stmt_bind_param() error.\n", __func__);
		goto END;
	}
	
	printf("1 -------- LESS THAN --------\n");
	if (!CUList_ForEach(&cu->LessThan, CUListNode_StmtExecUpdateCategories1, &Stmt))
		goto END;
	
	printf("2 -------- GREATER THAN --------\n");
	if (!CUList_ForEach(&cu->GreaterThan, CUListNode_StmtExecUpdateCategories2, &Stmt))
		goto END;
	
	printf("3 -------- LESS THAN --------\n");
	if (!CUList_ForEach(&cu->LessThan, CUListNode_StmtExecUpdateCategories3, &Stmt))
		goto END;
	
	mysql_stmt_close(Stmt.stmt);    Stmt.stmt = NULL;
	
	Stmt.stmt = mysql_stmt_init(cu->MySQL);
	if (Stmt.stmt == NULL)
	{
		fprintf(stderr, "%s(): mysql_stmt_init() error.\n", __func__);
		goto END;
	}
	
	if (mysql_stmt_prepare(Stmt.stmt, Stmt.stmt_update2_str, strlen(Stmt.stmt_update2_str)))
	{
		fprintf(stderr, "%s(): mysql_stmt_prepare() error.\n", __func__);
		goto END;
	}
	
	memset(Stmt.param_update, 0, sizeof(Stmt.param_update));
	
	Stmt.param_update[0].buffer_type    = MYSQL_TYPE_LONG;
	Stmt.param_update[0].buffer         = (void*) &Stmt.row_id;
	Stmt.param_update[0].is_unsigned    = 1;
	
	Stmt.param_update[1].buffer_type    = MYSQL_TYPE_LONG;
	Stmt.param_update[1].buffer         = (void*) &Stmt.row_old_id;
	Stmt.param_update[1].is_unsigned    = 1;
	
	if (mysql_stmt_bind_param(Stmt.stmt, Stmt.param_update))
	{
		fprintf(stderr, "%s(): mysql_stmt_bind_param() error.\n", __func__);
		goto END;
	}
	
	printf("1 -------- LESS THAN --------\n");
	if (!CUList_ForEach(&cu->LessThan, CUListNode_StmtExecUpdateCategories1, &Stmt))
		goto END;
	
	printf("2 -------- GREATER THAN --------\n");
	if (!CUList_ForEach(&cu->GreaterThan, CUListNode_StmtExecUpdateCategories2, &Stmt))
		goto END;
	
	printf("3 -------- LESS THAN --------\n");
	if (!CUList_ForEach(&cu->LessThan, CUListNode_StmtExecUpdateCategories3, &Stmt))
		goto END;
	
/*	printf("NewCatID: %d\n", cu->new_cat_id);
	printf("1 -------- LESS THAN --------\n");
	CUList_ForEach(&cu->LessThan, CUListNode_Print1, NULL);
	printf("2 -------- GREATER THAN --------\n");
	CUList_ForEach(&cu->GreaterThan, CUListNode_Print2, NULL);
	printf("3 -------- LESS THAN --------\n");
	CUList_ForEach(&cu->LessThan, CUListNode_Print3, NULL); */
	
END:
	if (Stmt.stmt != NULL)
		mysql_stmt_close(Stmt.stmt);
}

void CatUpdate_Free(CatUpdate* cu)
{
	CUList_Free(&cu->LessThan);
	CUList_Free(&cu->GreaterThan);
	
	/* CatUpdate_Init(cu); Необязательно, потомучто CUList_Free автоматически
	                       потом вызывает CUList_Init() */
}

void CatUpdate_Init(CatUpdate* cu)
{
	CUList_Init(&cu->LessThan);
	CUList_Init(&cu->GreaterThan);
}

void CatUpdate_Print(CatUpdate* cu)
{
	CUListNode* CurNode;
	
	printf("LESS THAN:\n");
	CUList_Print(&cu->LessThan);
	printf("GREATER THAN:\n");
	CUList_Print(&cu->GreaterThan);
}

void CUList_AddAllocatedAndSort(CUList* list, CUListNode* NewNode, CatUpdateSorting us)
{
	CUListNode* CurNode;
	CUListNode* PrevNode;
	
	
	if (list->first == NULL)
	{
		NewNode->next = NULL;
		list->first = NewNode;
		list->last = NewNode;
		
		return ;
	}
	
	CurNode = list->first;
	PrevNode = NULL;
	
	for ( ; ; )
	{
		if ((us == CAT_UPDATE_SORT_ASCENDANT && NewNode->id <= CurNode->id)
			|| (us == CAT_UPDATE_SORT_DESCENDANT && NewNode->id >= CurNode->id))
		{
			if (PrevNode == NULL)
			{
				list->first = NewNode;
			}
			else
			{
				PrevNode->next = NewNode;
			}
			NewNode->next = CurNode;
			break;
		}
		
		PrevNode = CurNode;
		CurNode = CurNode->next;
		
		if (CurNode == NULL)
		{
			PrevNode->next = NewNode;
			NewNode->next = NULL;
			list->last = NewNode;
			break;
		}
	}
}

int CUList_ForEach(CUList* list, int (*f)(CUListNode*, void*), void* arg)
{
	CUListNode* CurNode;
	
	
	for (CurNode = list->first; CurNode != NULL; CurNode = CurNode->next)
		if (!f(CurNode, arg))
			return 0;
	return 1;
}

void CUList_Free(CUList* list)
{
	CUListNode* node;
	CUListNode* next;
	
	
	for (node = list->first; node != NULL; )
	{
		next = node->next;
		free(node);
		node = next;
	}
	
	CUList_Init(list);
}

void CUList_Init(CUList* list)
{
	list->first = NULL;
	list->last = NULL;
}

void CUList_Print(CUList* list)
{
	CUListNode* CurNode;
	
	
	for (CurNode = list->first; CurNode != NULL; CurNode = CurNode->next)
	{
		printf("id = %d, parent_id = %d, old_id = %d, old_parent_id = %d, middle = %d\n",
				CurNode->id, CurNode->parent_id, CurNode->old_id, CurNode->old_parent_id, CurNode->middle);
	}
}

CUListNode* CUListNode_Alloc()
{
	CUListNode* NewNode;
	
	
	NewNode = (CUListNode*) malloc(sizeof(CUListNode));
	
	if (NewNode == NULL)
	{
		fprintf(stderr, "NewNode = NULL\n");
	}
	
	return NewNode;
}

int CUListNode_StmtExecUpdateCategories1(CUListNode* ListNode, void* arg)
{
	CatUpdateStmt* Stmt = arg;
	
	
	Stmt->row_id         = ListNode->middle;
	Stmt->row_parent_id  = ListNode->parent_id;
	Stmt->row_old_id     = ListNode->old_id;
	
	CUListNode_Print1(ListNode, NULL);
	
	if (mysql_stmt_execute(Stmt->stmt))
	{
		fprintf(stderr, "%s(): mysql_stmt_execute() error.\n", __func__);
		return 0;
	}
	
	return 1;
}

int CUListNode_StmtExecUpdateCategories2(CUListNode* ListNode, void* arg)
{
	CatUpdateStmt* Stmt = arg;
	
	
	Stmt->row_id         = ListNode->id;
	Stmt->row_parent_id  = ListNode->parent_id;
	Stmt->row_old_id     = ListNode->old_id;
	
	CUListNode_Print2(ListNode, NULL);
	
	if (mysql_stmt_execute(Stmt->stmt))
	{
		fprintf(stderr, "%s(): mysql_stmt_execute() error.\n", __func__);
		return 0;
	}
	
	return 1;
}

int CUListNode_StmtExecUpdateCategories3(CUListNode* ListNode, void* arg)
{
	CatUpdateStmt* Stmt = arg;
	
	
	Stmt->row_id         = ListNode->id;
	Stmt->row_parent_id  = ListNode->parent_id;
	Stmt->row_old_id     = ListNode->middle;
	
	CUListNode_Print3(ListNode, NULL);
	
	if (mysql_stmt_execute(Stmt->stmt))
	{
		fprintf(stderr, "%s(): mysql_stmt_execute() error.\n", __func__);
		return 0;
	}
	
	return 1;
}

int CUListNode_Print1(CUListNode* ListNode, void* arg)
{
	printf("UPDATE categories SET id = %d, parent_id = %d WHERE id = %d;    "
			"(id = %d, parent_id = %d, old_id = %d, old_parent_id = %d, middle = %d).\n",
			ListNode->middle, ListNode->parent_id, ListNode->old_id,
			ListNode->id, ListNode->parent_id, ListNode->old_id, ListNode->old_parent_id, ListNode->middle);
	return 1;
}

int CUListNode_Print2(CUListNode* ListNode, void* arg)
{
	printf("UPDATE categories SET id = %d, parent_id = %d WHERE id = %d;    "
			"(id = %d, parent_id = %d, old_id = %d, old_parent_id = %d, middle = %d).\n",
			ListNode->id, ListNode->parent_id, ListNode->old_id,
			ListNode->id, ListNode->parent_id, ListNode->old_id, ListNode->old_parent_id, ListNode->middle);
	return 1;
}

int CUListNode_Print3(CUListNode* ListNode, void* arg)
{
	printf("UPDATE categories SET id = %d, parent_id = %d WHERE id = %d;    "
			"(id = %d, parent_id = %d, old_id = %d, old_parent_id = %d, middle = %d).\n",
			ListNode->id, ListNode->parent_id, ListNode->middle,
			ListNode->id, ListNode->parent_id, ListNode->old_id, ListNode->old_parent_id, ListNode->middle);
	return 1;
}
