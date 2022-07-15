#ifndef SYNCENTITY_H
#define SYNCENTITY_H
#include <QString>

class SyncEntity
{ 
public:
    SyncEntity() = default;
    bool operator==(const SyncEntity& other);
    bool operator!=(const SyncEntity& other) { return !(*this == other);}
    inline const QString &name() const                          {return m_name;}
    inline void setName(const QString &newName)                 {m_name = newName;}
    inline const QString &description() const                   {return m_description;}
    inline void setDescription(const QString &newDescription)   {m_description = newDescription;}
    inline const QString &srcPath() const                       {return m_srcPath;}
    inline void setSrcPath(const QString &newSrcPath)           {m_srcPath = newSrcPath;}
    inline const QString &dstPath() const                       {return m_dstPath;}
    inline void setDstPath(const QString &newDstPath)           {m_dstPath = newDstPath;}
    inline uint64_t syncSize() const                            {return m_syncSize;}
    inline void setSyncSize(uint64_t newSyncSize)               {m_syncSize = newSyncSize;}
    inline void addSyncSize(uint64_t size)                      {m_syncSize += size;}
    inline uint64_t syncCount() const                           {return m_syncCount;}
    inline void setSyncCount(uint64_t newSyncCount)             {m_syncCount = newSyncCount;};
    inline void addSyncCount(uint64_t count)                    {m_syncCount += count;};


private:
    QString     m_name;
    QString     m_description;
    QString     m_srcPath;
    QString     m_dstPath;
    uint64_t    m_syncSize{0};
    uint64_t    m_syncCount{0};
};

#endif // SYNCENTITY_H
