#include "include/NodeView.h"
#include <QMouseEvent>
#include <cmath>

NodeView::NodeView(QWidget* parent)
	: QGraphicsView(parent), m_scene(nullptr), m_panning(false)
{
	setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
	setRenderHint(QPainter::Antialiasing, false);
	setRenderHint(QPainter::TextAntialiasing, false);
	setRenderHint(QPainter::SmoothPixmapTransform, false);
	setDragMode(RubberBandDrag);
	setMouseTracking(true);
	setTransformationAnchor(AnchorUnderMouse);
	setResizeAnchor(AnchorViewCenter);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void NodeView::setScene(NodeScene* scene)
{
	m_scene = scene;
	QGraphicsView::setScene(scene);
}

void NodeView::wheelEvent(QWheelEvent* event)
{
	if (event->angleDelta().y() > 0) {
		ZoomValue *= 1.1;
		scale(m_zoomFactor, m_zoomFactor);
	}
	else {
		ZoomValue *= 0.9;
		scale(1 / m_zoomFactor, 1 / m_zoomFactor);
	}
}

void NodeView::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::RightButton) {
		m_panning = true;
		m_lastPanPos = event->pos();
		setCursor(Qt::ClosedHandCursor);
		event->accept();
		return;
	}

	QGraphicsView::mousePressEvent(event);
}

void NodeView::mouseMoveEvent(QMouseEvent* event)
{
	if (m_panning) {
		QPoint delta = event->pos() - m_lastPanPos;
		m_lastPanPos = event->pos();
		translate(delta.x(), delta.y());
		event->accept();
		return;
	}

	QGraphicsView::mouseMoveEvent(event);
}

void NodeView::mouseReleaseEvent(QMouseEvent* event)
{
	if (event->button() == Qt::RightButton && m_panning) {
		m_panning = false;
		setCursor(Qt::ArrowCursor);
		event->accept();
		return;
	}
	QGraphicsView::mouseReleaseEvent(event);
}

void NodeView::drawBackground(QPainter* painter, const QRectF& rect)
{
	painter->fillRect(rect, QColor(Qt::white));
	painter->setPen(QPen(QColor(200, 200, 200), 0.5));
	int left = static_cast<int>(std::floor(rect.left()));
	int right = static_cast<int>(std::ceil(rect.right()));
	int top = static_cast<int>(std::floor(rect.top()));
	int bottom = static_cast<int>(std::ceil(rect.bottom()));
	for (int x = left; x <= right; x += 30) {
		painter->drawLine(x, top, x, bottom);
	}
	for (int y = top; y <= bottom; y += 30) {
		painter->drawLine(left, y, right, y);
	}
}