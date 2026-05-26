#pragma once
#pragma execution_character_set("utf-8")
#include <QString>
#include <QList>

enum TaskStatus {
	TaskNotStarted,
	TaskRunning,
	TaskCompleted,
	TaskFailed,
	TaskJump
};

struct NodeData {
	int id;
	QString title;
	TaskStatus status = TaskNotStarted;
	QList<int> inputIds;
	QList<int> outputIds;
};
