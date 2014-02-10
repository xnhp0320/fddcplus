#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <list>
#include <map>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <cmath>
#include <vector>
#include <arpa/inet.h>
#include <string.h>


#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "fastrr.h"
#include "rulesutils.h"
#include "fdd.h"
#include "pdd.h"

using namespace std;


vector<pc_rule> classifier;
FILE *fpr;
FILE *fpt;
vector<int> order;




void parseargs(int argc, char *argv[])
{
    int	c;
    while ((c = getopt(argc, argv, "o:r:t:")) != -1) {
        switch (c) {
            case 'o':
                int tmp[MAXDIMENSIONS];
                sscanf(optarg,"%1d%1d%1d%1d%1d", tmp,tmp+1,tmp+2,tmp+3,tmp+4); 
                order.assign(tmp, tmp+5);
                break;
            case 'r':
                fpr = fopen(optarg, "r");
                break;
            case 't':
                fpt = fopen(optarg, "r");
                break;
            default:
                break;
        }
    }

    
    if(fpr == NULL){
        printf("can't open ruleset file\n");
        exit(-1);
    }
}



void expand_rule(pc_rule *pr, rule_boundary &rb, unsigned long long epsilon)
{
    for(int i=0; i < MAXDIMENSIONS; i++) {
        if(i<2) {
            if(pr->field[i].low >= epsilon) {
                rb.field[i].low = pr->field[i].low - epsilon;
            }
            else 
                rb.field[i].low = 0;

            if(pr->field[i].high <= 0xFFFFFFFFULL - epsilon) {
                rb.field[i].high = pr->field[i].high + epsilon;
            }
            else 
                rb.field[i].high = 0xFFFFFFFFULL;
        }
        else if (i<4) {
            if(pr->field[i].low >= epsilon) {
                rb.field[i].low = pr->field[i].low - epsilon;
            }
            else 
                rb.field[i].low = 0;

            if(pr->field[i].high <= 0xFFFFULL - epsilon) {
                rb.field[i].high = pr->field[i].high + epsilon;
            }
            else 
                rb.field[i].high = 0xFFFFULL;

        }
        else {
             if(pr->field[i].low >= epsilon) {
                rb.field[i].low = pr->field[i].low - epsilon;
            }
            else 
                rb.field[i].low = 0;

            if(pr->field[i].high <= 0xFFULL - epsilon) {
                rb.field[i].high = pr->field[i].high + epsilon;
            }
            else 
                rb.field[i].high = 0xFFULL;

        }
    }
}

int overlap_filed(range &a, range &b) 
{
    if(a.low > b.high || b.low > a.high) {
        return 0;
    }
    else
        return 1;

}

void search_with_rb(list<pc_rule*>::iterator it, 
        list<pc_rule*> & pc, rule_boundary & exp, 
        list<pc_rule*> & orl) 
{
    int count = 0;
    for(;it != pc.end(); it++) {
        for(int i = 0; i < MAXDIMENSIONS; i++) {
            count += overlap_filed(exp.field[i], (*it)->field[i]);
        }

        if(count == MAXDIMENSIONS) {
            orl.push_back(*it);
        }
        count = 0;
    }

    if(orl.size() > 500) 
        cout<<"orl.size() "<< orl.size()<<endl;
}

void search_overlap(pc_rule *pr, list<pc_rule*> & pc, 
        list<pc_rule*>::iterator it)
{
    list<pc_rule*> overlap_rl;
    rule_boundary exp;
    expand_rule(pr, exp, 0);

    search_with_rb(it, pc, exp, overlap_rl);

}

bool ls_rules(const range & sip, const range & dip) 
{
    double sip_d = (double)(sip.high - sip.low)/0xFFFFFFFFULL;
    double dip_d = (double)(dip.high - dip.low)/0xFFFFFFFFULL;

    if(sip_d > 0.05 && dip_d > 0.05) 
        return true;
    else
        return false;

}

void remove_by_fdd(list<pc_rule*> p_clfr, size_t count) 
{
    vector<fdd_node*> tmn_node;
    list<int> rule_redund;
    fdd_node * root = build_fdd_tree(p_clfr, tmn_node);
    remove_redund_fdd(p_clfr, tmn_node, count, rule_redund);

    for(auto i = rule_redund.begin();
            i != rule_redund.end();
            i++) {
        cout << *i << endl;
    }

    cout<<"allmatch tree remove "<<rule_redund.size()<<endl;
    cout<<endl;
    uint64_t bytes = 0;

    fdd_info fi;

    calc_fdd_mem(root, bytes, fi);
    cout<<"allmatch Tree mem: "<<bytes<<" Bytes "<<bytes/(1024)<<"KB "<<bytes/(1024*1024)<<"MB"<<endl;

    cout<<"allmatch Tree: "<<"node cnt: "
        <<fi.node_cnt<<endl<<"edge cnt: "
        <<fi.edge_cnt<<endl<<"range cnt: "
        <<fi.range_cnt<<endl;
}

void flt_rules(list<pc_rule*> pc, list<pc_rule*> &swws, list<pc_rule*> &ww) 
{
    for_each(pc.begin(), pc.end(), [&swws, &ww](pc_rule *it) {
            if(ls_rules(it->field[0], it->field[1])) {
                ww.push_back(it);
            }
            else {
                swws.push_back(it);
            }
            });
}


int main(int argc, char *argv[]) 
{
    rule_boundary rb;
    list<pc_rule*> p_classifier;

    parseargs(argc, argv);
    loadrules(fpr, classifier);
    load_rule_ptr(classifier, p_classifier, 0, classifier.size() - 1);
    init_boundary(rb); 
   

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    //fdd_node * fn = build_pdd_tree(classifier);
    fdd_node * fn = build_fdd_fast(classifier, order);
    clock_gettime(CLOCK_MONOTONIC, &end);

    unsigned nsec = start.tv_sec == end.tv_sec ? (end.tv_nsec - start.tv_nsec) : (end.tv_nsec - start.tv_nsec + 1e9);
    cout<<"Using time "<<nsec<<"ns "<<nsec/1e9<<"s"<<endl;


    //list<pc_rule*> swws;
    //list<pc_rule*> ww;
    //flt_rules(p_classifier, swws, ww);
    //remove_by_fdd(swws, p_classifier.size());
    //remove_by_fdd(ww,   p_classifier.size());
    //remove_by_fdd(p_classifier, p_classifier.size());


    //for(list<pc_rule*>::iterator it = p_classifier.begin();
    //        it != p_classifier.end();
    //        it ++ ) {
    //   search_overlap(*it, p_classifier, it);
    //}
    //remove_redund(p_classifier, rb);


#if 0 
    fdd_node *root = build_fdd_tree(p_classifier, tmn_node);
    //root = build_fdd_tree(p_classifier, tmn_node);
    int ret;
    int ret2;
    int trace_no = 0;
    int flag = 0;
    field_type ft[MAXDIMENSIONS];
    while((trace_no = load_ft(fpt, ft)) != 0) {
        //if(trace_no == 489)
        //    printf("here\n");
        ret = fdd_match(root, ft);
        ret2 = linear_search(p_classifier, ft);
        if(ret != ret2) {
            printf("ret %d, ret2 %d\n",ret, ret2);
            printf("trace no: %d\n", trace_no);
        }
        if(!flag) 
            flag = 1;
    }

    if(flag == 1) {
        printf("matching finished\n");
    }
#endif

    return 0;
}
