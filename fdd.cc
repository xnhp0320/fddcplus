#include "fdd.h"
#include "set.h"


void get_all_range(vector<edge>& edges, vector<range> & rs)
{
    for(auto it = edges.begin();
            it != edges.end();
            it ++ ) {

        for(auto rit = it->rl.begin();
                rit != it->rl.end();
                rit ++ ) {
            rs.push_back(*rit);
        }
    }
    sort(rs.begin(), rs.end(), [](const range &r1, const range &r2){ return r1.low < r2.low;});
}

void build_path(fdd_node *fn, pc_rule *r, int findex)
{

    if(findex == MAXDIMENSIONS) {
        fn->isLeaf = 1;
        fn->field = findex;
        fn->rulelist.push_back(r->priority);
        return;
    }

    fn->field = findex;
    fn->isLeaf = 0;

    struct edge * ne = new edge;
    struct fdd_node * nfn = new fdd_node;

    ne->rl.push_back(r->field[findex]);
    ne->fn = nfn;

    fn->edges.push_back(*ne);

    build_path(nfn, r, findex + 1);
}

void copy_graph(fdd_node *nn, fdd_node *on) 
{
    nn->field = on->field;
    nn->isLeaf = on->isLeaf;
   

    if(!on->isLeaf) {
        for(auto it = on->edges.begin();
                it != on->edges.end();
                it ++) 
        {
            edge *ne = new edge;
            ne->rl = it->rl;
            fdd_node *node = new fdd_node;
            ne->fn = node;
            nn->edges.push_back(*ne);

            copy_graph(ne->fn, it->fn);
        }
    }
    else {
        for(auto it = on->rulelist.begin();
                it != on->rulelist.end();
                it ++ ) {
            nn->rulelist.push_back(*it);
        }
    }
}

void append(fdd_node *fn, pc_rule *rule, int findex)
{
    if(findex == MAXDIMENSIONS) {
        fn->isLeaf = 1;
        fn->rulelist.push_back(rule->priority);
        return;
    }


    vector<range> ret;
    vector<range> rs;
    get_all_range(fn->edges, rs);
    auto k = fn->edges.size();

    if(rset_minus(rule->field[findex], rs, ret)) {
        struct edge * ne = new edge;
        ne->rl = ret;

        struct fdd_node *nfn = new fdd_node;

        ne->fn = nfn;
        fn->edges.push_back(*ne);

        build_path(nfn, rule, findex+1);

    }


    for(size_t i = 0; i<k;i++) { 
        //edges will be changed since the copy_graph, so we'd better remember the 
        //
        edge *e = &fn->edges[i];
        //if(i == 125) {
            //printf("new\n");
        //}
        
        //ret.clear();
        if(setr_minus(e->rl, 
                    rule->field[findex], ret) == false) {
           append(e->fn, rule, findex+1);
        }
        //else if I(e) join Sm != NULL
        else {
            //ret.clear();
            if(intersect(e->rl, rule->field[findex], ret)) {
                struct edge *ne = new edge;
                ne->rl = ret;
                fdd_node *nfn = new fdd_node;
                ne->fn = nfn;
                copy_graph(nfn, e->fn);

                vector<range> mret;
                setr_minus(e->rl,rule->field[findex], mret);
                if(mret.empty()) {
                    printf("error\n");
                }

                fn->edges.push_back(*ne);
                e = &fn->edges[i];
                e->rl = mret;

                append(ne->fn, rule, findex + 1);
            }
        }
    }
}

void get_tmn_list(fdd_node* node, vector<fdd_node*> & tmn_node)
{
    if(node->isLeaf) {
        tmn_node.push_back(node);
    }

    for_each(node->edges.begin(), node->edges.end(),[&tmn_node](const edge &e){
            get_tmn_list(e.fn, tmn_node);
    });

}


void change_order(pc_rule *r) 
{
    pc_rule orule;
    orule.field[0] = (r)->field[4];
    orule.field[1] = (r)->field[0];
    orule.field[2] = (r)->field[1];
    orule.field[3] = (r)->field[2];
    orule.field[4] = (r)->field[3];
    orule.priority = (r)->priority;
    *r = orule;
}

fdd_node *build_fdd_tree(list<pc_rule*> p_classifier, vector<fdd_node*>& tmn_node)
{

    fdd_node *fn = new fdd_node;
    int i = 1;

    for(auto l = p_classifier.begin() ;l != p_classifier.end();
            l ++ ) {
        //if(i == 302)
        //    printf("here\n");
        //if(filter_rules((*l)->field[0], (*l)->field[1])) 
        {
            change_order(*l);

            if(i==1) {
                build_path(fn, *l, 0);
            }
            else 
                append(fn, *l, 0);

            i++;
        }
    }

    get_tmn_list(fn, tmn_node);
    return fn;
}

#define NODE_SIZE 1
#define POINTER_SIZE 4
#define RULE_INDEX 2
#define RANGE_SIZE 8 


