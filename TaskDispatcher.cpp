#include "TaskDispatcher.h"


void TaskDispatcher::PushTask(Task* task)
{
	taskStack.push(task);
}

std::mutex glock_RequestTask;
Task* TaskDispatcher::RequestTask()
{
	Task* task = nullptr;

	glock_RequestTask.lock();
	if (!taskStack.empty())
	{
		task = taskStack.top();
		taskStack.pop();
	}
	glock_RequestTask.unlock();

	return task;
}

//���߳�֪ͨ���й����̣߳���ʼFlush����
void TaskDispatcher::Flush()
{
	std::vector<WorkThread*>::iterator it = workThreadPool.begin();
	for (; it != workThreadPool.end(); ++it)
	{
		std::unique_lock <std::mutex> lck((*it)->wait_mux);
		(*it)->wait_cv.notify_all();
		(*it)->doworking = true;
	}

}

//���̵߳ȴ����й����߳�Flush��������
void TaskDispatcher::Wait()
{
	std::vector<WorkThread*>::iterator it = workThreadPool.begin();
	for (; it != workThreadPool.end(); ++it)
	{
		std::unique_lock <std::mutex> lck((*it)->finish_mux);
		if ((*it)->doworking == true)
		{
			(*it)->finish_cv.wait(lck);

		}
	}
	//std::cout << "��������ִ�����" << std::endl;
}

void TaskDispatcher::Init()
{
	//�����й����߳���������
	for (int i = 0; i < workThreadNum; ++i)
	{
		workThreadPool.push_back(new WorkThread(this));
	}

	std::vector<WorkThread*>::iterator it = workThreadPool.begin();
	for (; it != workThreadPool.end(); ++it)
	{
		(*it)->Start();
	}
}

void TaskDispatcher::Free()
{
	std::vector<WorkThread*>::iterator it = workThreadPool.begin();
	for (; it != workThreadPool.end(); ++it)
	{
		(*it)->Stop();
		delete (*it);
	}
}


void TestTask::Execute()
{

	int count = 0;
	while (count < 1000)
	{
		i = i + j + k;

		count++;
	}

	double num = i;

	printf("[thread:%d] > num: %f\n", std::this_thread::get_id(), num);

}
