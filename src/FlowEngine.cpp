#include "include/FlowEngine.h"
#include "include/Task.h"
#include <QRandomGenerator>

FlowEngine::FlowEngine(QObject* parent)
	: QObject(parent)
{
	m_threadPool = QThreadPool::globalInstance();
}

void FlowEngine::execute(NodeGraph* graph)
{
	if (!graph)
		return;

	QList<int> sorted = graph->topologicalSort();
	if (sorted.isEmpty())
		return;

	for (int id : sorted)
	{
		graph->setNodeStatus(id, TaskNotStarted);
	}

	for (int id : sorted)
	{
		QList<int> inputs = graph->inputNodes(id);
		if (inputs.isEmpty())
		{
			executeNext(graph, id);
		}
	}
}

void FlowEngine::executeNext(NodeGraph* graph, int nodeId)
{
	NodeData* data = graph->node(nodeId);
	if (!data || data->status != TaskNotStarted)
		return;

	for (int inputId : graph->inputNodes(nodeId))
	{
		TaskStatus s = graph->nodeStatus(inputId);
		if (s != TaskCompleted && s != TaskFailed)
			return;
	}

	graph->setNodeStatus(nodeId, TaskRunning);

	QRandomGenerator* generator = QRandomGenerator::global();
	int min = -10;
	int max = 10;
	int rangeInt = generator->bounded(min, max + 1);

	Task* task = new Task(rangeInt,
		[this, graph, nodeId](const bool& result) {
		QMetaObject::invokeMethod(this, [this, graph, nodeId, result]() {
			if (result)
				graph->setNodeStatus(nodeId, TaskCompleted);
			else
				graph->setNodeStatus(nodeId, TaskFailed);

			for (int outputId : graph->outputNodes(nodeId))
			{
				executeNext(graph, outputId);
			}
		}, Qt::QueuedConnection);
	}
	);
	m_threadPool->start(task);
}