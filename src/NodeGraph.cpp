#include "include/NodeGraph.h"
#include <queue>

NodeGraph::NodeGraph(QObject* parent)
	: QObject(parent)
{
}

NodeGraph::~NodeGraph()
{
	clear();
}

void NodeGraph::clear()
{
	for (auto it = m_nodes.begin(); it != m_nodes.end(); ++it)
		delete it.value();
	m_nodes.clear();
}

NodeData* NodeGraph::addNode(int id, const QString& title)
{
	if (m_nodes.contains(id))
		return m_nodes[id];
	NodeData* data = new NodeData{ id, title, TaskNotStarted, {}, {} };
	m_nodes[id] = data;
	return data;
}

void NodeGraph::removeNode(int id)
{
	if (!m_nodes.contains(id))
		return;
	for (auto it = m_nodes.begin(); it != m_nodes.end(); ++it)
	{
		it.value()->inputIds.removeAll(id);
		it.value()->outputIds.removeAll(id);
	}
	delete m_nodes[id];
	m_nodes.remove(id);
}

bool NodeGraph::canConnect(int fromId, int toId) const
{
	if (fromId == toId)
		return false;
	if (!m_nodes.contains(fromId) || !m_nodes.contains(toId))
		return false;
	if (m_nodes[fromId]->outputIds.contains(toId))
		return false;
	return true;
}

bool NodeGraph::connectNodes(int outputId, int inputId)
{
	if (!canConnect(outputId, inputId))
		return false;
	m_nodes[outputId]->outputIds.append(inputId);
	m_nodes[inputId]->inputIds.append(outputId);
	if (hasCycle())
	{
		m_nodes[outputId]->outputIds.removeAll(inputId);
		m_nodes[inputId]->inputIds.removeAll(outputId);
		return false;
	}
	return true;
}

void NodeGraph::disconnectNodes(int outputId, int inputId)
{
	if (!m_nodes.contains(outputId) || !m_nodes.contains(inputId))
		return;
	m_nodes[outputId]->outputIds.removeAll(inputId);
	m_nodes[inputId]->inputIds.removeAll(outputId);
}

bool NodeGraph::hasCycleUtil(int nodeId, QMap<int, bool>& visited, QMap<int, bool>& recStack) const
{
	if (!visited[nodeId])
	{
		visited[nodeId] = true;
		recStack[nodeId] = true;
		for (int neighbor : m_nodes[nodeId]->outputIds)
		{
			if (!visited[neighbor] && hasCycleUtil(neighbor, visited, recStack))
				return true;
			else if (recStack[neighbor])
				return true;
		}
	}
	recStack[nodeId] = false;
	return false;
}

bool NodeGraph::hasCycle() const
{
	QMap<int, bool> visited;
	QMap<int, bool> recStack;
	for (auto it = m_nodes.begin(); it != m_nodes.end(); ++it)
		visited[it.key()] = false;
	for (auto it = m_nodes.begin(); it != m_nodes.end(); ++it)
	{
		if (hasCycleUtil(it.key(), visited, recStack))
			return true;
	}
	return false;
}

QList<int> NodeGraph::topologicalSort() const
{
	QList<int> result;
	if (hasCycle())
		return result;

	QMap<int, int> inDegree;
	for (auto it = m_nodes.begin(); it != m_nodes.end(); ++it)
		inDegree[it.key()] = it.value()->inputIds.size();

	std::queue<int> q;
	for (auto it = inDegree.begin(); it != inDegree.end(); ++it)
	{
		if (it.value() == 0)
			q.push(it.key());
	}

	while (!q.empty())
	{
		int current = q.front();
		q.pop();
		result.append(current);
		for (int neighbor : m_nodes[current]->outputIds)
		{
			inDegree[neighbor]--;
			if (inDegree[neighbor] == 0)
				q.push(neighbor);
		}
	}
	return result;
}

NodeData* NodeGraph::node(int id) const
{
	return m_nodes.value(id, nullptr);
}

QList<NodeData*> NodeGraph::allNodes() const
{
	return m_nodes.values();
}

QList<int> NodeGraph::inputNodes(int id) const
{
	if (!m_nodes.contains(id))
		return {};
	return m_nodes[id]->inputIds;
}

QList<int> NodeGraph::outputNodes(int id) const
{
	if (!m_nodes.contains(id))
		return {};
	return m_nodes[id]->outputIds;
}

void NodeGraph::setNodeStatus(int id, TaskStatus status)
{
	if (!m_nodes.contains(id))
		return;
	m_nodes[id]->status = status;
	emit nodeStatusChanged(id, status);
}

TaskStatus NodeGraph::nodeStatus(int id) const
{
	if (!m_nodes.contains(id))
		return TaskNotStarted;
	return m_nodes[id]->status;
}