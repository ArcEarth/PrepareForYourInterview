#include <algorithm>
#include <vector>
#include <string>
#include <iostream>
#include <set>
#include <memory>
#include <cassert>
#include <chrono>

#include "thread_pool.h"
//#include "seg_tree.h"
//#include "bst_map.h"

using namespace std;
using namespace stdx;

void thread_pool_test(int poolsize, int totaljob, int jobsize);
void threads_no_pooling_test(int totaljob, int jobsize);
void bst_test();
void seg_tree_test();


int main()
{
	//seg_tree_test();
	//bst_test();

	int poolsize = 4;
	int totaljob = 1000;
	int jobsize =  10000;
	
	//thread_pool_test(1, totaljob, jobsize);
	//thread_pool_test(2, totaljob, jobsize);
	thread_pool_test(4, totaljob, jobsize);
	threads_no_pooling_test(totaljob, jobsize);
	//thread_pool_test(8, totaljob, jobsize);
	//thread_pool_test(16, totaljob, jobsize);
	//thread_pool_test(32, totaljob, jobsize);

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