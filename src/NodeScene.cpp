#include "include/NodeScene.h"
#include <QGraphicsSceneMouseEvent>
#include <QAction>
#include <QColorDialog>
#include <QInputDialog>
#include <QMenu>
#include <QList>
#include <queue>

NodeScene::NodeScene(QObject* parent)
	: QGraphicsScene(parent), m_tempConnection(nullptr), m_startSocket(nullptr)
{
	setBackgroundBrush(QBrush(QColor(240, 240, 240)));
	setSceneRect(0, 0, 2000, 2000);
	m_graph = new NodeGraph(this);
	m_engine = new FlowEngine(this);
	connect(m_graph, &NodeGraph::nodeStatusChanged, this, &NodeScene::onGraphNodeStatusChanged);
}

NodeScene::~NodeScene()
{
}

void NodeScene::createNode(const QString& title, const QPointF& position)
{
	int id = 0;
	QList<int> NodesId;
	for (Node* item : m_nodes)
	{
		NodesId.append(item->id());
	}
	std::sort(NodesId.begin(), NodesId.end());
	bool m_bCheck = true;
	for (int i = 0; i < NodesId.size(); i++)
	{
		if (i == NodesId[i])
		{
			continue;
		}
		else
		{
			id = i;
			m_bCheck = false;
			break;
		}
	}
	if (m_bCheck)
	{
		id = NodesId.size();
	}

	m_graph->addNode(id, title);

	Node* node = new Node(id, title);
	connect(node, &Node::slot_OpenFunction, this, &NodeScene::slot_OpenFunction);
	node->setPos(position);
	node->addInputSocket();
	node->addOutputSocket();
	addItem(node);
	m_nodes.append(node);
	m_nodeMap[id] = node;
}

void NodeScene::createNode(const int& id, const QString& title, const QPointF& position)
{
	m_graph->addNode(id, title);

	Node* node = new Node(id, title);
	connect(node, &Node::slot_OpenFunction, this, &NodeScene::slot_OpenFunction);
	node->setPos(position);
	node->addInputSocket();
	node->addOutputSocket();
	addItem(node);
	m_nodes.append(node);
	m_nodeMap[id] = node;
}

void NodeScene::removeNode(Node* node)
{
	if (!node || !m_nodes.contains(node)) return;

	QVector<NodeConnection*> connectionsToRemove;
	for (NodeConnection* connection : m_connections) {
		if (connection->startSocket()->parentItem() == node ||
			connection->endSocket()->parentItem() == node) {
			connectionsToRemove.append(connection);
		}
	}

	disconnect(node, &Node::slot_OpenFunction, this, &NodeScene::slot_OpenFunction);
	removeItem(node->inputSockets());
	removeItem(node->outputSockets());

	int nodeId = node->id();
	for (NodeConnection* connection : connectionsToRemove) {
		removeConnection(connection);
	}

	removeItem(node);
	m_nodes.removeOne(node);
	m_nodeMap.remove(nodeId);
	m_graph->removeNode(nodeId);
	delete node;
	node = nullptr;
}

void NodeScene::removeConnection(NodeConnection* connection)
{
	if (!connection || !m_connections.contains(connection)) return;

	NodeSocket* startSock = connection->startSocket();
	NodeSocket* endSock = connection->endSocket();
	if (startSock && endSock)
	{
		Node* startNode = qgraphicsitem_cast<Node*>(startSock->parentItem());
		Node* endNode = qgraphicsitem_cast<Node*>(endSock->parentItem());
		if (startNode && endNode)
		{
			m_graph->disconnectNodes(startNode->id(), endNode->id());
		}
	}

	if (connection->startSocket()) {
		connection->startSocket()->setConnected(false);
	}
	if (connection->endSocket()) {
		connection->endSocket()->setConnected(false);
	}
	removeItem(connection);
	m_connections.removeOne(connection);
	delete connection;
	connection = nullptr;
}

bool NodeScene::canConnect(NodeSocket* socket1, NodeSocket* socket2) const
{
	if (socket1->parentItem() == socket2->parentItem()) {
		return false;
	}
	if (socket1->socketType() == socket2->socketType()) {
		return false;
	}

	Node* node1 = qgraphicsitem_cast<Node*>(socket1->parentItem());
	Node* node2 = qgraphicsitem_cast<Node*>(socket2->parentItem());
	if (!node1 || !node2) return false;

	int fromId = node1->id();
	int toId = node2->id();
	if (socket1->socketType() == NodeSocket::Input)
		std::swap(fromId, toId);

	return m_graph->canConnect(fromId, toId);
}

