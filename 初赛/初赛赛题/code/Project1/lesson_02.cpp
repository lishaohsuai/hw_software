/*
 * @Author: your name
 * @Date: 2020-04-17 10:43:11
 * @LastEditTime: 2020-04-17 10:43:42
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \code\lesson_1.cpp
 */
#include <iostream>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <fstream>
#include <time.h>
#include <thread>
#include "threadPool.h"
#define TEST
#define _CRT_SECURE_NO_WARNINGS 
//===================================================================
#ifndef _thread_pool_HPP
#define _thread_pool_HPP

#include <vector>
#include <deque>
#include <thread>
#include <functional>
#include <mutex>
#include <condition_variable>

//!
//! convenience macro to log with file and line information
//!
#ifdef __SOLA_LOGGING_ENABLED
#define __SOLA_LOG(level, msg) sola::level(msg, __FILE__, __LINE__);
#else
#define __SOLA_LOG(level, msg)
#endif /* __SOLA_LOGGING_ENABLED */

namespace sola {

class logger_iface {
public:
  //! ctor
  logger_iface(void) = default;
  //! dtor
  virtual ~logger_iface(void) = default;

  //! copy ctor
  logger_iface(const logger_iface&) = default;
  //! assignment operator
  logger_iface& operator=(const logger_iface&) = default;

public:
  //!
  //! debug logging
  //!
  //! \param msg message to be logged
  //! \param file file from which the message is coming
  //! \param line line in the file of the message
  //!
  virtual void debug(const std::string& msg, const std::string& file, std::size_t line) = 0;

  //!
  //! info logging
  //!
  //! \param msg message to be logged
  //! \param file file from which the message is coming
  //! \param line line in the file of the message
  //!
  virtual void info(const std::string& msg, const std::string& file, std::size_t line) = 0;

  //!
  //! warn logging
  //!
  //! \param msg message to be logged
  //! \param file file from which the message is coming
  //! \param line line in the file of the message
  //!
  virtual void warn(const std::string& msg, const std::string& file, std::size_t line) = 0;

  //!
  //! error logging
  //!
  //! \param msg message to be logged
  //! \param file file from which the message is coming
  //! \param line line in the file of the message
  //!
  virtual void error(const std::string& msg, const std::string& file, std::size_t line) = 0;
};

//!
//! default logger class provided by the library
//!
class logger : public logger_iface {
public:
  //!
  //! log level
  //!
  enum class log_level {
    error = 0,
    warn  = 1,
    info  = 2,
    debug = 3
  };

public:
  //! ctor
  logger(log_level level = log_level::info);
  //! dtor
  ~logger(void) = default;

  //! copy ctor
  logger(const logger&) = default;
  //! assignment operator
  logger& operator=(const logger&) = default;

public:
  //!
  //! debug logging
  //!
  //! \param msg message to be logged
  //! \param file file from which the message is coming
  //! \param line line in the file of the message
  //!
  void debug(const std::string& msg, const std::string& file, std::size_t line);

  //!
  //! info logging
  //!
  //! \param msg message to be logged
  //! \param file file from which the message is coming
  //! \param line line in the file of the message
  //!
  void info(const std::string& msg, const std::string& file, std::size_t line);

  //!
  //! warn logging
  //!
  //! \param msg message to be logged
  //! \param file file from which the message is coming
  //! \param line line in the file of the message
  //!
  void warn(const std::string& msg, const std::string& file, std::size_t line);

  //!
  //! error logging
  //!
  //! \param msg message to be logged
  //! \param file file from which the message is coming
  //! \param line line in the file of the message
  //!
  void error(const std::string& msg, const std::string& file, std::size_t line);

private:
  //!
  //! current log level in use
  //!
  log_level m_level;

