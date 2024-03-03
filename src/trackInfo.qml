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

import QtQuick 2.15
import QtQuick.Layouts 1.15

Item {
  id: item
  anchors.fill: parent

  signal tooltipRequested(string text)

  property var modelKeys: trackInfoModel.keys()
  property Component itemDelegate: Text {
    text: itemText
  }
  component DelegateLoader: Loader {
    sourceComponent: itemDelegate
    property int rowIndex
    property int columnIndex
    property string itemText: trackInfoModel[modelKeys[rowIndex * 3 + columnIndex]]
    Layout.alignment: columnIndex == 0 ? Qt.AlignLeft : (columnIndex == 2 ? Qt.AlignRight : Qt.AlignHCenter)
  }

  ColumnLayout {
    anchors.fill: parent
    spacing: 1
    Repeater {
      model: 3
      RowLayout {
        spacing: 1
        Layout.fillHeight: true
        Layout.alignment: index == 0 ? Qt.AlignTop : (index == 2 ? Qt.AlignBottom : Qt.AlignVCenter)

        DelegateLoader {
          rowIndex: index
          columnIndex: 0
        }
        Item {
          Layout.fillWidth: true
        }
        DelegateLoader {
          rowIndex: index
          columnIndex: 1
        }
        Item {
          Layout.fillWidth: true
        }
        DelegateLoader {
          rowIndex: index
          columnIndex: 2
        }
      }
    }
    onHeightChanged: {
      var heightThreshold = 40;
      children[0].visible = height > heightThreshold;
      children[2].visible = height > heightThreshold;
    }
  }

  Behavior on opacity {
    NumberAnimation {
      duration: 150
      easing.type: Easing.OutQuad
    }
  }
  MouseArea {
    id: mouseArea
    anchors.fill: parent
    hoverEnabled: true
    acceptedButtons: Qt.NoButton
    onEntered: {
      item.opacity = 0;
    }
    onExited: {
      item.opacity = 1;
      tooltipRequested("");
    }
    onPositionChanged: {
      tooltipRequested(trackInfoModel.formatTooltip(mouseArea.mouseX / mouseArea.width));
    }
  }
}
