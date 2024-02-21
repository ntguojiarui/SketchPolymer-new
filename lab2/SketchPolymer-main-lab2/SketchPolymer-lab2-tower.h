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
    Stage2(uint32_t memory, uint32_t dim){
        memory*=128;
        uint32_t cell_num = memory * 1024 / sizeof(uint32_t) / 36;
        frequency = new uint32_t* [d];
        value = new uint32_t* [d];
        for (int i = 0; i < d; ++i) {
            size[i] = cell_num * 32 / bits[i];
            frequency[i] = new uint32_t [cell_num];
            value[i] = new uint32_t [size[i]];
            memset(frequency[i], 0, cell_num * sizeof(uint32_t));
            memset(value[i], 0, size[i] * sizeof(uint32_t));
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
	uint32_t get(int i, int j, int s, int e) {
		// get the number in bit s-e in counter[i][j]
        if(s==0 && e==32) return frequency[i][j];
		return (frequency[i][j] & (((1 << (e - s)) - 1) << s)) >> s;
	}
	void increment(int i, int j, int s, int e) {
		// increment bit s-e in counter[i][j]
		uint32_t result = get(i, j, s, e);
		if (result <= max_value[i]) {
            if(s==0 && e==32){
                frequency[i][j] = result+1;
            }else{
                frequency[i][j] = ((frequency[i][j] & (~(((1 << (e - s)) - 1) << s))) | ((result + 1) << s));
            }
			uint32_t now = get(i, j, s, e);
            // if(result+1!=now)
            // {
            //     printf("%u %u %u %u %u %u %u %u\n",i,j,frequency[i][j],s,e,result,result+1,now);
            //     exit(0);
            // }
			assert(result + 1 == now);
		}
	}    
    void insert(ID_TYPE id, uint32_t t) {
        for (int i = 0; i < d; ++i) {
            uint32_t index = hash(id, 33 + i) % size[i];
            uint32_t cell = index * bits[i] / 32;
            uint32_t res = index - cell * 32 / bits[i];
            increment(i, cell, res * bits[i], res * bits[i] + bits[i]);
            //printf("2\n");
            value[i][index] = MAX(value[i][index], t);
        }
    }
    void query(ID_TYPE id, uint32_t* f, uint32_t* t) {
        *f = std::numeric_limits<uint32_t>::max();
        *t = std::numeric_limits<uint32_t>::max();
        for (int i = 0; i < d; ++i) {
            uint32_t index = hash(id, 33 + i) % size[i];
            uint32_t cell = index * bits[i] / 32;
            uint32_t res = index - cell * 32 / bits[i];
            uint32_t result = get(i, cell, res * bits[i], res * bits[i] + bits[i]);
            if(result <= max_value[i])
            {
                *f = MIN(*f, result);
                *t = MIN(*t, value[i][index]);
            }
        }
    }
private:
    uint32_t d=5;
    uint32_t mem;
    uint32_t bits[5] = {2, 4, 8, 16, 32};
	uint32_t max_value[5] = {0x2, 0xe, 0xfe, 0xfffe, 0xfffffffe};
	uint32_t size[5];
    uint32_t** frequency;
    uint32_t** value;
};

template<typename ID_TYPE>
class Stage3{
public:
	Stage3() {}
	Stage3(uint32_t memory,int d) {
		mem = memory;
		uint32_t cell_num = memory * 1024 / sizeof(uint32_t) / 3;
		counter = new uint32_t* [3];
		for (int i = 0; i < 3; ++i) {
			size[i] = cell_num * 32 / bits[i];
			counter[i] = new uint32_t[cell_num];
			memset(counter[i], 0, cell_num * sizeof(uint32_t));
		}
	}
	~Stage3() {
		for (int i = 0; i < 3; ++i) 
			delete[] counter[i];
		delete[] counter;
	}
	uint32_t get(int i, int j, int s, int e) {
		// get the number in bit s-e in counter[i][j]
		return (counter[i][j] & (((1 << (e - s)) - 1) << s)) >> s;
	}
	void increment(int i, int j, int s, int e) {
		// increment bit s-e in counter[i][j]
		uint32_t result = get(i, j, s, e);
		if (result <= max_value[i]) {
			counter[i][j] = ((counter[i][j] & (~(((1 << (e - s)) - 1) << s))) | ((result + 1) << s));
			uint32_t now = get(i, j, s, e);
			assert(result + 1 == now);
		}	
	}
	void insert(ID_TYPE id, uint32_t t) {
		for (int i = 0; i < 3; ++i) {
			uint32_t index = hash(id + t, i) % size[i];
			uint32_t cell = index * bits[i] / 32;
			uint32_t res = index - cell * 32 / bits[i];
			increment(i, cell, res * bits[i], res * bits[i] + bits[i]);
		}
	}
	uint32_t query(ID_TYPE id, uint32_t t) {
		uint32_t value = 255;
		for (int i = 0; i < 3; ++i) {
			uint32_t index = hash(id + t, i) % size[i];
			uint32_t cell = index * bits[i] / 32;
			uint32_t res = index - cell * 32 / bits[i];
			uint32_t result = get(i, cell, res * bits[i], res * bits[i] + bits[i]);
			if (result <= max_value[i]) 
				value = MIN(value, result);
		}
		return value;
	}
private:
	uint32_t bits[3] = {2, 4, 8};
	uint32_t max_value[3] = {2, 14, 254};
	uint32_t size[3];
	uint32_t** counter;
	uint32_t mem=-1;
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