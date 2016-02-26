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

#pragma once
#include <vector>
#include "abstractobjects.hpp"
using std::vector;

namespace Applications{

class DesktopAction;

class DesktopEntry final : public AlbertLeaf
{
    friend class Extension;
    class DesktopAction;

public:
    DesktopEntry() = delete;
    DesktopEntry(const DesktopEntry &) = delete;
    DesktopEntry(const QString &path, short usage = 0)
        : path_(path), usage_(usage) {}

    /*
     * Implementations of virtual functions
     */

    QUrl icon() const override;
    QString text() const override;
    QString subtext() const override;
    vector<QString> aliases() const override;
    void activate() override;
    uint16_t usageCount() const override {return usage_;}
    ActionSPtrVec actions() override;

    /*
     * Item specific members
     */

    /** Updates the desktop entry */
    bool readDesktopEntry();

    /** Return the path of the desktop entry */
    const QString& path() const {return path_;}

    /** The command to start applications in a terminal */
    static QString terminal;

    /** The list of supported graphical sudo applications */
    static QStringList supportedGraphicalSudo;

private:
    static QString escapeString(const QString &unescaped);
//    static void parseCommandLine(const QString & cmdLine, QString *program, QStringList *arguments);

    QString path_;
    QString name_;
    QString altName_;
    QUrl    iconUrl_;
    QString exec_;
    bool    term_;
    mutable uint16_t usage_;
    ActionSPtrVec actions_;
};
}
