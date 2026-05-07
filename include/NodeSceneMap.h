#pragma once
#include <QWidget>
#include <QObject>
#include "ui_NodeSceneMap.h"
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QTimer>

class NodeSceneMap : public QWidget
{
	Q_OBJECT

public:
	NodeSceneMap(QWidget* parent = nullptr);
	~NodeSceneMap();

	void bindMainView(QGraphicsScene* scene, QGraphicsView* view);
	void setThumbnailSize(const QSize& size);
	void refreshThumbnail();

protected:
	void paintEvent(QPaintEvent* event) override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;

private:
	Ui::NodeSceneMapClass ui;

private:
	QRectF calcThumbnailViewportRect() const;

private:
	QGraphicsScene* m_mainScene = nullptr;
	QGraphicsView* m_mainView = nullptr;
	QSize m_thumbSize = QSize(200, 200);
	QPixmap m_thumbnail;
	QTimer m_syncTimer;
	bool m_isDragging = false;
};
