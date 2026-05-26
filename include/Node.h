#pragma once
#pragma execution_character_set("utf-8")
#include <QObject>
#include <QGraphicsItem>
#include "NodeSocket.h"
#include "NodeConnection.h"
#include "NodeData.h"

class Node : public QObject, public QGraphicsItem
{
	Q_OBJECT

public:
	enum { Type = UserType + 3 };
	int type() const override { return Type; }

	Node(const int& id, const QString& title, QGraphicsItem* parent = nullptr);

	void addInputSocket();
	void addOutputSocket();

	NodeSocket* inputSockets() const { return m_inputSockets; }
	NodeSocket* outputSockets() const { return m_outputSockets; }

	void updateSocketPositions();

	void setTitle(const QString& title) { m_title = title; update(); }
	QString title() const { return m_title; }

	QRectF boundingRect() const override;
	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

	void setColor(const QColor& color) { m_color = color; update(); }
	QColor color() const { return m_color; }

	void setResultColor(const QColor& color) { m_ResultColor = color; update(); }
	QColor resultColor() const { return m_ResultColor; }

	TaskStatus status() const { return m_status; }
	void setStatus(TaskStatus status) { m_status = status; }

	int id() const { return m_id; }

protected:
	void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
	void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
	void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
	void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;
	QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

private:
	int m_id;
	QString m_title;
	QPixmap m_icon;
	TaskStatus m_status = TaskNotStarted;
	NodeSocket* m_inputSockets = nullptr;
	NodeSocket* m_outputSockets = nullptr;
	QColor m_color;
	QColor m_ResultColor = Qt::white;
	bool m_moving;
	QPointF m_lastPos;

	const int m_padding = 10;
	const int m_socketSpacing = 25;
	const int m_titleHeight = 25;

	bool m_bShowLeftLine = false;
	bool m_bShowTopLine = false;
	QPointF p1, p2, p3, p4;

private:
	void auto_Alignment();

signals:
	void slot_OpenFunction(QString m_title);
};
