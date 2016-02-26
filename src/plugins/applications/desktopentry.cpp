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

#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QProcess>
#include <QDirIterator>
#include <memory>
#include <map>
using std::map;
#include "desktopentry.h"
#include "desktopaction.hpp"
#include "albertapp.h"
#include "iconlookup/xdgiconlookup.h"


QString Applications::DesktopEntry::terminal;
QStringList Applications::DesktopEntry::supportedGraphicalSudo = {"gksu", "kdesu"};

/** ***************************************************************************/
QString Applications::DesktopEntry::text() const {
    return name_;
}



/** ***************************************************************************/
QString Applications::DesktopEntry::subtext() const {
    return (altName_.isNull()) ? exec_ : altName_;
}



/** ***************************************************************************/
QUrl Applications::DesktopEntry::icon() const {
    return iconUrl_;
}



/** ***************************************************************************/
void Applications::DesktopEntry::activate() {
    // Standard action
    qApp->hideMainWindow();
    // Finally since the exec key expects to be interpreted and escapes and
    // expanded and whatelse a shell does with a commandline it is the easiest
    // way to just let the shell do it (therminal has a subshell)
    if(term_)
        QProcess::startDetached(terminal.arg(exec_));
    else
        QProcess::startDetached(QString("sh -c \"%1\"").arg(exec_));
    ++usage_;
}



/** ***************************************************************************/
ActionSPtrVec Applications::DesktopEntry::actions() {
    return actions_;
}



/** ***************************************************************************/
std::vector<QString> Applications::DesktopEntry::aliases() const {
    return std::vector<QString>({name_, altName_, exec_.section(" ",0,0)});
}



/** ***************************************************************************/
bool Applications::DesktopEntry::readDesktopEntry() {
    // TYPES http://standards.freedesktop.org/desktop-entry-spec/latest/ar01s05.html
    map<QString,map<QString,QString>> sectionMap;

    // Read the file
    QFile desktopFile(path_);
    if (desktopFile.open(QIODevice::ReadOnly| QIODevice::Text)) {
        QTextStream stream(&desktopFile);
        QString key;
        QString value;
        QString currentGroup;
        for (QString line=stream.readLine(); !line.isNull(); line=stream.readLine()) {
            line = line.trimmed();

            if (line.startsWith('#') || line.isEmpty())
                continue;

            // Check for groups
            if (line.startsWith("[")){
                currentGroup = line.mid(1,line.size()-2);
                continue;
            }

            key = line.section('=', 0,0).trimmed();
            value = line.section('=', 1, -1).trimmed();
            sectionMap[currentGroup][key]=value;
        }
        desktopFile.close();
    } else return false;

    // Skip if there is no "Desktop Entry" section
    map<QString,map<QString,QString>>::iterator sectionIterator;
    if ((sectionIterator = sectionMap.find("Desktop Entry")) == sectionMap.end())
        return false;

    /*
     * Handle "Desktop Entry" section
     */

    map<QString,QString> &valueMap = sectionIterator->second;
    map<QString,QString>::iterator valueIterator;

    // Skip, if this desktop entry must not be shown
    if ((valueIterator = valueMap.find("NoDisplay")) != valueMap.end() && valueIterator->second == "true")
        return false;

    // Skip if the current desktop environment is not specified in "OnlyShowIn"
    if ((valueIterator = valueMap.find("OnlyShowIn")) != valueMap.end())
        if (!valueIterator->second.split(';',QString::SkipEmptyParts).contains(getenv("XDG_CURRENT_DESKTOP")))
            return false;

    // Try to get the (localized name)
    QString locale = QLocale().name();
    QString shortLocale = locale.left(2);
    if ( (valueIterator = valueMap.find(QString("Name[%1]").arg(locale))) != valueMap.end()
         || (valueIterator = valueMap.find(QString("Name[%1]").arg(shortLocale))) != valueMap.end()
         || (valueIterator = valueMap.find(QString("Name"))) != valueMap.end())
        name_ = escapeString(valueIterator->second);
    else return false;

    // Try to get the exec key (http://standards.freedesktop.org/desktop-entry-spec/latest/ar01s06.html)
    if ((valueIterator = valueMap.find("Exec")) != valueMap.end())
        exec_ = escapeString(valueIterator->second);
    else return false;

    exec_.replace("%f", ""); // Unhandled TODO
    exec_.replace("%F", ""); // Unhandled TODO
    exec_.replace("%u", ""); // Unhandled TODO
    exec_.replace("%U", ""); // Unhandled TODO
    exec_.replace("%d", ""); // Deprecated
    exec_.replace("%D", ""); // Deprecated
    exec_.replace("%n", ""); // Deprecated
    exec_.replace("%N", ""); // Deprecated
    if ((valueIterator = valueMap.find("Icon")) != valueMap.end())
        exec_.replace("%i", QString("--icon %1").arg(valueIterator->second));
    else exec_.replace("%i", "");
    exec_.replace("%c", name_);
    exec_.replace("%k", path_);
    exec_.replace("%v", ""); // Deprecated
    exec_.replace("%m", ""); // Deprecated
    exec_.replace("%%", "%");

    // Try to get the icon
    XdgIconLookup xdg;
    QString iconPath;
    iconUrl_ = QUrl();
    if ((valueIterator = valueMap.find("Icon")) != valueMap.end()){
        iconPath = xdg.lookupIcon(valueIterator->second);
        if (!iconPath.isNull())
            iconUrl_ = QUrl::fromLocalFile(iconPath);
        else{
            iconUrl_ = QUrl();
            qWarning() << valueIterator->second << "was not found";
        }
    }

    // Try to get a default icon if iconUrl_ is still empty
    if (iconUrl_.isEmpty()) {
        iconPath = xdg.lookupIcon("exec");
        if (!iconPath.isNull())
            iconUrl_ = QUrl::fromLocalFile(iconPath);
        else
            qWarning() << "exec" << "was not found";
    }

    // Skip this desktop entry, icons are mandatory
    if (iconUrl_.isEmpty())
        return false;

    // Try to get any [localized] secondary information comment
    if ( (valueIterator = valueMap.find(QString("Comment[%1]").arg(locale))) != valueMap.end()
         || (valueIterator = valueMap.find(QString("Comment[%1]").arg(shortLocale))) != valueMap.end()
         || (valueIterator = valueMap.find(QString("Comment"))) != valueMap.end()
         || (valueIterator = valueMap.find(QString("GenericName[%1]").arg(locale))) != valueMap.end()
         || (valueIterator = valueMap.find(QString("GenericName[%1]").arg(shortLocale))) != valueMap.end()
         || (valueIterator = valueMap.find(QString("GenericName"))) != valueMap.end())
        altName_ = escapeString(valueIterator->second);

    // Root action. (FistComeFirstsServed. TODO: more sophisticated solution)
    for (const QString &s : supportedGraphicalSudo){
        QProcess p;
        p.start("which", {s});
        p.waitForFinished(-1);
        if (p.exitCode() == 0){
            actions_.push_back(std::make_shared<DesktopAction>(
                                   this, QString("Run %1 as root").arg(name_),
                                   QString("%1 \"%2\"").arg(s, exec_)));
            break;
        }
    }

    /*
     * Handle "Desktop Action X" sections
     */

    actions_.clear();
    if ((valueIterator = valueMap.find("Actions")) != valueMap.end()){
        QStringList actionStrings = valueIterator->second.split(';',QString::SkipEmptyParts);
        QString name;
        QString exec;
        for (const QString &actionString: actionStrings){

            // Get iterator to action section
            if ((sectionIterator = sectionMap.find(QString("Desktop Action %1").arg(actionString))) == sectionMap.end())
                continue;
            map<QString,QString> &valueMap = sectionIterator->second;

            // Get action name
            if ((valueIterator = valueMap.find("Name")) == valueMap.end())
                continue;
            name = valueIterator->second;

            // Get action command
            if ((valueIterator = valueMap.find("Exec")) == valueMap.end())
                continue;
            exec = valueIterator->second;

            // App icon of none is specified or lookupf failed
            actions_.push_back(std::make_shared<DesktopAction>(this, name, exec));
        }
    }

    return true;
}



