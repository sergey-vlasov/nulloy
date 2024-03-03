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
import QtQuick.Window 2.2

import "utils.js" as NUtils

Item {
  id: root

  property Component handleDelegate: Rectangle {
    width: root.width
    height: 5
    color: "lightgray"
  }
  property var states: [] // 0 means collapsed
  property var _minimumHeights: []
  property var _handles: []
  property int _handleHeight

  function updateLayout() {
    let nextChildY = 0;
    for (let i = 0; i < children.length; ++i) {
      if (states[i] > 0) {
        children[i].y = nextChildY;
        let remainingTotalHeight = root.height - nextChildY;
        if (i < children.length - 1) {
          var maxExpandedHeight = remainingTotalHeight - _handleHeight * (_handles.length - i);
          if (states.slice(i + 1).reduce((acc, val) => acc + val, 0) == 0) {
            // all next items are collapsed:
            children[i].height = maxExpandedHeight;
          } else {
            for (var j = i + 1; j < children.length; ++j) {
              maxExpandedHeight -= Math.min(_minimumHeights[j], states[j]);
            }
            children[i].height = NUtils.bound(_minimumHeights[i], states[i], maxExpandedHeight);
          }
        } else {
          // expanding the last item:
          children[i].height = remainingTotalHeight;
        }
      } else {
        // collapsed:
        children[i].height = 0;
      }
      children[i].visible = children[i].height > 0;
      nextChildY = children[i].y + children[i].height + _handleHeight;
      if (i < children.length - 1) {
        _handles[i].y = mapToItem(_handles[i].parent, 0, nextChildY - _handleHeight).y;
      }
    }
  }

  onWidthChanged: {
    for (let i = 0; i < children.length; ++i) {
      children[i].width = root.width;
    }
  }

  onHeightChanged: {
    updateLayout();
  }

  Component.onCompleted: {
    for (let i = 0; i < children.length - 1; ++i) {
      _handles.push(handleComponent.createObject(Window.contentItem, {
        idx: i
      }));
    }
    _handleHeight = _handles[0].height;
    for (let i = 0; i < children.length; ++i) {
      _minimumHeights.push(NUtils.calculateMinimumHeight(children[i]) + (i < children.length - 1 ? _handleHeight : 0));
      if (states[i] === undefined) {
        states.push(_minimumHeights[i]);
      }
    }
    root.Layout.minimumHeight = _minimumHeights.reduce((acc, val) => acc + val, 0) + _handleHeight * _handles.length;
    updateLayout();
  }

  Component {
    id: handleComponent

    Loader {
      id: handle
      parent: Window.contentItem
      anchors.left: parent.left
      anchors.right: parent.right
      anchors.topMargin: root.anchors.topMargin
      anchors.bottomMargin: root.anchors.bottomMargin
      anchors.leftMargin: root.anchors.leftMargin
      anchors.rightMargin: root.anchors.rightMargin
      sourceComponent: root.handleDelegate

      property int idx: -1

      MouseArea {
        id: mouseArea
        anchors.fill: parent
        cursorShape: Qt.SplitVCursor

        property int pressY: 0

        onPressed: {
          pressY = mouseY;
          player.setOverrideCursor(Qt.SplitVCursor);
        }

        onReleased: {
          player.restoreOverrideCursor();
        }

        onPositionChanged: {
          const y = mapToItem(root, 0, mouseY - pressY).y;
          const minY = handle.idx > 0 ? root._handles[handle.idx - 1].y + _handleHeight : 0;
          const maxY = (handle.idx < root._handles.length - 1 ? root._handles[handle.idx + 1].y : root.height) - _handleHeight;
          const prevMinHeight = root._minimumHeights[handle.idx];
          const nextMinHeight = root._minimumHeights[handle.idx + 1];
          const distToPrev = y - minY;
          const distToNext = maxY - y;
          if (distToPrev < prevMinHeight / 2) {
            // collapse previous item:
            root.states[handle.idx] = 0;
            root.states[handle.idx + 1] = maxY - minY;
          } else if (distToPrev < prevMinHeight) {
            // resist collapsing to previous item:
            root.states[handle.idx] = prevMinHeight;
            root.states[handle.idx + 1] = maxY - minY - prevMinHeight;
          } else if (distToNext < nextMinHeight / 2) {
            // collapse next item:
            root.states[handle.idx] = maxY - minY;
            root.states[handle.idx + 1] = 0;
          } else if (distToNext < nextMinHeight) {
            // resist collapsing to next item:
            root.states[handle.idx] = maxY - minY - nextMinHeight;
            root.states[handle.idx + 1] = nextMinHeight;
          } else {
            // normal resizing:
            root.states[handle.idx] = distToPrev;
            root.states[handle.idx + 1] = distToNext;
          }
          updateLayout();
        }
      }
    }
  }
}
