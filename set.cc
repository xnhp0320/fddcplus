#include "set.h"



struct point 
{
    unsigned long long n;
    int color;
};


void get_points(const range & r, 
        vector<point> & ps) 
{
    point p;
    p.n = r.low;
    p.color = 0;
    ps.push_back(p);

    p.n = r.high;
    p.color = 1;
    ps.push_back(p);
}

#if 0 
void get_all_points(const vector<range> & rl, 
        vector<point> & ps) 
{
    point p;
    for(auto it = rl.begin();
            it != rl.end();
            it++ ) {

        p.n = it->low;
        p.color = 0;
        ps.push_back(p);

        p.n = it->high;
        p.color = 1;
        ps.push_back(p);
    }
}
bool forwardlh(const vector<point> & ps, int li, int hi, vector<range> &ret) 
{
    int index = li;
    bool flag = false;

    while(index<=hi) {
        if(ps[index].color == 0) {
            index ++;
        }

        //it's high point
        if(ps[index].color == 1) {
            int next = index + 1;
            if(next > hi){
                break;
            }
            else {
                if(ps[next].n - ps[index].n > 1) {
                    range nr;
                    nr.low = ps[index].n + 1;
                    nr.high = ps[next].n - 1;
                    ret.push_back(nr);
                    flag = true;
                }
                index ++;
            }
        }
    }

    return flag;
}


bool rset_minus(const range & a, const vector<range> & rl, 
        vector<range> & ret) 
{
    vector<point> ps;
    get_all_points(rl, ps);
    bool flag = false;
    //assume all points in rl.range are non-overlapping and sorted.

    auto l = find_if(ps.begin(), ps.end(), [&a] (point p) { return p.n >= a.low; });
    auto h = find_if(ps.rbegin(), ps.rend(), [&a] (point &p) { return a.high >= p.n; });


    if (l == ps.end()) {
        ret.push_back(a);
        return true;
    }

    if ( h == ps.rend()) {
        ret.push_back(a);
        return true;
    }

    //so the left is a.low -> l + l -> h + h->a.high
    
    int li = l - ps.begin();
    int hi = ps.size() - (h - ps.rbegin() + 1);

    if(hi<li){
        //if hi < li, there is no points inside a.low to a.high
        if(ps[hi].color == 0 && ps[li].color == 1) {
            //ret.push_back(a); 
            return false;
        }
        ret.push_back(a); 
        return true;
    }


    if(l->color == 0) {
        //it's a low point
        if(l->n == a.low) {
            flag = forwardlh(ps, li, hi, ret);
        }
        else {
            range nr;
            nr.low = a.low;
            nr.high = l->n - 1;
            ret.push_back(nr);
            flag = true;
            forwardlh(ps, li, hi, ret);
        }
    }
    else {
        //it's a high point
        flag = forwardlh(ps, li, hi, ret);
    }

    // h -> a.high
    // if it is a low point 
    // then it has been added by forwardlh
    if(h->color == 1) {
        if(a.high > h->n) {
            range nr;
            nr.low = h->n + 1;
            nr.high = a.high;
            ret.push_back(nr);
            flag = true;
        }
    }
    return flag;

}
#endif
#if 1
bool rset_minus(const range & a_, const vector<range> & rl,
        vector<range> & ret)
{
    ret.clear();
    range a = a_;
    for( size_t i = 0 ; i < rl.size() && a.low <= a.high; ++i ) {
        if( a.low < rl[i].low ) 
            ret.push_back( range(a.low, min(rl[i].low-1, a.high)) );
        a.low = max( a.low , rl[i].high + 1 );
    }

    if(a.low <= a.high) 
        ret.push_back(a);

    return ret.size() != 0 ;
}
#endif


bool range_insct(const range &r1, const range &r2, range &rret)
{
    vector<point> ps;
    get_points(r1, ps);
    get_points(r2, ps);

    sort(ps.begin(), ps.end(), [](point p1, point p2){return p1.n < p2.n;});

    if((ps[1].color == 0 && ps[2].color == 1) || ps[1].n == ps[2].n) {
        rret.low = ps[1].n;
        rret.high = ps[2].n;
        return true;
    }
    return false;
}

bool intersect(const vector<range> &rl, 
        const range &r, vector<range> &ret)
{
    ret.clear();
    for(auto rit = rl.begin();
            rit != rl.end();
            rit ++ ) {
        range rret;
        if(range_insct(*rit, r, rret)) {
            ret.push_back(rret);
        }
    }
    if(!ret.empty())
        return true;
    else
        return false;
}

bool range_minus(const range &r1, const range &r2, vector<range> &ret)
{
    range r=r1;
    if(r.low < r2.low) ret.push_back(range(r.low, min(r2.low -1, r.high)));
    r.low = max(r.low, r2.high+1);

    if(r.high >= r.low) {
        ret.push_back(r);
    }

    return !ret.empty();
}

bool setr_minus(vector<range> rl, range &r1, vector<range> &mret)
{
    mret.clear();
    for(auto l = rl.begin();
            l != rl.end();
            l ++ ) {
        range_minus(*l, r1, mret);
    }

    return !mret.empty();


}

void prange(const range &r1) 
{
    cout << r1.low <<" " << r1.high<<endl;
}

void pvrange(const vector<range> &r1) 
{
    for(auto rit = r1.begin();
            rit != r1.end();
            rit++) {
        prange(*rit);
    }
}


#ifdef TEST

int main()
{
    //test rset_minus
    range r1, r;
    vector<range> vr;
    vector<range> ret;

    r1.low = 1; 
    r1.high = 14;

    r.low = 9;
    r.high = 10;
    //range_minus(r1, r, ret);
    vr.push_back(r);
    
    r.low = 14; 
    r.high = 17;
    vr.push_back(r);

    rset_minus(r1, vr, ret); 
    //intersect(vr, r1, ret); 
    //setr_minus(vr, r1, ret);
    pvrange(ret);

}


#endif

