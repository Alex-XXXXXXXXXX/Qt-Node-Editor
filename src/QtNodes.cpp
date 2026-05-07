#include "include/QtNodes.h"
#include <QInputDialog>
#include <QColorDialog>
#include <QMessagebox>
#include <QKeyEvent>
#include <QTabBar>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>

QtNodes::QtNodes(QWidget* parent)
	: QMainWindow(parent)
{
	setupUI();
	setupMenus();
	setWindowTitle("节点流程编辑器");
	resize(1024, 768);
}

QtNodes::~QtNodes()
{
	delete m_sceneMap;
	m_sceneMap = nullptr;
	foreach(NodeScene * scene, m_scenes) {
		delete scene;
		scene = nullptr;
	}
	m_scenes.clear();
	foreach(NodeView * view, m_views) {
		delete view;
		view = nullptr;
	}
	m_views.clear();
	delete ui;
}

void QtNodes::setupUI()
{
	NodeScene* m_scene = new NodeScene(this);
	connect(m_scene, &NodeScene::slot_OpenFunction, this, &QtNodes::slot_OpenFunction);
	NodeView* m_view = new NodeView(this);
	m_view->setScene(m_scene);
	//多节点测试
	//for (int i = 0; i < 1000; i++)
	//{
	//	m_scene->createNode(QString("item %1").arg(i), QPointF(100 + (i % 50) * 150, 200 + (i / 50) * 150));
	//}
	m_scenes.append(m_scene);
	m_views.append(m_view);
	m_tabWidget = new QTabWidget(this);
	m_tabWidget->addTab(m_view, "节点编辑");
	connect(m_tabWidget->tabBar(), &QTabBar::tabBarDoubleClicked, [=](int index) {
		if (index != -1)
		{
			bool ok;
			QString currentName = m_tabWidget->tabText(index);
			QString newName = QInputDialog::getText(this,
				"重命名标签",
				"请输入新标签名:",
				QLineEdit::Normal,
				currentName,
				&ok);
			if (ok && !newName.isEmpty())
			{
				bool nameExists = false;
				for (int i = 0; i < m_tabWidget->count(); i++)
				{
					if (i != index && m_tabWidget->tabText(i) == newName)
					{
						nameExists = true;
						break;
					}
				}
				if (nameExists) {
					QMessageBox::warning(this, "警告", "标签名已存在，请使用唯一名称。");
					return;
				}
				else
				{
					m_tabWidget->setTabText(index, newName);
				}
			}
		}
	});
	connect(m_tabWidget->tabBar(), &QTabBar::currentChanged, [=](int index) {
		if (m_views.size() < index || index < 0)
		{
			return;
		}
		if (m_sceneMap)
		{
			m_sceneMap->bindMainView(m_scenes[index], m_views[index]);
		}
	});
	m_sceneMap = new NodeSceneMap();
	m_sceneMap->bindMainView(m_scene, m_view);
	QWidget* centralWidget = new QWidget;
	QHBoxLayout* mainLayout = new QHBoxLayout(centralWidget);
	mainLayout->setContentsMargins(0, 0, 0, 0);
	mainLayout->setSpacing(0);
	mainLayout->addWidget(m_tabWidget, 1);
	setCentralWidget(centralWidget);
	MapWidget = new QWidget(this);
	MapWidget->setFixedWidth(200);
	MapWidget->setFixedHeight(200);
	MapWidget->setGeometry(800, 544, 200, 200);
	MapWidget->setStyleSheet("background-color:rgba(0,0,0,0);");
	QHBoxLayout* MapLayout = new QHBoxLayout(MapWidget);
	MapLayout->setSpacing(0);
	MapLayout->setMargin(0);
	MapLayout->addWidget(m_sceneMap);
	MapLayout->addStretch();
}

