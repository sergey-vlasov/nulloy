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
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQml.Models 2.2

Item {
  property alias model: listView.model
  property alias itemSpacing: listView.spacing
  property alias dropIndicatorColor: dropIndicator.color
  property int itemHeight: 20
  property int scrollbarWidth: 14
  property int scrollbarPadding: 3
  property int scrollbarLeftPadding: -1
  property int scrollbarRightPadding: -1
  property int scrollbarTopPadding: -1
  property int scrollbarBottomPadding: -1
  property alias scrollbarSpacing: rowLayout.spacing

  property Component scrollbarContentItem: Rectangle {
    color: "dimGray"
    radius: 5
  }

  property Component itemDelegate: Text {
    text: itemData.text
    font.bold: itemData.isCurrent
  }

  function updateRowsPerPage() {
    playlistController.rowsPerPage = Math.floor(listView.height / (itemHeight + itemSpacing));
  }
  onHeightChanged: updateRowsPerPage()

  Component.onCompleted: {
    delegateModel.model = model;
    updateRowsPerPage();
  }

  property int _hoveredIndex: -1
  property int _dropIndex: 0
  property bool _canDrop: (_dropIndex == 0) || (_dropIndex == listView.count) || !delegateModel.items.get(_dropIndex - 1).model.isSelected || !delegateModel.items.get(_dropIndex).model.isSelected

  RowLayout {
    id: rowLayout
    anchors.fill: parent
    anchors.topMargin: parent.anchors.topMargin
    anchors.bottomMargin: parent.anchors.bottomMargin
    anchors.leftMargin: parent.anchors.leftMargin
    anchors.rightMargin: parent.anchors.rightMargin
    spacing: 0

    ListView {
      id: listView
      Layout.fillWidth: true
      Layout.fillHeight: true
      interactive: false

      DelegateModel {
        id: delegateModel
        model: listView.model
      }

      delegate: Item {
        width: listView.width
        height: itemHeight
        Loader {
          anchors.fill: parent
          sourceComponent: itemDelegate
          property var itemData: model
        }
      }

      Rectangle {
        id: dragTarget
        width: parent.width
        visible: false
      }

      Rectangle {
        id: dropIndicator
        width: parent.width
        height: 1
        color: dropIndicatorColor
        visible: dropArea.containsDrag && _canDrop
        y: (itemHeight + itemSpacing) * _dropIndex + 1
      }

      focus: true
      Keys.onPressed: {
        playlistController.keyPress(event.key, event.modifiers);
      }
      Keys.onReleased: {
        playlistController.keyRelease(event.key, event.modifiers);
      }

      contentY: scrollBar.position * listView.contentHeight
      MouseArea {
        id: mouseArea
        anchors.fill: parent
        scrollGestureEnabled: false
        acceptedButtons: Qt.AllButtons
        propagateComposedEvents: true
        hoverEnabled: true
        onWheel: {
          let speed = 0.1;
          let pos = scrollBar.position - wheel.angleDelta.y / 120 * speed;
          let upperLimit = Math.max(0.0, 1.0 - listView.height / listView.contentHeight);
          scrollBar.position = Math.min(upperLimit, Math.max(0.0, pos));
        }
        onPressed: {
          playlistController.mousePress(_hoveredIndex, mouse.modifiers);
        }
        onReleased: {
          playlistController.mouseRelease(_hoveredIndex, mouse.modifiers);
        }
        onDoubleClicked: playlistController.mouseDoubleClick(_hoveredIndex)
        onPositionChanged: {
          playlistController.mouseExit(_hoveredIndex);
          let newHoveredIndex = listView.indexAt(mouseX, mouseY);
          playlistController.mouseEnter(newHoveredIndex);
          _hoveredIndex = newHoveredIndex;
        }

        Drag.active: dragHandler.active
        Drag.dragType: Drag.Automatic
        Drag.supportedActions: Qt.CopyAction | Qt.MoveAction
        DragHandler {
          id: dragHandler
          dragThreshold: 2
          enabled: _hoveredIndex >= 0
          onActiveChanged: {
            if (active) {
              var uriList = [];
              for (var i = 0; i < delegateModel.items.count; ++i) {
                if (delegateModel.items.get(i).model.isSelected) {
                  uriList.push(utils.pathToUri(delegateModel.items.get(i).model.filePath));
                }
              }
              mouseArea.Drag.mimeData = {
                "text/uri-list": uriList.join("\n")
              };
            }
          }
        }

        DropArea {
          id: dropArea
          anchors.fill: parent
          keys: ["text/uri-list"]
          onDropped: {
            if (drop.source == mouseArea) {
              playlistController.moveSelected(_dropIndex);
            } else {
              playlistController.dropUrls(drop.urls, _dropIndex);
            }
          }
          onPositionChanged: {
            _dropIndex = listView.indexAt(listView.width / 2, drag.y);
            if (_dropIndex == -1) {
              _dropIndex = listView.count;
              return;
            }
            let yOffset = drag.y - listView.itemAtIndex(_dropIndex).y - itemHeight / 2;
            if (yOffset > 0) {
              ++_dropIndex;
            }
          }
        }
      }
    }

    ScrollBar {
      id: scrollBar
      Layout.fillHeight: true
      Layout.preferredWidth: scrollbarWidth
      padding: scrollbarPadding > -1 ? scrollbarPadding : undefined
      leftPadding: scrollbarLeftPadding > -1 ? scrollbarLeftPadding : undefined
      topPadding: scrollbarTopPadding > -1 ? scrollbarTopPadding : undefined
      bottomPadding: scrollbarBottomPadding > -1 ? scrollbarBottomPadding : undefined
      rightPadding: scrollbarRightPadding > -1 ? scrollbarRightPadding : undefined
      active: hovered || pressed
      orientation: Qt.Vertical
      policy: ScrollBar.AlwaysOn
      visible: listView.contentHeight > listView.height
      size: listView.height / listView.contentHeight
      contentItem: Loader {
        sourceComponent: scrollbarContentItem
        property var scrollbar: scrollBar
      }
    }
  }
}
