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
#include <QQuickView>
#include <QPoint>
#include <QIdentityProxyModel>
#include "history.hpp"

class MainWindow final : public QQuickView
{
    Q_OBJECT

public:
    MainWindow(QWindow *parent = 0);
    ~MainWindow();

    Q_PROPERTY(bool hideOnFocusLoss MEMBER hideOnFocusLoss_ NOTIFY hideOnFocusLossChanged)
    Q_PROPERTY(bool showCentered MEMBER showCentered_ NOTIFY showCenteredChanged)
    Q_PROPERTY(QUrl stylePath MEMBER stylePath_ NOTIFY stylePathChanged)

    void show();
    void hide();
    void toggleVisibility();
    void setSource(const QUrl & url);
    void setModel(QAbstractItemModel* model);

private:
    void focusOutEvent(QFocusEvent *event) override;
    void keyPressEvent(QKeyEvent * event) override;

    bool showCentered_;
    bool hideOnFocusLoss_;
    QUrl stylePath_;
    History history_;
    QIdentityProxyModel model_;

    static const QString CFG_CENTERED;
    static const bool    DEF_CENTERED;
    static const QString CFG_HIDEONFOCUSLOSS;
    static const bool    DEF_HIDEONFOCUSLOSS;
    static const QString CFG_STYLEPATH;
    static const QUrl    DEF_STYLEPATH;
    static const QString CFG_MAX_PROPOSALS;
    static const int     DEF_MAX_PROPOSALS;
    static const QString CFG_WND_POS;

signals:
    void hideOnFocusLossChanged(bool hideOnFocusLoss);
    void showCenteredChanged(bool showCentered);
    void stylePathChanged(QUrl stylePath);
    void queryChanged(const QString&);
    void indexActivated(int);
    void settingsWindowRequested();
};
