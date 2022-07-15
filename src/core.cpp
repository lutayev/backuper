#include <QApplication>
#include <QGuiApplication>
#include <QScreen>
#include <QFileSystemModel>
#include <QDirModel>
#include <QTreeView>
#include <QDebug>
#include <QDateTime>
#include <QVariant>
#include <QMessageBox>
#include <QFileDialog>

#include "core.h"
#include "ui_core.h"

Core::Core(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Core)
{
    ui->setupUi(this);
    QRect scrSize = QGuiApplication::primaryScreen()->geometry();
    this->resize(scrSize.width()/2, scrSize.height()/2);
    this->move(scrSize.width()/4, scrSize.height()/4);
    this->setWindowTitle("Backuper");

    reloadView();

    //Prevent switch tab when have unsaved changes in config
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, [&]() {
        if (m_confChanged) {
            QMessageBox::critical(this, QObject::tr("Error"), QObject::tr("Save or reset uncommitted changes of config first"));
            ui->tabWidget->blockSignals(true);
            ui->tabWidget->setCurrentIndex(1);
            ui->tabWidget->blockSignals(false);
        }
    });

    //Connects for "Backup" tab elements
    connect(ui->btnSync, &QPushButton::pressed, this, &Core::syncSelected);
    connect(ui->btnScan, &QPushButton::pressed, this, &Core::scanSelected);

    //Connects for "Settings" tab elements
    connect(ui->lstRules, &QListWidget::currentItemChanged, this, &Core::showRule);
    connect(ui->lstRules, &QListWidget::itemDoubleClicked, this, &Core::editRule);
    connect(ui->btnRuleAdd, &QPushButton::clicked, this, &Core::addRule);
    connect(ui->btnRuleDel, &QPushButton::clicked, this, &Core::delRule);
    connect(ui->btnRuleEdit, &QPushButton::clicked, this, &Core::editRule);
    connect(ui->btnRuleApply, &QPushButton::clicked, this, &Core::applyRule);
    connect(ui->btnRuleDiscard, &QPushButton::clicked, this, &Core::discardRule);
    connect(ui->btnConfSave, &QPushButton::clicked, this, &Core::saveConfig);
    connect(ui->btnConfReset, &QPushButton::clicked, this, &Core::reloadView);
    connect(ui->btnRuleSrc, &QPushButton::clicked, this, &Core::chooseSrcPath);
    connect(ui->btnRuleDst, &QPushButton::clicked, this, &Core::chooseDstPath);
}

Core::~Core()
{
    delete ui;
}

bool Core::syncDir(SyncEntity& entity, bool estimateRun)
{
    QString srcPath = entity.srcPath();
    QString dstPath = entity.dstPath();

    QDir srcDir(srcPath);
    QDir dstDir(dstPath);

    if (!dstDir.exists() && !estimateRun) {
        dstDir.mkdir(dstPath);
    }

    //Copy/replace outdated/non-existed
    bool ok = true;
    for (auto& it : srcDir.entryInfoList(QDir::NoFilter, QDir::SortFlag::DirsFirst)) {
        if (it.fileName() == "." || it.fileName() == "..") {
            continue;
        }

        if (it.isDir()) {
            SyncEntity sub;
            sub.setSrcPath(srcPath + "/" + it.fileName());
            sub.setDstPath(dstPath + "/" + it.fileName());
            sub.setSyncCount(entity.syncCount());
            sub.setSyncSize(entity.syncSize());
            syncDir(sub, estimateRun);
            entity.setSyncCount(sub.syncCount());
            entity.setSyncSize(sub.syncSize());
            continue;
        }

        QFile srcFile(srcDir.path() + "/" + it.fileName());
        QFile dstFile(dstDir.path() + "/" + it.fileName());
        QFileInfo srcInfo(srcFile);
        QFileInfo dstInfo(dstFile);

        //Safe-replace method
        if (!dstFile.exists() || !(srcFile.size() == dstFile.size()) || srcInfo.lastModified() != dstInfo.lastModified()) {
            entity.addSyncSize(srcFile.size());
            entity.addSyncCount(1);
            if (estimateRun)
                continue;
            ok = QFile::copy(srcPath + "/" + it.fileName(), dstPath + "/" + it.fileName() + "backuper_temporary");
            if (ok) {
                QFile::remove(dstPath + "/" + it.fileName());
                QFile::rename(dstPath + "/" + it.fileName() + "backuper_temporary", dstPath + "/" + it.fileName());
            } else {
                return false;
            }
            ui->progressBar->setValue(entity.syncSize() / 1000);
            QApplication::processEvents();
        }
    }

    //Remove files and directories that not exists in source
    for (auto& it : dstDir.entryInfoList(QDir::NoFilter, QDir::SortFlag::DirsFirst)) {
        if (it.fileName() == "." || it.fileName() == "..") {
            continue;
        }

        if (it.isDir()) {
            SyncEntity sub;
            sub.setSrcPath(srcPath + "/" + it.fileName());
            sub.setDstPath(dstPath + "/" + it.fileName());
            sub.setSyncCount(entity.syncCount());
            sub.setSyncSize(entity.syncSize());
            syncDir(sub, estimateRun);
            entity.setSyncCount(sub.syncCount());
            entity.setSyncSize(sub.syncSize());
            continue;
        }

        QFile srcFile(srcPath + "/" + it.fileName());
        QFile dstFile(dstPath + "/" + it.fileName());

        if (dstFile.exists() && !srcFile.exists()) {
            entity.addSyncCount(1);
            if (!estimateRun) {
                ok = QFile::remove(it.absoluteFilePath());
                if (!ok) {
                    return false;
                }
            }
        }
    }
    if (dstDir.exists() && dstDir.isEmpty() && !srcDir.exists()) {
        entity.addSyncCount(1);
        if (!estimateRun) {
            dstDir.rmdir(dstPath);
        }
    }

    return ok;
}

