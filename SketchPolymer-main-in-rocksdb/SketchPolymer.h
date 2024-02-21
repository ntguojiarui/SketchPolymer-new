#ifndef _SP_H_
#define _SP_H_

#include <bits/stdc++.h>

/********************************************************************/
#define FORCE_INLINE __attribute__((always_inline)) inline
#define MAX(a, b) (a > b? a:b)
#define MIN(a, b) (a < b? a:b)

struct CAIDA_Tuple {
    uint64_t timestamp;
    uint64_t id;
};

struct LoadResult{
    void* start;
    uint64_t length;
};

LoadResult Load(const char* PATH){
    LoadResult ret;

    int32_t fd = open(PATH, O_RDONLY);
    if(fd == -1) {
        std::cerr << "Cannot open " << PATH << std::endl;
        throw;
    }

    struct stat sb;
    if(fstat(fd, &sb) == -1){
        std::cerr << "Fstat Error" << std::endl;
        throw;
    }

    ret.length = sb.st_size;
    ret.start = mmap(nullptr, ret.length, PROT_READ, MAP_PRIVATE, fd, 0u);

    if (ret.start == MAP_FAILED) {
        std::cerr << "Cannot mmap " << PATH << " of length " << ret.length << std::endl;
        throw;
    }

    return ret;
}

void UnLoad(LoadResult result){
    munmap(result.start, result.length);
}

static FORCE_INLINE uint32_t rotl32 ( uint32_t x, int8_t r )
{
  return (x << r) | (x >> (32 - r));
}

static FORCE_INLINE uint64_t rotl64 ( uint64_t x, int8_t r )
{
  return (x << r) | (x >> (64 - r));
}

#define	ROTL32(x,y)	rotl32(x,y)
#define ROTL64(x,y)	rotl64(x,y)

#define BIG_CONSTANT(x) (x##LLU)

//-----------------------------------------------------------------------------
// Block read - if your platform needs to do endian-swapping or can only
// handle aligned reads, do the conversion here

#define getblock(p, i) (p[i])

//-----------------------------------------------------------------------------
// Finalization mix - force all bits of a hash block to avalanche

static FORCE_INLINE uint32_t fmix32 ( uint32_t h )
{
  h ^= h >> 16;
  h *= 0x85ebca6b;
  h ^= h >> 13;
  h *= 0xc2b2ae35;
  h ^= h >> 16;

  return h;
}

//----------

static FORCE_INLINE uint64_t fmix64 ( uint64_t k )
{
  k ^= k >> 33;
  k *= BIG_CONSTANT(0xff51afd7ed558ccd);
  k ^= k >> 33;
  k *= BIG_CONSTANT(0xc4ceb9fe1a85ec53);
  k ^= k >> 33;

  return k;
}


void MurmurHash3_x86_32 ( const void * key, int len,
                          uint32_t seed, void * out )
{
  const uint8_t * data = (const uint8_t*)key;
  const int nblocks = len / 4;
  int i;

  uint32_t h1 = seed;

  uint32_t c1 = 0xcc9e2d51;
  uint32_t c2 = 0x1b873593;

  //----------
  // body

  const uint32_t * blocks = (const uint32_t *)(data + nblocks*4);

  for(i = -nblocks; i; i++)
  {
    uint32_t k1 = getblock(blocks,i);

    k1 *= c1;
    k1 = ROTL32(k1,15);
    k1 *= c2;
    
    h1 ^= k1;
    h1 = ROTL32(h1,13); 
    h1 = h1*5+0xe6546b64;
  }

  //----------
  // tail

  const uint8_t * tail = (const uint8_t*)(data + nblocks*4);

  uint32_t k1 = 0;

  switch(len & 3)
  {
  case 3: k1 ^= tail[2] << 16;
  case 2: k1 ^= tail[1] << 8;
  case 1: k1 ^= tail[0];
          k1 *= c1; k1 = ROTL32(k1,15); k1 *= c2; h1 ^= k1;
  };

  //----------
  // finalization

  h1 ^= len;

  h1 = fmix32(h1);

  *(uint32_t*)out = h1;
} 

template<typename T>
inline uint32_t hash(const T& data, uint32_t seed){
    // return BOBHash::BOBHash32((uint8_t*)&data, sizeof(T), seed);
    uint32_t output;
    MurmurHash3_x86_32(&data, sizeof(T), seed, &output);
    return output;
}

/********************************************************************/


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
            if(counter[i][index]+1) counter[i][index]++;
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
    SketchPolymer(uint32_t memory, uint32_t d1, uint32_t d2, uint32_t d3, uint32_t d4, uint32_t t, double a):
    threshold(t), log_base(a) {
        stage1 = new Stage1<ID_TYPE>(0.2 * memory, d1);
        stage2 = new Stage2<ID_TYPE>(0.3 * memory, d2);
        stage3 = new Stage3<ID_TYPE>(0.4 * memory, d3);
        stage4 = new Stage4<ID_TYPE>(0.1 * memory, d4);
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
        uint32_t f=0, t=0;
        stage2->query(id, &f, &t);
        int m = (1 - w) * f;
        int counter =0;
        while (m > 0 && t > 0) {
            if (stage4->query(id, t)) {
                m -= stage3->query(id, t);
            }
            t--;
            counter++;
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