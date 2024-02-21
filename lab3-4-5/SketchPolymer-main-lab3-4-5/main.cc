#include <bits/stdc++.h>
#include "Benchmark-lab3.h"
using namespace std;

int main() {
	string path = "/share/datasets/CAIDA2016/CAIDA.dat";
	CAIDABenchmark benchmark(path);
	benchmark.Run(3000);
	benchmark.Run(4000);
	benchmark.Run(5000);
	benchmark.Run(6000);
	benchmark.Run(7000);
	benchmark.Run(8000);
	return 0;
}