#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include "utils.h"
#include "cat_report_tree.h"




void CatRepOperations_Add(CatRepOperations* operations, CatRepOperation* oper);
void CatRepOperations_Add2(CatRepOperations* operations, CatRepOperation* oper);
void CatRepOperations_Add3(CatRepOperations* opers1, CatRepOperations* opers2);
void CatRepOperations_Free(CatRepOperations* operations);
void CatRepOperations_Init(CatRepOperations* operations);
void CatRepOperations_Print(CatRepOperations* operations, char* Name);
CatRepOperation* CatRepOperations_Unlink(CatRepOperations* operations, CatRepOperation* oper);

unsigned int CatTree_Add(CatTree* ct, unsigned int id, unsigned int parent_id, char* name);
void CatTree_Filter2(CatTree* ct, CatTreeNode* ctn);
unsigned int CatTree_Filter3(CatTree* ct, CatRepOperation* oper);
void CatTree_Filter4(CatTree* ct);
void CatTree_HaveOperationsUpdate(CatTree* ct);
void CatTree_LoadOperations(CatTree* ct);
void CatTree_Percent(CatTree* ct);
void CatTree_Sum(CatTree* ct);

void CatTreeNode_FreeRecursive(CatTreeNode* ctn);
void CatTreeNode_HaveOperationsUpdate(CatTreeNode* ctn);
void CatTreeNode_Percent(CatTreeNode* ctn);
void CatTreeNode_Print(CatTreeNode* ctn, int a);
void CatTreeNode_Print2(CatTreeNode* ctn, int a);
void CatTreeNode_Print3(CatTreeNode* ctn);
void CatTreeNode_Sum(CatTreeNode* ctn);




void CatRepOperations_Add(CatRepOperations* operations, CatRepOperation* oper)
{
	CatRepOperation* NewNode;
	
	
	NewNode = malloc(sizeof(CatRepOperation));
	
	NewNode->id           = oper->id;
	NewNode->Day          = oper->Day;
	NewNode->Month        = oper->Month;
	NewNode->Year         = oper->Year;
	NewNode->Description  = strdup(oper->Description);
	NewNode->Money        = oper->Money;
	NewNode->Category     = oper->Category;
	
	CatRepOperations_Add2(operations, NewNode);
}

void CatRepOperations_Add2(CatRepOperations* operations, CatRepOperation* oper)
{
	oper->next = NULL;
	
	if (operations->First == NULL)
	{
		oper->prev = NULL;
		operations->First = oper;
	}
	else
	{
		oper->prev = operations->Last;
		operations->Last->next = oper;
	}
	
	operations->Last  = oper;
}

void CatRepOperations_Add3(CatRepOperations* opers1, CatRepOperations* opers2)
{
	if (opers1->First == NULL)
	{
		opers1->First = opers2->First;
		opers1->Last = opers2->Last;
		opers2->First = NULL;
		opers2->Last = NULL;
	}
	else
	if (opers2->First != NULL)
	{
		opers1->Last->next = opers2->First;
		opers2->First->prev = opers1->Last;
		opers1->Last = opers2->Last;
		opers2->First = NULL;
		opers2->Last = NULL;
	}
}

void CatRepOperations_Free(CatRepOperations* operations)
{
	CatRepOperation* oper;
	CatRepOperation* next;
	
	
	for (oper = operations->First; oper; oper = next)
	{
		next = oper->next;
		if (oper->Description)
			free(oper->Description);
		free(oper);
	}
	
	CatRepOperations_Init(operations);
}

void CatRepOperations_Init(CatRepOperations* operations)
{
	operations->First = NULL;
	operations->Last = NULL;
}

void CatRepOperations_Print(CatRepOperations* operations, char* Name)
{
	CatRepOperation* oper;
	
	if (Name)
		printf("%s(): %s:\n\n", __func__, Name);
	
	for (oper = operations->First; oper != NULL; oper = oper->next)
	{
		printf("%s(): %04u-%02u-%02u    %u. %s\n",
				__func__,
				oper->Year,
				oper->Month,
				oper->Day,
				oper->id,
				oper->Description);
	}
	
	printf("\n");
}

