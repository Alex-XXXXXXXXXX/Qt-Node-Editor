#pragma once
#include <QRunnable>
#include <functional>

class Task : public QRunnable
{
public:
	using CompleteFunc = std::function<void(const bool&)>;

	Task(const int& id, CompleteFunc onComplete);
	void run() override;

private:
	int m_id;
	CompleteFunc m_onComplete;
};
