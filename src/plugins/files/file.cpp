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

#include <QApplication>
#include <QFileInfo>
#include "file.h"
#include "fileactions.h"
#include "iconlookup/xdgiconlookup.h"

map<QString, QUrl> Files::File::iconCache_;

/** ***************************************************************************/
Files::File::File(QString path, QMimeType mimetype, short usage)
    : path_(path), mimetype_(mimetype), usage_(usage){}



/** ***************************************************************************/
QString Files::File::text() const {
    return QFileInfo(path_).fileName();
}



/** ***************************************************************************/
QString Files::File::subtext() const {
    return path_;
}



/** ***************************************************************************/
QUrl Files::File::iconUrl() const {

    // First check cache
    QString xdgIconName = mimetype_.iconName();
    if (iconCache_.count(xdgIconName))
        return iconCache_[xdgIconName];

    // Lookup iconName
    XdgIconLookup xdg;
    QString iconPath;
    iconPath = xdg.lookupIcon(xdgIconName);
    if (!iconPath.isNull()){
        iconCache_.emplace(xdgIconName, QUrl::fromLocalFile(iconPath));
        return QUrl::fromLocalFile(iconPath);
    }

    // Lookup genericIconName
    QString genericIconName = mimetype_.genericIconName();
    iconPath = xdg.lookupIcon(genericIconName);
    if (!iconPath.isNull()){
        iconCache_.emplace(xdgIconName, QUrl::fromLocalFile(iconPath)); // Intended
        return QUrl::fromLocalFile(iconPath);
    }

    // Lookup "unknown"
    iconPath = xdg.lookupIcon("unknown");
    if (!iconPath.isNull()){
        iconCache_.emplace(xdgIconName, QUrl::fromLocalFile(iconPath)); // Intended
        return QUrl::fromLocalFile(iconPath);
    }

    // Return empty url
    return QUrl();
}



/** ***************************************************************************/
void Files::File::activate() {
//    Qt::KeyboardModifiers mods = QApplication::keyboardModifiers(); // TODO:ActionMap
    // Standard action for a file
    OpenFileAction(this).activate();
}



/** ***************************************************************************/
ActionSPtrVec Files::File::actions() {
    // Lazy instaciate actions
    if (!children_){
        children_ = unique_ptr<ActionSPtrVec>(new ActionSPtrVec);
        children_->push_back(std::make_shared<RevealFileAction>(this));
        children_->push_back(std::make_shared<CopyFileAction>(this));
        children_->push_back(std::make_shared<CopyPathAction>(this));
    }
    return *children_;
}



/** ***************************************************************************/
std::vector<QString> Files::File::aliases() const {
    return std::vector<QString>({QFileInfo(path_).fileName()});
}




/** ***************************************************************************/
void Files::File::clearIconCache() {
    iconCache_.clear();
}
