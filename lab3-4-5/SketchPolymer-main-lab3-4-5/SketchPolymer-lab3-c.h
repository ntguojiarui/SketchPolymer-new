#ifndef _SP_H_
#define _SP_H_

#include <bits/stdc++.h>

template<typename ID_TYPE>
class Stage1 {
public:
    Stage1() {}
    Stage1(uint32_t memory, uint32_t dim): d(dim) {
        m = memory * 1024 / sizeof(int8_t) / d;
        counter = new int8_t* [d];
        for (int i = 0; i < d; ++i) {
            counter[i] = new int8_t [m];
            memset(counter[i], 0, m * sizeof(int8_t));
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
            int32_t sign = hash(id,11*i) % 2;
            if (sign == 1 && counter[i][index] != std::numeric_limits<int8_t>::max())
                counter[i][index]++;
            else if(sign == 0 && counter[i][index] != std::numeric_limits<int8_t>::min())
                counter[i][index]--;
        }
    }
    uint32_t query(ID_TYPE id) {
        int8_t *result = new int8_t[d];
        for (int i = 0; i < d; ++i) {
            uint32_t index = hash(id, i) % m;
            int32_t sign = hash(id,11 * i) % 2;
            result[i] = sign ? counter[i][index] : -counter[i][index]; 
        }
        int32_t frequency;
        std::sort(result,result+d);
        int mid = d/2;
        if(d%2==0) frequency = (result[mid]+result[mid-1])/2;
        else frequency = result[mid];        
        delete[] result;
        return frequency;
    }
private:
    uint32_t d;
    uint32_t m;
    int8_t** counter;
};

template<typename ID_TYPE>
class Stage2 {
public:
    Stage2() {}
    Stage2(uint32_t memory, uint32_t dim): d(dim) {
        m = memory * 1024 / (sizeof(uint32_t) + sizeof(uint32_t)) / d;
        frequency = new uint32_t* [d];
        value = new uint32_t* [d];
        for (int i = 0; i < d; ++i) {
            frequency[i] = new uint32_t [m];
            value[i] = new uint32_t [m];
            memset(frequency[i], 0, m * sizeof(uint32_t));
            memset(value[i], 0, m * sizeof(uint32_t));
        }
    }
    ~Stage2() {
        for (int i = 0; i < d; ++i) {
            delete[] frequency[i];
            delete[] value[i];
        }
        delete[] frequency;
        delete[] value;
    }
    void insert(ID_TYPE id, uint32_t t) {
        for (int i = 0; i < d; ++i) {
            uint32_t index = hash(id, 33 + i) % m;
            frequency[i][index]++;
            value[i][index] = MAX(value[i][index], t);
        }
    }
    void query(ID_TYPE id, uint32_t* f, uint32_t* t) {
        *f = std::numeric_limits<uint32_t>::max();
        *t = std::numeric_limits<uint32_t>::max();
        for (int i = 0; i < d; ++i) {
            uint32_t index = hash(id, 33 + i) % m;
            *f = MIN(*f, frequency[i][index]);
            *t = MIN(*t, value[i][index]);
        }
    }
private:
    uint32_t d;
    uint32_t m;
    uint32_t** frequency;
    uint32_t** value;
};

template<typename ID_TYPE>
class Stage3 {
public:
    Stage3() {}
    Stage3(uint32_t memory, uint32_t dim): d(dim) {
        m = memory * 1024 / sizeof(uint8_t) / d;
        counter = new uint8_t* [d];
        for (int i = 0; i < d; ++i) {
            counter[i] = new uint8_t [m];
            memset(counter[i], 0, m * sizeof(uint8_t));
        }
    }
    ~Stage3() {
        for (int i = 0; i < d; ++i) {
            delete[] counter[i];
        }
        delete[] counter;
    }
    void insert(ID_TYPE id, uint32_t t) {
        for (int i = 0; i < d; ++i) {
            uint32_t index = hash(id * t, i) % m;
            if (counter[i][index] != std::numeric_limits<uint8_t>::max())
                counter[i][index]++;
        }
    }
    uint32_t query(ID_TYPE id, uint32_t t) {
        uint8_t frequency = std::numeric_limits<uint8_t>::max();
        for (int i = 0; i < d; ++i) {
            uint32_t index = hash(id * t, i) % m;
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