void Core::scanSelected()
{
    uint64_t totalSize = 0;
    for (int i = 0; i < m_syncEntities.size(); ++i) {
        if (ui->srcViewTree->topLevelItem(i)->checkState(0) == Qt::Unchecked) {
            continue;
        }
        SyncEntity& it = m_syncEntities[i];
        it.setSyncCount(0);
        it.setSyncSize(0);
        ui->tabWidget->setEnabled(false);
        syncDir(it, true);
        ui->tabWidget->setEnabled(true);
        it.setSyncCount(it.syncCount());
        it.setSyncSize(it.syncSize());
        totalSize += it.syncSize();

        ui->srcViewTree->topLevelItem(i)->setData(2, Qt::DisplayRole, it.syncCount());
        ui->srcViewTree->topLevelItem(i)->setData(3, Qt::DisplayRole, sizeToStr(it.syncSize()));
    }
    if (totalSize) {
        ui->progressBar->setMaximum(totalSize / 1000);
    } else {
        ui->progressBar->setMaximum(1);
    }
    ui->progressBar->setValue(0);
}

void Core::syncSelected()
{
    scanSelected();
    ui->tabWidget->setEnabled(false);
    QString resStatus;
    int i = 0;
    uint64_t syncedSize = 0;
    uint64_t syncedCount = 0;
    for (SyncEntity& it : m_syncEntities) {
        it.setSyncSize(syncedSize);
        it.setSyncCount(syncedCount);
        if (syncDir(it, false)) {
            resStatus = "OK";
        } else {
            resStatus = "ERROR";
        }
        syncedSize += it.syncSize();
        syncedCount += it.syncCount();
        ui->srcViewTree->topLevelItem(i)->setData(2, Qt::DisplayRole, 0);
        ui->srcViewTree->topLevelItem(i)->setData(3, Qt::DisplayRole, resStatus);
        i++;
    }
    ui->tabWidget->setEnabled(true);
    ui->progressBar->setValue(ui->progressBar->maximum());
}

void Core::loadSyncView()
{
    ui->srcViewTree->clear();
    ui->progressBar->setMaximum(0);

    for (const SyncEntity& it : m_syncEntities) {
        QStringList itemData;
        itemData << it.srcPath() << it.dstPath() << "SCAN REQUIRED" << "SCAN REQUIRED";
        QTreeWidgetItem* item = new QTreeWidgetItem(itemData);
        item->setCheckState(0, Qt::Checked);
        ui->srcViewTree->addTopLevelItem(item);
    }
    ui->srcViewTree->resizeColumnToContents(0);
    ui->progressBar->setMaximum(0);
    ui->progressBar->setValue(0);
}

void Core::loadSettingsView()
{
    ui->lstRules->clear();
    ui->leRuleName->clear();
    ui->txtRuleDescription->clear();
    ui->leRuleSrc->clear();
    ui->leRuleDst->clear();
    ui->ltRulesList->setEnabled(true);
    ui->ltRuleEdit->setEnabled(false);

    for (const SyncEntity& it : m_syncEntities) {
        ui->lstRules->addItem(it.name());
    }
    setRuleEditMode(false);
}

void Core::reloadView()
{
    m_confChanged = false;
    m_syncEntities = ConfigController::loadSyncEntities();

    loadSyncView();
    loadSettingsView();
}

void Core::addRule()
{
    SyncEntity newItem;
    newItem.setName("New Rule");

    m_syncEntities.push_back(newItem);
    loadSettingsView();
    ui->lstRules->setCurrentRow(m_syncEntities.size() - 1);
    editRule();
}

void Core::delRule()
{
    auto it = m_syncEntities.begin();
    for (int i = 0; i < ui->lstRules->currentRow(); ++i) {
        ++it;
    }

    m_syncEntities.erase(it);
    loadSettingsView();
}

