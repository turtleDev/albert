// albert - a simple application launcher for linux
// Copyright (C) 2014-2015 Manuel Schneider
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <QDir>
#include <QDirIterator>
#include <QDebug>
#include <QStandardPaths>
#include <QMessageBox>
#include <QCloseEvent>
#include <QShortcut>
#include <QDesktopWidget>
#include <QFocusEvent>
#include "settingswidget.h"
#include "mainwindow.h"
#include "hotkeymanager.h"
#include "pluginmanager.h"
#include "pluginmodel.h"
#include "iextension.h"
#include "propertyeditor.h"

/** ***************************************************************************/
SettingsWidget::SettingsWidget(MainWindow *mainWindow, HotkeyManager *hotkeyManager, PluginManager *pluginManager, QWidget *parent, Qt::WindowFlags f)
    : QWidget(parent, f), mainWindow_(mainWindow), hotkeyManager_(hotkeyManager), pluginManager_(pluginManager) {

    ui.setupUi(this);
    setWindowFlags(Qt::Window|Qt::WindowCloseButtonHint);
    setAttribute(Qt::WA_DeleteOnClose);


    /*
     * GENERAL TAB
     */

    // HOTKEY STUFF
    QSet<int> hks = hotkeyManager->hotkeys();
    if (hks.size() < 1)
        ui.grabKeyButton_hotkey->setText("Press to set hotkey");
    else
        ui.grabKeyButton_hotkey->setText(QKeySequence(*hks.begin()).toString()); // OMG
    connect(ui.grabKeyButton_hotkey, &GrabKeyButton::keyCombinationPressed,
            this, &SettingsWidget::changeHotkey);

    // ALWAYS ON TOP
    ui.checkBox_onTop->setChecked(mainWindow_->alwaysOnTop());
    connect(ui.checkBox_onTop, &QCheckBox::toggled, mainWindow_, &MainWindow::setAlwaysOnTop);

    // HIDE ON FOCUS OUT
    ui.checkBox_hideOnFocusOut->setChecked(mainWindow_->hideOnFocusLoss());
    connect(ui.checkBox_hideOnFocusOut, &QCheckBox::toggled, mainWindow_, &MainWindow::setHideOnFocusLoss);

    // ALWAYS CENTER
    ui.checkBox_center->setChecked(mainWindow_->showCentered());
    connect(ui.checkBox_center, &QCheckBox::toggled, mainWindow_, &MainWindow::setShowCentered);

    // STYLES
    // Get style dirs
    QStringList styleDirPaths = QStandardPaths::locateAll(
                QStandardPaths::AppDataLocation,
                "styles", QStandardPaths::LocateDirectory);
    // Get style files
    QFileInfoList styles;
    for (QString &styleDirPath : styleDirPaths){
        QDirIterator it(styleDirPath, {"*.qml"}, QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            it.next();
            styles << it.fileInfo();
        }
    }
    // Fill the combobox
    int i = 0 ;
    ui.comboBox_style->addItem("Standard", QUrl(mainWindow_->DEF_STYLEPATH));
    for (QFileInfo &style : styles) {
        ui.comboBox_style->addItem(style.baseName(), QUrl(style.canonicalFilePath()));
        if ( QUrl(style.canonicalFilePath()) == mainWindow_->source() )
            ui.comboBox_style->setCurrentIndex(i);
        ++i;
    }
    connect(ui.comboBox_style, (void (QComboBox::*)(int))&QComboBox::currentIndexChanged,
            this, &SettingsWidget::onThemeChanged);

    // PROPERTY EDITOR
    connect(ui.toolButton_propertyEditor, &QToolButton::clicked, [this](){
        PropertyEditor pe(mainWindow_, this);
        pe.exec();
    });

    // PRESETS
    ui.comboBox_presets->clear();
    ui.comboBox_presets->addItem("Load preset...");
    ui.comboBox_presets->addItems(mainWindow_->availablePresets());
    connect(ui.comboBox_presets, (void (QComboBox::*)(const QString &))&QComboBox::currentIndexChanged,
            this, &SettingsWidget::onPresetChanged);



    /*
     * PLUGIN  TAB
     */

    // Show the plugins. This* widget takes ownership of the model
    ui.listView_plugins->setModel(new PluginModel(pluginManager_, ui.listView_plugins));

    // Update infos when item is changed
    connect(ui.listView_plugins->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &SettingsWidget::updatePluginInformations);

    connect(ui.listView_plugins->model(), &QAbstractListModel::dataChanged,
            this, &SettingsWidget::onPluginDataChanged);

    /*
     * ABOUT TAB
     */
    ui.about_text->setText(QString(ui.about_text->text()).replace("___versionstring___", qApp->applicationVersion()));

    QDesktopWidget *dw = QApplication::desktop();
    move(dw->availableGeometry(dw->screenNumber(QCursor::pos())).center()
                -QPoint(width()/2,height()/2));
    raise();
    activateWindow();
}



