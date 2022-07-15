#ifndef CORE_H
#define CORE_H

#include <QDialog>
#include <QDir>
#include <QTreeWidgetItem>
#include "configcontroller.h"
#include "syncentity.h"

namespace Ui {
class Core;
}

class Core : public QWidget
{
    Q_OBJECT

public:
    explicit Core(QWidget *parent = nullptr);
    ~Core();

private slots:
    //Backup tab slots
    void scanSelected();
    void syncSelected();
    void loadSyncView();
    void loadSettingsView();
    void reloadView();

    //Settings tab slots
    void addRule();
    void delRule();
    void showRule();
    void editRule();
    void applyRule();
    void discardRule();
    void saveConfig();
    void setRuleEditMode(bool isEdit);
    void chooseSrcPath();
    void chooseDstPath();

private:
    bool syncDir(SyncEntity& entity, bool estimateRun = false);
    QString sizeToStr(uint64_t size) const;

    Ui::Core *ui;
    std::vector<SyncEntity> m_syncEntities;
    bool m_confChanged{false};

};

#endif // CORE_H
