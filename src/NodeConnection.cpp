#include "include/NodeConnection.h"
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <qmath.h>
#include "include/NodeScene.h"

NodeConnection::NodeConnection(NodeSocket* startSocket, NodeSocket* endSocket, QGraphicsItem* parent)
	: QGraphicsLineItem(parent), m_startSocket(startSocket), m_endSocket(endSocket), m_color(Qt::darkGray)
{
	setFlag(QGraphicsItem::ItemIsSelectable, true);
	setZValue(-1);
	if (m_startSocket)
	{
		m_startPoint = m_startSocket->connectionPoint();
	}
	m_endPoint = (m_endSocket) ? m_endSocket->connectionPoint() : m_startPoint;
	updatePosition();
}

void NodeConnection::setEndPoint(const QPointF& point) {
	m_endPoint = point;
	m_endSocket = nullptr;
	updatePosition();
}

void NodeConnection::setEndSocket(NodeSocket* endSocket) {
	if (endSocket) {
		m_endSocket = endSocket;
		m_endPoint = endSocket->connectionPoint();
		updatePosition();
	}
}

void NodeConnection::updatePosition()
{
	QLineF line(m_startSocket->connectionPoint(), m_endSocket->connectionPoint());
	setLine(line);
}

void NodeConnection::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);
	QPen pen;
	pen.setColor(Qt::darkGray);
	pen.setWidth(m_lineWidth);
	if (isSelected()) {
		pen.setStyle(Qt::DashLine);
	}
	else {
		pen.setStyle(Qt::SolidLine);
	}
	painter->setPen(pen);
	QPointF in = QPointF(this->line().p2().x() + 6, this->line().p2().y());
	QPointF out = QPointF(this->line().p1().x() - 6, this->line().p1().y());
	QPainterPath cubic;
	if (in.y() - 40 >= out.y()) {
		cubic.moveTo(in);
		qreal midy = (in.y() + out.y()) / 2.0;
		cubic.lineTo(in.x(), midy);
		cubic.lineTo(out.x(), midy);
		cubic.lineTo(out);
	}
	else {
		cubic.moveTo(in);
		cubic.lineTo(in.x(), in.y() - 30);
		qreal midx = (in.x() + out.x()) / 2.0;
		cubic.lineTo(midx, in.y() - 30);
		cubic.lineTo(midx, out.y() + 30);
		cubic.lineTo(out.x(), out.y() + 30);
		cubic.lineTo(out);
	}
	drawArrow(painter, out, in);
	painter->drawPath(cubic);
}

void NodeConnection::drawArrow(QPainter* painter, const QPointF& start, const QPointF& end)
{
	const double arrowSize = 8.0;
	QPointF point = QPointF(end.x(), end.y() - 20);
	QPointF arrowP1 = point + QPointF(sin(M_PI / 3) * arrowSize,
		cos(M_PI / 3) * arrowSize);
	QPointF arrowP2 = point + QPointF(sin(-M_PI / 3) * arrowSize,
		cos(-M_PI / 3) * arrowSize);
	painter->drawPolygon(QPolygonF() << end << arrowP1 << arrowP2);
}

void NodeConnection::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
	if (event->button() == Qt::LeftButton) {
		setSelected(true);
		event->accept();
	}
	QGraphicsLineItem::mousePressEvent(event);
}

void NodeConnection::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
	QGraphicsLineItem::mouseReleaseEvent(event);
}