void calc_fdd_mem(fdd_node *node, uint64_t &bytes, struct fdd_info & fi)
{
    //for this node 
    bytes += node->edges.size() * POINTER_SIZE + NODE_SIZE;
    fi.node_cnt ++;

    if(node->isLeaf) {
        bytes += node-> rulelist.size() * RULE_INDEX;
        return;
    }

    fi.edge_cnt += node->edges.size();
    for(auto it = node->edges.begin();
            it != node->edges.end();
            it++ ) {
        bytes += it->rl.size() * RANGE_SIZE;
        fi.range_cnt += it->rl.size();
        calc_fdd_mem(it->fn, bytes, fi);
    } 

}

void calc_list_mem(uint64_t &bytes, const vector<fdd_node*>& tmn_node, const vector<recy_node>& recy_vec)
{
    cout<<"TMN nodes "<<tmn_node.size()<<endl;
    bytes += tmn_node.size() * POINTER_SIZE;
    cout<<"recy nodes "<<recy_vec.size()<<endl;
    bytes += recy_vec.size() * POINTER_SIZE;

    for_each(recy_vec.begin(), recy_vec.end(), [&bytes](const recy_node& e) {
            bytes += e.nl.size() * POINTER_SIZE;
    });
}

void build_recidency_vec(const vector<fdd_node*> &tmn_node, 
        vector<recy_node> & recy_vec) 
{
    for(auto n = tmn_node.begin();
            n != tmn_node.end();
            n ++ ) {
        for(auto i = (*n)->rulelist.begin();
                i != (*n)->rulelist.end();
                i++ ) {
            int node_index = n - tmn_node.begin();
            recy_vec[*i].nl.push_back(node_index);
        }
    }
}

void remove_redund_fdd(const list<pc_rule*>& p_classifier,
        const vector<fdd_node*> &tmn_node, int count,
        list<int>& rule_redund) 
{
    vector<recy_node> recy_vec;
    recy_vec.resize(count);

    build_recidency_vec(tmn_node, recy_vec);


    uint64_t lbytes = 0;
    calc_list_mem(lbytes, tmn_node, recy_vec);
    cout<<"allmatch Tree: list mem: "<<lbytes<<" Bytes "<<lbytes/(1024)<<"KB "<<lbytes/(1024*1024)<<"MB"<<endl;
    //map<int,int> hidelist;


    for(auto n = recy_vec.begin();
            n != recy_vec.end();
            n ++ ) {
        int redund = 1;

        if(n->nl.empty()) {
            redund = 0;
        }
        for (auto l = n->nl.begin();
                l != n->nl.end();
                l ++ ) {

            int head = *((tmn_node[*l])->rulelist.begin());
            if (head == n - recy_vec.begin()) {
                redund = 0;
                //hidelist.clear();
                break;
            }
            //else {
            //    hidelist[head]++;
            //}
        }
        if(redund){ 
            rule_redund.push_back((int)(n - recy_vec.begin()));
            //cout<<n-recy_vec.begin()<<" hide by";
            //for_each(hidelist.begin(), hidelist.end(), [](pair<int,int> i){cout<<i.first<<" ";});
            //cout<<endl;
        }
    } 

}



bool range_in(const range &r, field_type ft) 
{
    if(ft >= r.low && ft<= r.high) {
        return true;
    }
    return false;
}

bool is_in(vector<edge>::iterator e, field_type ft) 
{
    for(auto r = e->rl.begin();
            r != e->rl.end();
            r ++ ) {
        if(range_in(*r, ft)) {
            return true;
        }
    }
    return false;
}
int fdd_match(fdd_node* n, field_type *ft)
{
    int go = 0;

    while(!n->isLeaf) {
        go = 0;
        for(auto e = n->edges.begin();
                e != n->edges.end();
                e ++ ) {
            if(is_in(e, ft[n->field])) {
                n = e->fn;
                go = 1;
                break;
            }
        }

        if(go == 0) {
            return -1;
        }
    } 

    return *(n->rulelist.begin());
}

#ifdef TEST

int main() 
{
    list<pc_rule*> pc;

    pc_rule *r1 =  new pc_rule;
    r1->field[0].low = 1;
    r1->field[0].high = 5;

    r1->field[1].low = 1;
    r1->field[1].high = 10;
    r1->priority = 1;
    pc.push_back(r1);
    
    pc_rule *r2 =  new pc_rule;
    r2->field[0].low = 1;
    r2->field[0].high = 5;

    r2->field[1].low = 5;
    r2->field[1].high = 10;
    r2->priority = 2;
    pc.push_back(r2);


    pc_rule *r3 =  new pc_rule;
    r3->field[0].low = 6;
    r3->field[0].high = 10;

    r3->field[1].low = 1;
    r3->field[1].high = 3;
    r3->priority = 3;
    pc.push_back(r3);

    pc_rule *r4 =  new pc_rule;
    r4->field[0].low = 1;
    r4->field[0].high = 10;

    r4->field[1].low = 1;
    r4->field[1].high = 10;
    r4->priority = 4;
    pc.push_back(r4);

    build_fdd_tree(pc);





}


#endif


