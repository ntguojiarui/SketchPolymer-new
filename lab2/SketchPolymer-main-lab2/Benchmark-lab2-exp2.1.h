#ifndef _BENCHMARK_H_
#define _BENCHMARK_H_

#include <bits/stdc++.h>
#include <hash.h>
#include <Mmap.h>
#include <time.h>
#include "CorrectDetector.h"
#include "SketchPolymer-lab2-salsa.h"

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
                insert_list.push_back(dataset[i]);
            }
        }
        for (auto i : id_map) 
        {
            if (i.second < 500)
                continue;
            target_list[num] = i.first;
            num++;
        }
        id_map.clear();
        id_set.clear();
    }
    ~SeattleBenchmark() {}
    double Run(int memory)
    {
        double time_cost = 0.0;
        SketchPolymer<uint64_t, uint64_t> *sketchpolymer = new SketchPolymer<uint64_t, uint64_t>(memory);
        clock_t t1,t2,t3;
        t1 = clock();
        for(auto i : insert_list)
        {
            sketchpolymer->insert(i.id,i.timestamp);
        }
        t2 = clock();
        for (int i=0; i<num; i++) 
        {
            double predict = sketchpolymer->query(target_list[i], query_quantile);
        }
        t3 = clock();
        std::cout << "MEM: " << memory << "\n";
        std::cout << "Thr1: " << (double)(insert_list.size())/ (double)(t2 - t1) << "\n";
        std::cout << "Thr2: " << (double)(num)/ (double)(t3 - t2) << "\n";
        delete sketchpolymer;
        return 1;
    }

private:
    const std::string filename;
    LoadResult load_result;
    Seattle_Tuple *dataset;
    std::vector<Seattle_Tuple> insert_list;
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
