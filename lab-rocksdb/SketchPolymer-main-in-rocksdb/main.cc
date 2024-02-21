#include <assert.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <cstdio>
#include <iostream>
#include "SketchPolymer.h"
#include "rocksdb/db.h"
#include "rocksdb/options.h"
#include "rocksdb/slice.h"

using ROCKSDB_NAMESPACE::DB;
using ROCKSDB_NAMESPACE::Options;
using ROCKSDB_NAMESPACE::PinnableSlice;
using ROCKSDB_NAMESPACE::ReadOptions;
using ROCKSDB_NAMESPACE::Status;
using ROCKSDB_NAMESPACE::WriteBatch;
using ROCKSDB_NAMESPACE::WriteOptions;

#define DATA_LENGTH 9737242
#define MAXLINE 4096
#define MAX_VALUE_NUM 400000

long int rocksdb_time_list[500010]={0}; 
long int sketch_time_list[500010]={0};
long int key_num;

class CAIDABenchmark {
 public:
  CAIDABenchmark(std::string PATH) {
    load_result = Load(PATH.c_str());
    dataset = (CAIDA_Tuple *)load_result.start;
    length = load_result.length / sizeof(CAIDA_Tuple);
  }
  ~CAIDABenchmark() {};
  double Run(int memory) {
    double ans = 0;
    clock_t t1, t2;
    std::map<uint64_t, uint32_t> id_map;
    for (int i = 0; i < DATA_LENGTH; ++i) {
      if (id_map.find(dataset[i].id) == id_map.end()) {
        id_map[dataset[i].id] = 0;
      }
      id_map[dataset[i].id]++;
    }
    SketchPolymer<uint64_t, uint64_t> *sketchpolymer =
        new SketchPolymer<uint64_t, uint64_t>(memory, 3, 5, 5, 3, 50, 1.5);
    for (int i = 0; i < DATA_LENGTH; ++i) {
      sketchpolymer->insert(dataset[i].id,
                            dataset[i + 1].timestamp - dataset[i].timestamp);
    }
    ans += (double)(t2 - t1) / (double)(CLOCKS_PER_SEC);
    double query_quantile = 0.95;
    int counter=0;
    for (auto i : id_map) {
      if (i.second < 1000) continue;
      t1 = clock();
      int64_t predict = sketchpolymer->query(i.first, query_quantile);
      t2 = clock();
      sketch_time_list[counter++]=(t2 - t1);
      ans += (double)(t2 - t1) / (double)(CLOCKS_PER_SEC);
    }
    key_num=counter;
    UnLoad(load_result);
    return ans;
  }

 private:
  std::string filename;
  LoadResult load_result;
  CAIDA_Tuple *dataset;
  uint64_t length;
};

class Naive {
 public:
  Naive(std::string PATH) {
    load_result = Load(PATH.c_str());
    dataset = (CAIDA_Tuple *)load_result.start;
    length = load_result.length / sizeof(CAIDA_Tuple);
    options.IncreaseParallelism();
    options.OptimizeLevelStyleCompaction();
    options.create_if_missing = true;
    Status s = DB::Open(options, kDBPath, &db);
    assert(s.ok());
  }
  ~Naive() {
    delete db;
  }
  double Run() {
    clock_t t1, t2;
    double ans = 0;
    for (int i = 0; i < DATA_LENGTH-1; ++i) {
      Put_in(db, dataset[i].id,
             dataset[i + 1].timestamp - dataset[i].timestamp);
    }
    std::map<uint64_t, uint32_t> id_map;
    for (int i = 0; i < DATA_LENGTH; ++i) {
      if (id_map.find(dataset[i].id) == id_map.end()) {
        id_map[dataset[i].id] = 0;
      }
      id_map[dataset[i].id]++;
    }
    double query_quantile = 0.95;
    int counter=0;
    for (auto i : id_map) {
      if (i.second < 1000) continue;
      t1 = clock();
      int64_t predict = Query(i.first, query_quantile);
      t2 = clock();
      rocksdb_time_list[counter++]=(t2 - t1);
      ans += (double)(t2 - t1) / (double)(CLOCKS_PER_SEC);
    }
    UnLoad(load_result);
    return ans;
  }

 private:
  std::string filename;
  LoadResult load_result;
  CAIDA_Tuple *dataset;
  uint64_t length;
  std::string kDBPath = "/tmp/rocksdb_sketch_lab";
  DB *db;
  Options options;
  std::unordered_map<uint64_t, uint32_t> suffix;

  inline void Put_in(DB *dp, uint64_t key, uint64_t value) {
    if (suffix.find(key) == suffix.end()) {
      suffix[key] = 1;
    } else
      suffix[key]++;
    char key_str[25];
    char val_str[20];
    sprintf(key_str, "%016lx%08x", key, suffix[key]);
    sprintf(val_str, "%lx", value);
    Status s = db->Put(WriteOptions(), key_str, val_str);
    assert(s.ok());
  }

  int64_t Query(uint64_t id, double w) {
    uint64_t value_list[MAX_VALUE_NUM];
    int i = 0;
    char key_str[25];
    std::string val_str;
    sprintf(key_str, "%016lx%08x", id, i + 1);
    while (1) {
      Status s = db->Get(ReadOptions(), key_str, &val_str);
      if (s.ok()) {
        value_list[i++] = atouint64(val_str);
        sprintf(key_str, "%016lx%08x", id, i + 1);
      } else {
        break;
      }
    }
    std::sort(value_list, value_list + i);
    uint64_t index = (double)(i - 1) * w;
    return value_list[index];
  }

  inline uint64_t atouint64(std::string val_str) {
    uint64_t val_len = val_str.size();
    uint64_t ans = 0;
    uint64_t temp = 0;
    for (int i = val_len; i > 0; i--) {
      temp = (val_str[val_len - i] <= '9') ? val_str[val_len - i] - '0'
                                           : val_str[val_len - i] - 'a' + 10;
      ans = ans * 16 + temp;
    }
    return ans;
  }
};

int main(int argc, char **argv) {
  double sketch_time_cost, naive_time_cost;
  std::string path = "./test.dat";
  CAIDABenchmark sketch(path);
  sketch_time_cost = sketch.Run(atoi(argv[1]));
  printf("SketchPolomer time cost is %lfsec\n", sketch_time_cost);
  Naive naive(path);
  naive_time_cost = naive.Run();
  printf("Rocksdb time cost is %lfsec\n", naive_time_cost);
  std::sort(sketch_time_list,sketch_time_list+key_num);
  std::sort(rocksdb_time_list,rocksdb_time_list+key_num);
  FILE *fp=fopen("output.txt","w");
  fprintf(fp,"\nrocksdb_time_list\n");
  for(int i=70;i<100;i++)
  {
    fprintf(fp,"%lu ",rocksdb_time_list[(int)((double)key_num*(double)(i)*(double)(0.01))]);
  } 
  fprintf(fp,"\nsketch_time_list\n\n");
  for(int i=70;i<100;i++)
  {
    fprintf(fp,"%lu ",sketch_time_list[(int)((double)key_num*(double)(i)*(double)(0.01))]);
  }  
  fclose(fp);
  return 0;
}