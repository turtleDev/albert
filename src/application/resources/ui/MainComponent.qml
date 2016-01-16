import QtQuick 2.0
import QtGraphicalEffects 1.0

FocusScope {
    // ▼▲ Settable properties of this style ▲▼
    property color background_color
    property color foreground_color
    property color highlight_color
    property color frame_color
    property int settingsbutton_size
    property int input_fontsize
    property int item_height
    property int item_title_fontsize
    property int item_description_fontsize
    property int max_items
    property int space
    property int window_width
    // ▲▼ Settable properties of this style ▼▲

    width: bottomFrame.width
    height: bottomFrame.height
    focus: true

    Rectangle {
        id: bottomFrame
        width: window_width
        height: topFrame.height+2*topFrame.anchors.margins
        radius: space*2
        color: frame_color

        Rectangle {
            id: topFrame
            height: content.height+2*content.anchors.margins
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.left: parent.left
            anchors.margins: space
            radius: space
            color: background_color

            Column {
                id: content
                anchors.top: parent.top
                anchors.right: parent.right
                anchors.left: parent.left
                anchors.margins: space
                spacing: space

                Rectangle {
                    id: historyTextInputFrame
                    width: parent.width
                    height: historyTextInput.height
                    color: Qt.darker(background_color, 2)
                    radius: space/2

                    HistoryTextInput {
                        id: historyTextInput
                        clip: true
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.margins: 4
                        echoMode: TextInput.Normal
                        font.family: "DejaVu Sans"
                        font.pixelSize: input_fontsize
                        color: foreground_color
                        selectedTextColor: background_color
                        selectionColor: highlight_color
                        selectByMouse: true
                        focus: true
                        cursorDelegate : Component {
                            Item {
                                id: cursor
                                Rectangle {
                                    y: 2; width: 1
                                    height: parent.height-4
                                    color: highlight_color
                                }
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
                        width: settingsbutton_size
                        height: width
                        anchors.top: parent.top
                        anchors.right: parent.right
                        anchors.topMargin: 3
                        anchors.rightMargin: 3

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
                            color: background_color
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
                            onEntered: gearcolor.color=foreground_color
                            onExited: gearcolor.color=background_color
                            onPressed: gearcolor.color=highlight_color
                            onReleased: gearcolor.color=background_color
                        }
                    }
                }

                ListView {
                    id: resultsList
                    width: parent.width
                    height: Math.min(max_items, count)*(item_height+spacing)-spacing
                    model: resultsModel
                    visible : (count===0) ? false : true;
                    snapMode: ListView.SnapToItem
                    boundsBehavior: Flickable.StopAtBounds
                    clip: true
                    delegate: listDelegate
                    highlightMoveDuration : 200
                    highlightMoveVelocity : 1000
                    spacing: space

                    Component {
                        id: listDelegate
                        Item {
                            id: listItem
                            height: item_height
                            width: resultsList.width

                            Item {
                                id: listItemIconArea
                                width: parent.height
                                height: parent.height

                                Rectangle {
                                    width: parent.height
                                    height: width
                                    radius: space
                                    anchors.centerIn: parent
                                    color: Qt.darker(background_color, 2)
                                    Text {
                                        anchors.fill: parent
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                        font.pixelSize : parent.height-12
                                        color: resultsList.currentIndex===index?highlight_color:foreground_color
                                        text: display.slice(0,1);
                                        font.family: "DejaVu Sans ExtraLight"
                                        font.capitalization: Font.AllUppercase
                                        Behavior on color {ColorAnimation {duration:100;}}
                                    }
                                }
                            }

                            Column{
                                id: listItemTextArea
                                anchors.left: listItemIconArea.right
                                anchors.right: parent.right
                                anchors.leftMargin: space
                                anchors.verticalCenter: listItemIconArea.verticalCenter

                                Text {
                                    id: listItemTextfield
                                    width: parent.width
                                    text: model.display
                                    elide: Text.ElideMiddle
                                    color: resultsList.currentIndex===index?highlight_color:foreground_color
                                    font.pixelSize: item_title_fontsize
                                    Behavior on color {ColorAnimation {duration:100;}}
                                }

                                Text {
                                    id: listItemInfofield
                                    width: parent.width
                                    text: model.toolTip
                                    elide: Text.ElideMiddle
                                    color: resultsList.currentIndex===index?highlight_color:foreground_color
                                    font.pixelSize: item_description_fontsize
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
                resultsList.currentIndex - max_items < 0
                        ? resultsList.currentIndex = 0
                        : resultsList.currentIndex -= max_items
                return;
            case Qt.Key_PageDown:
                resultsList.currentIndex + max_items > resultsList.count
                        ? resultsList.currentIndex = resultsList.count-1
                        : resultsList.currentIndex += max_items
                return;
        }
        event.ignore
    }

    Component.onCompleted: setPreset("Default")


    function availableProperties() {
        return ["background_color",
                "foreground_color",
                "highlight_color",
                "frame_color",
                "input_fontsize",
                "item_title_fontsize",
                "item_description_fontsize",
                "item_height",
                "max_items",
                "space",
                "settingsbutton_size",
                "window_width"];
    }
    function availablePresets() {
        return ["Default","Orange","Magenta","Mint","Green","Blue","Violet"];
    }
    function setPreset(p) {
        switch (p) {
        case "Default":
            background_color = "#444"
            foreground_color = "#BBB"
            highlight_color = "#FFF"
            frame_color = "#80808080"
            input_fontsize = 36
            item_title_fontsize = 26
            item_description_fontsize = 12
            item_height = 48
            max_items = 5
            space = 7
            settingsbutton_size = 13
            window_width = 720
            break;
        case "Orange":
            setPreset("Default")
            highlight_color= "#FFA040"
            frame_color= "#80FFA040"
            break;
        case "Magenta":
            setPreset("Default")
            highlight_color= "#FF40A0"
            frame_color= "#80FF40A0"
            break;
        case "Mint":
            setPreset("Default")
            highlight_color= "#40FFA0"
            frame_color= "#8040FFA0"
            break;
        case "Green":
            setPreset("Default")
            highlight_color= "#A0FF40"
            frame_color= "#80A0FF40"
            break;
        case "Blue":
            setPreset("Default")
            highlight_color= "#40A0FF"
            frame_color= "#8040A0FF"
            break;
        case "Violet":
            setPreset("Default")
            highlight_color= "#A040FF"
            frame_color= "#80A040FF"
            break;
        }
    }


    // ▼ ▼ ▼ ▼ ▼ DO NOT CHANGE THIS UNLESS YOU KNOW WHAT YOU ARE DOING ▼ ▼ ▼ ▼ ▼
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
     * External invokation of 'onMainWindowVisibleChanged' (Focus out)
     * External invokation of availablePresets
     * External invokation of setPreset
     * External invokation of availableProperties
     * External mutations of the properties returned by availableProperties
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
