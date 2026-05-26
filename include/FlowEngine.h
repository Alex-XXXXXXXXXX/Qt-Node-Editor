#pragma once
#include <QObject>
#include <QThreadPool>
#include "NodeGraph.h"

class FlowEngine : public QObject
{
	Q_OBJECT

public:
	explicit FlowEngine(QObject* parent = nullptr);

	void execute(NodeGraph* graph);

private:
	void executeNext(NodeGraph* graph, int nodeId);
	QThreadPool* m_threadPool;
};
