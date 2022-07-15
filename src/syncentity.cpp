#include "syncentity.h"

bool SyncEntity::operator==(const SyncEntity &other)
{
    return (this->name() == other.name() &&
            this->description() == other.description() &&
            this->srcPath() == other.srcPath() &&
            this->dstPath() == other.dstPath());
}
