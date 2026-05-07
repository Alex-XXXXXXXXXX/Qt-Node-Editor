#pragma once
#pragma execution_character_set("utf-8")
#include <QGraphicsItem>

class NodeSocket : public QGraphicsItem
{
public:
	enum { Type = UserType + 1 };
	int type() const override { return Type; }
	enum SocketType { Input, Output };
	NodeSocket(SocketType type, QGraphicsItem* parent = nullptr);
	QRectF boundingRect() const override;
	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
	SocketType socketType() const { return m_type; }
	void setConnected(bool connected) { m_connected = connected; update(); }
	bool isConnected() const { return m_connected; }
	void setId(int id) { m_id = id; }
	int id() const { return m_id; }
	QPointF connectionPoint() const;

protected:
	void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
	void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;

private:
	SocketType m_type;
	bool m_connected;
	bool m_hovered;
	int m_id;
	const int m_radius = 6;
};
