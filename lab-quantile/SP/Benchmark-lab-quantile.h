#ifndef _BENCHMARK_H_
#define _BENCHMARK_H_

#include <bits/stdc++.h>
#include <hash.h>
#include <Mmap.h>
#include "CorrectDetector.h"
#include "SketchPolymer-lab-quantile.h"

struct CAIDA_Tuple
{
    uint64_t timestamp;
    uint64_t id;
};

class CAIDABenchmark
{
public:
    CAIDABenchmark(std::string PATH)
    {
        num=0;
        load_result = Load(PATH.c_str());
        dataset = (CAIDA_Tuple *)load_result.start;
    }
    ~CAIDABenchmark() {}
    double Run(int memory,double a,double b,double c,double d,int h1,int h2,int h3,int h4,double q)
    {
        num=0;
        double sum = a+b+c+d;
        SketchPolymer<uint64_t, uint64_t> *sketchpolymer = new SketchPolymer<uint64_t, uint64_t>(memory, h1, h2, h3, h4, 50, 1.5,a/sum,b/sum,c/sum,d/sum);
        correct_detector = new CorrectDetector<uint64_t>();

        for (int i = 0; i < running_length; ++i) 
        {
            if (id_set.find(dataset[i].id) == id_set.end()) 
            {
                id_set.insert(dataset[i].id);
                id_map[dataset[i].id] = 0;
            }
            id_map[dataset[i].id]++;
            if (dataset[i].timestamp > last_time[dataset[i].id]) 
            {
                correct_detector->insert(dataset[i].id, dataset[i].timestamp - last_time[dataset[i].id]);
                sketchpolymer->insert(dataset[i].id,  dataset[i].timestamp - last_time[dataset[i].id]);
            }
            last_time[dataset[i].id] = dataset[i].timestamp;
        }
        double ale = 0.0;
        for (auto i : id_map) 
        {
            if (i.second < 1000)
                continue;
            num++;
            uint64_t predict = sketchpolymer->query(i.first, q);
            uint64_t truth = correct_detector->query_value(i.first, q);
            ale += abs( log2(predict) - log2(truth) );
        }
        delete sketchpolymer;
        delete correct_detector;
        id_map.clear();
        id_set.clear();
        last_time.clear();
        return ale / num;
    }

private:
    const std::string filename;
    LoadResult load_result;
    CAIDA_Tuple *dataset;
    CorrectDetector<uint64_t> *correct_detector;
    std::unordered_map<uint64_t, uint32_t> id_map;
    std::unordered_map<uint64_t, uint64_t> last_time;
    std::unordered_set<uint64_t> id_set;
    const uint32_t running_length = 20000000;
    uint32_t num = 0;
};

#endif
