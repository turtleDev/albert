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
#include "application.h"
#include "desktopaction.h"
#include "albertapp.h"


QString Applications::Application::terminal;

/** ***************************************************************************/
QString Applications::Application::text() const {
    return name_;
}



/** ***************************************************************************/
QString Applications::Application::subtext() const {
    return (altName_.isNull()) ? exec_ : altName_;
}



/** ***************************************************************************/
QIcon Applications::Application::icon() const {
    return icon_;
}



/** ***************************************************************************/
void Applications::Application::activate() {
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
bool Applications::Application::hasChildren() const {
    return true;
}




/** ***************************************************************************/
vector<shared_ptr<AlbertItem> > Applications::Application::children() {
    return actions_;
}



/** ***************************************************************************/
std::vector<QString> Applications::Application::aliases() const {
    return std::vector<QString>({name_, altName_, exec_.section(" ",0,0)});
}



/** ***************************************************************************/
bool Applications::Application::readDesktopEntry() {
    // TYPES http://standards.freedesktop.org/desktop-entry-spec/latest/ar01s05.html
    map<QString,map<QString,QString>> values;

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
            values[currentGroup][key]=value;
        }
        desktopFile.close();
    } else return false;


    if (values["Desktop Entry"]["NoDisplay"] == "true")
        return false;


    // Try to get the (localized name)
    QString locale = QLocale().name();
    QString shortLocale = locale.left(2);
    if (values["Desktop Entry"].count(QString("Name[%1]").arg(locale)))
        name_ = escapeString(values["Desktop Entry"][QString("Name[%1]").arg(locale)]);
    else if (values["Desktop Entry"].count(QString("Name[%1]").arg(shortLocale)))
        name_ = escapeString(values["Desktop Entry"][QString("Name[%1]").arg(shortLocale)]);
    else if (values["Desktop Entry"].count("Name"))
        name_ = escapeString(values["Desktop Entry"]["Name"]);
    else return false;



    /*
     * The Exec Key - pretty complicated stuff
     * http://standards.freedesktop.org/desktop-entry-spec/latest/ar01s06.html
     */
    if (values["Desktop Entry"].count("Exec"))
        exec_ = escapeString(values["Desktop Entry"]["Exec"]);
    else return false;

    exec_.replace("%f", ""); // Unhandled TODO
    exec_.replace("%F", ""); // Unhandled TODO
    exec_.replace("%u", ""); // Unhandled TODO
    exec_.replace("%U", ""); // Unhandled TODO
    exec_.replace("%d", ""); // Deprecated
    exec_.replace("%D", ""); // Deprecated
    exec_.replace("%n", ""); // Deprecated
    exec_.replace("%N", ""); // Deprecated
    if (values["Desktop Entry"].count("Icon"))
        exec_.replace("%i", QString("--icon %1").arg(values["Desktop Entry"]["Icon"]));
    else exec_.replace("%i", "");
    exec_.replace("%c", name_);
    exec_.replace("%k", path_);
    exec_.replace("%v", ""); // Deprecated
    exec_.replace("%m", ""); // Deprecated
    exec_.replace("%%", "%");

    // Try to get the icon
    if (values["Desktop Entry"].count("Icon"))
        icon_ = getIcon(values["Desktop Entry"]["Icon"]);
    else {
        qWarning() << "No icon specified in " << path_;
        icon_ = QIcon::fromTheme("exec");
    }


    // Try to get any [localized] secondary information comment
    if (values["Desktop Entry"].count(QString("Comment[%1]").arg(locale)))
        altName_ = values["Desktop Entry"][QString("Comment[%1]").arg(locale)];
    else if (values["Desktop Entry"].count(QString("Comment[%1]").arg(shortLocale)))
        altName_ = values["Desktop Entry"][QString("Comment[%1]").arg(shortLocale)];
    else if (values["Desktop Entry"].count("Comment"))
        altName_ = values["Desktop Entry"]["Comment"];
    else if (values["Desktop Entry"].count(QString("GenericName[%1]").arg(locale)))
        altName_ = values["Desktop Entry"][QString("GenericName[%1]").arg(locale)];
    else if (values["Desktop Entry"].count(QString("GenericName[%1]").arg(shortLocale)))
        altName_ = values["Desktop Entry"][QString("GenericName[%1]").arg(shortLocale)];
    else if (values["Desktop Entry"].count("GenericName"))
        altName_ = values["Desktop Entry"]["GenericName"];

    // No additional actions for terminal apps
    term_ = values["Desktop Entry"]["Terminal"] == "true";
    if(term_)
        return true;

    // Root actions
    QStringList graphicalSudos({"gksu", "kdesu"});
    for (const QString &s : graphicalSudos){
        QProcess p;
        p.start("which", {s});
        p.waitForFinished(-1);
        if (p.exitCode() == 0)
            actions_.push_back(std::make_shared<DesktopAction>(this,
                                                               QString("Run %1 as root").arg(name_),
                                                               QString("%1 \"%2\"").arg(s, exec_),
                                                               icon_));
    }


    // Desktop entry actions
    if (values["Desktop Entry"].count("Actions")){
        QString actionsString = values["Desktop Entry"]["Actions"];
        QStringList actionStrings = actionsString.split(';',QString::SkipEmptyParts);
        QString name;
        QString exec;
        QString group;
        for (const QString &actionString: actionStrings){
            // Get the name
            group = QString("Desktop Action %1").arg(actionString);
            if (!values[group].count("Name")) continue;
            name = values[group]["Name"];

            // Get the command
            group = QString("Desktop Action %1").arg(actionString);
            if (!values[group].count("Exec")) continue;
            exec = values[group]["Exec"];

            // Try to get an icon
            group = QString("Desktop Action %1").arg(actionString);
            if (values[group].count("Icon"))
                actions_.push_back(std::make_shared<DesktopAction>(this, name, exec, getIcon(values[group]["Icon"])));
            else
                actions_.push_back(std::make_shared<DesktopAction>(this, name, exec, icon_));

        }
    }
    return true;
}



