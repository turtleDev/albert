import QtQuick 2.0
import QtGraphicalEffects 1.0

FocusScope {
    // ▼▲ Settable properties of this style ▲▼
    property color border_color
    property color background_color
    property color foreground_color
    property color inputline_color
    property color highlight_color
    property int settingsbutton_size
    property int input_fontsize
    property int item_height
    property int item_title_fontsize
    property int item_description_fontsize
    property int max_items
    property int space
    property int window_width
    // ▲▼ Settable properties of this style ▼▲

    width: frame.width
    height: frame.height
    focus: true

    Rectangle {
        id: frame
        width: window_width
        height: content.height+2*content.anchors.margins
        radius: space*2
        color: background_color
        border.color: border_color
        border.width: space

        Column {
            id: content
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.left: parent.left
            anchors.margins: space*2
            spacing: space

            Rectangle {
                id: historyTextInputFrame
                width: parent.width
                height: historyTextInput.height
                color: inputline_color
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

                            Image {
                                id: icon
                                source: model.decoration
                                width: parent.height
                                height: width
                                smooth: true
                                anchors.centerIn: parent
                                fillMode: Image.PreserveAspectFit
                            }
//                            Rectangle {
//                                width: parent.height-2
//                                height: width
//                                radius: space
//                                anchors.centerIn: parent
//                                color: Qt.darker(background_color, 2)
//                                Text {
//                                    anchors.fill: parent
//                                    horizontalAlignment: Text.AlignHCenter
//                                    verticalAlignment: Text.AlignVCenter
//                                    font.pixelSize : parent.height-12
//                                    color: resultsList.currentIndex===index?highlight_color:foreground_color
//                                    text: display.slice(0,1);
//                                    font.family: "DejaVu Sans ExtraLight"
//                                    font.capitalization: Font.AllUppercase
//                                    Behavior on color {ColorAnimation {duration:100;}}
//                                }
//                            }
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
                                elide: Text.ElideRight
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

    Component.onCompleted: setPreset("Dark")

    function availableProperties() {
        return ["background_color",
                "foreground_color",
                "highlight_color",
                "inputline_color",
                "border_color",
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
        return ["Dark", "DarkOrange", "DarkMagenta", "DarkMint",
                "DarkGreen", "DarkBlue", "DarkViolet",
                "Bright", "BrightOrange", "BrightMagenta", "BrightMint",
                "BrightGreen", "BrightBlue", "BrightViolet"];
    }
    function setPreset(p) {
        input_fontsize = 36
        item_title_fontsize = 26
        item_description_fontsize = 12
        item_height = 48
        max_items = 5
        space = 6
        settingsbutton_size = 14
        window_width = 640
        switch (p) {
        case "BrightOrange":
            setPreset("Bright")
            highlight_color= "#E07000"
            border_color= "#80FF8000"
            break;
        case "BrightMagenta":
            setPreset("Bright")
            highlight_color= "#E00070"
            border_color= "#80FF0080"
            break;
        case "BrightMint":
            setPreset("Bright")
            highlight_color= "#00c060"
            border_color= "#8000FF80"
            break;
        case "BrightGreen":
            setPreset("Bright")
            highlight_color= "#60c000"
            border_color= "#8080FF00"
            break;
        case "BrightBlue":
            setPreset("Bright")
            highlight_color= "#0070E0"
            border_color= "#800080FF"
            break;
        case "BrightViolet":
            setPreset("Bright")
            highlight_color= "#7000E0"
            border_color= "#808000FF"
            break;
        case "DarkOrange":
            setPreset("Dark")
            highlight_color= "#FF9020"
            border_color= "#80FF8000"
            break;
        case "DarkMagenta":
            setPreset("Dark")
            highlight_color= "#FF2090"
            border_color= "#80FF0080"
            break;
        case "DarkMint":
            setPreset("Dark")
            highlight_color= "#20FF90"
            border_color= "#8000FF80"
            break;
        case "DarkGreen":
            setPreset("Dark")
            highlight_color= "#90FF20"
            border_color= "#8080FF00"
            break;
        case "DarkBlue":
            setPreset("Dark")
            highlight_color= "#2090FF"
            border_color= "#800080FF"
            break;
        case "DarkViolet":
            setPreset("Dark")
            highlight_color= "#A040FF"
            border_color= "#808000FF"
            break;
        case "Bright":
            background_color = "#FFFFFF"
            foreground_color = "#808080"
            inputline_color = "#D0D0D0"
            highlight_color = "#000000"
            border_color = "#80808080"
            break;
        case "Dark":
        default:
            background_color = "#404040"
            foreground_color = "#808080"
            inputline_color = "#202020"
            highlight_color = "#E0E0E0"
            border_color = "#80808080"
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
