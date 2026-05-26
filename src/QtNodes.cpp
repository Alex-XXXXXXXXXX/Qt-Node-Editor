#include "include/QtNodes.h"
#include <QInputDialog>
#include <QColorDialog>
#include <QMessagebox>
#include <QKeyEvent>
#include <QTabBar>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include "include/FlowSerializer.h"

QtNodes::QtNodes(QWidget* parent)
	: QMainWindow(parent)
{
	setupUI();
	setupMenus();
	setWindowTitle(QString::fromUtf8("节点流程编辑器"));
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
	m_scenes.append(m_scene);
	m_views.append(m_view);
	m_tabWidget = new QTabWidget(this);
	m_tabWidget->addTab(m_view, QString::fromUtf8("节点编辑"));
	connect(m_tabWidget->tabBar(), &QTabBar::tabBarDoubleClicked, [=](int index) {
		if (index != -1)
		{
			bool ok;
			QString currentName = m_tabWidget->tabText(index);
			QString newName = QInputDialog::getText(this,
				QString::fromUtf8("重命名标签"),
				QString::fromUtf8("请输入新标签名:"),
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
					QMessageBox::warning(this, QString::fromUtf8("错误"), QString::fromUtf8("标签名已存在，请使用唯一名称。"));
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
	QMenu* fileMenu = menuBar()->addMenu(QString::fromUtf8("文件"));
	QAction* exitAction = fileMenu->addAction(QString::fromUtf8("退出"));
	connect(exitAction, &QAction::triggered, this, &QWidget::close);
	QMenu* editMenu = menuBar()->addMenu(QString::fromUtf8("编辑"));
	QAction* createFlowAction = editMenu->addAction(QString::fromUtf8("创建流程"));
	connect(createFlowAction, &QAction::triggered, this, &QtNodes::createFlow);
	QAction* deleteFlowAction = editMenu->addAction(QString::fromUtf8("删除流程"));
	editMenu->addSeparator();
	connect(deleteFlowAction, &QAction::triggered, this, &QtNodes::deleteFlow);
	QAction* loadFlowAction = editMenu->addAction(QString::fromUtf8("加载流程"));
	connect(loadFlowAction, &QAction::triggered, this, &QtNodes::loadFlow);
	QAction* saveFlowAction = editMenu->addAction(QString::fromUtf8("保存流程"));
	editMenu->addSeparator();
	connect(saveFlowAction, &QAction::triggered, this, &QtNodes::saveFlow);
	QAction* runFlowAction = editMenu->addAction(QString::fromUtf8("执行流程"));
	connect(runFlowAction, &QAction::triggered, this, [=]() {
		int m_iIndex = m_tabWidget->currentIndex();
		m_scenes[m_iIndex]->executeFlow();
	});
	QMenu* helpMenu = menuBar()->addMenu(QString::fromUtf8("帮助"));
	QAction* aboutAction = helpMenu->addAction(QString::fromUtf8("关于"));
	connect(aboutAction, &QAction::triggered, this, &QtNodes::about);
	QToolBar* toolBar = addToolBar(QString::fromUtf8("工具栏"));
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
	m_scenes[m_iIndex]->createNode(QString::fromUtf8("节点"), center);
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
	int m_iIndex = m_tabWidget->addTab(m_view, QString::fromUtf8("节点编辑_%1").arg(m_size));
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
		QMessageBox::warning(this, QString::fromUtf8("错误"), QString::fromUtf8("至少保留一个流程。"));
	}
}

void QtNodes::loadFlow()
{
	QList<SceneData> scenesData = FlowSerializer::loadFromFile("./solution/app_params.json");
	if (scenesData.isEmpty())
		return;

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

	for (const SceneData& sceneData : scenesData)
	{
		NodeScene* m_scene = new NodeScene(this);
		connect(m_scene, &NodeScene::slot_OpenFunction, this, &QtNodes::slot_OpenFunction);
		NodeView* m_view = new NodeView(this);
		m_view->setScene(m_scene);
		m_scenes.append(m_scene);
		m_views.append(m_view);
		int m_iIndex = m_tabWidget->addTab(m_view, sceneData.tabName);
		m_tabWidget->setCurrentIndex(m_iIndex);

		for (const SceneNodeInfo& info : sceneData.nodes)
		{
			m_scene->createNode(info.id, info.title, info.position);
		}

		for (const SceneConnectionInfo& conn : sceneData.connections)
		{
			NodeSocket* startSocket = nullptr;
			NodeSocket* endSocket = nullptr;
			for (Node* node : m_scene->nodes())
			{
				if (node->id() == conn.startNodeId)
					startSocket = node->outputSockets();
				if (node->id() == conn.endNodeId)
					endSocket = node->inputSockets();
			}
			if (startSocket && endSocket)
				m_scene->connectSockets(startSocket, endSocket);
		}
	}
}

void QtNodes::saveFlow()
{
	QList<SceneData> scenesData;
	int m_count = m_tabWidget->count();
	for (int i = 0; i < m_count; i++)
	{
		SceneData sceneData;
		sceneData.tabName = m_tabWidget->tabText(i);

		QVector<Node*> nodes = m_scenes[i]->nodes();
		for (Node* node : nodes)
		{
			SceneNodeInfo info;
			info.id = node->id();
			info.title = node->title();
			info.position = node->pos();
			sceneData.nodes.append(info);
		}

		QVector<NodeConnection*> connections = m_scenes[i]->connections();
		for (NodeConnection* conn : connections)
		{
			Node* startNode = qgraphicsitem_cast<Node*>(conn->startSocket()->parentItem());
			Node* endNode = qgraphicsitem_cast<Node*>(conn->endSocket()->parentItem());
			if (startNode && endNode)
			{
				SceneConnectionInfo info;
				info.startNodeId = startNode->id();
				info.endNodeId = endNode->id();
				sceneData.connections.append(info);
			}
		}

		scenesData.append(sceneData);
	}
	FlowSerializer::saveToFile(scenesData, "./solution/app_params.json");
}

void QtNodes::about()
{
	QMessageBox::about(this, QString::fromUtf8("关于节点流程编辑器"),
		QString::fromUtf8("节点流程编辑器\n\n"
		"一个基于Qt的节点流程编辑工具，支持创建、连接和管理节点。\n"
		"使用方法:\n"
		"- 右键点击空白处创建节点\n"
		"- 拖拽节点以移动位置\n"
		"- 从一个节点输出拖拽到另一个节点输入以创建连接\n"
		"- 右键点击节点或连接可打开相关菜单"));
}
