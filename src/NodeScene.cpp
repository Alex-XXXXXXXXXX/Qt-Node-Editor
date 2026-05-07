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
	Node* node = new Node(id, title);
	connect(node, &Node::slot_OpenFunction, this, &NodeScene::slot_OpenFunction);
	connect(node, &Node::taskCompleted, this, &NodeScene::onTaskCompleted);
	node->setPos(position);
	node->addInputSocket();
	node->addOutputSocket();
	addItem(node);
	m_nodes.append(node);
}

void NodeScene::createNode(const int& id, const QString& title, const QPointF& position)
{
	Node* node = new Node(id, title);
	connect(node, &Node::slot_OpenFunction, this, &NodeScene::slot_OpenFunction);
	connect(node, &Node::taskCompleted, this, &NodeScene::onTaskCompleted);
	node->setPos(position);
	node->addInputSocket();
	node->addOutputSocket();
	addItem(node);
	m_nodes.append(node);
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
	node->clearNodes();
	for (NodeConnection* connection : connectionsToRemove) {
		removeConnection(connection);
	}
	removeItem(node);
	m_nodes.removeOne(node);
	delete node;
	node = nullptr;
}

void NodeScene::removeConnection(NodeConnection* connection)
{
	if (!connection || !m_connections.contains(connection)) return;
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
	for (auto conn : m_connections)
	{
		if (conn->startSocket() == socket1 && conn->endSocket() == socket2)
		{
			return false;
		}
	}
	return true;
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
	if (Node* in_node = qgraphicsitem_cast<Node*>(inputSocket->parentItem()))
	{
		if (Node* out_node = qgraphicsitem_cast<Node*>(outputSocket->parentItem()))
		{
			out_node->addOutputNode(in_node);
			bool cycleExists = hasCycle();
			if (cycleExists) {
				in_node->outputNodes().removeOne(out_node);
				out_node->inputNodes().removeOne(in_node);
				return nullptr;
			}
		}
	}
	NodeConnection* connection = new NodeConnection(outputSocket, inputSocket);
	addItem(connection);
	m_connections.append(connection);
	outputSocket->setConnected(true);
	inputSocket->setConnected(true);
	return connection;
}

bool NodeScene::hasCycleUtil(Node* node, QMap<Node*, bool>& visited, QMap<Node*, bool>& recStack)
{
	if (!visited[node]) {
		visited[node] = true;
		recStack[node] = true;
		{
			for (auto neighbor : node->outputNodes()) {
				if (!visited[neighbor] && hasCycleUtil(neighbor, visited, recStack)) {
					return true;
				}
				else if (recStack[neighbor]) {
					return true;
				}
			}
		}
	}
	recStack[node] = false;
	return false;
}

bool NodeScene::hasCycle()
{
	QMap<Node*, bool> visited;
	QMap<Node*, bool> recStack;
	QList<Node*> nodes;
	for (auto item : items()) {
		if (Node* node = qgraphicsitem_cast<Node*>(item)) {
			if (node->type() == QGraphicsItem::UserType + 3)
			{
				nodes.append(node);
			}
		}
	}
	for (auto node : nodes) {
		if (hasCycleUtil(node, visited, recStack)) {
			return true;
		}
	}
	return false;
}

QList<Node*> NodeScene::TopologicalSorting()
{
	QList<Node*> result;
	if (hasCycle()) return result;
	QList<Node*> nodes;
	for (auto item : items()) {
		if (item->type() == QGraphicsItem::UserType + 3)
		{
			if (Node* node = qgraphicsitem_cast<Node*>(item)) {
				nodes.append(node);
			}
		}
	}
	QMap<Node*, int> inDegree;
	for (auto node : nodes) {
		inDegree[node] = node->inputNodes().size();
	}
	std::queue<Node*> q;
	for (auto node : nodes) {
		if (inDegree[node] == 0) {
			q.push(node);
		}
	}
	while (!q.empty()) {
		Node* current = q.front();
		q.pop();
		result.append(current);
		for (auto neighbor : current->outputNodes()) {
			inDegree[neighbor]--;
			if (inDegree[neighbor] == 0) {
				q.push(neighbor);
			}
		}
	}
	return result;
}

void NodeScene::onTaskCompleted(Node* node) {
	QList<Node*> m_outputNode = node->outputNodes();
	for (auto outputNode : node->outputNodes()) {
		bool allInputsCompleted = true;
		for (auto input : outputNode->inputNodes()) {
			if (input->status() != TaskCompleted) {
				allInputsCompleted = false;
				break;
			}
		}
		if (allInputsCompleted && outputNode->status() == TaskNotStarted) {
			outputNode->execute();
		}
	}
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
		QAction* createDefaultNodeAction = new QAction("创建默认节点", this);
		connect(createDefaultNodeAction, &QAction::triggered, [this, event]() {
			createNode("节点", event->scenePos());
		});
		QAction* createCustomNodeAction = new QAction("创建自定义节点", this);
		connect(createCustomNodeAction, &QAction::triggered, [this, event]() {
			bool ok;
			QString title = QInputDialog::getText(nullptr, "创建节点", "节点名称:", QLineEdit::Normal, "节点", &ok);
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

		QAction* deleteAction = new QAction("删除连接", this);
		connect(deleteAction, &QAction::triggered, [this, connection]() {
			removeConnection(connection);
		});
		QAction* changeColorAction = new QAction("更改颜色", this);
		connect(changeColorAction, &QAction::triggered, [connection]() {
			QColor newColor = QColorDialog::getColor(connection->color(), nullptr, "选择连接线颜色");
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