void Core::showRule()
{
    if (ui->lstRules->currentRow() >= m_syncEntities.size() || ui->lstRules->currentRow() < 0) {
        return;
    }
    SyncEntity& rule = m_syncEntities[ui->lstRules->currentRow()];
    ui->leRuleName->setText(rule.name());
    ui->txtRuleDescription->setPlainText(rule.description());
    ui->leRuleSrc->setText(rule.srcPath());
    ui->leRuleDst->setText(rule.dstPath());
}

void Core::editRule()
{
    if (ui->lstRules->currentRow() >= m_syncEntities.size() || ui->lstRules->currentRow() < 0) {
        return;
    }
    SyncEntity& rule = m_syncEntities[ui->lstRules->currentRow()];
    ui->leRuleName->setText(rule.name());
    ui->txtRuleDescription->setPlainText(rule.description());
    ui->leRuleSrc->setText(rule.srcPath());
    ui->leRuleDst->setText(rule.dstPath());

    setRuleEditMode(true);
}

void Core::applyRule()
{
    if (ui->lstRules->currentRow() >= m_syncEntities.size() || ui->lstRules->currentRow() < 0) {
        return;
    }
    SyncEntity& rule = m_syncEntities[ui->lstRules->currentRow()];
    rule.setName(ui->leRuleName->text());
    rule.setDescription(ui->txtRuleDescription->toPlainText());
    rule.setSrcPath(ui->leRuleSrc->text());
    rule.setDstPath(ui->leRuleDst->text());
    ui->lstRules->currentItem()->setText(rule.name());

    setRuleEditMode(false);
}

void Core::discardRule()
{
    if (ui->lstRules->currentRow() >= m_syncEntities.size() || ui->lstRules->currentRow() < 0) {
        return;
    }
    SyncEntity& rule = m_syncEntities[ui->lstRules->currentRow()];
    ui->leRuleName->setText(rule.name());
    ui->txtRuleDescription->setPlainText(rule.description());
    ui->leRuleSrc->setText(rule.srcPath());
    ui->leRuleDst->setText(rule.dstPath());

    setRuleEditMode(false);
}

void Core::saveConfig()
{
    m_confChanged = false;
    if (!ConfigController::saveSyncEntities(m_syncEntities)) {
        ui->lblConfPath->setText("ERROR SAVING CONFIG");
    } else {
        ui->lblConfPath->setText("Succesfully saved config");
    }
    loadSyncView();
}

void Core::setRuleEditMode(bool isEdit)
{
    //Switch state of rules list elements
    ui->lstRules->setEnabled(!isEdit);
    ui->btnConfReset->setEnabled(!isEdit);
    ui->btnConfSave->setEnabled(!isEdit);
    ui->btnRuleAdd->setEnabled(!isEdit);
    ui->btnRuleDel->setEnabled(!isEdit);
    ui->btnRuleEdit->setEnabled(!isEdit);

    //Switch state of rule edit elements
    ui->leRuleName->setEnabled(isEdit);
    ui->txtRuleDescription->setEnabled(isEdit);
    ui->leRuleSrc->setEnabled(isEdit);
    ui->leRuleDst->setEnabled(isEdit);
    ui->btnRuleApply->setEnabled(isEdit);
    ui->btnRuleDiscard->setEnabled(isEdit);
    ui->btnRuleSrc->setEnabled(isEdit);
    ui->btnRuleDst->setEnabled(isEdit);

    //Check is config was changed
    std::vector<SyncEntity> saved = ConfigController::loadSyncEntities();

    if (saved.size() != m_syncEntities.size()) {
        m_confChanged = true;
        return;
    }
    for (int i = 0; i < saved.size(); ++i) {
        if (saved[i] != m_syncEntities[i]) {
            m_confChanged = true;
            break;
        }
        m_confChanged = false;
    }
}

void Core::chooseSrcPath()
{
    ui->leRuleSrc->setText(QFileDialog::getExistingDirectory(this, "", ui->leRuleSrc->text()));
}

void Core::chooseDstPath()
{
    ui->leRuleDst->setText(QFileDialog::getExistingDirectory(this, "", ui->leRuleDst->text()));
}

QString Core::sizeToStr(uint64_t size) const
{
    int count = 0;
    QString suffix;
    while (size > 1024) {
        size /= 1024;
        ++count;
    }

    switch(count) {
    case 0:
        suffix = "B";
        break;
    case 1:
        suffix = "KB";
        break;
    case 2:
        suffix = "MB";
        break;
    case 3:
        suffix = "GB";
        break;
    case 4:
        suffix = "TB";
        break;
    case 5:
        suffix = "PB";
        break;
    case 6:
        suffix = "EB";
        break;
    default:
        suffix = "ERROR NUMBER";
    }

    return QString::number(size) + " " + suffix;
}