  //!
  //! mutex used to serialize logs in multithreaded environment
  //!
  std::mutex m_mutex;
};

//!
//! variable containing the current logger
//! by default, not set (no logs)
//!
extern std::unique_ptr<logger_iface> active_logger;

//!
//! debug logging
//! convenience function used internally to call the logger
//!
//! \param msg message to be logged
//! \param file file from which the message is coming
//! \param line line in the file of the message
//!
void debug(const std::string& msg, const std::string& file, std::size_t line);

//!
//! info logging
//! convenience function used internally to call the logger
//!
//! \param msg message to be logged
//! \param file file from which the message is coming
//! \param line line in the file of the message
//!
void info(const std::string& msg, const std::string& file, std::size_t line);

//!
//! warn logging
//! convenience function used internally to call the logger
//!
//! \param msg message to be logged
//! \param file file from which the message is coming
//! \param line line in the file of the message
//!
void warn(const std::string& msg, const std::string& file, std::size_t line);

//!
//! error logging
//! convenience function used internally to call the logger
//!
//! \param msg message to be logged
//! \param file file from which the message is coming
//! \param line line in the file of the message
//!
void error(const std::string& msg, const std::string& file, std::size_t line);


class thread_pool{
public:
  typedef std::function<void()> task_t;

  thread_pool(int init_size = 3);
  ~thread_pool();

  void stop();
  void add_task(const task_t&);  //thread safe;

private:
  thread_pool(const thread_pool&);//禁止复制拷贝.
  const thread_pool& operator=(const thread_pool&);
  
  bool is_started() { return m_is_started; }
  void start();

  void thread_loop();
  task_t take();

  typedef std::vector<std::thread*> threads_t;
  typedef std::deque<task_t> tasks_t;

  int m_init_threads_size;

  threads_t m_threads;
  tasks_t m_tasks;

  std::mutex m_mutex;
  std::condition_variable m_cond;
  bool m_is_started;
};

}
//===================================================================
#include <assert.h>
#include <iostream>
#include <sstream>
// #include "thread_pool.hpp"

namespace sola {

std::unique_ptr<logger_iface> active_logger = nullptr;

static const char black[]  = {0x1b, '[', '1', ';', '3', '0', 'm', 0};
static const char red[]    = {0x1b, '[', '1', ';', '3', '1', 'm', 0};
static const char yellow[] = {0x1b, '[', '1', ';', '3', '3', 'm', 0};
static const char blue[]   = {0x1b, '[', '1', ';', '3', '4', 'm', 0};
static const char normal[] = {0x1b, '[', '0', ';', '3', '9', 'm', 0};

logger::logger(log_level level)
: m_level(level) {}

void
logger::debug(const std::string& msg, const std::string& file, std::size_t line) {
  if (m_level >= log_level::debug) {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::cout << "[" << black << "DEBUG" << normal << "][sola::logger][" << file << ":" << line << "] " << msg << std::endl;
  }
}

void
logger::info(const std::string& msg, const std::string& file, std::size_t line) {
  if (m_level >= log_level::info) {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::cout << "[" << blue << "INFO " << normal << "][sola::logger][" << file << ":" << line << "] " << msg << std::endl;
  }
}

void
logger::warn(const std::string& msg, const std::string& file, std::size_t line) {
  if (m_level >= log_level::warn) {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::cout << "[" << yellow << "WARN " << normal << "][sola::logger][" << file << ":" << line << "] " << msg << std::endl;
  }
}

void
logger::error(const std::string& msg, const std::string& file, std::size_t line) {
  if (m_level >= log_level::error) {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::cerr << "[" << red << "ERROR" << normal << "][sola::logger][" << file << ":" << line << "] " << msg << std::endl;
  }
}

void
debug(const std::string& msg, const std::string& file, std::size_t line) {
  if (active_logger)
    active_logger->debug(msg, file, line);
}

void
info(const std::string& msg, const std::string& file, std::size_t line) {
  if (active_logger)
    active_logger->info(msg, file, line);
}

void
warn(const std::string& msg, const std::string& file, std::size_t line) {
  if (active_logger)
    active_logger->warn(msg, file, line);
}

void
error(const std::string& msg, const std::string& file, std::size_t line) {
  if (active_logger)
    active_logger->error(msg, file, line);
}

static std::string
get_tid(){
  std::stringstream tmp;
  tmp << std::this_thread::get_id();
  return tmp.str();
}

thread_pool::thread_pool(int init_size)
  :m_init_threads_size(init_size),
  m_mutex(),
  m_cond(),
  m_is_started(false)
{
  start();
}

thread_pool::~thread_pool()
{
  if(m_is_started)
  {
    stop();
  }
}

void thread_pool::start()
{
  assert(m_threads.empty());
  m_is_started = true;
  m_threads.reserve(m_init_threads_size);
  for (int i = 0; i < m_init_threads_size; ++i)
  {
    m_threads.push_back(new std::thread(std::bind(&thread_pool::thread_loop, this)));
  }

}

void thread_pool::stop()
{
  __SOLA_LOG(debug, "thread_pool::stop() stop.");
  {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_is_started = false;
    m_cond.notify_all();
    __SOLA_LOG(debug, "thread_pool::stop() notifyAll().");
  }

  for (threads_t::iterator it = m_threads.begin(); it != m_threads.end() ; ++it)
  {
    (*it)->join();
    delete *it;
  }
  m_threads.clear();
}


void thread_pool::thread_loop()
{
  __SOLA_LOG(debug, "thread_pool::threadLoop() tid : " + get_tid() + " start.");
  while(m_is_started)
  {
    task_t task = take();
    if(task)
    {
      task();
    }
  }
  __SOLA_LOG(debug, "thread_pool::threadLoop() tid : " + get_tid() + " exit.");
}

void thread_pool::add_task(const task_t& task)
{
  std::unique_lock<std::mutex> lock(m_mutex);
  /*while(m_tasks.isFull())
    {//when m_tasks have maxsize
      cond2.notify_one();
    }
  */
  m_tasks.push_back(task);
  m_cond.notify_one();
}

thread_pool::task_t thread_pool::take()
{
  std::unique_lock<std::mutex> lock(m_mutex);
  //always use a while-loop, due to spurious wakeup
  while(m_tasks.empty() && m_is_started)
  {
    __SOLA_LOG(debug, "thread_pool::take() tid : " + get_tid() + " wait.");
    m_cond.wait(lock);
  }

  __SOLA_LOG(debug, "thread_pool::take() tid : " + get_tid() + " wakeup.");

  task_t task;
  tasks_t::size_type size = m_tasks.size();
  if(!m_tasks.empty() && m_is_started)
  {
    task = m_tasks.front();
    m_tasks.pop_front();
    assert(size - 1 == m_tasks.size());
    /*if (TaskQueueSize_ > 0)
    {
      cond2.notify_one();
    }*/
  }

  return task;

}

}
//===================================================================
std::mutex g_mutex;
//===================================================================
// 防止fopen 和 fscanf 报错

