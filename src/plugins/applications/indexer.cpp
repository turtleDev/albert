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

#include <QDirIterator>
#include <QDebug>
#include <QThread>
#include <vector>
#include <memory>
#include <algorithm>
#include "extension.h"
#include "indexer.h"
#include "desktopentry.h"
using std::vector;
using std::shared_ptr;


/** ***************************************************************************/
void Applications::Extension::Indexer::run() {

    // Notification
    qDebug("[%s] Start indexing in background thread", extension_->name);
    emit statusInfo("Indexing desktop entries ...");

    // Get a new index [O(n)]
    vector<shared_ptr<DesktopEntry>> newIndex;

    // Iterate over all desktop files
    for (const QString &path : extension_->rootDirs_) {
        QDirIterator fIt(path, QStringList("*.desktop"), QDir::Files,
                         QDirIterator::Subdirectories|QDirIterator::FollowSymlinks);
        while (fIt.hasNext()) {
            QString path = fIt.next();

            // Abortion requested
            if (abort_)
                return;

            // Check if desktop entry exists in current index
            vector<shared_ptr<DesktopEntry>>::iterator indexIt =
                    std::find_if(extension_->appIndex_.begin(), extension_->appIndex_.end(),
                                [&path](const shared_ptr<DesktopEntry>& de){ return de->path() == path; });

            // If not make a new desktop entry, else reuse existing
            shared_ptr<DesktopEntry> application =
                    (indexIt == extension_->appIndex_.end()) ? std::make_shared<DesktopEntry>(path) : *indexIt;

            // Update the desktop entry, add to index if succeeded
            if (application->readDesktopEntry())
                newIndex.push_back(application);
        }
    }


    /*
     *  ▼ CRITICAL ▼
     */

    // Lock the access
    extension_->indexAccess_.lock();

    // Set the new index (use swap to shift destruction out of critical area)
    std::swap(extension_->appIndex_, newIndex);

    // Reset the offline index
    extension_->searchIndex_.clear();

    // Build the new offline index
    for (const shared_ptr<DesktopEntry> &de : extension_->appIndex_)
        extension_->searchIndex_.add(de);

    // Unlock the accress
    extension_->indexAccess_.unlock();

    /*
     *  ▲ CRITICAL ▲
     */

    // Finally update the watches (maybe folders changed)
    if (!extension_->watcher_.directories().isEmpty())
        extension_->watcher_.removePaths(extension_->watcher_.directories());
    for (const QString &path : extension_->rootDirs_) {
        QDirIterator dit(path, QDir::Dirs|QDir::NoDotAndDotDot);
        while (dit.hasNext())
            extension_->watcher_.addPath(dit.next());
    }

    // Notification
    qDebug("[%s] Indexing done (%d items)", extension_->name, static_cast<int>(extension_->appIndex_.size()));
    emit statusInfo(QString("Indexed %1 desktop entries").arg(extension_->appIndex_.size()));
}
