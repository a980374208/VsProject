import QtQuick 2.9
import QtQuick.Window 2.2
import QtQuick.Controls 2.15
import CustomComponents 1.0  // 确保导入了正确的模块和版本

Window {
    visible: true
    width: 640
    height: 480
    title: "qml_DX11_test"
    color:"red"
 //  Text {
 //      anchors.fill: parent
 //      horizontalAlignment: Text.AlignHCenter
 //      verticalAlignment: Text.AlignVCenter
 //      font.bold: true
 //      font.pointSize: 42
 //      text: "Hello World!"
 //  }
     DXItem {
        anchors.fill: parent
     }
}
