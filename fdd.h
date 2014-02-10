#ifndef __FDD_H__
#define __FDD_H__

#include <string>
#include <list>
#include <map>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <cmath>
#include <vector>



#ifndef TEST
#include "rulesutils.h"
#endif

using namespace std;

#include "range.h"

#ifdef TEST

struct pc_rule {
    int priority;
    struct range field[2];
};

#define MAXDIMENSIONS 2

#endif


struct fdd_node;

struct edge 
{
    vector<range> rl;
    struct fdd_node *fn;
};

struct recy_node
{
    list<int> nl;
};


struct fdd_node
{
    int field;
    int isLeaf;
    vector<edge> edges;
    vector<int> rulelist;
    vector<int> availdim;
    fdd_node(): field(-1), isLeaf(0) {

    }
};


struct fdd_info{
    int node_cnt;
    int edge_cnt;
    int range_cnt;

    fdd_info():node_cnt(0),edge_cnt(0),range_cnt(0) {

    }
};


fdd_node *build_fdd_tree(list<pc_rule*> p_classifer, vector<fdd_node*>&);

int fdd_match(fdd_node* n, field_type *ft);

void remove_redund_fdd(const list<pc_rule*>& p_classifier,
        const vector<fdd_node*> &tmn_node, int count, 
        list<int>& rule_redund);
void calc_fdd_mem(fdd_node *node, uint64_t &bytes, fdd_info &fi);
void calc_list_mem(uint64_t &bytes, vector<fdd_node*>& tmn_node, vector<recy_node>& recy_vec);
#endif
