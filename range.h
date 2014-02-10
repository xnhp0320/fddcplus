#ifndef __RANGE_H__
#define __RANGE_H__

struct range{
    unsigned long long low;
    unsigned long long high;


    range(unsigned long l, 
            unsigned long h): low(l),high(h) {
    }
    range() {
        low = 0;
        high = 0;
    }

    bool operator == (const range &r2) const {
        return this->low == r2.low && this->high == r2.high; 
    }

    bool operator < (const range &r2) const {
        return this->low < r2.low;
    }
    
};

#endif

