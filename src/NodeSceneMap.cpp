#include "include/NodeSceneMap.h"
#include <QPainter>
#include <QMouseEvent>
#include <QtMath>
#include <QGraphicsItem>

NodeSceneMap::NodeSceneMap(QWidget* parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	setMinimumSize(m_thumbSize);
	m_syncTimer.setInterval(30);
	m_syncTimer.setSingleShot(false);
	connect(&m_syncTimer, &QTimer::timeout, [=]() {
		refreshThumbnail();
	});
	m_syncTimer.start();
}

NodeSceneMap::~NodeSceneMap()
{
	m_syncTimer.stop();
}

void NodeSceneMap::bindMainView(QGraphicsScene* scene, QGraphicsView* view)
{
	if (!scene || !view)
		return;
	m_mainScene = scene;
	m_mainView = view;
}

void NodeSceneMap::setThumbnailSize(const QSize& size)
{
	m_thumbSize = size;
	setMinimumSize(m_thumbSize);
}

void NodeSceneMap::refreshThumbnail()
{
	if (!m_mainScene)
	{
		return;
	}
	QRectF sceneRect = m_mainScene->sceneRect();
	m_thumbnail = QPixmap(m_thumbSize);
	m_thumbnail.fill(Qt::transparent);
	qreal scaleX = m_thumbSize.width() / sceneRect.width();
	qreal scaleY = m_thumbSize.height() / sceneRect.height();
	qreal scale = qMin(scaleX, scaleY);
	scale = 1;
	QPainter painter(&m_thumbnail);
	painter.scale(scale, scale);
	m_mainScene->render(&painter, m_thumbnail.rect(), sceneRect, Qt::KeepAspectRatio);
	painter.end();
	update();
}

void NodeSceneMap::paintEvent(QPaintEvent* event)
{
	Q_UNUSED(event);
	if (m_thumbnail.isNull())
		return;
	QPainter painter(this);
	QPointF thumbDrawRect = QRectF(rect()).center() - QRectF(m_thumbnail.rect()).center();
	painter.drawPixmap(thumbDrawRect, m_thumbnail);
	QPen pen(QColor("#3b82f6"), 2);
	pen.setStyle(Qt::SolidLine);
	painter.setPen(pen);
	painter.setBrush(Qt::NoBrush);
	painter.drawRect(rect());
	QRectF viewportRect = calcThumbnailViewportRect();
	if (!viewportRect.isEmpty())
	{
		painter.drawRect(viewportRect.translated(thumbDrawRect));
	}
}

void NodeSceneMap::mousePressEvent(QMouseEvent* event)
{
	if (!m_mainScene || !m_mainView || m_thumbnail.isNull())
		return;
	QPointF thumbDrawRect = QRectF(rect()).center() - QRectF(m_thumbnail.rect()).center();
	QPointF clickPosInThumb = event->pos() - thumbDrawRect;
	QRectF viewportRect = calcThumbnailViewportRect();
	if (viewportRect.contains(clickPosInThumb))
	{
		m_isDragging = true;
		return;
	}
	QRectF sceneRect = m_mainScene->sceneRect();
	qreal scaleX = m_thumbSize.width() / sceneRect.width();
	qreal scaleY = m_thumbSize.height() / sceneRect.height();
	qreal scale = qMin(scaleX, scaleY);
	QPointF scenePos(clickPosInThumb.x() / scale, clickPosInThumb.y() / scale);
	m_mainView->centerOn(scenePos);
}

void NodeSceneMap::mouseMoveEvent(QMouseEvent* event)
{
	if (!m_isDragging || !m_mainScene || !m_mainView || m_thumbnail.isNull())
		return;
	QPointF thumbDrawRect = QRectF(rect()).center() - QRectF(m_thumbnail.rect()).center();
	QPointF dragPosInThumb = event->pos() - thumbDrawRect;
	QRectF sceneRect = m_mainScene->sceneRect();
	qreal scaleX = m_thumbSize.width() / sceneRect.width();
	qreal scaleY = m_thumbSize.height() / sceneRect.height();
	qreal scale = qMin(scaleX, scaleY);
	QPointF scenePos(dragPosInThumb.x() / scale, dragPosInThumb.y() / scale);
	m_mainView->centerOn(scenePos);
}

QRectF NodeSceneMap::calcThumbnailViewportRect() const
{
	if (!m_mainScene || !m_mainView || m_thumbnail.isNull())
		return QRectF();
	QRectF sceneRect = m_mainScene->sceneRect();
	if (sceneRect.isEmpty())
		return QRectF();
	QRectF viewportSceneRect = m_mainView->mapToScene(m_mainView->viewport()->rect()).boundingRect();
	qreal scaleX = m_thumbSize.width() / sceneRect.width();
	qreal scaleY = m_thumbSize.height() / sceneRect.height();
	qreal scale = qMin(scaleX, scaleY);
	return QRectF(
		viewportSceneRect.x() * scale,
		viewportSceneRect.y() * scale,
		viewportSceneRect.width() * scale,
		viewportSceneRect.height() * scale
	);
}