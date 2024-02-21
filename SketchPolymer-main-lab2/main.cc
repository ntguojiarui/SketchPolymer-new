#include <bits/stdc++.h>
#include "Benchmark-lab2-exp2.2.h"
using namespace std;

int main() {
	// string path = "/share/datasets/CAIDA2016/CAIDA.dat";
	// CAIDABenchmark benchmark(path);
	// string path = "../dataset/SeattleData_all";
	// SeattleBenchmark benchmark(path);
	string path = "../dataset/Webget_all_simplify";
	WebgetBenchmark benchmark(path);
	benchmark.Run(3000);
	benchmark.Run(4000);
	benchmark.Run(5000);
	benchmark.Run(6000);
	benchmark.Run(7000);
	benchmark.Run(8000);
	return 0;
}