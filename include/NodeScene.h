#pragma once
#pragma execution_character_set("utf-8")
#include <QObject>
#include <QGraphicsScene>
#include <QMap>
#include "Node.h"
#include "NodeGraph.h"
#include "FlowEngine.h"

class NodeScene : public QGraphicsScene
{
	Q_OBJECT

public:
	NodeScene(QObject* parent = nullptr);
	~NodeScene();

	void createNode(const QString& title, const QPointF& position);
	void createNode(const int& id, const QString& title, const QPointF& position);
	void removeNode(Node* node);
	void removeConnection(NodeConnection* connection);
	bool canConnect(NodeSocket* socket1, NodeSocket* socket2) const;
	NodeConnection* connectSockets(NodeSocket* startSocket, NodeSocket* endSocket);

	QVector<Node*> nodes() const { return m_nodes; }
	QVector<NodeConnection*> connections() const { return m_connections; }

	QList<int> topologicalSort();
	void executeFlow();

	NodeGraph* graph() const { return m_graph; }

protected:
	void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
	void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
	void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
	void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;

private:
	QVector<Node*> m_nodes;
	QMap<int, Node*> m_nodeMap;
	QVector<NodeConnection*> m_connections;
	NodeConnection* m_tempConnection;
	NodeSocket* m_startSocket;
	NodeSocket* m_endSocket;
	NodeGraph* m_graph;
	FlowEngine* m_engine;

private:
	void onGraphNodeStatusChanged(int id, TaskStatus status);

signals:
	void slot_OpenFunction(QString m_title);
};
