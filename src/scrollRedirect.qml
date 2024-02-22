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

MouseArea {
  property ScrollView target: null
  anchors.fill: parent
  onPressed: {
    mouse.accepted = false;
  }
  onReleased: {
    mouse.accepted = false;
  }
  onWheel: {
    if ((wheel.angleDelta.y < 0 && target.flickableItem.atYEnd) || (wheel.angleDelta.y > 0 && target.flickableItem.atYBeginning)) {
      return;
    }
    target.flickableItem.contentY -= wheel.angleDelta.y / 120 * target.__wheelAreaScrollSpeed;
  }
}
