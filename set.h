#ifndef __SET_H_
#define __SET_H_


#include <string>
#include <list>
#include <map>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <cmath>
#include <vector>

#include "range.h"
using namespace std;

bool rset_minus(const range & a, const vector<range> & rl, 
        vector<range> & ret);

bool intersect(const vector<range> &rl, 
        const range &r, vector<range> &ret);

bool setr_minus(vector<range> rl, range &r1, vector<range> &mret);
#endif