/** ***************************************************************************/
QString Applications::DesktopEntry::escapeString(const QString &unescaped) {
    QString result;
    result.reserve(unescaped.size());

    /*
     * http://standards.freedesktop.org/desktop-entry-spec/latest/ar01s03.html
     *
     * The escape sequences \s, \n, \t, \r, and \\ are supported for values of
     * type string and localestring, meaning ASCII space, newline, tab, carriage
     * return, and backslash, respectively.
     * Some keys can have multiple values. In such a case, the value of the key
     * is specified as a plural: for example, string(s). The multiple values
     * should be separated by a semicolon and the value of the key may be
     * optionally terminated by a semicolon. Trailing empty strings must always
     * be terminated with a semicolon. Semicolons in these values need to be
     * escaped using \;.
     */
    QString::const_iterator it = unescaped.begin();
    while (it != unescaped.end()) {
        if (*it == '\\'){
            ++it;
            if (it == unescaped.end())
                break;
            else if (*it=='s')
                result.append(' ');
            else if (*it=='n')
                result.append('\n');
            else if (*it=='t')
                result.append('\t');
            else if (*it=='r')
                result.append('\r');
            else if (*it=='\\')
                result.append('\\');
        }
        else
            result.append(*it);
        ++it;
    }
    return result;
}



///** ***************************************************************************/
//void Applications::Application::parseCommandLine(const QString &cmdLine, QString *program, QStringList *arguments) {
//    *program = cmdLine.section(' ', 0, 0);

//    QString arg;
//    QString argsString = cmdLine.section(' ', 1);
//    QString::iterator it = argsString.begin();

//    while(1){

//        // End of sting -> copy last param and quit loop
//        if (it == argsString.end()){
//            if (!arg.isEmpty())
//                arguments->append(arg);
//            goto quit_loop;
//        }

//        // Space/parameter delimiter -> copy param
//        else if (*it == ' '){
//            if (!arg.isEmpty())
//                arguments->append(arg);
//            arg.clear();
//        }


//        // Quotes introduce a unescaped sequence
//        else if (*it == '"'){

//            ++it;

//            // Iterate until end of sequence is found
//            while (*it != '"'){

//                // If end of string this command line is invalid. Be tolerant
//                // and store the last param anyway
//                if (it == argsString.end()){
//                    if (!arg.isEmpty())
//                        arguments->append(arg);
//                    goto quit_loop;
//                }

//                // Well no EO, no Quotes -> usual char
//                arg.append(*it);
//                ++it;
//            }

//            // End of sequence, store parameter
//            if (!arg.isEmpty())
//                arguments->append(arg);
//            arg.clear();
//        }


//        // Free (unquoted) escapechar just copy the char after it
//        else if (*it == '\\'){
//            ++it;
//            arg.append(*it);
//        }

//        // usual character
//        else
//            arg.append(*it);

//        ++it;
//    }
//    quit_loop:;
//}
