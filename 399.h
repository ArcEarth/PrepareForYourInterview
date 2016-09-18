#pragma once
#include <vector>
#include <tuple>
#include <map>
#include <algorithm>
#include <queue>

using namespace std;
// 399
class Solution {
public:
	int cno;
	map<string, int> strno;
	struct edge_t {
		int b, e; double val;
		bool operator<(const edge_t rhs) const {
			return b < rhs.b || (b == rhs.b && e < rhs.e);
		}
	};
	vector<edge_t> edges;
	map<int, vector<int>> results;

	int lookup(const string& name)
	{
		auto itr = strno.find(name);
		if (itr != strno.end())
			return itr->second;
		strno[name] = cno++;
		return cno - 1;
	}

	double query(int s, int t)
	{
		if (s == t) return 1.0;
		vector<int> D(cno,0);
		vector<double> V(cno);
		queue<int> vqueue;
		vqueue.push(s);
		D[s] = 1; V[s] = 1.0;
		while(!vqueue.empty())
		{
			int v = vqueue.front(); 
			vqueue.pop();
			auto est = lower_bound(edges.begin(), edges.end(), edge_t{ v,0,0 });
			for (;est != edges.end() && est->b == v; ++est)
			{ 
				int to = est->e; 
				if (D[to]++ == 0) {
					V[to] = V[v] * est->val;
					if (to == t) return V[to];
					vqueue.push(to);
				}
			}
		}
		return -1.0;
	}

	vector<double> calcEquation(vector<pair<string, string>> equations, vector<double>& values, vector<pair<string, string>> queries) {
		cno = 0;
		for (int i = 0; i < equations.size(); i++)
		{
			const auto& eq = equations[i];
			auto e = edge_t{ lookup(eq.first), lookup(eq.second), values[i] };
			edges.push_back(e);
			swap(e.b, e.e);
			e.val = 1.0 / e.val;
			edges.push_back(e);
		}
		sort(edges.begin(), edges.end());

		int cnof = cno;
		vector<double> answer;
		for (auto& q : queries)
		{
			int b = lookup(q.first);
			int e = lookup(q.second);
			if (b >= cnof || e >= cnof) {
				answer.push_back(-1.0);
			} else
				answer.push_back(query(b, e));
		}
		return answer;
	}
};