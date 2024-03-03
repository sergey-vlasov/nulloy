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
import QtGraphicalEffects 1.0

Item {
  property int fadeoutWidth: 20

  default property alias childItem: innerItem.children

  Item {
    id: innerItem
    anchors.fill: parent
    visible: false
  }

  LinearGradient {
    id: mask
    anchors.fill: parent
    start: Qt.point(parent.width - fadeoutWidth, 0)
    end: Qt.point(parent.width, 0)
    gradient: Gradient {
      GradientStop {
        position: 0.0
        color: "transparent"
      }
      GradientStop {
        position: 1.0
        color: "white"
      }
    }
    visible: false
  }

  OpacityMask {
    anchors.fill: innerItem
    source: innerItem
    maskSource: mask
    invert: true
  }
}
