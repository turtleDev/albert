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
#include <QStandardPaths>
#include <QMessageBox>
#include <QDebug>
#include <csignal>
#include "albertapp.h"
#include "mainwindow.h"
#include "settingswidget.h"
#include "hotkeymanager.h"
#include "pluginmanager.h"
#include "extensionmanager.h"

/** ***************************************************************************/
AlbertApp::AlbertApp(int &argc, char *argv[]) : QApplication(argc, argv) {

    // MAKE SURE THE NEEDED DIRECTORIES EXIST
    QDir dataDir(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation));
    if (!dataDir.exists())
        dataDir.mkpath(".");

    QDir configDir(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
    if (!configDir.exists())
        configDir.mkpath(".");

    /*
     *  INITIALIZE APPLICATION
     */

    this->setOrganizationDomain("albert");
    this->setApplicationName("albert");
    this->setApplicationDisplayName("Albert");
    this->setApplicationVersion("v0.9.0");
    this->setWindowIcon(QIcon(":app_icon"));
    this->setQuitOnLastWindowClosed(false); // Dont quit after settings close

    /*
     * INITIALIZE WINDOW
     */

    mainWindow_ = new MainWindow;
    hotkeyManager_ = new HotkeyManager;
    pluginManager_ = new PluginManager;
    extensionManager_ = new ExtensionManager;


    // TODO das muss ohne gehen. Loader muss laszyload können
    // Propagade the extensions once
    for (const unique_ptr<PluginSpec> &p : pluginManager_->plugins())
        if (p->isLoaded())
            extensionManager_->registerExtension(p->instance());

    /*
     *  SETUP SIGNAL FLOW
     */
    // Publish loaded plugins to the specific interface handlers
    QObject::connect(pluginManager_, &PluginManager::pluginLoaded, extensionManager_, &ExtensionManager::registerExtension);
    QObject::connect(pluginManager_, &PluginManager::pluginAboutToBeUnloaded, extensionManager_, &ExtensionManager::unregisterExtension);

    // Show mainWindow if hotkey is pressed
    QObject::connect(hotkeyManager_, &HotkeyManager::hotKeyPressed, mainWindow_, &MainWindow::toggleVisibility);

    // Setup and teardown query sessions with the state of the widget
    QObject::connect(mainWindow_, &MainWindow::visibleChanged, extensionManager_, &ExtensionManager::setSessionActive);
    // A change in text triggers requests
    QObject::connect(mainWindow_, &MainWindow::queryChanged, extensionManager_, &ExtensionManager::startQuery);
    // Activation forwarding
    QObject::connect(mainWindow_, &MainWindow::indexActivated, extensionManager_, &ExtensionManager::activateIndex);
    // Click on settingsButton (or shortcut) closes albert + opens settings dialog
    QObject::connect(mainWindow_, &MainWindow::settingsWindowRequested, this, &AlbertApp::openSettingsWindow);
    QObject::connect(mainWindow_, &MainWindow::settingsWindowRequested, mainWindow_, &MainWindow::hide);

    // Extrension manager signals new proposals
    QObject::connect(extensionManager_, &ExtensionManager::newModel, mainWindow_, &MainWindow::setModel);

    // Quit gracefully on SIGTERM
    signal(SIGTERM, [](int){ QMetaObject::invokeMethod(qApp, "quit", Qt::QueuedConnection); });
}



/** ***************************************************************************/
AlbertApp::~AlbertApp() {
    // Unload the plugins
    delete extensionManager_;
    delete pluginManager_;
    delete hotkeyManager_;
    delete mainWindow_;
}



/** ***************************************************************************/
int AlbertApp::exec() {
    //  HOTKEY  //  Albert without hotkey is useless. Force it!
    QSettings s;
    QVariant v;
    if (!(s.contains("hotkey") && (v=s.value("hotkey")).canConvert(QMetaType::QString)
            && hotkeyManager_->registerHotkey(v.toString()))){
        QMessageBox msgBox(QMessageBox::Critical, "Error",
                           "Hotkey is not set or invalid. Press ok to open "
                           "the settings or press close to quit albert.",
                           QMessageBox::Close|QMessageBox::Ok);
        msgBox.exec();
        if ( msgBox.result() == QMessageBox::Ok ) {
            //hotkeyManager->disable();
            openSettingsWindow();
            //QObject::connect(settingsWidget, &QWidget::destroyed, hotkeyManager, &HotkeyManager::enable);
        }
        else
            return EXIT_FAILURE;
    }
    return QApplication::exec();
}



/** ***************************************************************************/
QSettings AlbertApp::gSettings() {
    qFatal("gSettings: Not implemented");
}



/** ***************************************************************************/
void AlbertApp::openSettingsWindow() {
    if (!settingsWidget_)
        settingsWidget_ = new SettingsWidget(hotkeyManager_, pluginManager_);
    settingsWidget_->show();
    settingsWidget_->raise();
}



/** ***************************************************************************/
void AlbertApp::showMainWindow() {
    mainWindow_->show();
}



/** ***************************************************************************/
void AlbertApp::hideMainWindow() {
    mainWindow_->hide();
}
