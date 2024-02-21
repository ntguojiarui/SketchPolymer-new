#ifndef _BENCHMARK_H_
#define _BENCHMARK_H_

#include <bits/stdc++.h>
#include <hash.h>
#include <Mmap.h>
#include "CorrectDetector.h"
#include "SketchPolymer-lab2-salsa.h"

struct Webget_Tuple {
    uint64_t id;
    uint64_t timestamp;
};

class WebgetBenchmark
{
public:
    WebgetBenchmark(std::string PATH)
    {
        uint64_t temp_ma = 0;
        load_result = Load(PATH.c_str());
        dataset = (Webget_Tuple *)load_result.start;
        running_length = 7982157*2+1;
        correct_detector = new CorrectDetector<uint64_t>();
        for (int i = 0; i < running_length; ++i) 
        {
            if (id_set.find(dataset[i].id) == id_set.end()) 
            {
                id_set.insert(dataset[i].id);
                id_map[dataset[i].id] = 0;
            }
            id_map[dataset[i].id]++;
            if (id_map[dataset[i].id] > 1) 
            {
                correct_detector->insert(dataset[i].id, dataset[i].timestamp);
                temp_ma = std::max(temp_ma , dataset[i].timestamp);
            }
        }
        printf("%lu\n",temp_ma);
        for (auto i : id_map) 
        {
            if (i.second < 1000)
                continue;
            target_list[num] = i.first;
            answer_list[num] = correct_detector->query_value(i.first, query_quantile);
            num++;
        }
        id_map.clear();
        id_set.clear();
    }
    ~WebgetBenchmark() {}
    double Run(int memory)
    {
        SketchPolymer<uint64_t, uint64_t> *sketchpolymer = new SketchPolymer<uint64_t, uint64_t>(memory);
        for (int i = 0; i < running_length; ++i) 
        {
            if (id_set.find(dataset[i].id) == id_set.end()) 
            {
                id_set.insert(dataset[i].id);
                id_map[dataset[i].id] = 0;
            }
            id_map[dataset[i].id]++;
            if (dataset[i].id > 1) 
            {
                sketchpolymer->insert(dataset[i].id,  dataset[i].timestamp);
            }
        }
        double ale = 0.0,are = 0.0,aqe = 0.0;
        for (int i=0; i<num; i++) 
        {
            double predict = sketchpolymer->query(target_list[i], query_quantile);
            ale += 1.0 * abs( log2(predict) - log2(answer_list[i]));
            double temp = fabs(predict - answer_list[i]);
            are += temp / answer_list[i];
            double truth_quantile = correct_detector->query_quantile(target_list[i], predict);
            aqe += 1.0 * fabs( truth_quantile - query_quantile);
        }
        std::cout << "MEM: " << memory << "\n";
        std::cout << "ARE: " << are / num << "\n";
        std::cout << "ALE: " << ale / num << "\n";
        std::cout << "AQE: " << aqe / num << "\n";
        delete sketchpolymer;
        id_map.clear();
        id_set.clear();
        return ale / num;
    }

private:
    const std::string filename;
    LoadResult load_result;
    Webget_Tuple *dataset;
    CorrectDetector<uint64_t> *correct_detector;
    std::unordered_map<uint64_t, uint32_t> id_map;
    std::unordered_set<uint64_t> id_set;
    uint32_t running_length = 0;
    uint64_t target_list[40000];
    double answer_list[40000];
    double qanswer_list[40000];
    uint32_t num = 0;
    const double query_quantile = 0.95;
};

#endif
