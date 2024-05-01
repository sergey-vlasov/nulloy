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
  color: "#C8C8C8"

  Rectangle {
    anchors.fill: parent
    color: "transparent"
    border.color: "#767676"
    z: 1
  }

  Layout.minimumWidth: 450

  Rectangle {
    anchors.top: parent.top
    anchors.left: parent.left
    anchors.right: parent.right
    anchors.margins: 1
    height: 22
    gradient: Gradient {
      GradientStop {
        position: 0.0
        color: "#F0F0F0"
      }
      GradientStop {
        position: 1.0
        color: "#C8C8C8"
      }
    }
  }

  property string svgSource: "design.svg"
  property string settingsPrefix: "SilverSkin/"
  property var playbackButtonColors: ["#FAFAFA", "#9B9B9B", "#FFFFFE", "#FBFFDD", "#666666", "#9E9E9E"]
  property var windowButtonColors: ["#F8F8F8", "#B4B4B4", "#FFFFFB", "#FBFFE0", "#6F6F6F", "#B5B5B5"]

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
    anchors.bottomMargin: -1
    width: 18
    height: 18
    NSvgImage {
      anchors.fill: parent
      source: svgSource
      elementId: "sizegrip"
    }
  }

  component Rivet: Item {
    id: root
    anchors.fill: parent
    z: -1

    property bool hovered: parent.hovered
    property bool pressed: parent.pressed
    property int radius: 2
    property bool leftClip: false
    property bool rightClip: false
    property bool rightSeparator: false
    property var colors: playbackButtonColors
    property alias border: background.border
    property alias bottomBorder: bottomBorder

    clip: true

    Item {
      anchors.fill: parent
      anchors.leftMargin: leftClip ? -radius : 0
      anchors.rightMargin: rightClip ? -radius : 0

      Rectangle {
        id: bottomBorder
        anchors.fill: parent
        border.color: "#DEDEDE"
        radius: root.radius
      }

      Rectangle {
        id: background
        anchors.fill: parent
        anchors.bottomMargin: 1

        gradient: Gradient {
          GradientStop {
            position: 0.1
            color: pressed ? colors[4] : (hovered ? colors[2] : colors[0])
          }
          GradientStop {
            position: 0.9
            color: pressed ? colors[5] : (hovered ? colors[3] : colors[1])
          }
        }

        border.color: "#4A4A4A"
        radius: root.radius
      }
    }

    Rectangle {
      anchors.top: root.top
      anchors.right: root.right
      color: root.border.color
      height: root.height - 1
      width: 1
      visible: rightSeparator
    }
  }

  ColumnLayout {
    anchors.fill: parent
    spacing: 0

    RowLayout {
      id: titleBar
      Layout.topMargin: 5
      Layout.bottomMargin: 3
      Layout.leftMargin: 10
      Layout.rightMargin: 10
      Layout.minimumHeight: 17
      Layout.fillWidth: true
      Layout.fillHeight: false
      spacing: 6

      NSvgImage {
        Layout.preferredWidth: 17
        Layout.fillHeight: true
        source: svgSource
        elementId: "icon"
      }

      NFadeOut {
        Layout.fillWidth: true
        Layout.fillHeight: true
        Text {
          id: textItem
          text: oldMainWindow.windowTitle
          color: "#000000"
          font.bold: true
          font.pixelSize: 12
          anchors.centerIn: parent.width >= textItem.width ? parent : undefined
          anchors.left: parent.width >= textItem.width ? undefined : parent.left
          anchors.top: parent.top
          anchors.bottom: parent.bottom
          anchors.topMargin: -5
          verticalAlignment: Text.AlignVCenter
        }

        layer.enabled: true
        layer.effect: DropShadow {
          verticalOffset: 1
          color: "#FFFFFF"
          radius: 0
          samples: 0
        }
      }

      NSvgButton {
        Layout.preferredWidth: 16
        Layout.fillHeight: true
        onClicked: mainWindow.showMinimized()
        source: svgSource
        elementId: "minimize"
        image.anchors.topMargin: pressed ? 2 : 0
        Rivet {
          radius: Math.min(width, height) / 2
          bottomBorder.color: "#FFFFFF"
          colors: windowButtonColors
        }
      }
      NSvgButton {
        Layout.preferredWidth: 16
        Layout.fillHeight: true
        onClicked: mainWindow.close()
        source: svgSource
        elementId: "close"
        image.anchors.topMargin: pressed ? 2 : 0
        Rivet {
          radius: Math.min(width, height) / 2
          bottomBorder.color: "#FFFFFF"
          colors: windowButtonColors
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
        height: 5
        NSvgImage {
          anchors.top: parent.top
          anchors.left: parent.left
          anchors.right: parent.right
          implicitHeight: 4
          source: svgSource
          elementId: "splitter"
        }
        Rectangle {
          anchors.bottom: parent.bottom
          anchors.left: parent.left
          anchors.right: parent.right
          height: 1
          color: "#6C6C6C"
        }
      }

      Item {
        z: 1

        Rectangle {
          anchors.bottom: parent.bottom
          anchors.left: parent.left
          anchors.right: parent.right
          anchors.leftMargin: 1
          anchors.rightMargin: 1
          anchors.bottomMargin: -5
          height: 50
          gradient: Gradient {
            GradientStop {
              position: 0.0
              color: "#C8C8C8"
            }
            GradientStop {
              position: 1.0
              color: "#A0A0A0"
            }
          }
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
          anchors.topMargin: 1
          anchors.leftMargin: 10
          anchors.rightMargin: 10
          spacing: 0

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
                border.color: "#FAFAFA"
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
                id: waveformInnerContainer
                anchors.fill: parent
                visible: false

                Rectangle {
                  anchors.fill: parent
                  gradient: Gradient {
                    GradientStop {
                      position: 0.0
                      color: "#28323D"
                    }
                    GradientStop {
                      position: 1.0
                      color: "#3F4F61"
                    }
                  }
                }

                NWaveformBar {
                  id: waveformBar
                  anchors.fill: parent
                  gradientStops: [
                    {
                      position: 0.0,
                      color: "#55A807"
                    },
                    {
                      position: 1.0,
                      color: "#366B04"
                    }
                  ]
                  borderColor: "#8AE234"
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
                    color: playbackEngine.state == N.PlaybackPlaying ? "#0080FF" : "#CE8419"
                  }
                }

                Blend {
                  anchors.fill: parent
                  source: waveformBar
                  foregroundSource: leftSide
                  mode: "hardLight"
                }
              }

              Rectangle {
                id: waveformRoundedMask
                anchors.fill: parent
                color: "white"
                radius: 5
                visible: false
              }

              OpacityMask {
                anchors.fill: parent
                maskSource: waveformRoundedMask
                source: waveformInnerContainer
              }

              NTrackInfo {
                anchors.margins: 2
                itemDelegate: Text {
                  color: "#FFFFFF"
                  leftPadding: 3
                  rightPadding: 3
                  font.bold: true
                  font.pixelSize: (rowIndex == 1 && columnIndex == 1) ? 12 : 11
                  text: itemText
                  Rectangle {
                    visible: itemText != ""
                    anchors.fill: parent
                    color: "#000000"
                    opacity: 0.75
                    radius: 7
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

          Item {
            Layout.minimumHeight: 9
          }

          RowLayout {
            Layout.fillWidth: true
            Layout.minimumHeight: 25
            Layout.maximumHeight: 25
            spacing: 0

            NSvgButton {
              Layout.preferredWidth: 54
              Layout.fillHeight: true
              source: svgSource
              elementId: "prev"
              image.anchors.topMargin: pressed ? 2 : 0
              //onClicked: playlistWidget.playPrevItem();
              Rivet {
                rightClip: true
                rightSeparator: true
              }
            }
            NSvgButton {
              Layout.preferredWidth: 53
              Layout.fillHeight: true
              source: svgSource
              elementId: playbackEngine.state == N.PlaybackPlaying ? "pause" : "play"
              image.anchors.topMargin: pressed ? 2 : 0
              onClicked: {
                if (playbackEngine.state == N.PlaybackPlaying) {
                  playbackEngine.pause();
                } else {
                  playbackEngine.play();
                }
              }
              Rivet {
                leftClip: true
                rightClip: true
                rightSeparator: true
              }
            }
            NSvgButton {
              Layout.preferredWidth: 53
              Layout.fillHeight: true
              source: svgSource
              elementId: "stop"
              image.anchors.topMargin: pressed ? 2 : 0
              onClicked: playbackEngine.stop()
              Rivet {
                leftClip: true
                rightClip: true
                rightSeparator: true
              }
            }
            NSvgButton {
              Layout.preferredWidth: 53
              Layout.fillHeight: true
              source: svgSource
              elementId: "next"
              image.anchors.topMargin: pressed ? 2 : 0
              //onClicked: playlistWidget.playNextItem();
              Rivet {
                leftClip: true
              }
            }

            Item {
              Layout.maximumWidth: 25
              Layout.minimumWidth: 2
              Layout.fillWidth: true
              //Layout.horizontalStretchFactor: 2 // Qt6 only
            }

            RowLayout {
              Layout.maximumHeight: 21
              spacing: 0

              NSvgButton {
                Layout.preferredWidth: 30
                Layout.fillHeight: true
                source: svgSource
                elementId: "repeat"
                image.anchors.topMargin: pressed ? 2 : 0
                //onClicked: playlistWidget.setRepeatMode(!playlistWidget.repeatMode);
                Rivet {
                  rightClip: true
                  rightSeparator: true
                }
              }

              NSvgButton {
                Layout.preferredWidth: 29
                Layout.fillHeight: true
                source: svgSource
                elementId: "shuffle"
                image.anchors.topMargin: pressed ? 2 : 0
                //onClicked: playlistWidget.setShuffleMode(!playlistWidget.shuffleMode);
                Rivet {
                  leftClip: true
                  //leftSeparator: true
                }
              }
            }

            Item {
              Layout.minimumWidth: 15
              Layout.fillWidth: true
            }

            Item {
              Layout.preferredHeight: 11
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

                background: Rivet {
                  hovered: false
                  pressed: false
                  border.color: "#4A4A4A"
                  bottomBorder.color: "#DEDEDE"
                  colors: ["#AAAAAA", "#8C8C8C"]
                  radius: 4
                }

                handle: Rivet {
                  anchors.fill: undefined
                  z: 0

                  x: volumeSlider.visualPosition * (parent.width - width)
                  y: 0
                  width: 25
                  height: parent.height

                  pressed: false
                  hovered: handleMouseArea.containsMouse
                  border.color: "#4A4A4A"
                  bottomBorder.visible: false
                  colors: ["#F8F8F8", "#B4B4B4", "#F8F8F8", "#FBFFE0"]
                  radius: 4

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

          Item {
            Layout.minimumHeight: 5
            Layout.maximumHeight: 5
          }
        }
      }

      NPlaylist {
        anchors.topMargin: 3
        anchors.bottomMargin: 3
        anchors.leftMargin: 5
        anchors.rightMargin: 5
        Layout.minimumHeight: 80

        model: playlistController.model()
        itemHeight: 20
        itemSpacing: -2
        dropIndicatorColor: "#636363"

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
              color: itemData.isFailed ? "#949494" : "#000000"
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
              color: "#E9E9E9"
            }

            Rectangle {
              anchors.fill: parent
              visible: itemData.isSelected
              color: "#9E9E9E"
              opacity: 0.7
            }

            Rectangle {
              anchors.fill: parent
              visible: itemData.isHovered
              color: "transparent"
              border.color: "#949494"

              Rectangle {
                anchors.fill: parent
                anchors.margins: 1
                color: itemData.isSelected ? "#A3A3A3" : "transparent"
                opacity: 0.6
              }
            }

            Canvas {
              anchors.fill: parent
              visible: itemData.isFocused
              onPaint: {
                var ctx = getContext("2d");
                ctx.strokeStyle = "#555555";
                ctx.lineWidth = 2;
                ctx.setLineDash([1, 1]);
                ctx.beginPath();
                ctx.rect(0, 0, width, height);
                ctx.stroke();
              }
            }
          }
        }

        scrollbarWidth: 8
        scrollbarPadding: 0
        scrollbarSpacing: 4
        scrollbarBottomPadding: 15
        scrollbarContentItem: Rectangle {
          radius: 5
          gradient: Gradient {
            orientation: Gradient.Horizontal
            GradientStop {
              position: 0.1
              color: scrollbarHoverHandler.hovered ? "#919191" : "#AAAAAA"
            }
            GradientStop {
              position: 0.9
              color: scrollbarHoverHandler.hovered ? "#828282" : "#919191"
            }
          }
          HoverHandler {
            id: scrollbarHoverHandler
            acceptedDevices: PointerDevice.Mouse
          }
        }

        Rectangle {
          anchors.fill: parent
          anchors.margins: 1
          anchors.topMargin: 0
          color: "#FFFFFF"
          z: -1
        }

        Rectangle {
          anchors.top: parent.top
          anchors.left: parent.left
          anchors.right: parent.right
          anchors.margins: 1
          anchors.topMargin: 0
          height: 11

          gradient: Gradient {
            GradientStop {
              position: 0.0
              color: "#6E000000"
            }
            GradientStop {
              position: 0.5
              color: "#23000000"
            }
            GradientStop {
              position: 0.7
              color: "#0F000000"
            }
            GradientStop {
              position: 1.0
              color: "#00000000"
            }
          }
        }
      }
    }
  }
}
