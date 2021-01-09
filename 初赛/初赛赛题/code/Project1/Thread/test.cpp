#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "MutexLock.hh"
#include "Thread.hh"
#include <unistd.h>
#include "Condition.hh"
#include "ThreadPool.hh"
#include <vector>

//threadPool test

MutexLock CntLock;

int cnt = 0;

void test(void){
	unsigned long i = 0xfffffff;
	//MutexLockGuard loo(CntLock);
	//CntLock.lock();
	while(i--);
	printf("%d\n", ++cnt);
	//CntLock.unlock();
	sleep(1);
}

int main()
{
//ThreadPool Test

	ThreadPool tp(5, 2);
	tp.start();

	sleep(3);
	for(int i = 0; i < 8; i++)
		tp.addTask(test);
		
	getchar();

	return 0;
}
