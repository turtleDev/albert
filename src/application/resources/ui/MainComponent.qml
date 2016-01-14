import QtQuick 2.0
import QtGraphicalEffects 1.0

FocusScope {
    property string color1: "#999"
    property string color2: "#555"
    property string color3: "#80FFC0"
    width: bottomFrame.width
    height: bottomFrame.height
    focus: true
    Keys.onPressed: {
        event.accept
        switch (event.key) {
            case Qt.Key_Up:
                (resultsList.currentIndex===-1 || event.modifiers===Qt.ControlModifier)
                        ? historyTextInput.nextIteration()
                        : resultsList.decrementCurrentIndex()
                return;
            case Qt.Key_Down:
                (event.modifiers===Qt.ControlModifier)
                        ? historyTextInput.prevIteration()
                        : resultsList.incrementCurrentIndex()
                return;
            case Qt.Key_PageUp:
                resultsList.currentIndex - maxProposals < 0
                        ? resultsList.currentIndex = 0
                        : resultsList.currentIndex -= maxProposals
                return;
            case Qt.Key_PageDown:
                resultsList.currentIndex + maxProposals > resultsList.count
                        ? resultsList.currentIndex = resultsList.count-1
                        : resultsList.currentIndex += maxProposals
                return;
        }
        event.ignore
    }

    Rectangle {
        id: bottomFrame
        width: 720
        height: topFrame.height+2*topFrame.anchors.margins
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.left: parent.left
        radius: 16
        color: "#8080FFC0"
        NumberAnimation on opacity {
            duration : 600
            easing.type: Easing.InOutQuint
            from: 0
            to: 1
        }

        Rectangle {
            id: topFrame
            height: content.height+2*content.anchors.margins
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.left: parent.left
            anchors.margins: 8
            radius: 8
            color: color1

            Column {
                id: content
                anchors.top: parent.top
                anchors.right: parent.right
                anchors.left: parent.left
                anchors.margins: 8
                spacing: 8

                Rectangle {
                    id: historyTextInputFrame
                    width: parent.width
                    height: historyTextInput.height
                    color: color2
                    radius: 4

                    HistoryTextInput {
                        id: historyTextInput
                        clip: true
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.margins: 4
                        echoMode: TextInput.Normal
                        font.family: "DejaVu Sans"
                        font.pixelSize: 36
                        color: color1
                        selectedTextColor: color2
                        selectionColor: color1
                        selectByMouse: true
                        focus: true

                        cursorDelegate : Component {
                            Item {
                                id: cursor
                                Behavior on x { NumberAnimation { } }
                                Behavior on y { NumberAnimation { } }
                                Behavior on height { NumberAnimation { duration: 200 } }
                                Rectangle {y: 2; width: 1; color: color3; height: parent.height-4;}
                                SequentialAnimation on opacity {
                                    loops: Animation.Infinite;
                                    NumberAnimation { to: 0; duration: 1000; easing.type: Easing.InOutCubic}
                                    NumberAnimation { to: 1; duration: 1000; easing.type: Easing.InOutCubic}
                                }
                            }
                        }
                        onTextChanged: {
                            queryChanged(text)
                            resultsList.currentIndex = -1;
                        }
                        onAccepted: {
                            // Ignore empty list
                            if (resultsList.count === 0)
                                return
                            // Activate selected item, first if none is selected
                            indexActivated(resultsList.currentIndex===-1?0:resultsList.currentIndex)
                        }

                        // Workaround (Shortcuts are 5.5 only)
                        Keys.onPressed: {
                            if (event.key===Qt.Key_Comma && event.modifiers===Qt.AltModifier)
                                settingsWindowRequested()
                        }
                    }

                    Item {
                        id: settingsbutton
                        width: 14
                        height: 14
                        anchors.top: parent.top
                        anchors.right: parent.right
                        anchors.topMargin: 2
                        anchors.rightMargin: 2

                        Image {
                            id: gearmask
                            anchors.fill: parent
                            sourceSize: Qt.size(width, height)
                            source: "../icons/gear.svg"
                            smooth: true
                            visible: false
                        }
                        Rectangle {
                            id: gearcolor
                            anchors.fill: parent
                            color: color1
                            visible: false
                        }
                        OpacityMask {
                            id: gear
                            anchors.fill: settingsbutton
                            source: gearcolor
                            maskSource: gearmask
                            RotationAnimation on rotation {
                                duration : 15000
                                easing.type: Easing.Linear
                                loops: Animation.Infinite
                                from: 0
                                to: 360
                            }
                        }
                        MouseArea {
                            anchors.fill: parent
                            hoverEnabled: true
                            onClicked: settingsWindowRequested()
                            onEntered: gearcolor.color=color3
                            onExited: gearcolor.color=color1
                        }
                    }
                }

                ListView {
                    id: resultsList
                    width: parent.width
                    height: Math.min(maxProposals, count)*48
                    model: resultsModel
                    visible : (count===0) ? false : true;
                    snapMode: ListView.SnapToItem
                    boundsBehavior: Flickable.StopAtBounds
                    clip: true
                    delegate: listDelegate
                    highlightMoveDuration : 200
                    highlightMoveVelocity : 1000

                    Component {
                        id: listDelegate
                        Item {
                            id: listItem

                            height: 48
                            width: resultsList.width

                            Item {
                                id: listItemIconArea
                                width: parent.height
                                height: parent.height

                                Rectangle {
                                    width: parent.height-8
                                    height: width
                                    radius: 8
                                    anchors.centerIn: parent
                                    gradient: Gradient {
                                        GradientStop { position: 0.0; color: color1 }
                                        GradientStop { position: 1.0; color: color2 }
                                    }

                                    Text {
                                        anchors.fill: parent
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                        font.pixelSize : parent.height-12
                                        color: resultsList.currentIndex===index?color3:color2
                                        text: display.slice(0,1);
                                        font.family: "DejaVu Sans ExtraLight"
                                        font.capitalization: Font.AllUppercase
                                        Behavior on color {ColorAnimation {duration:100;}}
                                    }
                                }
                            }

                            Item {
                                id: listItemTextArea
                                anchors.left: listItemIconArea.right
                                anchors.right: parent.right
                                anchors.leftMargin: 8
                                height: parent.height

                                Text {
                                    id: listItemTextfield
                                    width: parent.width
                                    text: model.display
                                    elide: Text.ElideMiddle
                                    color: resultsList.currentIndex===index?color3:color2
                                    font.pixelSize: 26
                                    Behavior on color {ColorAnimation {duration:100;}}
                                }

                                Text {
                                    id: listItemInfofield
                                    anchors.top: listItemTextfield.bottom
                                    width: parent.width
                                    text: model.toolTip
                                    elide: Text.ElideMiddle
                                    color: resultsList.currentIndex===index?color3:color2
                                    font.pixelSize:12
                                    Behavior on color {ColorAnimation {duration:100;}}
                                }
                            }

                            MouseArea {
                                id: mouseArea
                                anchors.fill: parent
                                hoverEnabled: true
                                onClicked: resultsList.currentIndex = index
                                onDoubleClicked: {
                                    resultsList.currentIndex = index
                                    indexActivated(index)
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // ▼ ▼ ▼ ▼ ▼ DO NOT CHANGE THIS UNLESS YOU KNOW WHAT YOU ARE DOING ▼ ▼ ▼ ▼ ▼
    property int maxProposals
    signal indexActivated(int index)
    signal queryChanged(string text)
    signal settingsWindowRequested()
    function onMainWindowVisibleChanged() { historyTextInput.clearLine(); }
    /*
     * Currently the interface with the program logic comprises the following:
     *
     * Context property 'model'
     * Context property 'history'
     * Listeners on signal: 'indexActivated'
     * Listeners on signal: 'queryChanged'
     * Listeners on signal: 'settingsWindowRequested'
     * External mutations of the property maxProposals
     * External invokation of 'onMainWindowVisibleChanged' (Focus out)
     *
     * Canges to this interface will increment the minor version of the
     * interface version, if the new interface is a superset of the last one,
     * i.e. it is backwards compatible, otherwise the major version will be
     * incremented.
     *
     * Note: As long albert is in alpha stage the interface may break anytime.
     */
    property string interfaceVersion: "0.1"
}
