#ifndef CAT_REPORT_TREE_H
#define CAT_REPORT_TREE_H

#include <mysql/mysql.h>
#include <mysql/mysqld_error.h>




struct CatRepOperation_
{
	unsigned int id;
	unsigned int Day;
	unsigned int Month;
	unsigned int Year;
	char* Description;
	double Money;
	unsigned int Category;
	struct CatRepOperation_* next;
	struct CatRepOperation_* prev;
};

typedef struct CatRepOperation_ CatRepOperation;

struct CatRepOperations_
{
	CatRepOperation* First;
	CatRepOperation* Last;
};

typedef struct CatRepOperations_ CatRepOperations;

struct CatTreeNode_
{
	unsigned int have_operations;
	unsigned int id;
	unsigned int is_sum;
	unsigned int mark;
	char* name;
	unsigned int parent_id;
	double percent;
	double sum;
	struct CatTreeNode_* child;
	struct CatTreeNode_* next;
	struct CatTreeNode_* parent;
	CatRepOperations opers;
};

typedef struct CatTreeNode_ CatTreeNode;

struct CatTree_
{
	CatTreeNode* First;
	MYSQL* MySQL;
	CatRepOperations operations;
	double sum;
	CatRepOperations trash;
	unsigned int d1;
	unsigned int m1;
	unsigned int y1;
	unsigned int d2;
	unsigned int m2;
	unsigned int y2;
};

typedef struct CatTree_ CatTree;




void CatTree_Filter(CatTree* ct,
		unsigned int d1, unsigned int m1, unsigned int y1,
		unsigned int d2, unsigned int m2, unsigned int y2);
void CatTree_Close(CatTree* ct);
void CatTree_Init(CatTree* ct);
void CatTree_Load(CatTree* ct, MYSQL* MySQL);

CatTreeNode* CatTreeNode_FindByID(CatTreeNode* tree_node, unsigned int parent_id);




#endif