void QtNodes::setupMenus()
{
	QMenu* fileMenu = menuBar()->addMenu("文件");
	QAction* exitAction = fileMenu->addAction("退出");
	connect(exitAction, &QAction::triggered, this, &QWidget::close);
	QMenu* editMenu = menuBar()->addMenu("编辑");
	QAction* createFlowAction = editMenu->addAction("创建流程");
	connect(createFlowAction, &QAction::triggered, this, &QtNodes::createFlow);
	QAction* deleteFlowAction = editMenu->addAction("删除流程");
	editMenu->addSeparator();
	connect(deleteFlowAction, &QAction::triggered, this, &QtNodes::deleteFlow);
	QAction* loadFlowAction = editMenu->addAction("加载流程");
	connect(loadFlowAction, &QAction::triggered, this, &QtNodes::loadFlow);
	QAction* saveFlowAction = editMenu->addAction("保存流程");
	editMenu->addSeparator();
	connect(saveFlowAction, &QAction::triggered, this, &QtNodes::saveFlow);
	QAction* runFlowAction = editMenu->addAction("执行流程");
	connect(runFlowAction, &QAction::triggered, this, [=]() {
		int m_iIndex = m_tabWidget->currentIndex();
		for (auto item : m_scenes[m_iIndex]->items()) {
			if (item->type() == QGraphicsItem::UserType + 3)
			{
				if (Node* node = qgraphicsitem_cast<Node*>(item)) {
					node->setResultColor(Qt::white);
					node->setStatus(TaskNotStarted);
				}
			}
		}
		QList<Node*> sortedNodes = m_scenes[m_iIndex]->TopologicalSorting();
		for (auto node : sortedNodes) {
			if (node->inputNodes().isEmpty()) {
				node->execute();
			}
		}
	});
	QMenu* helpMenu = menuBar()->addMenu("帮助");
	QAction* aboutAction = helpMenu->addAction("关于");
	connect(aboutAction, &QAction::triggered, this, &QtNodes::about);
	QToolBar* toolBar = addToolBar("工具栏");
}

void QtNodes::keyPressEvent(QKeyEvent* event)
{
	if (event->key() == Qt::Key_Delete) {
		deleteSelectedItems();
		event->accept();
	}
	else {
		QWidget::keyPressEvent(event);
	}
}

void QtNodes::resizeEvent(QResizeEvent* event)
{
	QSize m_Size = event->size();
	MapWidget->setGeometry(m_Size.width() - 224, m_Size.height() - 224, 200, 200);
}

void QtNodes::createDefaultNode()
{
	int m_iIndex = m_tabWidget->currentIndex();
	QPointF center = m_views[m_iIndex]->mapToScene(m_views[m_iIndex]->viewport()->rect().center());
	m_scenes[m_iIndex]->createNode("节点", center);
}

void QtNodes::deleteSelectedItems()
{
	int m_iIndex = m_tabWidget->currentIndex();
	QList<QGraphicsItem*> selectedItems = m_scenes[m_iIndex]->selectedItems();
	QList<QGraphicsItem*> nodesToDelete;
	QList<QGraphicsItem*> connectionsToDelete;
	for (QGraphicsItem* item : selectedItems)
	{
		if (item)
		{
			if (item->type() == QGraphicsItem::UserType + 3)
			{
				nodesToDelete.append(item);
			}
			if (item->type() == QGraphicsItem::UserType + 2)
			{
				connectionsToDelete.append(item);
			}
		}
	}
	for (QGraphicsItem* item : connectionsToDelete)
	{
		NodeConnection* connection = qgraphicsitem_cast<NodeConnection*>(item);
		if (connection) {
			m_scenes[m_iIndex]->removeConnection(connection);
			continue;
		}
	}
	for (QGraphicsItem* item : nodesToDelete)
	{
		if (item)
		{
			Node* node = qgraphicsitem_cast<Node*>(item);
			if (node) {
				m_scenes[m_iIndex]->removeNode(node);
				continue;
			}
		}
	}
}

