/********************************************************************
**  Nulloy Music Player, http://nulloy.com
**  Copyright (C) 2010-2024 Sergey Vlasov <sergey@vlasov.me>
**
**  This program can be distributed under the terms of the GNU
**  General Public License version 3.0 as published by the Free
**  Software Foundation and appearing in the file LICENSE.GPL3
**  included in the packaging of this file.  Please review the
**  following information to ensure the GNU General Public License
**  version 3.0 requirements will be met:
**
**  http://www.gnu.org/licenses/gpl-3.0.html
**
*********************************************************************/

import QtQuick 2.2
import QtQuick.Controls 1.4
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.4
import src 1.0

Dialog {
  id: dialog
  objectName: "dialog"

  title: qsTr("About ") + Qt.application.name
  width: 450
  visible: true
  standardButtons: StandardButton.Close
  modality: Qt.ApplicationModal

  Timer {
    id: onCompletedTimer
    interval: 0
    repeat: false
    onTriggered: {
      dialogHandler.centerToParent();
    }
  }

  Component.onCompleted: {
    onCompletedTimer.start();
  }

  TabView {
    id: tabView
    anchors.fill: parent

    Tab {
      title: qsTr("Common")
      NScrollView {
        padding: 0
        ColumnLayout {
          id: innerColumnLayout

          Timer {
            id: resizeDialogTimer
            interval: 0
            repeat: false
            onTriggered: {
              dialog.height = innerColumnLayout.implicitHeight + 120; // + window decoration + standard + tabs row
            }
          }
          Component.onCompleted: {
            resizeDialogTimer.start();
          }

          Image {
            width: 96
            height: 96
            source: "qrc:icon.svg"
            fillMode: Image.PreserveAspectFit
            sourceSize: Qt.size(width, height)
            Layout.alignment: Qt.AlignHCenter
          }

          NText {
            Layout.fillWidth: true
            horizontalAlignment: TextInput.AlignHCenter
            wrapMode: Text.WordWrap
            textFormat: Text.MarkdownText
            Component.onCompleted: {
              let txt = "";
              txt += "**%1 Music Player**".arg(text += Qt.application.name);
              txt += "\n\n";
              txt += "https://%1".arg(Qt.application.domain);
              txt += "\n\n";
              txt += qsTr("Version: ") + Qt.application.version;
              txt += "\n\n";
              txt += "Copyright (C) 2010-2024 Sergey Vlasov <sergey\\@vlasov.me>";
              text = txt;
            }
          }
        }
      }
    }

    Tab {
      title: qsTr("Thanks")
      NScrollView {
        ColumnLayout {
          NText {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            text: utils.readFile(":/THANKS").replace(/(\S)\n(\S)/g, '$1 $1')
          }
          Item {
            Layout.fillHeight: true
          }
        }
      }
    }

    Tab {
      title: qsTr("Changelog")
      NScrollView {
        ColumnLayout {
          NText {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            textFormat: Text.RichText
            text: utils.readFile(":/ChangeLog").replace(/\n/g, '<br>').replace(/(\*[^<]*)(<br>)/g, '<b>$1</b>$2')
          }
        }
        Item {
          Layout.fillHeight: true
        }
      }
    }

    Tab {
      title: qsTr("License")
      NScrollView {
        ColumnLayout {
          NText {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            textFormat: Text.MarkdownText
            Component.onCompleted: {
              let txt = "";
              txt += "This program is free software: you can redistribute it and/or modify ";
              txt += "it under the terms of the GNU General Public License version 3.0 ";
              txt += "as published by the Free Software Foundation.";
              txt += "\n\n";
              txt += "This program is distributed in the hope that it will be useful, ";
              txt += "but **WITHOUT ANY WARRANTY**; without even the implied warranty of ";
              txt += "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the ";
              txt += "GNU General Public License for more details.";
              txt += "\n\n";
              txt += "You should have received a copy of the GNU General Public License ";
              txt += "along with this program. If not, see ";
              txt += "http://www.gnu.org/licenses/gpl-3.0.html";
              text = txt;
            }
          }
        }
      }
    }
  }
}
