#pragma once
#include <QString>
#include <QList>
#include <QPointF>

struct SceneNodeInfo
{
	int id;
	QString title;
	QPointF position;
};

struct SceneConnectionInfo
{
	int startNodeId;
	int endNodeId;
};

struct SceneData
{
	QString tabName;
	QList<SceneNodeInfo> nodes;
	QList<SceneConnectionInfo> connections;
};

class FlowSerializer
{
public:
	static QList<SceneData> loadFromFile(const QString& filePath);
	static void saveToFile(const QList<SceneData>& scenes, const QString& filePath);
};
