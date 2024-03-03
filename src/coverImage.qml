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
import NImage 1.0

NImage {
  id: item
  objectName: "coverImage"
  property alias containsMouse: mouseArea.containsMouse

  MouseArea {
    id: mouseArea
    anchors.fill: parent
    hoverEnabled: true

    onClicked: {
      var popup = Qt.createQmlObject(`
        import QtQuick 2.15
        import NImage 1.0

        Item {
          id: coverPopup
          anchors.fill: parent

          property int margin: 50
          opacity: 0.0

          OpacityAnimator {
            id: fadeInAnimator
            target: coverPopup
            from: 0.0
            to: 1.0
            duration: 150
            easing.type: Easing.OutQuad
          }

          OpacityAnimator {
            id: fadeOutAnimator
            target: coverPopup
            from: 1.0
            to: 0.0
            duration: 150
            easing.type: Easing.OutQuad
            onStopped: coverPopup.destroy()
          }

          Component.onCompleted: {
            fadeInAnimator.start();
          }

          Rectangle {
            anchors.fill: parent
            color: "black"
            opacity: 0.6
            MouseArea {
              anchors.fill: parent
              hoverEnabled: true
              onClicked: {
                fadeOutAnimator.start();
              }
            }
          }

          NImage {
            id: popupImage
            anchors.fill: parent
            margin: coverPopup.margin
            upscale: false
            image: item.image
          }

          Rectangle {
            anchors.centerIn: parent
            width: popupImage.implicitWidth - coverPopup.margin * 2 + 1
            height: popupImage.implicitHeight - coverPopup.margin * 2 + 1
            border.color: "white"
            opacity: 0.8
            color: "transparent"
          }
        }
      `, mainWindow);
    }
  }
}