/** ***************************************************************************/
void SettingsWidget::updatePluginInformations(const QModelIndex & current) {
    // Hidde the placehodler text
    QLayoutItem *i = ui.widget_pluginInfos->layout()->takeAt(0);
    delete i->widget();
    delete i;

    if (pluginManager_->plugins()[current.row()]->isLoaded()){
        ui.widget_pluginInfos->layout()->addWidget(
                    dynamic_cast<IExtension*>(pluginManager_->plugins()[current.row()]->instance())->widget()); // Takes ownership
    }
    else{
        QLabel *lbl = new QLabel("Plugin not loaded.");
        lbl->setEnabled(false);
        lbl->setAlignment(Qt::AlignCenter);
        ui.widget_pluginInfos->layout()->addWidget(lbl);
    }
}



/** ***************************************************************************/
void SettingsWidget::changeHotkey(int newhk) {
    int oldhk = *hotkeyManager_->hotkeys().begin(); //TODO Make cool sharesdpointer design

    // Try to set the hotkey
    if (hotkeyManager_->registerHotkey(newhk)) {
        QString hkText(QKeySequence((newhk&~Qt::GroupSwitchModifier)).toString());//QTBUG-45568
        ui.grabKeyButton_hotkey->setText(hkText);
        QSettings().setValue("hotkey", hkText);
        hotkeyManager_->unregisterHotkey(oldhk);
    } else {
        ui.grabKeyButton_hotkey->setText(QKeySequence(oldhk).toString());
        QMessageBox(QMessageBox::Critical, "Error",
                    QKeySequence(newhk).toString()
                    + " could not be registered.").exec();
    }
}



/** ***************************************************************************/
void SettingsWidget::onThemeChanged(int i) {
    // Apply the theme
    QUrl url = ui.comboBox_style->itemData(i).toUrl();
    mainWindow_->setSource(url);

    // Fill presets
    ui.comboBox_presets->clear();
    ui.comboBox_presets->addItem("Load preset...");
    ui.comboBox_presets->addItems(mainWindow_->availablePresets());
}



/** ***************************************************************************/
void SettingsWidget::onPresetChanged(const QString &text) {
    // Remove the placeholder
    if (text != "Load preset..." && ui.comboBox_presets->itemText(0) == "Load preset...")
        ui.comboBox_presets->removeItem(0);

    // Apply the preset
    mainWindow_->setPreset(text);
}



/** ***************************************************************************/
void SettingsWidget::onPluginDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles) {
    Q_UNUSED(bottomRight)
    if (topLeft == ui.listView_plugins->currentIndex())
        for (int role : roles)
            if (role == Qt::CheckStateRole)
                updatePluginInformations(topLeft);
}



/** ***************************************************************************/
void SettingsWidget::keyPressEvent(QKeyEvent *event) {
    if (event->modifiers() == Qt::NoModifier && event->key() == Qt::Key_Escape ) {
        close();
    }
}



/** ***************************************************************************/
void SettingsWidget::closeEvent(QCloseEvent *event) {
    if (hotkeyManager_->hotkeys().empty()) {
        QMessageBox msgBox(QMessageBox::Critical, "Error",
                           "Hotkey is invalid, please set it. Press Ok to go "\
                           "back to the settings, or press Cancel to quit albert.",
                           QMessageBox::Close|QMessageBox::Ok);
        msgBox.exec();
        if ( msgBox.result() == QMessageBox::Ok ) {
            ui.tabs->setCurrentIndex(0);
            show();
        }
        else
            qApp->quit();
        event->ignore();
        return;
    }
    event->accept();
}
