#ifndef _PDD_H_
#define _PDD_H_

fdd_node *build_pdd_tree(const vector<pc_rule> &classifier);
fdd_node *build_fdd_fast(const vector<pc_rule> &classifier, const vector<int> &order);




#endif
