/********************************************************************
**  Nulloy Music Player, http://nulloy.com
**  Copyright (C) 2010-2024 Sergey Vlasov <sergey@vlasov.me>
**
**  This skin package including all images, cascading style sheets,
**  UI forms, and JavaScript files are released under
**  Attribution-ShareAlike Unported License 3.0 (CC-BY-SA 3.0).
**  Please review the following information to ensure the CC-BY-SA 3.0
**  License requirements will be met:
**
**  http://creativecommons.org/licenses/by-sa/3.0/
**
*********************************************************************/

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtGraphicalEffects 1.0
import QtQuick.Window 2.2

import Nulloy 1.0
import NWaveformBar 1.0
import NSvgImage 1.0

Rectangle {
  id: root
  color: lightTheme ? "#E8E8EB" : "#2D2D30"

  Rectangle {
    anchors.fill: parent
    color: "transparent"
    border.color: lightTheme ? "#B8B8B8" : "#3D3D3D"
    z: 1
  }

  Layout.minimumWidth: 470

  property string svgSource: "design.svg"
  property string settingsPrefix: "MetroSkin/"
  property bool lightTheme: settings.value(settingsPrefix + "LightTheme")
  property color svgOverlay: lightTheme ? "#3D3D3D" : "#FFFFFF"
  property color resizerSvgOverlay: "#565659"
  property color toggleSvgOverlay: "#808082"

  Connections {
    target: Qt.application
    function onAboutToQuit() {
    //settings.setValue(settingsPrefix + "Splitter", splitter.states);
    }
  }

  NSizeGrip {
    parent: Window.contentItem
    anchors.right: parent.right
    anchors.bottom: parent.bottom
    width: 18
    height: 18
    NSvgImage {
      anchors.fill: parent
      source: svgSource
      elementId: "sizegrip"
      colorOverlay: resizerSvgOverlay
    }
  }

  component ButtonBorder: Item {
    property color backgroundColor: lightTheme ? "#CFDCF1" : "#303E59"
    property color borderColor: lightTheme ? "#1FAEFF" : "#5882CF"
    property bool hovered: parent.hovered
    property bool pressed: parent.pressed

    anchors.fill: parent
    z: -1

    Rectangle {
      anchors.fill: parent
      color: "transparent"
      border.color: hovered || pressed ? borderColor : "transparent"
      border.width: 1
    }

    Rectangle {
      anchors.fill: parent
      anchors.margins: 1
      color: pressed ? backgroundColor : "transparent"
    }
  }

  component ButtonCircle: Rectangle {
    anchors.centerIn: parent
    width: 30
    height: 30
    color: "transparent"
    border.color: lightTheme ? "#3D3D3D" : "#FFFFFF"
    border.width: 2
    radius: 30
  }

  ColumnLayout {
    anchors.fill: parent
    spacing: 0

    RowLayout {
      id: titleBar
      Layout.margins: 6
      Layout.bottomMargin: 0
      Layout.minimumHeight: 21
      Layout.fillWidth: true
      Layout.fillHeight: false
      spacing: 5

      NSvgImage {
        Layout.preferredWidth: 18
        Layout.preferredHeight: 18
        source: svgSource
        elementId: "icon"
        colorOverlay: svgOverlay
      }

      NSvgButton {
        Layout.preferredWidth: 30
        Layout.preferredHeight: 22
        onClicked: lightTheme = !lightTheme
        source: svgSource
        elementId: "theme"
        image.colorOverlay: svgOverlay
        ButtonBorder {}
      }

      NFadeOut {
        Layout.fillWidth: true
        Layout.fillHeight: true
        Text {
          id: textItem
          text: oldMainWindow.windowTitle
          color: lightTheme ? "#3D3D3D" : "#FFFFFF"
          font.bold: true
          font.pixelSize: 12
          anchors.centerIn: parent.width >= textItem.width ? parent : undefined
          anchors.left: parent.width >= textItem.width ? undefined : parent.left
          anchors.top: parent.top
          anchors.bottom: parent.bottom
          verticalAlignment: Text.AlignVCenter
        }
      }

      NSvgButton {
        Layout.preferredWidth: 30
        Layout.preferredHeight: 22
        onClicked: mainWindow.showMinimized()
        source: svgSource
        elementId: "minimize"
        image.colorOverlay: svgOverlay
        ButtonBorder {}
      }
      NSvgButton {
        Layout.preferredWidth: 30
        Layout.preferredHeight: 22
        onClicked: mainWindow.close()
        source: svgSource
        elementId: "close"
        image.colorOverlay: svgOverlay
        ButtonBorder {
          borderColor: lightTheme ? "#FF7B59" : "#FF2E12"
          backgroundColor: lightTheme ? "#FFCBBF" : "#781212"
        }
      }
    }

    NSplitter {
      id: splitter
      Layout.fillWidth: true
      Layout.fillHeight: true

      // FIXME: temporal adjustment for compatibility with the old skin:
      states: [parseInt(settings.value(settingsPrefix + "Splitter")[0]) - titleBar.Layout.minimumHeight - 6]

      handleDelegate: Item {
        height: 7
        Rectangle {
          anchors.fill: parent
          anchors.margins: 1
          color: lightTheme ? "#E8E8EB" : "#2D2D30"
        }
        NSvgImage {
          anchors.fill: parent
          source: svgSource
          elementId: "splitter"
          colorOverlay: resizerSvgOverlay
        }
      }

      Item {
        z: 1

        Rectangle {
          anchors.fill: parent
          color: lightTheme ? "#E8E8EB" : "#2D2D30"
        }

        MouseArea {
          anchors.fill: parent
          acceptedButtons: Qt.NoButton
          onWheel: {
            if (wheel.angleDelta.y < 0) {
              volumeSlider.decrease();
            } else {
              volumeSlider.increase();
            }
            volumeSlider.moved();
          }
        }

        ColumnLayout {
          anchors.fill: parent
          anchors.margins: 6
          anchors.bottomMargin: 0
          spacing: 5

          RowLayout {
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.minimumHeight: 50
            spacing: 4

            NCoverImage {
              Layout.fillHeight: true
              growHorizontally: true

              Rectangle {
                anchors.fill: parent
                border.color: lightTheme ? "#1FAEFF" : "#5882CF"
                visible: parent.containsMouse
                color: "transparent"
              }
            }

            Item {
              id: waveformContainer
              Layout.fillHeight: true
              Layout.fillWidth: true

              property real position: playbackEngine.position

              Item {
                anchors.fill: parent

                Rectangle {
                  anchors.fill: parent
                  color: lightTheme ? "#CDCDD4" : "#252526"
                }

                NWaveformBar {
                  id: waveformBar
                  anchors.fill: parent
                  color: lightTheme ? "#1FAEFF" : "#1B58B8"
                  borderColor: lightTheme ? "#0080FF" : "#1FAEFF"
                  borderWidth: 1.0 / Screen.devicePixelRatio
                }

                Item {
                  id: leftSide
                  anchors.fill: parent
                  Rectangle {
                    anchors {
                      top: parent.top
                      bottom: parent.bottom
                      left: parent.left
                    }
                    width: parent.width * waveformContainer.position
                    color: playbackEngine.state == N.PlaybackPlaying ? (lightTheme ? "#4BD161" : "#15992A") : (lightTheme ? "#D9941C" : "#CE8419")
                  }
                }

                Blend {
                  anchors.fill: parent
                  source: waveformBar
                  foregroundSource: leftSide
                  mode: lightTheme ? (playbackEngine.state == N.PlaybackPlaying ? "darken" : "hardLight") : "hardLight"
                }
              }

              NTrackInfo {
                anchors.margins: 2
                itemDelegate: Text {
                  color: "#FFFFFF"
                  leftPadding: 2
                  rightPadding: 2
                  font.bold: true
                  font.pixelSize: (rowIndex == 1 && columnIndex == 1) ? 12 : 11
                  text: itemText
                  Rectangle {
                    visible: itemText != ""
                    anchors.fill: parent
                    color: "#C8242525"
                    opacity: lightTheme ? 0.75 : 1.0
                    z: -1
                  }
                }
                onTooltipRequested: player.showToolTip(text)
              }

              MouseArea {
                anchors.fill: parent
                onPressed: {
                  let position = mouseX / parent.width;
                  playbackEngine.position = position;
                  // to avoid waiting for positionChanged signal:
                  playbackEngine.positionChanged(position);
                }
              }
            }
          }

          RowLayout {
            Layout.fillWidth: true
            Layout.minimumHeight: 35
            spacing: 0

            NSvgButton {
              Layout.preferredWidth: 60
              Layout.preferredHeight: 36
              source: svgSource
              elementId: "prev"
              //onClicked: playlistWidget.playPrevItem();
              image.colorOverlay: svgOverlay
              ButtonBorder {}
              ButtonCircle {}
            }
            NSvgButton {
              Layout.preferredWidth: 60
              Layout.preferredHeight: 36
              source: svgSource
              elementId: playbackEngine.state == N.PlaybackPlaying ? "pause" : "play"
              image.colorOverlay: svgOverlay
              onClicked: {
                if (playbackEngine.state == N.PlaybackPlaying) {
                  playbackEngine.pause();
                } else {
                  playbackEngine.play();
                }
              }
              ButtonBorder {}
              ButtonCircle {}
            }
            NSvgButton {
              Layout.preferredWidth: 60
              Layout.preferredHeight: 36
              source: svgSource
              elementId: "stop"
              onClicked: playbackEngine.stop()
              image.colorOverlay: svgOverlay
              ButtonBorder {}
              ButtonCircle {}
            }
            NSvgButton {
              Layout.preferredWidth: 60
              Layout.preferredHeight: 36
              source: svgSource
              elementId: "next"
              //onClicked: playlistWidget.playNextItem();
              image.colorOverlay: svgOverlay
              ButtonBorder {}
              ButtonCircle {}
            }

            Item {
              Layout.maximumWidth: 25
              Layout.minimumWidth: 5
              Layout.fillWidth: true
              //Layout.horizontalStretchFactor: 2 // Qt6 only
            }

            NSvgButton {
              Layout.preferredWidth: 30
              Layout.preferredHeight: 30
              source: svgSource
              elementId: "repeat"
              //onClicked: playlistWidget.setRepeatMode(!playlistWidget.repeatMode);
              image.colorOverlay: toggleSvgOverlay
              ButtonBorder {}
            }

            Item {
              Layout.preferredWidth: 5
            }

            NSvgButton {
              Layout.preferredWidth: 30
              Layout.preferredHeight: 30
              source: svgSource
              elementId: "shuffle"
              //onClicked: playlistWidget.setShuffleMode(!playlistWidget.shuffleMode);
              image.colorOverlay: toggleSvgOverlay
              ButtonBorder {}
            }

            Item {
              Layout.minimumWidth: 15
              Layout.fillWidth: true
            }

            Item {
              Layout.preferredHeight: 10
              Layout.preferredWidth: 120

              Slider {
                id: volumeSlider
                orientation: Qt.Horizontal
                anchors.fill: parent

                stepSize: 0.02
                value: playbackEngine.volume
                onMoved: {
                  playbackEngine.volume = value;
                  player.showToolTip(player.volumeTooltipText(value));
                }

                background: Rectangle {
                  anchors.fill: parent
                  color: lightTheme ? "#D2D3D9" : "#434346"

                  Rectangle {
                    height: parent.height
                    width: volumeSlider.visualPosition * parent.width
                    color: lightTheme ? "#B7B7BF" : "#5D5D61"
                  }
                }

                handle: Rectangle {
                  x: volumeSlider.visualPosition * (parent.width - width)
                  y: 0
                  width: 20
                  height: parent.height
                  color: handleMouseArea.containsMouse || parent.pressed ? (lightTheme ? "#78A7FF" : "#6798F2") : (lightTheme ? "#3D3D3D" : "#FFFFFF")
                  MouseArea {
                    id: handleMouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    acceptedButtons: Qt.NoButton
                  }
                }
              }
            }
          }
        }
      }

      NPlaylist {
        anchors.topMargin: 3
        anchors.bottomMargin: 3
        anchors.leftMargin: 5
        anchors.rightMargin: 5
        Layout.minimumHeight: 80

        itemHeight: 20
        itemSpacing: -2
        dropIndicatorColor: lightTheme ? "#676D70" : "#B6C0C6"
        model: playlistController.model()

        Rectangle {
          anchors.fill: parent
          anchors.margins: 1
          anchors.topMargin: 0
          color: lightTheme ? "#F9F9FC" : "#252526"
          z: -1
        }

        itemDelegate: Item {
          property bool isAlternate: itemData.index % 2 == 1

          NFadeOut {
            anchors.fill: parent
            anchors.topMargin: 2
            anchors.leftMargin: 4

            Text {
              text: itemData.text
              font.bold: itemData.isCurrent
              font.pixelSize: 13
              color: {
                if (lightTheme) {
                  return itemData.isFailed ? "#565659" : (itemData.isSelected ? "#353535" : (itemData.isCurrent ? "#3D3D3D" : "#4F4F4F"));
                } else {
                  return itemData.isFailed ? "#565659" : (itemData.isSelected ? "#FFFFFF" : (itemData.isCurrent ? "#FAFAFA" : "#D4D4D4"));
                }
              }
            }
          }

          Item {
            id: itemBackground
            anchors.fill: parent
            anchors.margins: 1
            z: -1

            Rectangle {
              anchors.fill: parent
              visible: isAlternate
              color: lightTheme ? "#E8E8EB" : "#2D2D30"
            }

            Rectangle {
              anchors.fill: parent
              visible: itemData.isSelected
              color: lightTheme ? "#A1DAFF" : "#2E4773"
              opacity: 0.5
            }

            Rectangle {
              anchors.fill: parent
              visible: itemData.isHovered
              color: "transparent"
              border.color: lightTheme ? "#1FAEFF" : "#5882CF"

              Rectangle {
                anchors.fill: parent
                anchors.margins: 1
                color: itemData.isSelected ? (lightTheme ? "#AEE0FF" : "#304C7D") : "transparent"
                opacity: 0.7
              }
            }

            Rectangle {
              anchors.fill: parent
              visible: itemData.isFocused
              color: "transparent"
              border.color: lightTheme ? "#7AC8FF" : "#476CA8"
              opacity: 0.8
            }
          }
        }

        scrollbarWidth: 10
        scrollbarPadding: 0
        scrollbarSpacing: 4
        scrollbarBottomPadding: 15
        scrollbarContentItem: Rectangle {
          color: (scrollbarHoverHandler.hovered || scrollbar.pressed) ? (lightTheme ? "#78A7FF" : "#6798F2") : (lightTheme ? "#D0D1D7" : "#434346")
          HoverHandler {
            id: scrollbarHoverHandler
            acceptedDevices: PointerDevice.Mouse
          }
        }
      }
    }
  }
}
