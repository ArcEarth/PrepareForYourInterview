#include <algorithm>
#include <vector>
#include <string>
#include <iostream>
#include <set>
#include <memory>
#include <cassert>
#include <chrono>

#include "thread_pool.h"
#include "399.h"
#include "max_assignment.h"
//#include "seg_tree.h"
//#include "bst_map.h"

using namespace std;
using namespace stdx;

void thread_pool_test(int poolsize, int totaljob, int jobsize);
void threads_no_pooling_test(int totaljob, int jobsize);
void bst_test();
void seg_tree_test();

template <typename T>
struct mat_t {
	T *a;
	size_t nr, nc;
	T operator()(int i, int j) const { return a[i * nc + j]; }
	T& operator()(int i, int j) { return a[i * nc + j]; }
	size_t rows() const { return nr; }
	size_t cols() const { return nc; }
};

struct empty_struct {};

class IVector {
	int ixx;
	//virtual ~IVector() = 0;
	virtual int get(int) const = 0;
	virtual void set(int,int) = 0;
	virtual int size() const = 0;
};

constexpr size_t szIv = sizeof(IVector); // == sizeof(void*)
constexpr size_t szes = sizeof(empty_struct);

struct _IVector {
	struct _IVectorVirtualTable {
		int(*get)(const _IVector*, int);
		void(*set)(_IVector*, int, int);
		int(*size)(const _IVector*);
	};
	_IVectorVirtualTable* __vfptr;
};

class IVirtualInheritVector : public virtual IVector {
	int get(int) const override {
		cout << "get";
		return 0;
	}
	void set(int, int) override {
		cout << "set";
	}
	int size() const override {
		cout << "get";
		return 0;
	}

};

class IVirtualInheritVector2 : public virtual IVector {
};

class IVirtualInheritVector3 : public IVirtualInheritVector2, public IVirtualInheritVector {
};

class IVirtualInheritVector4 : public IVirtualInheritVector2, public IVirtualInheritVector3 {
};


class IInheritVector : public IVector {
	virtual int foo();
};

constexpr size_t szIVv = sizeof(IVirtualInheritVector);
constexpr size_t szIVv3 = sizeof(IVirtualInheritVector3);
constexpr size_t szIVv4 = sizeof(IVirtualInheritVector4);
constexpr size_t szIRv = sizeof(IInheritVector);


int main()
{
	//seg_tree_test();
	//bst_test();

	auto ptr = new IVirtualInheritVector();
	auto ptr3 = new IVirtualInheritVector3();
	auto ptr4 = new IVirtualInheritVector4();

	using it = decay_t<const int &>;

	int poolsize = 4;
	int totaljob = 1000;
	int jobsize = 10000;

	float a[] =
	{ 3.0f,2.0f,3.0f,
	4.0f,9.0f,6.0f,
	7.0f,15.0f,9.0f };

	mat_t<float> m{ a,3,3 };

	auto matching = stdx::max_weight_bipartite_matching(m);
	auto cost = stdx::matching_cost(m, matching);

	cout << "Assignment = {";
	for (int i = 0; i < 3; i++)
		cout << matching[i] << ", ";
	cout << "\b\b}" << endl << "Cost = " << cost << endl;

	getchar();
	exit(0);
	//thread_pool_test(4, totaljob, jobsize);
	//threads_no_pooling_test(totaljob, jobsize);
	//thread_pool_test(1, totaljob, jobsize);
	//thread_pool_test(2, totaljob, jobsize);
	//thread_pool_test(8, totaljob, jobsize);
	//thread_pool_test(16, totaljob, jobsize);
	//thread_pool_test(32, totaljob, jobsize);

	Solution sol;
	vector<pair<string, string>> equations = { { "a", "b" },{ "b", "c" } };
	vector<double> values = { 2.0, 3.0 };
	vector<pair<string, string>> queries = { { "a", "c" },{ "b", "a" },{ "a", "e" },{ "a", "a" },{ "x", "x" } };
	auto answers = sol.calcEquation(equations, values, queries);
	cout << "answers = {";
	for (auto& a : answers)
	{
		cout << a << ", ";
	}
	cout << "\b\b}" << endl;
	getchar();
}

void threads_no_pooling_test(int totaljob, int jobsize)
{
	auto beg = chrono::high_resolution_clock::now();
	vector<thread> tasks(totaljob);
	for (int i = 0; i < totaljob; i++)
	{
		tasks[i] = thread([jobsize] {
			volatile int s = 0;
			for (int j = 0; j < jobsize; j++)
				s += j;
			//cout << '[' << i << "] = " << s << endl;
		});
	}

	// wait all the tasks
	for (int i = 0; i < totaljob; i++)
		tasks[i].join();

	auto end = chrono::high_resolution_clock::now();

	cout << "Excution time for " << totaljob << " jobs without pooling = \t" << (end - beg).count() << " ns" << endl;

}


void thread_pool_test(int poolsize, int totaljob, int jobsize)
{
	auto beg = chrono::high_resolution_clock::now();
	stdx::thread_pool pool(poolsize);
	vector<stdx::thread_pool::awaiter_t> tasks(totaljob);
	for (int i = 0; i < totaljob; i++)
	{
		tasks[i] = pool.create_thread([jobsize] {
			volatile int s = 0;
			for (int j = 0; j < jobsize; j++)
				s += j;
			//cout << '[' << i << "] = " << s << endl;
		});
	}

	// wait all the tasks
	for (int i = 0; i < totaljob; i++)
		tasks[i].wait();

	auto end = chrono::high_resolution_clock::now();

	cout << "Excution time for " << totaljob << " jobs in [" << poolsize << "] pool = \t" << (end - beg).count() << " ns" << endl;

}

//
//void seg_tree_test()
//{
//	seg_tree seg(14);
//	seg.insert(3, 8, 1);
//
//	cout << "tree == " << endl;
//	for (int i = 0; i < 14; i++)
//		cout << seg.get(i) << ' ';
//	cout << endl;
//
//	seg.insert(1, 5, 2);
//
//	cout << "tree == " << endl;
//	for (int i = 0; i < 14; i++)
//		cout << seg.get(i) << ' ';
//	cout << endl;
//}
//
//void bst_test()
//{
//	bst_map map;
//	map.try_insert(5, 3);
//	map.try_insert(3, 2);
//	cout << map.contains(5) << ' ';
//	cout << map.contains(3) << ' ';
//	cout << map.contains(2) << ' ';
//	map.erase(3);
//	cout << map.contains(3) << ' ';
//	cout << endl;
//}