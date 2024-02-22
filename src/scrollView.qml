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
import QtQuick.Layouts 1.4
import QtQuick.Controls 1.4

ScrollView {
  id: scrollView
  clip: true
  horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff

  default property alias childItem: columnLayout.children
  property int padding: 10

  contentItem: ColumnLayout {
    id: columnLayout

    anchors {
      topMargin: padding
      leftMargin: padding
      top: parent.top
      left: parent.left
    }
    width: scrollView.width - scrollView.__verticalScrollBar.width - padding * 2
    height: Math.max(columnLayout.implicitHeight, scrollView.height)

    Component.onCompleted: {
      let spacer = Qt.createQmlObject('import QtQuick 2.15; Item {}', columnLayout);
      spacer.Layout.preferredHeight = padding * 2;
    }
  }
}
