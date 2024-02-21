#include <bits/stdc++.h>
#include "Benchmark-lab-quantile.h"
using namespace std;

int main(int argc, char **argv)
{
	string path = "/share/datasets/CAIDA2016/CAIDA.dat";
	CAIDABenchmark benchmark(path);
	printf("0,");
	for (int q = 10; q <= 95; q += 1)
	{
		printf("%d,",q);
	}
	printf("0\n");
	for (int mem = 4000; mem <= 8000; mem += 2000)
	{
		printf("%d,",mem);
		for (double q = 0.10; q <= 0.95; q += 0.01)
		{
			double a = 0.2, b = 0.3, c = 0.4, d = 0.1;
			int h1 = 3, h2 = 5, h3 = 3, h4 = 2;
			double t = benchmark.Run(mem,a,b,c,d,h1,h2,h3,h4,q);
			printf("%lf,",t);
		}
		printf("0\n");
	}
}
