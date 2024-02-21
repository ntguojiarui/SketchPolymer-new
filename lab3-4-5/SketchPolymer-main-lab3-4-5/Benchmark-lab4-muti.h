#ifndef _BENCHMARK_H_
#define _BENCHMARK_H_

#include <bits/stdc++.h>
#include <hash.h>
#include <Mmap.h>
#include <time.h>
#include "CorrectDetector.h"
#include "SketchPolymer-lab4-muti.h"

using namespace std::chrono;
#define th_num 16

struct CAIDA_Tuple
{
    uint64_t timestamp;
    uint64_t id;
};

std::vector<CAIDA_Tuple> insert_list;
std::mutex list_mtx;

void thread_fun(int tnum,SketchPolymer<uint64_t, uint64_t> *sketchpolymer)
{
    list_mtx.lock();
    int length = insert_list.size();
    list_mtx.unlock();
    int start = (length/th_num)*tnum;
    int end = (length/th_num)*(tnum+1);
    if(tnum==th_num-1) end = length;
    for(int i=start;i<end;i++)
    {
        sketchpolymer->insert(insert_list[i].id,insert_list[i].timestamp);
    }
}

class CAIDABenchmark
{
public:
    CAIDABenchmark(std::string PATH)
    {
        load_result = Load(PATH.c_str());
        dataset = (CAIDA_Tuple *)load_result.start;
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
                insert_list.push_back(dataset[i]);
                insert_list.back().timestamp -= last_time[dataset[i].id];
            }
            last_time[dataset[i].id] = dataset[i].timestamp;
        }
        id_map.clear();
        id_set.clear();
        last_time.clear();
        printf("%ld\n",insert_list.size());
        exit(0);
    }
    ~CAIDABenchmark() {}
    double Run(int memory)
    {
        double time_cost = 0.0;
        SketchPolymer<uint64_t, uint64_t> *sketchpolymer = new SketchPolymer<uint64_t, uint64_t>(memory);
        clock_t t1,t2;
        auto begin_t = system_clock::now();
        std::thread mythread0(thread_fun,0,sketchpolymer);
        std::thread mythread1(thread_fun,1,sketchpolymer);
        std::thread mythread2(thread_fun,2,sketchpolymer);
        std::thread mythread3(thread_fun,3,sketchpolymer);
        std::thread mythread4(thread_fun,4,sketchpolymer);
        std::thread mythread5(thread_fun,5,sketchpolymer);
        std::thread mythread6(thread_fun,6,sketchpolymer);
        std::thread mythread7(thread_fun,7,sketchpolymer);
        std::thread mythread8(thread_fun,8,sketchpolymer);
        std::thread mythread9(thread_fun,9,sketchpolymer);
        std::thread mythread10(thread_fun,10,sketchpolymer);
        std::thread mythread11(thread_fun,11,sketchpolymer);
        std::thread mythread12(thread_fun,12,sketchpolymer);
        std::thread mythread13(thread_fun,13,sketchpolymer);
        std::thread mythread14(thread_fun,14,sketchpolymer);
        std::thread mythread15(thread_fun,15,sketchpolymer);
        mythread0.join();
        mythread1.join();
        mythread2.join();
        mythread3.join();
        mythread4.join();
        mythread5.join();
        mythread6.join();
        mythread7.join();
        mythread8.join();
        mythread9.join();
        mythread10.join();
        mythread11.join();
        mythread12.join();
        mythread13.join();
        mythread14.join();
        mythread15.join();
        auto end_t = system_clock::now();
        duration<double> dur_t = end_t - begin_t;
        std::cout << "MEM: " << memory << "\n";
        std::cout << "Time: " << dur_t.count() << "sec" << "\n";
        delete sketchpolymer;
        return 1;
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
    uint64_t target_list[4000];
    double answer_list[4000];
    double qanswer_list[4000];
    uint32_t num = 0;
    const double query_quantile = 0.95;
};

#endif
