#include "include/Task.h"
#include <QThread>

Task::Task(const int& id, CompleteFunc onComplete)
	: m_id(id), m_onComplete(onComplete)
{
	setAutoDelete(true);
}

void Task::run()
{
	if (m_id >= 0)
	{
		QThread::msleep(200);
		m_onComplete(true);
	}
	else
	{
		QThread::msleep(500);
		m_onComplete(false);
	}
}
