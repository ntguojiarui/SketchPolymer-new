#ifndef _BENCHMARK_H_
#define _BENCHMARK_H_

#include <bits/stdc++.h>
#include <hash.h>
#include <Mmap.h>
#include <time.h>
#include "SketchPolymer.h"

#define DATA_LENGTH 9737242

struct CAIDA_Tuple {
    uint64_t timestamp;
    uint64_t id;
};

class CAIDABenchmark {
public:
	CAIDABenchmark(std::string PATH) {
        load_result = Load(PATH.c_str());
        dataset = (CAIDA_Tuple*)load_result.start;
        length = load_result.length / sizeof(CAIDA_Tuple);
    }
	~CAIDABenchmark() {}
    double Run(int memory) {
        double ans=0;
        clock_t t1,t2;
        std::map<uint64_t, uint32_t> id_map;
        for (int i = 0; i < DATA_LENGTH; ++i) {
            if (id_map.find(dataset[i].id) == id_map.end()) {
                id_map[dataset[i].id] = 0;
            }
            id_map[dataset[i].id]++;
        }
        SketchPolymer<uint64_t, uint64_t>* sketchpolymer = new SketchPolymer<uint64_t, uint64_t>(memory, 3, 5, 3, 2, 50, 1.5);
        for (int i = 0; i < DATA_LENGTH; ++i) {
            sketchpolymer->insert(dataset[i].id, dataset[i + 1].timestamp - dataset[i].timestamp);
        }
        ans+=(double)(t2-t1)/(double)(1000);
        double query_quantile = 0.95;
        for (auto i : id_map) {
            if (i.second < 1000)
                continue;    
            t1 = clock();
            int64_t predict = sketchpolymer->query(i.first, query_quantile);
            t2 = clock();
            ans+=(double)(t2-t1)/(double)(1000);
        }
        return ans;
    }
private:
	std::string filename;
    LoadResult load_result;
    CAIDA_Tuple *dataset;
    uint64_t length;
};

#endif