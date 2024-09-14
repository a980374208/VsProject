import QtQuick 2.15
import QtQuick.Window 2.15
import CustomComponents 1.0

Window {
    id: window
    visible: true
    width: 640
    height: 480
    title: "qml_DX11_test"
    color: "red"

    Text {
        id: textt
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        width: parent.width
        height: 50  // 设置固定高度
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        font.bold: true
        font.pointSize: 42
        text: "Hello World!"
    }

    Rectangle {
        id: blueRect
        anchors.top: textt.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: 50  // 设置固定高度
        color: "blue"
    }

    DirectXRendererItem {
        id: dxItem
        anchors.top: blueRect.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
    }
}
