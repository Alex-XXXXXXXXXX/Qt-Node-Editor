#pragma once
#include <QtWidgets/QMainWindow>
#include "ui_QtNodes.h"
#include "NodeScene.h"
#include "NodeView.h"
#include "NodeSceneMap.h"

#pragma execution_character_set("utf-8")

QT_BEGIN_NAMESPACE
namespace Ui { class QtNodesClass; };
QT_END_NAMESPACE

class QtNodes : public QMainWindow
{
	Q_OBJECT

public:
	QtNodes(QWidget* parent = nullptr);
	~QtNodes();

private:
	Ui::QtNodesClass* ui;

private slots:
	void createDefaultNode();
	void deleteSelectedItems();
	void createFlow();
	void deleteFlow();
	void loadFlow();
	void saveFlow();
	void about();

private:
	void setupUI();
	void setupMenus();

	QVector<NodeView*> m_views;
	QVector<NodeScene*> m_scenes;
	QTabWidget* m_tabWidget;
	NodeSceneMap* m_sceneMap;
	QWidget* MapWidget;

protected:
	void keyPressEvent(QKeyEvent* event) override;
	void resizeEvent(QResizeEvent* event) override;

signals:
	void slot_OpenFunction(QString m_title);
};
