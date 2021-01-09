#include"myThread.h"
Thread::Thread(const threadFun_t &threadRoutine, void *arg)
	:m_isRuning(false),
	 m_threadId(0),
	 m_threadRoutine(threadRoutine),
	 m_threadArg(arg){
}

Thread::~Thread(){
	if(m_isRuning){//如果线程正在执行，则分离此线程.
		CHECK(!pthread_detach(m_threadId));
	}
}

void *Thread::threadGuide(void *arg){
	Thread *p = static_cast<Thread *>(arg);
	p->m_threadRoutine(p->m_threadArg);
	return NULL;
}

void Thread::join(){
	VERIFY(m_isRuning);
	CHECK(!pthread_join(m_threadId, NULL));
	m_isRuning = false;
}

void Thread::start(){

	pthread_attr_t attr;

	CHECK(!pthread_attr_init(&attr));

	//CHECK(!pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED));    //set thread separation state property

	CHECK(!pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED));    //Set thread inheritance

	CHECK(!pthread_attr_setschedpolicy(&attr, SCHED_OTHER));                //set thread scheduling policy

	CHECK(!pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM));             //Set thread scope

	CHECK(!pthread_create(&m_threadId, &attr, threadGuide, this));

	m_isRuning = true;

}{
		MutexLockGuard lock(m_mutex);
		m_isRuning = false;
	}
}