CatRepOperation* CatRepOperations_Unlink(CatRepOperations* operations, CatRepOperation* oper)
{
	CatRepOperation* prev;
	CatRepOperation* next;
	
	
	if (!operations->First)
		return NULL;
	
	prev = oper->prev;
	next = oper->next;
	
	if (prev == NULL)
		operations->First = next;
	else
	{
		prev->next = next;
		oper->prev = NULL;
	}
	
	if (next == NULL)
		operations->Last = prev;
	else
	{
		next->prev = prev;
		oper->next = NULL;
	}
	
	return next;
}

unsigned int CatTree_Add(CatTree* ct, unsigned int id, unsigned int parent_id, char* name)
{
	CatTreeNode* ctn;
	CatTreeNode* new_node;
	CatTreeNode* parent_node;
	CatTreeNode* prev;
	
	
	new_node = malloc(sizeof(CatTreeNode));
	if (new_node == NULL)
	{
		return 0;
	}
	
	new_node->id        = id;
	new_node->is_sum    = 1;
	new_node->parent_id = parent_id;
	new_node->percent   = 0.0;
	new_node->name      = strdup(name);
	new_node->child     = NULL;
	new_node->mark      = 0;
	
	CatRepOperations_Init(&new_node->opers);
	
	if (parent_id == 0)
	{
		parent_node = NULL;
		new_node->parent  = NULL;
		
		if (ct->First == NULL)
		{
			new_node->next      = NULL;
			ct->First = new_node;
			
			return 1;
		}
		ctn = ct->First;
	}
	else
	{
		parent_node = CatTreeNode_FindByID(ct->First, parent_id);
		if (parent_node == NULL)
		{
			g_print("%s(): fatal error.\n", __func__);
			return 0;
		}
		
		new_node->parent = parent_node;
		
		if (parent_node->child == NULL)
		{
			new_node->next = NULL;
			parent_node->child = new_node;
			
			return 1;
		}
		ctn = parent_node->child;
	}
	
	for (prev = NULL; ; )
	{
		if (ctn->id >= id)
		{
			if (prev == NULL)
			{
				if (parent_id == 0)
					ct->First = new_node;
				else
					parent_node->child = new_node;
			}
			else
				prev->next = new_node;
			new_node->next = ctn;
			
			return 1;
		}
		prev = ctn;
		ctn = ctn->next;
		if (ctn == NULL)
		{
			prev->next = new_node;
			new_node->next = NULL;
			
			return 1;
		}
	}
	
	return 1;
}

void CatTree_Filter(CatTree* ct,
		unsigned int d1, unsigned int m1, unsigned int y1,
		unsigned int d2, unsigned int m2, unsigned int y2)
{
	ct->d1 = d1;
	ct->m1 = m1;
	ct->y1 = y1;
	ct->d2 = d2;
	ct->m2 = m2;
	ct->y2 = y2;
	
	CatTree_Filter2(ct, ct->First);
	CatTree_Filter4(ct);
	CatRepOperations_Add3(&ct->operations, &ct->trash);
	CatTree_HaveOperationsUpdate(ct);
	CatTree_Sum(ct);
	CatTree_Percent(ct);
}

void CatTree_Filter2(CatTree* ct, CatTreeNode* ctn)
{
	CatRepOperation* oper;
	CatRepOperation* next;
	
	
	for ( ; ctn != NULL; ctn = ctn->next)
	{
		for (oper = ctn->opers.First; oper != NULL; )
		{
			if (CatTree_Filter3(ct, oper))
				oper = oper->next;
			else
			{
				next = CatRepOperations_Unlink(&ctn->opers, oper);
				CatRepOperations_Add2(&ct->trash, oper);
				oper = next;
			}
		}
		
		if (ctn->child)
			CatTree_Filter2(ct, ctn->child);
	}
}

