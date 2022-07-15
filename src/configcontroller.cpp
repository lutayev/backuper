#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFile>
#include <QFileInfo>
#include <QDir>


#include "configcontroller.h"

QString ConfigController::configPath = "config.json";

ConfigController::ConfigController()
{

}

std::vector<SyncEntity> ConfigController::loadSyncEntities()
{
    std::vector<SyncEntity> result;

    //Open JSON file
    QFile file(configPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Error open json file";
        return result;
    }

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &err);
    file.close();

    if (err.error != QJsonParseError::NoError) {
        qDebug() << "Can't load json file, error: " + err.errorString();
        return result;
    }

    //Read rules
    QJsonArray rules = doc.object().value("rules").toArray();

    for (const QJsonValue& it : qAsConst(rules)) {
        SyncEntity item;
        item.setName(it.toObject().value("name").toString());
        item.setDescription(it.toObject().value("description").toString());
        item.setSrcPath(it.toObject().value("source").toString());
        item.setDstPath(it.toObject().value("destination").toString());

        result.push_back(item);
    }

    return result;
}

bool ConfigController::saveSyncEntities(std::vector<SyncEntity> rules)
{
    //Write rules
    QJsonObject object;
    QJsonArray array;
    for (const SyncEntity& it : rules) {

        QJsonObject item;
        QJsonValue name(it.name());
        QJsonValue description(it.description());
        QJsonValue source(it.srcPath());
        QJsonValue destination(it.dstPath());


        item.insert("name", name);
        item.insert("description", description);
        item.insert("source", source);
        item.insert("destination", destination);

        array.push_back(item);
    }

    if (array.empty()) {
        qDebug() << "Empty JSON array";
        return false;
    }

    object.insert("rules", array);

    QFileInfo fileInfo(configPath);
    QDir::setCurrent(fileInfo.path());

    //Create a file, write JSON object to it
    QFile jsonFile(configPath);
    if (!jsonFile.open(QIODevice::WriteOnly))
    {
        qDebug() << "Can't open file to write";
        return false;
    }

    jsonFile.write(QJsonDocument(object).toJson(QJsonDocument::Indented));
    jsonFile.close();
    return true;
}
