/*****************************************************************
版权声明:	(C), 2012-2013, Jovision Tech. Co., Ltd.

文件名  :	thread.cc

作者    :	张帅   版本: v1.0    创建日期: 2013-09-22

功能描述:	线程封装基类

其他说明:   其他线程继承此类，并实现Run

修改记录:
*****************************************************************/
#include "thread.h"
#include <pthread.h>

extern "C" {static void *start_thread(void *t);}

class CThreadPrivate
{
public:
	pthread_t m_nThreadID;

	CThreadPrivate():m_nThreadID(0)
	{
	}

	~CThreadPrivate()
	{
		m_nThreadID = 0;
	}

	bool Init(CThread *that)
	{
		pthread_attr_t attr;
		int ret;

		pthread_attr_init(&attr);
		pthread_attr_setinheritsched(&attr, PTHREAD_INHERIT_SCHED);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

		ret = pthread_create(&m_nThreadID, &attr, start_thread, (void *)that);

		pthread_attr_destroy(&attr);

		if (ret == 0)
			return true;
		else
			return false;
	}

	static void InternalRun(CThread *that)
	{
		that->Run();
		that->SetExitState();
	}
};

extern "C"
{
	static void *start_thread(void *t)
	{
		CThreadPrivate::InternalRun((CThread*)t);
		return 0;
	}
}

CThread::CThread()
{
	pthread_mutexattr_t mutexattr;
	pthread_mutexattr_init(&mutexattr);
	pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&lastTime_Lock_, &mutexattr);
	pthread_mutexattr_destroy(&mutexattr);

	pThreadPrivate_ = new CThreadPrivate;
}

CThread::~CThread()
{
	pthread_mutex_destroy(&lastTime_Lock_);

	if (pThreadPrivate_ != NULL)
		delete pThreadPrivate_;
}

bool CThread::Start(void *i_pParam)
{
	if (i_pParam != NULL)
	{
		pParam_ = i_pParam;
	}
	setRunFlag(true);
	return pThreadPrivate_->Init(this);
}

bool CThread::IsRunning()
{
	return pThreadPrivate_->m_nThreadID != 0 ? true : false;
}

void CThread::SetExitState()
{
	pThreadPrivate_->m_nThreadID = 0;
}

void CThread::Exit()
{
	setRunFlag(false);
	pthread_cancel(pThreadPrivate_->m_nThreadID);
	pThreadPrivate_->m_nThreadID = 0;
}

void* CThread::GetParam()
{
	return pParam_;
}
