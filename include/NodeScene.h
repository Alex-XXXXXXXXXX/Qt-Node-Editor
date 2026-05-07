#pragma once
#pragma execution_character_set("utf-8")
#include <QObject>
#include <QGraphicsScene>
#include "Node.h"

class NodeScene : public QGraphicsScene
{
	Q_OBJECT

public:
	NodeScene(QObject* parent = nullptr);

	void createNode(const QString& title, const QPointF& position);
	void createNode(const int& id, const QString& title, const QPointF& position);
	void removeNode(Node* node);
	void removeConnection(NodeConnection* connection);
	bool canConnect(NodeSocket* socket1, NodeSocket* socket2) const;
	NodeConnection* connectSockets(NodeSocket* startSocket, NodeSocket* endSocket);

	QVector<Node*> nodes() const { return m_nodes; }
	QVector<NodeConnection*> connections() const { return m_connections; }

	QList<Node*> TopologicalSorting();

protected:
	void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
	void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
	void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
	void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;

private:
	QVector<Node*> m_nodes;
	QVector<NodeConnection*> m_connections;
	NodeConnection* m_tempConnection;
	NodeSocket* m_startSocket;
	NodeSocket* m_endSocket;

private:
	bool hasCycleUtil(Node* node, QMap<Node*, bool>& visited, QMap<Node*, bool>& recStack);
	bool hasCycle();
	void onTaskCompleted(Node* node);

signals:
	void slot_OpenFunction(QString m_title);
};
