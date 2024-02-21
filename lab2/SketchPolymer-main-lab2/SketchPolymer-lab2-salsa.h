#ifndef _SP_H_
#define _SP_H_

#include <bits/stdc++.h>

template<typename ID_TYPE>
class Stage1 {
public:
    Stage1() {}
    Stage1(uint32_t memory, uint32_t dim): d(dim) {
        m = memory * 1024 / sizeof(uint8_t) / d;
        counter = new uint8_t* [d];
        for (int i = 0; i < d; ++i) {
            counter[i] = new uint8_t [m];
            memset(counter[i], 0, m * sizeof(uint8_t));
        }
    }
    ~Stage1() {
        for (int i = 0; i < d; ++i) {
            delete[] counter[i];
        }
        delete[] counter;
    }
    void insert(ID_TYPE id) {
        for (int i = 0; i < d; ++i) {
            uint32_t index = hash(id, i) % m;
            if(counter[i][index]+1)
                counter[i][index]++;
        }
    }
    uint32_t query(ID_TYPE id) {
        uint32_t frequency = std::numeric_limits<uint32_t>::max();
        for (int i = 0; i < d; ++i) {
            uint32_t index = hash(id, i) % m;
            frequency = MIN(frequency, counter[i][index]);
        }
        return frequency;
    }
private:
    uint32_t d;
    uint32_t m;
    uint8_t** counter;
};

