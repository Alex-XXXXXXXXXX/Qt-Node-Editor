#pragma once
#pragma execution_character_set("utf-8")
#include <QObject>
#include <QGraphicsView>
#include "nodescene.h"

class NodeView : public QGraphicsView
{
	Q_OBJECT

public:
	NodeView(QWidget* parent = nullptr);

	void setScene(NodeScene* scene);
	NodeScene* nodeScene() const { return m_scene; }
	double getZoomLevel() const { return ZoomValue; }

protected:
	void wheelEvent(QWheelEvent* event) override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
	void drawBackground(QPainter* painter, const QRectF& rect) override;

private:
	NodeScene* m_scene;
	bool m_panning;
	QPoint m_lastPanPos;
	const qreal m_zoomFactor = 1.1;
	double ZoomValue = 1.0;
};