unsigned int CatTree_Filter3(CatTree* ct, CatRepOperation* oper)
{
	if (    (
		(ct->y1 < oper->Year) ||
		(ct->y1 == oper->Year && ct->m1 < oper->Month) ||
		(ct->y1 == oper->Year && ct->m1 == oper->Month && ct->d1 <= oper->Day)) &&
		(
		(ct->y2 > oper->Year) ||
		(ct->y2 == oper->Year && ct->m2 > oper->Month) ||
		(ct->y2 == oper->Year && ct->m2 == oper->Month && ct->d2 >= oper->Day)))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

void CatTree_Filter4(CatTree* ct)
{
	CatTreeNode* ctn;
	CatRepOperation* oper;
	CatRepOperation* next;
	
	
	for (oper = ct->operations.First; oper != NULL; )
	{
		if (CatTree_Filter3(ct, oper))
		{
			ctn = CatTreeNode_FindByID(ct->First, oper->Category);
			if (ctn)
			{
				next = CatRepOperations_Unlink(&ct->operations, oper);
				CatRepOperations_Add2(&ctn->opers, oper);
				oper = next;
			}
			else
			{
				oper = oper->next;
			}
		}
		else
		{
			oper = oper->next;
		}
	}
}

void CatTree_HaveOperationsUpdate(CatTree* ct)
{
	CatTreeNode* ctn;
	
	
	for (ctn = ct->First; ctn != NULL; ctn = ctn->next) {
		CatTreeNode_HaveOperationsUpdate(ctn);
	}
}

void CatTree_Close(CatTree* ct)
{
	CatTreeNode_FreeRecursive(ct->First);
	ct->First = NULL;
	CatRepOperations_Free(&ct->operations);
	CatRepOperations_Free(&ct->trash);
}

void CatTreeNode_FreeRecursive(CatTreeNode* ctn)
{
	CatTreeNode* next;
	
	
	for ( ; ctn; ctn = next)
	{
		next = ctn->next;
		if (ctn->child)
			CatTreeNode_FreeRecursive(ctn->child);
		if (ctn->name)
			free(ctn->name);
		CatRepOperations_Free(&ctn->opers);
		free(ctn);
	}
}

void CatTreeNode_HaveOperationsUpdate(CatTreeNode* ctn)
{
	CatTreeNode* child;
	
	
	ctn->have_operations = 0;
	for (child = ctn->child; child != NULL; child = child->next) {
		CatTreeNode_HaveOperationsUpdate(child);
		if (child->have_operations) {
			ctn->have_operations = 1;
		}
	}
	ctn->have_operations = ctn->have_operations || ctn->opers.First;
}

void CatTree_Init(CatTree* ct)
{
	memset(ct, 0, sizeof(*ct));
}

void CatTree_Load(CatTree* ct, MYSQL* MySQL)
{
	MYSQL_RES* res_set = NULL;
	MYSQL_ROW row;
	unsigned int id;
	unsigned int parent_id;
	
	
	ct->MySQL = MySQL;
	CatRepOperations_Init(&ct->operations);
	CatRepOperations_Init(&ct->trash);
	
	if (mysql_query(ct->MySQL, "SELECT id, parent_id, name FROM categories ORDER BY id, parent_id;"))
	{
		fprintf(stderr, "%s(): mysql_query() error: \n", __func__);
		goto END;
	}
	
	res_set = mysql_store_result(ct->MySQL);
	if (res_set == NULL)
	{
		fprintf(stderr, "%s(): mysql_store_result() error.\n", __func__);
		goto END;
	}
	
	while (row = mysql_fetch_row(res_set))
	{
		id = strtol(row[0], NULL, 10);
		parent_id = strtol(row[1], NULL, 10);
		if (!CatTree_Add(ct, id, parent_id, row[2]))
		{
			goto END;
		}
	}
	
	CatTree_LoadOperations(ct);
	
END:
	
	if (res_set != NULL)
		mysql_free_result(res_set);
}

void CatTree_LoadOperations(CatTree* ct)
{
	MYSQL_RES* ResSet = NULL;
	MYSQL_ROW row;
	CatRepOperation oper;
	
	
	if (mysql_query(ct->MySQL,
			"SELECT\n"
			"    id,\n"           /* 00 */
			"    date,\n"         /* 01 */
			"    description,\n"  /* 02 */
			"    money,\n"        /* 03 */
			"    category\n"      /* 04 */
			"FROM operations;"))
	{
		fprintf(stderr, "%s(): mysql_query() error.\n", __func__);
		goto END;
	}
	ResSet = mysql_store_result(ct->MySQL);
	if (ResSet == NULL)
	{
		fprintf(stderr, "%s(): mysql_store_result() error.\n", __func__);
		goto END;
	}
	
	while (row = mysql_fetch_row(ResSet))
	{
		oper.id            = strtol(row[0], NULL, 10);
		DateStrToDMY(row[1], &oper.Year, &oper.Month, &oper.Day);
		oper.Description   = row[2];
		oper.Money         = strtod(row[3], NULL);
		oper.Category      = strtol(row[4], NULL, 10);
		
		CatRepOperations_Add(&ct->operations, &oper);
	}
	
END:
	if (ResSet)
		mysql_free_result(ResSet);
}

void CatTree_Percent(CatTree* ct)
{
	CatTreeNode* ctn;
	
	
	for (ctn = ct->First; ctn != NULL; ctn = ctn->next)
	{
		if (ctn->have_operations)
		{
			CatTreeNode_Percent(ctn);
			if (ctn->is_sum && ctn->sum <= 0.0 && ct->sum <= 0.0)
				ctn->percent = ctn->sum / ct->sum;
			else
				ctn->percent = 0.0;
		}
	}
}

void CatTree_Sum(CatTree* ct)
{
	CatTreeNode* ctn;
	
	
	ct->sum = 0;
	
	for (ctn = ct->First; ctn != NULL; ctn = ctn->next)
	{
		if (ctn->have_operations)
		{
			CatTreeNode_Sum(ctn);
			if (ctn->is_sum)
				ct->sum += ctn->sum;
		}
	}
}

CatTreeNode* CatTreeNode_FindByID(CatTreeNode* tree_node, unsigned int id)
{
	CatTreeNode* tn;
	
	
	for (; tree_node != NULL; tree_node = tree_node->next)
	{
		if (tree_node->id == id)
		{
			return tree_node;
		}
		if (tree_node->child != NULL)
		{
			tn = CatTreeNode_FindByID(tree_node->child, id);
			if (tn != NULL)
			{
				return tn;
			}
		}
	}
	
	return NULL;
}

void CatTreeNode_Percent(CatTreeNode* ctn)
{
	CatTreeNode* child;
	CatRepOperation* oper;
	
	
	for (child = ctn->child; child != NULL; child = child->next)
	{
		if (child->have_operations)
		{
			CatTreeNode_Percent(child);
			if (child->is_sum && child->sum <= 0.0 && ctn->sum <= 0.0)
				child->percent = child->sum / ctn->sum;
			else
				child->percent = 0.0;
		}
	}
}

void CatTreeNode_Print(CatTreeNode* ctn, int a)
{
	int i;
	
	for ( ; ctn != NULL; ctn = ctn->next)
	{
		printf("%10u%10u", ctn->id, ctn->parent_id);
		for (i = 0; i < a; ++i)
			printf("   ");
		printf("    %s\n", ctn->name);
		if (ctn->child)
			CatTreeNode_Print(ctn->child, a+1);
	}
}

void CatTreeNode_Print2(CatTreeNode* ctn, int a)
{
	int i;
	CatRepOperation* oper;
	
	
	for ( ; ctn != NULL; ctn = ctn->next)
	{
		printf("%10u%10u", ctn->id, ctn->parent_id);
		for (i = 0; i < a; ++i)
			printf("   ");
		printf("    %s, ctn->opers.First = 0x%08X, ctn->opers.Last = 0x%08X\n\n",
				ctn->name,
				(unsigned int) ctn->opers.First,
				(unsigned int) ctn->opers.Last);
		for (oper = ctn->opers.First; oper != NULL; oper = oper->next)
		{
			printf("                    ");
			for (i = 0; i < a; ++i)
				printf("   ");
			printf("    %02u.%02u.%04u    \"%s\"\n",
					oper->Day,
					oper->Month,
					oper->Year,
					oper->Description);
		}
		printf("\n");
		if (ctn->child)
			CatTreeNode_Print2(ctn->child, a+1);
	}
}

void CatTreeNode_Print3(CatTreeNode* ctn)
{
	if (ctn->parent) {
		CatTreeNode_Print3(ctn->parent);
		printf("::%s", ctn->name);
	}
	else {
		printf("%s", ctn->name);
	}
}

void CatTreeNode_Sum(CatTreeNode* ctn)
{
	CatTreeNode* child;
	CatRepOperation* oper;
	
	
	ctn->sum = 0;
	
	for (child = ctn->child; child != NULL; child = child->next)
	{
		if (child->have_operations)
		{
			CatTreeNode_Sum(child);
			if (child->is_sum)
				ctn->sum += child->sum;
		}
	}
	
	for (oper = ctn->opers.First; oper != NULL; oper = oper->next)
		ctn->sum += oper->Money;
}
