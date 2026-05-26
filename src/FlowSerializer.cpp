#include "include/FlowSerializer.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

QList<SceneData> FlowSerializer::loadFromFile(const QString& filePath)
{
	QList<SceneData> result;
	QFile file(filePath);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		qWarning() << QString::fromUtf8("无法打开文件进行读取:") << file.errorString();
		return result;
	}

	QByteArray data = file.readAll();
	file.close();

	QJsonDocument doc = QJsonDocument::fromJson(data);
	if (doc.isNull() || !doc.isObject())
	{
		qWarning() << QString::fromUtf8("无效的JSON格式");
		return result;
	}

	QJsonObject root = doc.object();
	QJsonArray nodesArrays = root["Node"].toArray();
	QJsonArray connectionsArrays = root["Connection"].toArray();

	for (int i = 0; i < nodesArrays.size(); i++)
	{
		QJsonObject nodeTab = nodesArrays[i].toObject();
		QString tabName = nodeTab.keys().first();
		QJsonArray nodesArray = nodeTab[tabName].toArray();

		SceneData sceneData;
		sceneData.tabName = tabName;

		for (int j = 0; j < nodesArray.size(); j++)
		{
			QJsonObject nodeObj = nodesArray[j].toObject();
			QJsonObject idObj = nodeObj["NodeId"].toObject();
			QJsonObject titleObj = nodeObj["NodeTitle"].toObject();
			QJsonObject posObj = nodeObj["NodePos"].toObject();

			SceneNodeInfo info;
			info.id = idObj["id"].toInt();
			info.title = titleObj["title"].toString();
			info.position = QPointF(posObj["x"].toDouble(), posObj["y"].toDouble());
			sceneData.nodes.append(info);
		}
		result.append(sceneData);
	}

	for (int i = 0; i < connectionsArrays.size(); i++)
	{
		QJsonObject connTab = connectionsArrays[i].toObject();
		QString tabName = connTab.keys().first();
		QJsonArray connArray = connTab[tabName].toArray();

		for (int j = 0; j < result.size(); j++)
		{
			if (result[j].tabName == tabName)
			{
				for (int k = 0; k < connArray.size(); k++)
				{
					QJsonObject connObj = connArray[k].toObject();
					SceneConnectionInfo info;
					info.startNodeId = connObj["startNodeId"].toInt();
					info.endNodeId = connObj["endNodeId"].toInt();
					result[j].connections.append(info);
				}
				break;
			}
		}
	}

	return result;
}

void FlowSerializer::saveToFile(const QList<SceneData>& scenes, const QString& filePath)
{
	QJsonObject root;
	QJsonArray nodesArrays;
	QJsonArray connectionsArrays;

	for (const SceneData& scene : scenes)
	{
		QJsonObject nodeTab;
		QJsonArray nodesArray;
		for (const SceneNodeInfo& info : scene.nodes)
		{
			QJsonObject idObj;
			idObj["id"] = info.id;
			QJsonObject titleObj;
			titleObj["title"] = info.title;
			QJsonObject posObj;
			posObj["x"] = info.position.x();
			posObj["y"] = info.position.y();

			QJsonObject nodeObj;
			nodeObj["NodeId"] = idObj;
			nodeObj["NodeTitle"] = titleObj;
			nodeObj["NodePos"] = posObj;
			nodesArray.append(nodeObj);
		}
		nodeTab[scene.tabName] = nodesArray;
		nodesArrays.append(nodeTab);

		QJsonObject connTab;
		QJsonArray connArray;
		for (const SceneConnectionInfo& info : scene.connections)
		{
			QJsonObject connObj;
			connObj["startNodeId"] = info.startNodeId;
			connObj["inPortIndex"] = 0;
			connObj["endNodeId"] = info.endNodeId;
			connObj["outPortIndex"] = 0;
			connArray.append(connObj);
		}
		connTab[scene.tabName] = connArray;
		connectionsArrays.append(connTab);
	}

	root["Node"] = nodesArrays;
	root["Connection"] = connectionsArrays;

	QJsonDocument doc(root);
	QFile file(filePath);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		qWarning() << QString::fromUtf8("无法打开文件进行写入:") << file.errorString();
		return;
	}
	file.write(doc.toJson(QJsonDocument::Indented));
	file.close();
}