/*****************************************************************
版权声明:	(C), 2012-2013, Jovision Tech. Co., Ltd.

文件名  :	thread.h

作者    :	张帅   版本: v1.0    创建日期: 2013-09-22

功能描述:	线程封装基类

其他说明:   其他线程继承此类，并实现Run

修改记录:
*****************************************************************/
#ifndef THREAD_H_
#define THREAD_H_

#include <pthread.h>
#include <time.h>

class CThreadPrivate;

class CThread
{
public:
	CThread();
	virtual ~CThread();

	// 启动线程
	bool Start(void *i_pParam=NULL);

	// 退出线程
	void Exit();

	// 得到线程参数
	void* GetParam();

	// 是否在运行
	bool IsRunning();

	// 线程运行的纯虚函数
	virtual void Run()=0;

	// 设置退出标志
	void SetExitState();
	CThreadPrivate* GetPrivatePtr();

	void setLastTime(time_t& t)
	{
		pthread_mutex_lock(&lastTime_Lock_);
		lastTime_ = t;
		pthread_mutex_unlock(&lastTime_Lock_);
	}

	time_t getLastTime()
	{
		time_t t;
		pthread_mutex_lock(&lastTime_Lock_);
		t = lastTime_;
		pthread_mutex_unlock(&lastTime_Lock_);
		return t;
	}

	void setRunFlag(bool b)
	{
		runFlag_ = b;
	}

	bool getRunFlag()
	{
		return runFlag_;
	}

private:
	// 线程的具体实现
	CThreadPrivate *pThreadPrivate_;

	// 线程参数
	void *pParam_;

	// 刷新时间
	time_t lastTime_;
	pthread_mutex_t lastTime_Lock_;

	bool runFlag_;
};

#endif