struct Path {
	//ID最小的第一个输出；
	//总体按照循环转账路径长度升序排序；
	//同一级别的路径长度下循环转账账号ID序列，按照字典序（ID转为无符号整数后）升序排序
	int length;
	std::vector<unsigned int> path;

	Path(int length, const std::vector<unsigned int> &path) : length(length), path(path) {}

	bool operator<(const Path&rhs)const {//在排序的时候有用
		if (length != rhs.length) return length < rhs.length; // length 从小到大
		for (int i = 0; i < length; i++) {
			if (path[i] != rhs.path[i])// 字典序
				return path[i] < rhs.path[i];
		}
	}
};

class Solution {
public:
	//maxN=560000
	//maxE=280000 ~avgN=26000
	//vector<int> *G;
	std::vector<std::vector<int>> G;// 定义二维图
	std::unordered_map<unsigned int, int> idHash; //sorted id to 0...n
	std::vector<unsigned int> ids; //0...n to sorted id
	std::vector<unsigned int> inputs; //u-v pairs  先存储账号A 再存储账号B
	std::vector<int> inDegrees;
	std::vector<bool> vis;
	std::vector<Path> ans;
	int nodeCnt;
	sola::thread_pool thread_pool;
	void parseInput(std::string &testFile) {
		FILE* file = fopen(testFile.c_str(), "r");
		unsigned int u, v, c;
		int cnt = 0;
		while (fscanf(file, "%u,%u,%u", &u, &v, &c) != EOF) {
			inputs.push_back(u);
			inputs.push_back(v);
			++cnt;
		}
#ifdef TEST
		printf("%d Records in Total\n", cnt);// 输出总共存储了多少条数据
#endif
	}