template<typename ID_TYPE>
class Stage2 {
public:
    Stage2() {}
    Stage2(uint32_t memory, uint32_t dim): d(dim) {
        uint32_t m = memory * 1024 / sizeof(uint32_t) / d;
        merge_lookup_num = m/41;
        cell_num = merge_lookup_num * 8;
        count_num = cell_num * 4;
        frequency = new uint32_t* [d];
        value = new uint32_t* [d];
        merge_lookup = new uint32_t* [d];
        for (int i = 0; i < d; ++i) {
            frequency[i] = new uint32_t [cell_num];
            value[i] = new uint32_t [count_num];
            merge_lookup[i] = new uint32_t [merge_lookup_num];
            memset(frequency[i], 0, cell_num * sizeof(uint32_t));
            memset(value[i], 0, count_num * sizeof(uint32_t));
            memset(merge_lookup[i], 0, merge_lookup_num * sizeof(uint32_t));
        }
    }
    ~Stage2() {
        for (int i = 0; i < d; ++i) {
            delete[] frequency[i];
            delete[] value[i];
            delete[] merge_lookup[i];
        }
        delete[] frequency;
        delete[] value;
        delete[] merge_lookup;
    }
	inline uint32_t get(uint32_t **C, int i, int j, int s, int e) {
		return (C[i][j] & (((1 << (e - s)) - 1) << s)) >> s;
	}
	void increment(uint32_t **C, int i, int j, int s, int e) {
		uint32_t result = get(C, i, j, s, e);
		C[i][j] = ((C[i][j] & (~(((1 << (e - s)) - 1) << s))) | ((result + 1) << s));
		uint32_t now = get(C, i, j, s, e);
		assert(result + 1 == now);
	}
    inline void put(uint32_t **C, int i, int j, int s, int e,int src)
    {
        src &= ((1 << (e - s)) - 1);
        C[i][j] = ((C[i][j] & (~(((1 << (e - s)) - 1) << s))) | ((src) << s));
    } 
    void insert(ID_TYPE id, uint32_t t) {
        for (int i = 0; i < d; ++i) {
            uint32_t index = hash(id, 33 + i) % count_num;
            int s = index - index%4;
            uint32_t look_up = get(merge_lookup,i,s/32,s%32,(s%32)+4);
            if(look_up & 0x4)
            {
                assert(frequency[i][index/4]<0xffffffff);
                frequency[i][index/4]++;
                value[i][s+1] = MAX(value[i][s+1] , t);
            }else{
                if(index%4<=1)
                {
                    if(look_up & 0x2)
                    {
                        uint32_t temp = get(frequency, i, index/4, 0, 16);
                        if(temp == 0xffff)
                        {
                            if(look_up & 0x8==0)
                            {
                                uint32_t a,b;
                                a = get(frequency, i, index/4, 16, 24);
                                b = get(frequency, i, index/4, 24, 32);
                                put(frequency, i, index/4, 16, 32, a+b);
                                look_up |= 0x8;
                                value[i][s] = MAX(value[i][s+1] , value[i][s]);
                            }
                            uint32_t a,b;
                            a = get(frequency, i, index/4, 16, 32);
                            b = 0xffff;
                            put(frequency, i, index/4, 0, 32, a+b+1);
                            value[i][s+1] = MAX(value[i][s] , value[i][s+2]);
                            value[i][s+1] = MAX(value[i][s+1] , t);
                            look_up |= 0x4;
                        }else{
                            increment(frequency, i, index/4, 0, 16);
                            value[i][s+2] = MAX(value[i][s+2] , t);
                        }
                    }else{
                        if((index%4 == 0 && get(frequency, i, index/4, 0, 8)==0xff) || ((index%4 == 1) && get(frequency, i, index/4, 8, 16)==0xff))
                        {
                            uint32_t a,b;
                            a = get(frequency, i, index/4, 0, 8);
                            b = get(frequency, i, index/4, 8, 16);
                            put(frequency, i, index/4, 0, 16, a+b+1);
                            value[i][s+2] = MAX(value[i][s+2] , value[i][s+3]);
                            value[i][s+2] = MAX(value[i][s+2] , t);
                            look_up |= 0x2;
                        }else if(index%4 == 0)
                        {
                            increment(frequency, i, index/4, 0, 8);
                            value[i][s+3] = MAX(value[i][s+3] , t);
                        }else if(index%4 == 1)
                        {
                            increment(frequency, i, index/4, 8, 16);
                            value[i][s+2] = MAX(value[i][s+2] , t);
                        }
                    }
                }else if(index%4>=2)
                {
                    if(look_up & 0x8)
                    {
                        uint32_t temp = get(frequency, i, index/4, 16, 32);
                        if(temp == 0xffff)
                        {
                            if(look_up & 0x2==0)
                            {
                                uint32_t a,b;
                                a = get(frequency, i, index/4, 0, 8);
                                b = get(frequency, i, index/4, 8, 16);
                                put(frequency, i, index/4, 0, 16, a+b);
                                look_up |= 0x2;
                                value[i][s+2] = MAX(value[i][s+2] , value[i][s+3]);
                            }
                            uint32_t a,b;
                            a = get(frequency, i, index/4, 0, 16);
                            b = 0xffff;
                            put(frequency, i, index/4, 0, 32, a+b+1);
                            value[i][s+1] = MAX(value[i][s] , value[i][s+2]);
                            value[i][s+1] = MAX(value[i][s+1] , t);
                            look_up |= 0x4;
                        }else{
                            increment(frequency, i, index/4, 16, 32);
                            value[i][s] = MAX(value[i][s] , t);
                        }
                    }else{
                        if((index%4 == 2 && get(frequency, i, index/4, 16, 24)==0xff) || ((index%4 == 3) && get(frequency, i, index/4, 24, 32)==0xff))
                        {
                            uint32_t a,b;
                            a = get(frequency, i, index/4, 16, 24);
                            b = get(frequency, i, index/4, 24, 32);
                            put(frequency, i, index/4, 16, 32, a+b);
                            value[i][s] = MAX(value[i][s] , value[i][s+1]);
                            value[i][s] = MAX(value[i][s] , t);
                            look_up |= 0x8;
                        }else if(index%4 == 2)
                        {
                            increment(frequency, i, index/4, 16, 24);
                            value[i][s+1] = MAX(value[i][s+1] , t);
                        }else if(index%4 == 3)
                        {
                            increment(frequency, i, index/4, 24, 32);
                            value[i][s] = MAX(value[i][s] , t);
                        }
                    }
                }
                put(merge_lookup, i, s/32, s%32, (s%32)+4, look_up);
            }
        }
    }
    void query(ID_TYPE id, uint32_t* f, uint32_t* t) {
        *f = std::numeric_limits<uint32_t>::max();
        *t = std::numeric_limits<uint32_t>::max();
        for (int i = 0; i < d; ++i) {
            uint32_t index = hash(id, 33 + i) % count_num;
            int s = index - index%4;
            uint32_t look_up = get(merge_lookup,i,s/32,s%32,(s%32)+4);
            if(look_up & 0x4)
            {
                *f = MIN(*f, frequency[i][index/4]);
                *t = MIN(*t, value[i][s+1]);
            }else if(index%4<=1){
                if(look_up & 0x2)
                {
                    *f = MIN(*f, get(frequency, i, index/4, 0, 16));
                    *t = MIN(*t, value[i][s+2]);
                }else{
                    if(index%4==0){
                        *f = MIN(*f, get(frequency, i, index/4, 0, 8));
                        *t = MIN(*t, value[i][s+3]);
                    }
                    else if(index%4==1){
                        *f = MIN(*f, get(frequency, i, index/4, 8, 16));
                        *t = MIN(*t, value[i][s+2]);
                    }
                }
            }else if(index%4>=2)
            {
                if(look_up & 0x8)
                {
                    *f = MIN(*f, get(frequency, i, index/4, 16, 32));
                    *t = MIN(*t, value[i][s]);
                }else{
                    if(index%4==2){
                        *f = MIN(*f, get(frequency, i, index/4, 16, 24));
                        *t = MIN(*t, value[i][s+1]);
                    }
                    else if(index%4==3){
                        *f = MIN(*f, get(frequency, i, index/4, 24, 32));
                        *t = MIN(*t, value[i][s]);
                    }
                }
            }
        }
        return;
    }
private:
    uint32_t d;
    uint32_t cell_num;
    uint32_t count_num;
    uint32_t merge_lookup_num;
    uint32_t** frequency;
    uint32_t** value;
    uint32_t** merge_lookup;
};

