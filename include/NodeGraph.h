#pragma once
#pragma execution_character_set("utf-8")
#include <QObject>
#include <QMap>
#include <QList>
#include "NodeData.h"

class NodeGraph : public QObject
{
	Q_OBJECT

public:
	explicit NodeGraph(QObject* parent = nullptr);
	~NodeGraph();

	NodeData* addNode(int id, const QString& title);
	void removeNode(int id);
	bool connectNodes(int outputId, int inputId);
	void disconnectNodes(int outputId, int inputId);

	bool canConnect(int fromId, int toId) const;
	bool hasCycle() const;
	QList<int> topologicalSort() const;

	NodeData* node(int id) const;
	QList<NodeData*> allNodes() const;
	QList<int> inputNodes(int id) const;
	QList<int> outputNodes(int id) const;

	void setNodeStatus(int id, TaskStatus status);
	TaskStatus nodeStatus(int id) const;

	void clear();

signals:
	void nodeStatusChanged(int id, TaskStatus status);

private:
	bool hasCycleUtil(int nodeId, QMap<int, bool>& visited, QMap<int, bool>& recStack) const;
	QMap<int, NodeData*> m_nodes;
};
