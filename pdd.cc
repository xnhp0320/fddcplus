#include "fdd.h"
#include <functional>

using namespace std;
using namespace std::placeholders;

/* PDD is just another way to build fdd trees
 * so we share the most of fdd's data strucuture definitions
 */

typedef unsigned long long ull;
struct point{
     unsigned int x;
     int s;

     point(): x(0), s(0) {

     }

     point(unsigned int x, int s): x(x), s(s) {

     }

     bool operator<(const point &p) const {
         if(this->x < p.x) {
             return true;
         }
         else if( this->x == p.x) {
             //make starting points come first
             if(this->s >0 && p.s < 0) {
                 return true;
             }
             else { 
                 return false;
             }
         }
         //complete equal, will return false
         return false;
     }
};

list<point> build_line_points(const vector<pc_rule> &pc, const vector<int> pcind, int dim) 
{
    list<point> lp;
    vector<range> rangeset;
    for(auto p = pcind.begin(); p != pcind.end(); p++) {
        rangeset.push_back(pc[*p].field[dim]);
    }

    vector<range>::iterator uit;
    sort(rangeset.begin(), rangeset.end());
    uit = unique(rangeset.begin(), rangeset.end());
    rangeset.resize(distance(rangeset.begin(), uit));

    for(auto r = rangeset.begin(); r != rangeset.end(); r++) {
        lp.push_back(point(r->low, 1));
        lp.push_back(point(r->high, -1));
    }
    lp.sort();

    auto prev = lp.begin();
    auto pos = lp.begin();
    pos ++;
    //pos start from 1
    while(pos != lp.end()) {
        if(prev->x == pos->x) {
            if((prev->s>0) == (pos->s>0)) {
                //either they are both start points 
                //or they are both end points
                //we merge these points to 1 point
                //
                //for two intervals [1,2][2,3],the two 2 points 
                //will not be merged. they are different types.
                prev->s += (pos->s >0) ? 1 : -1;
                pos = lp.erase(pos);
                continue;
            }
        }
        prev = pos;
        pos ++;
    }

    return std::move(lp);
}

list<range> build_interval(list<point> &lp)
{
    list<range> lr;
    auto pos = lp.begin();
    pos ++;
    auto prevpos = lp.begin();

    unsigned int start = prevpos->x;
    int stack = prevpos->s;

    for(; pos != lp.end(); pos++) {
        if(pos->s> 0) {
            if(stack != 0) {
                if(start == pos->x && prevpos->s <0) {
                    //we don't need to add an interval here
                    //since the start == pos->x, meaning that 
                    //the [start, pos->x-1] is an invalid 
                    //interval
                }
                else {
                    lr.push_back(range(start, pos->x-1));
                }
            }
            start = pos->x;
            stack += pos->s;
        }
        else {
            lr.push_back(range(start, pos->x));
            start = pos->x+1;
            stack += pos->s;
        }
        prevpos = pos;
    }
    return move(lr);
}

void choose_dim(const vector<pc_rule> &pc, fdd_node* fn, int &dimension, list<range> &lr_choose)
{
    double min_hist = 0xffffffff;
    int dim_to_choose = 0;

    for(auto dim = fn->availdim.begin(); dim!= fn->availdim.end(); dim++) {
        unsigned int hist = 0; 
        list<point> lp = build_line_points(pc, fn->rulelist, *dim);
        list<range> lr = build_interval(lp);
        
        for(auto rit = lr.begin(); rit != lr.end(); rit++) {
            for(auto pcit = fn->rulelist.begin(); pcit != fn->rulelist.end(); pcit++) {
                if(pc[*pcit].field[*dim].low <= rit->low && 
                        pc[*pcit].field[*dim].high >= rit->high) {
                    hist ++;
                }
            }
        }
        //double avg_hist_dim = (double)hist/lr.size();
        if(hist < min_hist) {
            min_hist = hist;
            dim_to_choose = *dim;
            lr_choose = move(lr);
        }
    }
    dimension = dim_to_choose;
}

void make_edges(fdd_node *fn, list<range> &lr, 
        const vector<pc_rule> &pc)
{
    vector<vector<int> > bms;
    bms.resize(lr.size());
    map<vector<int>, vector<range> > equal_set;
    
    int ind = 0;
    for(auto rit = lr.begin(); rit != lr.end(); rit++, ind++) {
        for(auto pcind = fn->rulelist.begin(); pcind != fn->rulelist.end();
                pcind ++) {
            if(pc[*pcind].field[fn->field].low <= rit->low
                    && pc[*pcind].field[fn->field].high >= rit->high) {
                bms[ind].push_back(*pcind);
            }
        }
        equal_set[bms[ind]].push_back(*rit);
    }

    fn->edges.resize(equal_set.size());

    vector<int> tmp = move(fn->availdim);
    tmp.erase(remove_if(tmp.begin(), tmp.end(), 
                bind(equal_to<int>(),  _1, fn->field)), tmp.end()); 
    ind = 0; 
    for(auto set = equal_set.begin();
            set != equal_set.end();
            set++) {
        edge e;
        e.rl = move(set->second);
        e.fn = new fdd_node;
        e.fn->rulelist = move(set->first);
        
        e.fn->availdim = tmp;
        fn->edges[ind] = move(e);
        ind ++;
    }
    //save memory
    fn->rulelist.clear();
}


fdd_node *build_pdd_tree(const vector<pc_rule> &classifier)
{
    fdd_node *fn = new fdd_node;
    for(size_t i = 0; i< classifier.size(); i++) {
        fn->rulelist.push_back(i);
    }
    for(int i = 0; i< MAXDIMENSIONS;i++)
        fn->availdim.push_back(i);

    vector<fdd_node*> thislevel;
    vector<fdd_node*> nextlevel;
    thislevel.push_back(fn);

    for(int level= 0; level< MAXDIMENSIONS; level++) {
        for(auto fnit = thislevel.begin();
                fnit != thislevel.end();
                fnit ++) {
            fdd_node *fn = *fnit;

            int dim;
            list<range> lr;

            choose_dim(classifier, fn, dim, lr); 
            fn->field = dim;
            make_edges(fn, lr, classifier);

            for(auto e = fn->edges.begin(); e != fn->edges.end(); e++) {
                nextlevel.push_back(e->fn);
            }
        }
        cout<<"level "<<level<<" "<<thislevel.size()<<endl;
        thislevel = move(nextlevel);
    }

    for(auto fnit = thislevel.begin(); 
            fnit != thislevel.end();
            fnit ++) {
        (*fnit)->isLeaf = 1;
    }
    cout<<"leaf level "<<thislevel.size()<<endl;
    return fn;
}


