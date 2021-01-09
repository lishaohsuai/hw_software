#include <assert.h>

//#include "Logger.hh" // debug
//#include "CurrentThread.hh" // debug
#include "ThreadPool.hh"

ThreadPool::ThreadPool()
  :m_mutex(),
  m_cond(m_mutex),
  m_isStarted(false)
{

}

ThreadPool::~ThreadPool()
{
  if(m_isStarted)
  {
    stop();
  }
}

void ThreadPool::start()
{
  assert(m_threads.empty());
  m_isStarted = true;
  m_threads.reserve(kInitThreadsSize);
  for (int i = 0; i < kInitThreadsSize; ++i)
  {
    m_threads.push_back(new std::thread(std::bind(&ThreadPool::threadLoop, this)));
  }

}

void ThreadPool::stop()
{
  LOG_TRACE << "ThreadPool::stop() stop.";
  {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_isStarted = false;
    m_cond.notifyAll();
    LOG_TRACE << "ThreadPool::stop() notifyAll().";
  }

  for (Threads::iterator it = m_threads.begin(); it != m_threads.end() ; ++it)
  {
    (*it)->join();
    delete *it;
  }
  m_threads.clear();
}


void ThreadPool::threadLoop()
{
  LOG_TRACE << "ThreadPool::threadLoop() tid : " << CurrentThread::tid() << " start.";
  while(m_isStarted)
  {
    Task task = take();
    if(task)
    {
      task();
    }
  }
  LOG_TRACE << "ThreadPool::threadLoop() tid : " << CurrentThread::tid() << " exit.";
}

void ThreadPool::addTask(const Task& task)
{
  std::unique_lock<std::mutex> lock(m_mutex);
  /*while(m_tasks.isFull())
    {//when m_tasks have maxsize
      cond2.wait();
    }
  */
  TaskPair taskPair(level2, task);
  m_tasks.push(taskPair);
  m_cond.notify();
}

void ThreadPool::addTask(const TaskPair& taskPair)
{
  std::unique_lock<std::mutex> lock(m_mutex);
  /*while(m_tasks.isFull())
    {//when m_tasks have maxsize
      cond2.wait();
    }
  */
  m_tasks.push(taskPair);
  m_cond.notify();
}

ThreadPool::Task ThreadPool::take()
{
  std::unique_lock<std::mutex> lock(m_mutex);
  //always use a while-loop, due to spurious wakeup
  while(m_tasks.empty() && m_isStarted)
  {
    LOG_TRACE << "ThreadPool::take() tid : " << CurrentThread::tid() << " wait.";
    m_cond.wait(lock);
  }

  LOG_TRACE << "ThreadPool::take() tid : " << CurrentThread::tid() << " wakeup.";

  Task task;
  Tasks::size_type size = m_tasks.size();
  if(!m_tasks.empty() && m_isStarted)
  {
    task = m_tasks.top().second;
    m_tasks.pop();
    assert(size - 1 == m_tasks.size());
    /*if (TaskQueueSize_ > 0)
    {
      cond2.notify();
    }*/
  }

  return task;

}