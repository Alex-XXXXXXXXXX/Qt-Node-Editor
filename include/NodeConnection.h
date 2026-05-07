#pragma once
#pragma execution_character_set("utf-8")
#include <QObject>
#include <QGraphicsLineItem>
#include "NodeSocket.h"

class NodeConnection : public QGraphicsLineItem
{
public:
	enum { Type = UserType + 2 };
	int type() const override { return Type; }

	NodeConnection(NodeSocket* startSocket, NodeSocket* endSocket = nullptr, QGraphicsItem* parent = nullptr);

	void updatePosition();
	NodeSocket* startSocket() const { return m_startSocket; }
	NodeSocket* endSocket() const { return m_endSocket; }

	void setColor(const QColor& color) { m_color = color; update(); }
	QColor color() const { return m_color; }

	void setEndSocket(NodeSocket* endSocket);
	void setEndPoint(const QPointF& point);

	void drawArrow(QPainter* painter, const QPointF& start, const QPointF& end);

protected:
	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
	void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
	void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

private:
	NodeSocket* m_startSocket;
	NodeSocket* m_endSocket;
	QPointF m_startPoint;
	QPointF m_endPoint;
	QColor m_color;
	int m_lineWidth = 4;
};