void QtNodes::createFlow()
{
	NodeScene* m_scene = new NodeScene(this);
	connect(m_scene, &NodeScene::slot_OpenFunction, this, &QtNodes::slot_OpenFunction);
	NodeView* m_view = new NodeView(this);
	m_view->setScene(m_scene);
	m_scenes.append(m_scene);
	m_views.append(m_view);
	int m_size = m_tabWidget->count();
	int m_iIndex = m_tabWidget->addTab(m_view, QString("节点编辑_%1").arg(m_size));
	m_tabWidget->setCurrentIndex(m_iIndex);
	m_sceneMap->bindMainView(m_scene, m_view);
}

void QtNodes::deleteFlow()
{
	int m_iIndex = m_tabWidget->currentIndex();
	if (m_tabWidget->count() > 1) {
		m_tabWidget->removeTab(m_iIndex);
		m_scenes.removeAt(m_iIndex);
		m_views.removeAt(m_iIndex);
	}
	else {
		QMessageBox::warning(this, "警告", "至少保留一个流程。");
	}
}

void QtNodes::loadFlow()
{
	QFile file("./solution/app_params.json");
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		qWarning() << "无法打开文件进行读取:" << file.errorString();
		return;
	}
	QByteArray data = file.readAll();
	file.close();
	QJsonDocument doc = QJsonDocument::fromJson(data);
	if (doc.isNull() || !doc.isObject()) {
		qWarning() << "无效的JSON格式";
		return;
	}
	QJsonObject NodeTabs = doc.object();
	while (m_tabWidget->count() > 0) {
		m_tabWidget->removeTab(0);
	}
	foreach(NodeScene * scene, m_scenes) {
		delete scene;
		scene = nullptr;
	}
	m_scenes.clear();
	foreach(NodeView * view, m_views) {
		delete view;
		view = nullptr;
	}
	m_views.clear();
	QJsonArray NodesArrays = NodeTabs["Node"].toArray();
	for (int i = 0; i < NodesArrays.size(); i++)
	{
		QJsonObject NodeTab = NodesArrays[i].toObject();
		QString TabName = NodeTab.keys().first();
		QJsonArray NodesArray = NodeTab[TabName].toArray();
		NodeScene* m_scene = new NodeScene(this);
		connect(m_scene, &NodeScene::slot_OpenFunction, this, &QtNodes::slot_OpenFunction);
		NodeView* m_view = new NodeView(this);
		m_view->setScene(m_scene);
		m_scenes.append(m_scene);
		m_views.append(m_view);
		int m_iIndex = m_tabWidget->addTab(m_view, TabName);
		m_tabWidget->setCurrentIndex(m_iIndex);
		for (int j = 0; j < NodesArray.size(); j++)
		{
			QJsonObject NodeObject = NodesArray[j].toObject();
			int id = NodeObject["NodeId"].toObject()["id"].toInt();
			QString title = NodeObject["NodeTitle"].toObject()["title"].toString();
			double x = NodeObject["NodePos"].toObject()["x"].toDouble();
			double y = NodeObject["NodePos"].toObject()["y"].toDouble();
			m_scene->createNode(id, title, QPointF(x, y));
		}
	}

	QJsonArray NodesConnections = NodeTabs["Connection"].toArray();
	for (int i = 0; i < NodesConnections.size(); i++)
	{
		QJsonObject ConnectionsTab = NodesConnections[i].toObject();
		QString TabName = ConnectionsTab.keys().first();
		QJsonArray ConnectionsArray = ConnectionsTab[TabName].toArray();
		int m_iIndex = 0;
		for (int i = 0; i < m_tabWidget->count(); i++)
		{
			if (m_tabWidget->tabText(i) == TabName)
			{
				m_iIndex = i;
			}
		}
		for (int j = 0; j < ConnectionsArray.size(); j++)
		{
			QJsonObject ConnectionObject = ConnectionsArray[j].toObject();
			int startNodeId = ConnectionObject["startNodeId"].toInt();
			int inPortIndex = ConnectionObject["inPortIndex"].toInt();
			NodeSocket* startSocket = nullptr;
			NodeSocket* endSocket = nullptr;
			for (Node* item : m_scenes[m_iIndex]->nodes())
			{
				if (item->id() == startNodeId)
				{
					startSocket = item->outputSockets();
				}
			}
			int endNodeId = ConnectionObject["endNodeId"].toInt();
			int outPortIndex = ConnectionObject["outNodeIndex"].toInt();
			for (Node* item : m_scenes[m_iIndex]->nodes())
			{
				if (item->id() == endNodeId)
				{
					endSocket = item->inputSockets();
				}
			}
			m_scenes[m_iIndex]->connectSockets(startSocket, endSocket);
		}
	}
}

