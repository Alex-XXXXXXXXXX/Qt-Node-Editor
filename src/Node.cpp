#include "include/Node.h"
#include <QPainter>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QAction>
#include <QMenu>
#include <QColorDialog>
#include <QInputDialog>
#include <QFontMetrics>
#include <QDebug>
#include "include/NodeView.h"
#include "include/NodeScene.h"

Node::Node(const int& id, const QString& title, QGraphicsItem* parent)
	: QGraphicsItem(parent), m_id(id), m_title(title), m_color(QColor(60, 60, 80)), m_moving(false)
{
	setFlag(ItemIsMovable);
	setFlag(ItemIsSelectable);
	setFlag(ItemSendsGeometryChanges);
	setFlag(ItemIsFocusable);
}

void Node::addInputSocket()
{
	m_inputSockets = new NodeSocket(NodeSocket::Input, this);
	m_inputSockets->setId(0);
	updateSocketPositions();
}

void Node::addOutputSocket()
{
	m_outputSockets = new NodeSocket(NodeSocket::Output, this);
	m_outputSockets->setId(0);
	updateSocketPositions();
}

void Node::updateSocketPositions()
{
	int totalHeight = m_titleHeight + m_socketSpacing;
	if (m_inputSockets)
	{
		m_inputSockets->setPos(boundingRect().width() / 2, 0);
	}
	if (m_outputSockets)
	{
		m_outputSockets->setPos(boundingRect().width() / 2, totalHeight + 20);
	}
}

QRectF Node::boundingRect() const
{
	QFontMetrics metrics(QFont());
	int titleWidth = m_padding * 2;
	int minWidth = 120;
	int width = qMax(titleWidth, minWidth);
	int totalHeight = m_titleHeight + m_socketSpacing + m_padding * 2;
	return QRectF(0, 0, width, totalHeight);
}

void Node::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);
	painter->setRenderHint(QPainter::Antialiasing);
	QRectF rect = boundingRect();
	painter->setBrush(m_color);
	QPen pen(QColor("#3b82f6"));
	pen.setWidth(isSelected() ? 4 : 3);
	pen.setStyle(isSelected() ? Qt::DashLine : Qt::SolidLine);
	painter->setPen(pen);
	painter->drawRoundedRect(rect.adjusted(0, 0, -1, -1), 5, 5);
	painter->setPen(m_ResultColor);
	painter->setBrush(m_ResultColor);
	painter->drawRoundedRect(QRectF(2, 2, 35, rect.height() - 4).adjusted(0, 0, -1, -1), 3, 3);
	painter->setPen(Qt::white);
	QFont font;
	font.setFamily("SimHei");
	font.setBold(true);
	font.setPointSizeF(5);
	painter->setFont(font);
	painter->drawText(QRectF(m_padding + 15, 21, rect.width() - m_padding * 2, m_titleHeight),
		Qt::AlignCenter, QString("%1 %2").arg(m_id).arg(m_title));
	painter->drawPixmap(QRect(m_padding - 5, 18, 30, 30), m_icon);
	pen.setColor(QColor("#3b82f6"));
	pen.setWidth(3);
	pen.setStyle(Qt::DashLine);
	painter->setPen(pen);
	if (m_bShowLeftLine)
	{
		painter->drawLine(mapFromScene(p1), mapFromScene(p2));
		this->scene()->update();
	}
	if (m_bShowTopLine)
	{
		painter->drawLine(mapFromScene(p3), mapFromScene(p4));
		this->scene()->update();
	}
}

void Node::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
	if (event->button() == Qt::LeftButton) {
		m_moving = true;
		m_lastPos = event->pos();
	}
	QGraphicsItem::mousePressEvent(event);
}

void Node::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
	if (m_moving && (event->buttons() & Qt::LeftButton)) {
		QGraphicsItem::mouseMoveEvent(event);
	}
	auto_Alignment();
}

void Node::auto_Alignment()
{
	QList<QGraphicsItem*> items = scene()->items();
	m_bShowLeftLine = false;
	m_bShowTopLine = false;
	int m_iThreshold = 5;
	for (QGraphicsItem* item : items) {
		if (item->type() != QGraphicsItem::UserType + 3)
		{
			continue;
		}
		if (item == this)
		{
			continue;
		}
		QRectF currentRect = mapRectToScene(this->boundingRect());
		QRectF otherRect = item->mapRectToScene(item->boundingRect());
		if (qAbs(qAbs(currentRect.left()) - qAbs(otherRect.left())) <= m_iThreshold) {
			setPos(mapToScene(QPointF(otherRect.left() - currentRect.left(), 0)));
			p1 = mapRectToScene(this->boundingRect()).bottomLeft();
			p2 = otherRect.bottomLeft();
			m_bShowLeftLine = true;
		}
		if (qAbs(qAbs(currentRect.top()) - qAbs(otherRect.top())) <= m_iThreshold) {
			setPos(mapToScene(QPointF(0, otherRect.top() - currentRect.top())));
			p3 = mapRectToScene(this->boundingRect()).topLeft();
			p4 = otherRect.topLeft();
			m_bShowTopLine = true;
		}
	}
}

void Node::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
	m_moving = false;
	m_bShowLeftLine = false;
	m_bShowTopLine = false;
	QGraphicsItem::mouseReleaseEvent(event);
}

void Node::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
	emit slot_OpenFunction(m_title);
	qDebug() << QString("%1").arg(m_title);
	QGraphicsItem::mouseDoubleClickEvent(event);
}

QVariant Node::itemChange(GraphicsItemChange change, const QVariant& value)
{
	if (change == ItemPositionHasChanged) {
		if (m_inputSockets)
		{
			for (QGraphicsItem* item : scene()->items()) {
				NodeConnection* connection = qgraphicsitem_cast<NodeConnection*>(item);
				if (connection && (connection->endSocket() == m_inputSockets || connection->startSocket() == m_inputSockets)) {
					connection->updatePosition();
				}
			}
		}
		if (m_outputSockets)
		{
			for (QGraphicsItem* item : scene()->items()) {
				NodeConnection* connection = qgraphicsitem_cast<NodeConnection*>(item);
				if (connection && (connection->endSocket() == m_outputSockets || connection->startSocket() == m_outputSockets)) {
					connection->updatePosition();
				}
			}
		}
	}
	return QGraphicsItem::itemChange(change, value);
}
