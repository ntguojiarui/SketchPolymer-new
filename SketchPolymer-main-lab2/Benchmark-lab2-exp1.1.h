#ifndef _BENCHMARK_H_
#define _BENCHMARK_H_

#include <bits/stdc++.h>
#include <hash.h>
#include <Mmap.h>
#include "CorrectDetector.h"
#include "Baseline.h"

struct Seattle_Tuple {
    uint64_t timestamp;
    uint64_t id;
};

class SeattleBenchmark
{
public:
    SeattleBenchmark(std::string PATH)
    {
        
        int datalen=0;
        std::vector<double> numbers;
        std::ifstream file(PATH.c_str());
        while( ! file.eof() )
        {
            double temp;
            file>>temp;
            numbers.push_back(temp*100000);
            datalen++;
        }
        file.close();
        std::cout<<datalen<<" "<<numbers.size()<<"\n";

        //seattle: 688file, each file has 99*99 item

        dataset = new Seattle_Tuple[datalen];
        int index = 0;
        for (int filenum=0;filenum<688;filenum++)
        {
            for (int flow_id=0;flow_id<99;flow_id++)
            {
                for (int i=0;i<99;i++)
                {
                    dataset[index].id=index % (9801) * 41 + 41; //use [node i to node j] as id
                    dataset[index].timestamp=numbers[index];
                    index++;
                }   
            }
        }
        std::cout<<"index="<<index<<", datalen="<<datalen<<std::endl;
        running_length = index;
        correct_detector = new CorrectDetector<uint64_t>();
        for (int i = 0; i < running_length; ++i) 
        {
            if(dataset[i].timestamp == 0) continue;
            if (id_set.find(dataset[i].id) == id_set.end()) 
            {
                id_set.insert(dataset[i].id);
                id_map[dataset[i].id] = 0;
            }
            id_map[dataset[i].id]++;
            if (id_map[dataset[i].id] > 1) 
            {
                correct_detector->insert(dataset[i].id, dataset[i].timestamp);
            }
        }
        for (auto i : id_map) 
        {
            if (i.second < 500)
                continue;
            target_list[num] = i.first;
            answer_list[num] = correct_detector->query_value(i.first, query_quantile);
            num++;
        }
        id_map.clear();
        id_set.clear();
        printf("%d\n",num);
    }
    ~SeattleBenchmark() {}
    double Run(int memory)
    {
        //SketchPolymer<uint64_t, uint64_t> *sketchpolymer = new SketchPolymer<uint64_t, uint64_t>(memory);
        Baseline<uint64_t> *baseline = new Baseline<uint64_t>(memory);
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
                //sketchpolymer->insert(dataset[i].id,  dataset[i].timestamp);
                baseline->insert(dataset[i].id,  dataset[i].timestamp);
            }
        }
        double ale = 0.0,are = 0.0,aqe = 0.0;
        for (int i=0; i<num; i++) 
        {
            //double predict = sketchpolymer->query(target_list[i], query_quantile);
            double predict = baseline->query(target_list[i], query_quantile);
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
        //delete sketchpolymer;
        delete baseline;
        id_map.clear();
        id_set.clear();
        return ale / num;
    }

private:
    const std::string filename;
    LoadResult load_result;
    Seattle_Tuple *dataset;
    CorrectDetector<uint64_t> *correct_detector;
    std::unordered_map<uint64_t, uint32_t> id_map;
    std::unordered_set<uint64_t> id_set;
    uint32_t running_length = 0;
    uint64_t target_list[10000];
    double answer_list[10000];
    double qanswer_list[10000];
    uint32_t num = 0;
    const double query_quantile = 0.95;
};

#endif
