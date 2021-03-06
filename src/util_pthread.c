/**
 * @file util_pthread.c
 * @author 余王亮 (wotsen@outlook.com)
 * @brief 
 * @version 0.1
 * @date 2019-11-02
 * 
 * @copyright Copyright (c) 2019
 * 
 */
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/prctl.h>
#include "util_pthread.h"

/**
 * @brief 创建线程
 * 
 * @param tid 线程id
 * @param stacksize 线程栈大小
 * @param priority 线程优先级
 * @param fn 线程人物接口
 * @param arg 传递给线程的参数
 * @return true 创建成功
 * @return false 创建失败
 */
bool create_thread(pthread_t *tid, const size_t stacksize,
					const int priority, thread_func fn, void *arg)
{
	pthread_t _tid = INVALID_PTHREAD_TID;
	pthread_attr_t attr;
	struct sched_param param;
	int min_pri = sched_get_priority_min(SCHED_RR);
	int max_pri = sched_get_priority_max(SCHED_RR);
	size_t _stacksize = stacksize;
	int _pri = priority;

	memset(&attr, 0, sizeof(attr));
	memset(&param, 0, sizeof(param));

	/* 矫正线程栈 */
	_stacksize = _stacksize < PTHREAD_STACK_MIN ? PTHREAD_STACK_MIN : _stacksize;

	/* 矫正优先级 */
	if (min_pri > _pri)
	{
		_pri = min_pri;
	}
	else if (max_pri < _pri)
	{
		_pri = max_pri;
	}
	else
	{
		// PASS
	}

	param.sched_priority = _pri;

	if (pthread_attr_init(&attr) < 0)
	{
		printf("init thread attr error : %s", strerror(errno));
		return false;
	}

	/* 设置线程分离 */
	if (pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) < 0)
	{
		printf("set attr detachstate error : %s", strerror(errno));
		pthread_attr_destroy(&attr);
		return false;
	}

	/* 设置调度策略FIFO */
	if (pthread_attr_setschedpolicy(&attr, SCHED_RR) < 0)
	{
		printf("set attr schedpolicy error : %s\n", strerror(errno));
		pthread_attr_destroy(&attr);
        return false;
	}

	/* 设置线程优先级 */
	if (pthread_attr_setschedparam(&attr, &param) < 0)
	{
		printf("set attr schedparam error : %s\n", strerror(errno));
		pthread_attr_destroy(&attr);
        return false;
	}

	/* 设置线程栈大小 */
	if (pthread_attr_setstacksize(&attr, stacksize) < 0)
    {
        printf("set attr stacksize error : %s\n", strerror(errno));
		pthread_attr_destroy(&attr);
        return false;
    }

	/* 创建线程 */
	if (pthread_create(&_tid, &attr, fn, arg) < 0)
	{
		printf("pthread_create failed : %s\n", strerror(errno));
		pthread_attr_destroy(&attr);
		return false;
	}

	if (NULL != tid)
	{
		*tid = _tid;
	}

	pthread_attr_destroy(&attr);

	return true;
}

/**
 * @brief 设置线程名称
 * 
 * @param name 
 */
void set_thread_name(const char *name)
{
	char pname[MAX_THREAD_NAME_LEN + 1] = {'\0'};
	pthread_t tid = pthread_self();

	if (NULL != name)
    {
        sprintf(pname, "%s", (char *)name);
        prctl(PR_SET_NAME, pname);
    }
    else
    {
        sprintf(pname, "p%zu", tid);
        prctl(PR_SET_NAME, pname);
    }
}

/**
 * @brief 释放线程
 * 
 * @param tid 线程号
 * @return true 释放成功
 * @return false 释放失败
 */
bool release_thread(const pthread_t tid)
{
	if (if_thread_exsit(tid))
	{
		return pthread_cancel(tid) == 0;
	}

	return true;
}

/**
 * @brief 检测线程存活
 * 
 * @param tid 线程号
 * @return true 线程存在
 * @return false 线程不存在
 */
bool if_thread_exsit(const pthread_t tid)
{
	if (INVALID_PTHREAD_TID == tid)
	{
		return false;
	}

	int pthread_kill_err = pthread_kill(tid, 0);

	if (ESRCH == pthread_kill_err)
    {
        printf("task=[%ld] is not exist!\n", tid);
        return false;
    }
    else if (EINVAL == pthread_kill_err)
    {
        printf("task=[%ld], send invalid signal to it!/n", tid);
        return false;
    }
    else
    {
        return true;
    }
}