void QtNodes::saveFlow()
{
	int m_count = m_tabWidget->count();
	QJsonObject NodeTabs;
	QJsonArray NodesArrays;
	QJsonArray NodesConnections;
	for (int i = 0; i < m_count; i++)
	{
		QJsonObject NodeTab;
		QString TabName = m_tabWidget->tabText(i);
		QJsonArray NodesArray;
		QVector<Node*> Nodes = m_scenes[i]->nodes();
		for (Node* item : Nodes)
		{
			int id = item->id();
			QJsonObject NodeId;
			NodeId["id"] = id;
			QString title = item->title();
			QJsonObject NodeTitle;
			NodeTitle["title"] = title;
			QPointF pos = item->pos();
			QJsonObject NodePos;
			NodePos["x"] = pos.x();
			NodePos["y"] = pos.y();
			QJsonObject NodeObject;
			NodeObject["NodeId"] = NodeId;
			NodeObject["NodeTitle"] = NodeTitle;
			NodeObject["NodePos"] = NodePos;
			NodesArray.append(NodeObject);
		}
		NodeTab[TabName] = NodesArray;
		NodesArrays.append(NodeTab);

		QJsonObject ConnectionsTab;
		QJsonArray ConnectionsArray;
		QVector<NodeConnection*> connections = m_scenes[i]->connections();
		for (NodeConnection* item : connections)
		{
			NodeSocket* start = item->startSocket();
			Node* parentNode = (Node*)start->parentItem();
			int startNodeId = parentNode->id();
			NodeSocket* end = item->endSocket();
			Node* endParentNode = (Node*)end->parentItem();
			int endNodeId = endParentNode->id();
			QJsonObject ConnectionObject;
			ConnectionObject["startNodeId"] = startNodeId;
			ConnectionObject["inPortIndex"] = start->id();
			ConnectionObject["endNodeId"] = endNodeId;
			ConnectionObject["outPortIndex"] = end->id();
			ConnectionsArray.append(ConnectionObject);
		}
		ConnectionsTab[TabName] = ConnectionsArray;
		NodesConnections.append(ConnectionsTab);
	}
	NodeTabs["Node"] = NodesArrays;
	NodeTabs["Connection"] = NodesConnections;
	QJsonDocument doc(NodeTabs);
	QFile file("./solution/app_params.json");
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		qWarning() << "无法打开文件进行写入:" << file.errorString();
	}
	file.write(doc.toJson(QJsonDocument::Indented));
	file.close();
}

void QtNodes::about()
{
	QMessageBox::about(this, "关于节点流程编辑器",
		"节点流程编辑器\n\n"
		"一个基于Qt的节点流程编辑工具，支持创建、连接和管理节点。\n"
		"使用方法:\n"
		"- 右键点击空白处创建节点\n"
		"- 拖拽节点可以移动它们\n"
		"- 从一个节点的输出拖拽到另一个节点的输入来创建连接\n"
		"- 右键点击节点或连接可以打开上下文菜单");
}