/** ***************************************************************************/
QString Applications::Application::escapeString(const QString &unescaped) {
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



/** ***************************************************************************/
QIcon Applications::Application::getIcon(const QString &iconName) {

    // http://standards.freedesktop.org/icon-theme-spec/icon-theme-spec-latest.html
    // http://standards.freedesktop.org/desktop-entry-spec/latest/

    /*
     * Icon to display in file manager, menus, etc. If the name is an absolute
     * path, the given file will be used. If the name is not an absolute path,
     * the algorithm described in the Icon Theme Specification will be used to
     * locate the icon. oh funny qt-bug h8 u
     */

    if (iconName.startsWith('/'))
        // If it is a full path
        return QIcon(iconName);
    else if (QIcon::hasThemeIcon(iconName))
        // If it is in the theme
        return QIcon::fromTheme(iconName);
    else {
        QIcon result;
        // Try hicolor
        QString currentTheme = QIcon::themeName(); // missing fallback (qt-bug)
        QIcon::setThemeName("hicolor");
        if (QIcon::hasThemeIcon(iconName)) {
            result = QIcon::fromTheme(iconName);
            QIcon::setThemeName(currentTheme);
        } else {
            QIcon::setThemeName(currentTheme);
            // if it is in the pixmaps
            QDirIterator it("/usr/share/pixmaps", QDir::Files, QDirIterator::Subdirectories);
            bool found = false;
            while (it.hasNext()) {
                it.next();
                QFileInfo fi = it.fileInfo();
                if (fi.isFile() && (fi.fileName() == iconName || fi.baseName() == iconName)) {
                    result = QIcon(fi.canonicalFilePath());
                    found = true;
                    break;
                }
            }
            if (!found) {
                // If it is still not found use a generic one
                qWarning() << "Unknown icon:" << iconName;
                result = QIcon::fromTheme("exec");
            }
        }
        return result;
    }
}