template<typename ID_TYPE>
class Stage3 {
public:
    Stage3() {}
    Stage3(uint32_t memory, uint32_t dim): d(dim) {
        uint32_t m = memory * 1024 / sizeof(uint32_t) / d;
        merge_lookup_num = m/9;
        cell_num = merge_lookup_num * 8;
        count_num = cell_num * 4;
        counter = new uint32_t* [d];
        merge_lookup = new uint32_t* [d];
        for (int i = 0; i < d; ++i) {
            counter[i] = new uint32_t [cell_num];
            merge_lookup[i] = new uint32_t [merge_lookup_num];
            memset(counter[i], 0, cell_num * sizeof(uint32_t));
            memset(merge_lookup[i], 0, merge_lookup_num * sizeof(uint32_t));
        }
    }
    ~Stage3() {
        for (int i = 0; i < d; ++i) {
            delete[] counter[i];
            delete[] merge_lookup[i];
        }
        delete[] counter;
        delete[] merge_lookup;
    }
	inline uint32_t get(uint32_t **C, int i, int j, int s, int e) {
		// get the number in bit s-e in counter[i][j]
		return (C[i][j] & (((1 << (e - s)) - 1) << s)) >> s;
	}
	void increment(uint32_t **C, int i, int j, int s, int e) {
		// increment bit s-e in counter[i][j]
		uint32_t result = get(C, i, j, s, e);
		C[i][j] = ((C[i][j] & (~(((1 << (e - s)) - 1) << s))) | ((result + 1) << s));
		uint32_t now = get(C, i, j, s, e);
		assert(result + 1 == now);
	}
    inline void put(uint32_t **C, int i, int j, int s, int e,int src)
    {
        src &= ((1 << (e - s)) - 1);
        C[i][j] = ((C[i][j] & (~(((1 << (e - s)) - 1) << s))) | ((src) << s));
    } 
    void insert(ID_TYPE id, uint32_t t) {
        for (int i = 0; i < d; ++i) {
            uint32_t index = hash(id * t, i) % count_num;
            int s = index - index%4;
            uint32_t look_up = get(merge_lookup,i,s/32,s%32,(s%32)+4);
            if(look_up & 0x4)
            {
                assert(counter[i][index/4]<0xffffffff);
                counter[i][index/4]++;
            }else{
                if(index%4<=1)
                {
                    if(look_up & 0x2)
                    {
                        uint32_t temp = get(counter, i, index/4, 0, 16);
                        if(temp == 0xffff)
                        {
                            if(look_up & 0x8==0)
                            {
                                uint32_t a,b;
                                a = get(counter, i, index/4, 16, 24);
                                b = get(counter, i, index/4, 24, 32);
                                put(counter, i, index/4, 16, 32, a+b);
                                look_up |= 0x8;
                            }
                            uint32_t a,b;
                            a = get(counter, i, index/4, 16, 32);
                            b = 0xffff;
                            put(counter, i, index/4, 0, 32, a+b+1);
                            look_up |= 0x4;
                        }else{
                            increment(counter, i, index/4, 0, 16);
                        }
                    }else{
                        if((index%4 == 0 && get(counter, i, index/4, 0, 8)==0xff) || ((index%4 == 1) && get(counter, i, index/4, 8, 16)==0xff))
                        {
                            uint32_t a,b;
                            a = get(counter, i, index/4, 0, 8);
                            b = get(counter, i, index/4, 8, 16);
                            put(counter, i, index/4, 0, 16, a+b+1);
                            look_up |= 0x2;
                        }else if(index%4 == 0)
                        {
                            increment(counter, i, index/4, 0, 8);
                        }else if(index%4 == 1)
                        {
                            increment(counter, i, index/4, 8, 16);
                        }
                    }
                }else if(index%4>=2)
                {
                    if(look_up & 0x8)
                    {
                        uint32_t temp = get(counter, i, index/4, 16, 32);
                        if(temp == 0xffff)
                        {
                            if(look_up & 0x2==0)
                            {
                                uint32_t a,b;
                                a = get(counter, i, index/4, 0, 8);
                                b = get(counter, i, index/4, 8, 16);
                                put(counter, i, index/4, 0, 16, a+b);
                                look_up |= 0x2;
                            }
                            uint32_t a,b;
                            a = get(counter, i, index/4, 0, 16);
                            b = 0xffff;
                            put(counter, i, index/4, 0, 32, a+b+1);
                            look_up |= 0x4;
                        }else{
                            increment(counter, i, index/4, 16, 32);
                        }
                    }else{
                        if((index%4 == 2 && get(counter, i, index/4, 16, 24)==0xff) || ((index%4 == 3) && get(counter, i, index/4, 24, 32)==0xff))
                        {
                            uint32_t a,b;
                            a = get(counter, i, index/4, 16, 24);
                            b = get(counter, i, index/4, 24, 32);
                            put(counter, i, index/4, 16, 32, a+b+1);
                            look_up |= 0x8;
                        }else if(index%4 == 2)
                        {
                            increment(counter, i, index/4, 16, 24);
                        }else if(index%4 == 3)
                        {
                            increment(counter, i, index/4, 24, 32);
                        }
                    }
                }
                put(merge_lookup, i, s/32, s%32, (s%32)+4, look_up);
            }
        }
    }
    uint32_t query(ID_TYPE id, uint32_t t) {
        uint32_t frequency = 1 << 30;
        for (int i = 0; i < d; ++i) {
            uint32_t index = hash(id * t, i) % count_num;
            int s = index - index%4;
            uint32_t look_up = get(merge_lookup,i,s/32,s%32,(s%32)+4);
            if(look_up & 0x4)
            {
                frequency = MIN(frequency, counter[i][index/4]);
            }else if(index%4<=1){
                if(look_up & 0x2)
                {
                    frequency = MIN(frequency, get(counter, i, index/4, 0, 16));
                }else{
                    if(index%4==0) frequency = MIN(frequency, get(counter, i, index/4, 0, 8));
                    else if(index%4==1) frequency = MIN(frequency, get(counter, i, index/4, 8, 16));
                }
            }else if(index%4>=2)
            {
                if(look_up & 0x8)
                {
                    frequency = MIN(frequency, get(counter, i, index/4, 16, 32));
                }else{
                    if(index%4==2) frequency = MIN(frequency, get(counter, i, index/4, 16, 24));
                    else if(index%4==3) frequency = MIN(frequency, get(counter, i, index/4, 24, 32));
                }
            }
        }
        return frequency;
    }
private:
    uint32_t d;
    uint32_t cell_num;
    uint32_t count_num;
    uint32_t merge_lookup_num;
    uint32_t** counter;
    uint32_t** merge_lookup;    
};


