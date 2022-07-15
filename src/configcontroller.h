#ifndef CONFIGCONTROLLER_H
#define CONFIGCONTROLLER_H
#include <vector>
#include "syncentity.h"

class ConfigController
{
public:
    ConfigController();
    static std::vector<SyncEntity> loadSyncEntities();
    static bool saveSyncEntities(std::vector<SyncEntity> rules);
    static QString configPath;
};

#endif // CONFIGCONTROLLER_H
