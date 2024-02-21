#include <bits/stdc++.h>
#include "Benchmark-simplfy-lab1.h"
using namespace std;

int main(int argc, char **argv)
{
	string path = "/share/datasets/CAIDA2016/CAIDA.dat";
	double t4, t6, t8, s1, s2, s3, s4, log_base, threshold, query_quantile;
	int h1, h2, h3, h4;
	scanf("%lf %lf %lf %lf %d %d %d %d %lf %lf %lf", &s1, &s2, &s3, &s4, &h1, &h2, &h3, &h4, &log_base, &threshold, &query_quantile);
	CAIDABenchmark benchmark(path, log_base, threshold, query_quantile);
	t4 = benchmark.Run(4000, s1, s2, s3, s4, h1, h2, h3, h4);
	t6 = benchmark.Run(6000, s1, s2, s3, s4, h1, h2, h3, h4);
	t8 = benchmark.Run(8000, s1, s2, s3, s4, h1, h2, h3, h4);
	printf("4000KB: %lf\n6000KB: %lf\n8000KB: %lf\n", t4, t6, t8);
}
