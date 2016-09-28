#include <thread>
#include <condition_variable>

#include <queue>
#include <functional>
#include <unordered_set>
#include <new>

using namespace std;

namespace stdx
{
	class thread_pool
	{
	public:
		using task_func_t = function<void(void)>;
	private:
		// holds a task's detail and metadata (id/priority)
		struct job_t
		{
			int			id;
			int			priority;
			task_func_t task;
			// since the priority queue is max-heap, we define the less operator in conversive way
			bool operator< (const job_t& rhs) const
			{
				return this->priority > rhs.priority ||
					this->priority == rhs.priority && this->id > rhs.id;
			}
		};

		// the struct to hold a 'physical' thread, and put it to work
		// thread will be in two state:
		// state 0: NEET, standby blocking as no work a submmited in the queue
		// state 1: WORK, excuting a job (the top in the work queue)
		struct worker_t
		{
		private:
			job_t		job;		// current job
			bool		has_job;	// flag signals if we have a work to do
			bool		terminated;	// use to terminate the thread
			thread_pool*pool;		// parent pool

		public:
			thread		runner;		// actual thread running the job

		public:
			explicit worker_t(thread_pool *_pool) // span the thread_proxy in blocked state
				: has_job(false), terminated(false), pool(_pool), runner(&worker_t::thread_runtime, this)
			{}
			// one should acquire a lock before calling this method!
			void terminate() { 
				terminated = true; 
			}

		private:
			void thread_runtime()
			{
				// loop to run the work until we signal its termination
				while (!terminated)
				{
					if (!wait_for_job()) return;

					try {
						job.task(); // do our work
					}
					catch (...) { // prevent exceptions destroy everything
						//std::cout << '[' << this_thread::get_id() << "] Exception occured within the worker thread..." << endl;
					}

					signal_finish();
				}
			}

			void signal_finish()
			{
				{
					lock_guard<mutex> lk(pool->awaiter_mtx);
					pool->finished_jobs.insert(job.id);
				} // release mutex lock, before notity
				pool->awaiter.notify_all(); // TODO, as all the caller thread are waiting 
			}

			// store the job into this->job if find one
			bool wait_for_job()
			{
				unique_lock<mutex> lk(pool->jobs_mtx);
				pool->job_notify.wait(lk, [this]() {
					return this->terminated || !pool->jobs.empty();
				});
				if (terminated)	return false;
				this->job = move(pool->jobs.top());
				pool->jobs.pop();
				return true;


				struct s_ptr {
					struct control_block {
						int ref_count;
					} *ctl;
					int * ptr;
					void make(int val)
					{
						char* buffer = (char*)malloc(sizeof(int) + sizeof(control_block));
						ptr = new(buffer)int(val);
						ctl = new(buffer + sizeof(int))control_block{0};
					}
				};

				std::vector<int> v;
				v.emplace_back(5);
				auto spi = make_shared<int>(5); // 1 malloc
				auto spi2 = shared_ptr<int>(new int(5)); // 2 malloc
			}
		};

	public:
		// A simple class simulate std::future<void>
		// That provide wait() method
		// Note: you can use std::packaged_task & std::future instead of customized awaiter
		struct awaiter_t
		{
			int jid;
			thread_pool *pool;

			bool is_done() const {
				//lock_guard<mutex> lk(pool->awaiter_mtx);
				return pool->finished_jobs.find(jid) != pool->finished_jobs.end();
			}

			// block the current thread until the task is finished
			// aka: join, await
			void wait()
			{
				unique_lock<mutex> lk(pool->awaiter_mtx);
				pool->awaiter.wait(lk, [this] {return this->is_done();});
			}

			void cancel() {}
		};

	protected:
		int					 jid;		 // timestep for jobs
		vector<worker_t>	 workers;	 // internal workers

		priority_queue<job_t>jobs;		 // queuing the income jobs
		mutex				 jobs_mtx;	 // protect the shared job queue
		condition_variable	 job_notify; // use to wake a neet thread

		unordered_set<int>	 finished_jobs;
		mutex				 awaiter_mtx;
		condition_variable	 awaiter;

	public:
		thread_pool(int capacity)
			: jid(0)
		{
			workers.reserve(capacity);
			for (int i = 0; i < capacity; i++)
				workers.emplace_back(this);
		}

		~thread_pool()
		{
			{
				lock_guard<mutex> lk(this->jobs_mtx);
				for (auto& t : workers)
					t.terminate();
			}
			// notity all NEET threads they are released
			job_notify.notify_all();
			// than we can join all the threads
			for (auto& t : workers)
				t.runner.join();
		}

		auto create_thread(task_func_t&& func, int priority = 2)
		{
			int cjid;
			{
				lock_guard<mutex> lk(this->jobs_mtx);
				cjid = jid;
				jobs.push(job_t{ jid++, priority, move(func) });
			}
			job_notify.notify_one();
			return awaiter_t{ cjid, this }; // cannot use jid-1 here
		}

		template <typename Iter>
		enable_if_t<is_same<typename Iter::value_type, awaiter_t>::value>
			wait_all(Iter _begin, Iter _end)
		{
			// TODO
		}

	};
}