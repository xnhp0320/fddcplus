#ifndef __RULES_UTIL_H__
#define __RULES_UTIL_H__

#include <string>
#include <list>
#include <map>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <cmath>
#include <vector>
#include "range.h"


#define MAXDIMENSIONS 5


struct rule_boundary
{
    struct range field[MAXDIMENSIONS];
};

struct pc_rule{
  int priority;
  struct range field[MAXDIMENSIONS];
  int siplen, diplen;
  unsigned sip[4], dip[4];
};



using namespace std;

typedef unsigned int field_type;
void remove_redund(list<pc_rule*> &pr, rule_boundary &rb);
int loadrules(FILE *fp, vector<pc_rule> &classifier);

void load_rule_ptr(vector <pc_rule> &rule_list,list <pc_rule*> &ruleptr_list,int start,int end);

void init_boundary(rule_boundary & rb);
void dump_rules(list<pc_rule*> ruleset, string str);

int linear_search(list<pc_rule*> &p_classifier, field_type *ft);
int load_ft(FILE *fpt, field_type *ft);

#endif
