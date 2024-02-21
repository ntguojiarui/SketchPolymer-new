#ifndef _CORRECT_H_
#define _CORRECT_H_

#include <bits/stdc++.h>

template<typename ID_TYPE>
class CorrectDetector {
public:
	CorrectDetector() {
		record.clear();
		id_set.clear();
	}
	~CorrectDetector() {}
	void insert(ID_TYPE id, uint64_t t) {
		if (id_set.find(id) == id_set.end()) {
			id_set.insert(id);
			record[id] = {};
		}
		record[id].push_back(t);
	}
	uint64_t query_value(ID_TYPE id, double w) {
		uint32_t index = record[id].size() * w;
		sort(record[id].begin(), record[id].end());
		return record[id][index - 1];
	}
	double query_quantile(ID_TYPE id, uint64_t t) {
		sort(record[id].begin(), record[id].end());
		auto upper = std::upper_bound(record[id].begin(), record[id].end(), t);
		int count = std::distance(record[id].begin(), upper);
		return 1.0 * count / record[id].size();
	}

private:
	std::map<ID_TYPE, std::vector<uint64_t>> record;
	std::set<ID_TYPE> id_set;
};



#endif