NodeConnection* NodeScene::connectSockets(NodeSocket* startSocket, NodeSocket* endSocket)
{
	if (!canConnect(startSocket, endSocket)) {
		return nullptr;
	}
	NodeSocket* outputSocket = startSocket;
	NodeSocket* inputSocket = endSocket;
	if (startSocket->socketType() == NodeSocket::Input) {
		outputSocket = endSocket;
		inputSocket = startSocket;
	}

	Node* outNode = qgraphicsitem_cast<Node*>(outputSocket->parentItem());
	Node* inNode = qgraphicsitem_cast<Node*>(inputSocket->parentItem());
	if (!outNode || !inNode) return nullptr;

	if (!m_graph->connectNodes(outNode->id(), inNode->id()))
		return nullptr;

	NodeConnection* connection = new NodeConnection(outputSocket, inputSocket);
	addItem(connection);
	m_connections.append(connection);
	outputSocket->setConnected(true);
	inputSocket->setConnected(true);
	return connection;
}

QList<int> NodeScene::topologicalSort()
{
	return m_graph->topologicalSort();
}

void NodeScene::executeFlow()
{
	QList<int> sorted = m_graph->topologicalSort();
	if (sorted.isEmpty())
		return;

	for (auto it = m_nodeMap.begin(); it != m_nodeMap.end(); ++it)
	{
		it.value()->setResultColor(Qt::white);
		it.value()->setStatus(TaskNotStarted);
	}

	m_engine->execute(m_graph);
}

void NodeScene::onGraphNodeStatusChanged(int id, TaskStatus status)
{
	if (!m_nodeMap.contains(id))
		return;
	Node* node = m_nodeMap[id];
	node->setStatus(status);
	if (status == TaskCompleted)
		node->setResultColor(QColor(10, 191, 61));
	else if (status == TaskFailed)
		node->setResultColor(QColor(238, 0, 0));
}

void NodeScene::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
	if (event->button() == Qt::LeftButton) {
		QGraphicsItem* item = itemAt(event->scenePos(), QTransform());
		if (!item)
		{
			clearSelection();
			return;
		}
		if (m_tempConnection && m_startSocket)
		{
			NodeSocket* socket = qgraphicsitem_cast<NodeSocket*>(item);
			if (socket)
			{
				m_endSocket = socket;
				if (m_endSocket != m_startSocket) {
					connectSockets(m_startSocket, m_endSocket);
				}
				removeItem(m_tempConnection);
				delete m_tempConnection;
				m_tempConnection = nullptr;
				m_startSocket = nullptr;
				return;
			}
			else
			{
				removeItem(m_tempConnection);
				delete m_tempConnection;
				m_tempConnection = nullptr;
				m_startSocket = nullptr;
				return;
			}
		}
		else
		{
			QGraphicsItem* item = itemAt(event->scenePos(), QTransform());
			NodeSocket* socket = qgraphicsitem_cast<NodeSocket*>(item);
			if (socket) {
				m_startSocket = socket;
				m_tempConnection = new NodeConnection(socket, socket);
				m_tempConnection->setColor(Qt::gray);
				addItem(m_tempConnection);
				return;
			}
		}
	}
	QGraphicsScene::mousePressEvent(event);
}

void NodeScene::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
	if (m_tempConnection && m_startSocket) {
		QLineF line(m_startSocket->connectionPoint(), event->scenePos());
		m_tempConnection->setLine(line);
	}

	QGraphicsScene::mouseMoveEvent(event);
}

void NodeScene::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
	QGraphicsScene::mouseReleaseEvent(event);
}

void NodeScene::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
	QGraphicsItem* item = itemAt(event->scenePos(), QTransform());
	if (!item) {
		QMenu menu;
		QAction* createDefaultNodeAction = new QAction(QString::fromUtf8("创建默认节点"), this);
		connect(createDefaultNodeAction, &QAction::triggered, [this, event]() {
			createNode(QString::fromUtf8("节点"), event->scenePos());
		});
		QAction* createCustomNodeAction = new QAction(QString::fromUtf8("创建自定义节点"), this);
		connect(createCustomNodeAction, &QAction::triggered, [this, event]() {
			bool ok;
			QString title = QInputDialog::getText(nullptr, QString::fromUtf8("创建节点"),
				QString::fromUtf8("节点名称:"), QLineEdit::Normal, QString::fromUtf8("节点"), &ok);
			if (ok && !title.isEmpty()) {
				createNode(title, event->scenePos());
			}
		});
		menu.addAction(createDefaultNodeAction);
		menu.addAction(createCustomNodeAction);
		menu.exec(event->screenPos());
	}
	else if (qgraphicsitem_cast<NodeConnection*>(item)) {
		NodeConnection* connection = qgraphicsitem_cast<NodeConnection*>(item);
		QMenu menu;

		QAction* deleteAction = new QAction(QString::fromUtf8("删除连接"), this);
		connect(deleteAction, &QAction::triggered, [this, connection]() {
			removeConnection(connection);
		});
		QAction* changeColorAction = new QAction(QString::fromUtf8("更改颜色"), this);
		connect(changeColorAction, &QAction::triggered, [connection]() {
			QColor newColor = QColorDialog::getColor(connection->color(), nullptr, QString::fromUtf8("选择连接线颜色"));
			if (newColor.isValid()) {
				connection->setColor(newColor);
			}
		});
		menu.addAction(deleteAction);
		menu.addAction(changeColorAction);
		menu.exec(event->screenPos());
	}
	QGraphicsScene::contextMenuEvent(event);
}
