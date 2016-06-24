#include "sql_expr.h"

UCD::SQLExpression* rewrite_rule1(UCD::SQLExpression*);
UCD::SQLExpression* rewrite_rule2(UCD::SQLExpression*);
UCD::SQLExpression* rewrite_rule3(UCD::SQLExpression*);
UCD::SQLExpression* rewrite_rule4(UCD::SQLExpression*);
//UCD::SQLExpression* rewrite_rule_test(UCD::SQLExpression*);

UCD::SQLExpression* rcompile(UCD::SQLExpression *e) {
	int func = e->getFunc();
	UCD::SQLExpression *ep = e;

	if(func == OP_PROJECTION) {
		if(e->getExpression(0)->getFunc() == OP_ITERATE) {
			ep = rewrite_rule1(e);
		} else if(e->getExpression(0)->getFunc() == OP_TABLENAME) {
			ep = rewrite_rule2(e);
		}
	} if(func == OP_SELECTION) {
		if(e->getExpression(0)->getFunc() == OP_ITERATE) {
			ep = rewrite_rule3(e);
		} else if(e->getExpression(0)->getFunc() == OP_TABLENAME) {
			ep = rewrite_rule4(e);
		}
	}

	return ep;
}

UCD::SQLExpression* rewrite_rule_test(UCD::SQLExpression* e) {
	return e;
}

UCD::SQLExpression* rewrite_rule1(UCD::SQLExpression* e) {
	
	UCD::SQLExpression* ep = new UCD::SQLExpression;
	ep->setFunc(OP_ITERATE);
	ep->setCount(2);
	ep->setExpression(0,e->getExpression(0)->getExpression(0));

	UCD::SQLExpression* rightChild = new UCD::SQLExpression;
	rightChild->setFunc(OP_PROJECTROW);
	rightChild->setCount(2);
	rightChild->setExpression(0,e->getExpression(0)->getExpression(1));
	rightChild->setExpression(1,e->getExpression(1));

	ep->setExpression(1,rightChild);

	return ep;
}

UCD::SQLExpression* rewrite_rule2(UCD::SQLExpression* e) {
	
	UCD::SQLExpression* ep = new UCD::SQLExpression;
	ep->setFunc(OP_ITERATE);
	ep->setCount(2);
	ep->setExpression(0,e->getExpression(0));

	UCD::SQLExpression* rightChild = new UCD::SQLExpression;
	rightChild->setFunc(OP_PROJECTROW);
	rightChild->setCount(2);

	UCD::SQLExpression* layer2LeftChild = new UCD::SQLExpression;
	layer2LeftChild->setFunc(OP_GETNEXT);
	layer2LeftChild->setCount(1);
	layer2LeftChild->setExpression(0,e->getExpression(0));
	
	rightChild->setExpression(0,layer2LeftChild);
	rightChild->setExpression(1,e->getExpression(1));

	ep->setExpression(1,rightChild);

	return ep;
}

UCD::SQLExpression* rewrite_rule3(UCD::SQLExpression* e) {
	UCD::SQLExpression* ep = new UCD::SQLExpression;
	ep->setFunc(OP_ITERATE);
	ep->setCount(2);
	ep->setExpression(0,e->getExpression(0)->getExpression(0));

	UCD::SQLExpression* rightChild = new UCD::SQLExpression;
	rightChild->setFunc(OP_SELECTROW);
	rightChild->setCount(2);

	rightChild->setExpression(0,e->getExpression(0)->getExpression(1));
	rightChild->setExpression(1,e->getExpression(1));
	
	ep->setExpression(1,rightChild);

	return ep;
}

UCD::SQLExpression* rewrite_rule4(UCD::SQLExpression* e) {
	UCD::SQLExpression* ep = new UCD::SQLExpression;
	ep->setFunc(OP_ITERATE);
	ep->setCount(2);
	ep->setExpression(0,e->getExpression(0));

	UCD::SQLExpression* rightChild = new UCD::SQLExpression;
	rightChild->setFunc(OP_SELECTROW);
	rightChild->setCount(2);

	UCD::SQLExpression* layer2LeftChild = new UCD::SQLExpression;
	layer2LeftChild->setFunc(OP_GETNEXT);
	layer2LeftChild->setCount(1);
	layer2LeftChild->setExpression(0,e->getExpression(0));
	
	rightChild->setExpression(0,layer2LeftChild);
	rightChild->setExpression(1,e->getExpression(1));

	ep->setExpression(1,rightChild);

	return ep;
}