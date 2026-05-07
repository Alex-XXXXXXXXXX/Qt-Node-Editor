#include "include/NodeSocket.h"
#include <QPainter>
#include "include/NodeView.h"
#include "include/NodeScene.h"

NodeSocket::NodeSocket(SocketType type, QGraphicsItem* parent)
	: QGraphicsItem(parent), m_type(type), m_connected(false), m_hovered(false), m_id(-1)
{
	setFlag(ItemIsSelectable);
	setAcceptHoverEvents(true);
}

QRectF NodeSocket::boundingRect() const
{
	return QRectF(-m_radius * 2, -m_radius * 2, m_radius * 4, m_radius * 4);
}

void NodeSocket::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);
	if (dynamic_cast<NodeView*>(this->scene()->views()[0])->getZoomLevel() > 0.25)
	{
		QBrush brush(m_hovered ? QColor("#3b82f6") : Qt::lightGray);
		painter->setBrush(brush);
		QPen pen(m_connected ? QColor("#3b82f6") : Qt::black);
		pen.setWidth(2);
		painter->setPen(pen);
		painter->drawEllipse(-m_radius, -m_radius, m_radius * 2, m_radius * 2);
		if (m_connected) {
			painter->setBrush(QColor("#3b82f6"));
			painter->drawEllipse(-m_radius / 2, -m_radius / 2, m_radius, m_radius);
		}
	}
}

QPointF NodeSocket::connectionPoint() const
{
	QPointF point;
	if (m_type == Input) {
		point = QPointF(-m_radius, 0);
	}
	else {
		point = QPointF(m_radius, 0);
	}
	return mapToScene(point);
}

void NodeSocket::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
	m_hovered = true;
	update();
	QGraphicsItem::hoverEnterEvent(event);
}

void NodeSocket::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
	m_hovered = false;
	update();
	QGraphicsItem::hoverLeaveEvent(event);
}