	void constructGraph() {
		auto tmp = inputs;
		sort(tmp.begin(), tmp.end());// 对所有id 进行排序是什么意思？？
#ifdef TEST
		std::cout << "开始输出数据" << tmp.size() << std::endl;
		for (int i = 0; i < 10; i++) {
			std::cout << tmp[i] << std::endl;
		}
		std::cout << "结束输出数据" << std::endl;
#endif
		tmp.erase(unique(tmp.begin(), tmp.end()), tmp.end());//独一无二的tmp了
#ifdef TEST
		std::cout << "开始输出数据" << tmp.size() << std::endl;
		for (int i = 0; i < 10; i++) {
			std::cout << tmp[i] << std::endl;
		}
		std::cout << "结束输出数据" << std::endl;
#endif
		nodeCnt = tmp.size();
		ids = tmp;
		nodeCnt = 0;
		for (unsigned int &x : tmp) {
			idHash[x] = nodeCnt++;//map 中存储对应的账号id 所映射的位置？？？
		}
#ifdef TEST
		printf("%d Nodes in Total\n", nodeCnt);
#endif
		int sz = inputs.size();
		//G=new vector<int>[nodeCnt];
		G = std::vector<std::vector<int>>(nodeCnt);
		inDegrees = std::vector<int>(nodeCnt, 0);// 对于每一个节点的入度
		for (int i = 0; i < sz; i += 2) {
			int u = idHash[inputs[i]], v = idHash[inputs[i + 1]];
			G[u].push_back(v);//u节点指向v节点
			++inDegrees[v];// v节点的入度增加
		}
	}

	void dfs(int head, int cur, int depth, std::vector<int> &path) {
		vis[cur] = true;//当前节点已经访问过
		path.push_back(cur);//路径中存入当前节点
		for (int &v : G[cur]) {
			if (v == head && depth >= 3 && depth <= 7) { // 如果当前节点的一个出度是头结点，那么就是一个答案
				std::vector<unsigned int> tmp;
				for (int &x : path)
					tmp.push_back(ids[x]);
				ans.emplace_back(Path(depth, tmp));//等于push_back 不会触发 不需要触发拷贝构造和转移构造
			}
			if (depth<7 && !vis[v] && v>head) {
				dfs(head, v, depth + 1, path);
			}
		}
		vis[cur] = false;
		path.pop_back();
	}

	//search from 0...n
	//由于要求id最小的在前，因此搜索的全过程中不考虑比起点id更小的节点
	void solve() {
		vis = std::vector<bool>(nodeCnt, false);
		std::vector<int> path;
		for (int i = 0; i < nodeCnt; i++) {
			// if (i % 100 == 0)
			// 	std::cout << i << "/" << nodeCnt << std::endl;//每100个节点输出一个字符
			if (!G[i].empty()) {// 如果节点i对应的节点 有出度的话开启这个节点的dfs
				dfs(i, i, 1, path);
			}
			if(!G[i].empty()){
				thread_pool.add_task(testFunc);
			}
		}
		sort(ans.begin(), ans.end());
	}

	void save(std::string &outputFile) {
		printf("Total Loops %d\n", (int)ans.size());
		std::ofstream out(outputFile);
		out << ans.size() << std::endl;
		for (auto &x : ans) {
			auto path = x.path;
			int sz = path.size();
			out << path[0];
			for (int i = 1; i < sz; i++)
				out << "," << path[i];
			out << std::endl;
		}
	}
};
int main()
{

    sola::thread_pool thread_pool;

  	for(int i = 0; i < 5 ; i++)
    	thread_pool.add_task(testFunc);

	std::string testFile = "/home/lee/桌面/code/HW_code/初赛/初赛赛题/test_data.txt";
	std::string outputFile = "/home/lee/桌面/code/HW_code/初赛/初赛赛题/result_data.txt";
#ifdef TEST
	std::string answerFile = "C:/Users/lee/Desktop/code/华为软件精英挑战赛/初赛/初赛赛题/result.txt";
#endif
	auto t = clock();
	//    for(int i=0;i<100;i++){
	Solution solution;// 构建一个解决方案类
	solution.parseInput(testFile); // 读取数据将 id1,id2,money  id1和id2 存储在 std::vector<unsigned int> inputs; 
	solution.constructGraph();// 构建图
	//solution.topoSort();
	solution.solve();//解决它
	solution.save(outputFile);
	std::cout << clock() - t << std::endl;
	//    }
	system("pause");
	return 0;
}


