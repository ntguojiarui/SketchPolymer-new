#ifndef _SPA_H
#define _SPA_H
using namespace std;

class SPA
{
public:
	virtual void insert(uint64_t key, int f) = 0;
};

#endif // _SPA_H