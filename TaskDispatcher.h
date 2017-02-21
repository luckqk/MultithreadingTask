#pragma once
#include "../base/baseinc.h"

struct Task
{
	virtual ~Task() {}
	virtual void Execute() = 0;
};


class WorkThread;
class TaskDispatcher
{
public:
	TaskDispatcher() : workThreadNum(7/* Cpu ������*/){};
	~TaskDispatcher() {};

	std::stack<Task*> taskStack;
	std::vector<WorkThread*> workThreadPool;
	int workThreadNum;

	void PushTask(Task* task);
	Task* RequestTask();
	void Flush();
	void Wait();


	void Init();
	void Free();
};

class WorkThread
{
public:
	WorkThread(TaskDispatcher* inDispatcher) : dispatcher(inDispatcher) {}
	~WorkThread() {}

	TaskDispatcher* dispatcher;
	std::thread* _thread;
	bool exitThread;

	void Stop()
	{
		exitThread = true;
	}

	void Start()
	{
		exitThread = false;
		_thread = new std::thread(&WorkThread::Working, this);
		_thread->detach();

	}

	std::mutex finish_mux;
	std::condition_variable finish_cv;

	std::mutex wait_mux;
	std::condition_variable wait_cv;

	//ʵ���ڹ����߳������еĺ���
	// ������־��ʼ����֮ǰ����һֱ�ȴ���ֱ������־��ʼ������
	// ������ɺ��л���ɹ�����״̬
	bool doworking = false;
	void Working()
	{
		while (!exitThread)//�߳�ѭ��
		{
			std::unique_lock <std::mutex> lck_w(wait_mux);
			wait_cv.wait(lck_w);

			while (true)//����ѭ��
			{

				Task* task = dispatcher->RequestTask();
				if (!task)
				{
					//��������ѭ��
					break;
				}

				task->Execute();
				delete task;
				task = nullptr;
			}

			doworking = false;
			std::unique_lock <std::mutex> lck_f(finish_mux);
			finish_cv.notify_all();
		}

	}
};


struct TestTask : public Task
{
	TestTask(double inI, double inJ, double inK) : i(inI), j(inJ), k(inK) {}
	~TestTask() {}
	virtual void Execute() override;

	double i;
	double j;
	double k;

};