template<typename ID_TYPE>
class Stage4 {
public:
    Stage4() {}
    Stage4(uint32_t memory, uint32_t dim): d(dim) {
        m = memory * 1024 / d;
        counter = new bool* [d];
        for (int i = 0; i < d; ++i) {
            counter[i] = new bool [m];
            memset(counter[i], false, m * sizeof(bool));
        }
    }
    ~Stage4(){
        for (int i = 0; i < d; ++i) {
            delete[] counter[i];
        }
        delete[] counter;
    }
    void insert(ID_TYPE id, uint32_t t) {
        for (int i = 0; i < d; ++i) {
            uint32_t index = hash(id - t, 23 + t) % m;
            counter[i][index] = true;
        }
    }
    bool query(ID_TYPE id, uint32_t t) {
        for (int i = 0; i < d; ++i) {
            uint32_t index = hash(id - t, 23 + t) % m;
            if (!counter[i][index])
                return false;
        }
        return true;
    }
private:
    uint32_t d;
    uint32_t m;
    bool** counter;
};

template<typename ID_TYPE, typename DATA_TYPE>
class SketchPolymer {
public:
    SketchPolymer() {}
    SketchPolymer(uint32_t memory){
        threshold = 50;
        log_base = 1.5;
        stage1 = new Stage1<ID_TYPE>(0.2 * memory, 3);
        stage2 = new Stage2<ID_TYPE>(0.3 * memory, 5);
        stage3 = new Stage3<ID_TYPE>(0.4 * memory, 3);
        stage4 = new Stage4<ID_TYPE>(0.1 * memory, 2);
    }
    ~SketchPolymer()
    {
        delete stage1;
        delete stage2;
        delete stage3;
        delete stage4;
    }
    void insert(ID_TYPE id, DATA_TYPE T) {
        if (stage1->query(id) < threshold) {
            stage1->insert(id);
            return;
        }
        uint32_t t = (log(T) / log(log_base));
        stage2->insert(id, t);
        stage3->insert(id, t);
        stage4->insert(id, t);
    }
    DATA_TYPE query(ID_TYPE id, double w) {
        uint32_t f, t;
        stage2->query(id, &f, &t);
        int m = (1 - w) * f;
        while (m > 0 && t) {
            if (stage4->query(id, t)) {
                m -= stage3->query(id, t);
            }
            t--;
        }
        return pow(log_base, t + 0.5);
    }
private:
    Stage1<ID_TYPE>* stage1;
    Stage2<ID_TYPE>* stage2;
    Stage3<ID_TYPE>* stage3;
    Stage4<ID_TYPE>* stage4;
    uint32_t threshold;
    double log_base;
};



#endif