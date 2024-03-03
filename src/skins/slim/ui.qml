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

import Nulloy 1.0
import NWaveformBar 1.0
import NSvgImage 1.0

Rectangle {
  color: "#4F545B"

  Rectangle {
    anchors.fill: parent
    color: "transparent"
    border.color: "#646A73"
    z: 1
  }

  Layout.minimumWidth: 300

  property string svgSource: "design.svg"
  property string settingsPrefix: "SlimSkin/"

  Connections {
    target: Qt.application
    function onAboutToQuit() {
    //settings.setValue(settingsPrefix + "Splitter", splitter.states);
    }
  }

  ColumnLayout {
    anchors.fill: parent
    spacing: 0

    RowLayout {
      id: titleBar
      Layout.topMargin: 4
      Layout.leftMargin: 6
      Layout.rightMargin: 6
      Layout.minimumHeight: 16
      Layout.fillWidth: true
      Layout.fillHeight: false
      spacing: 8

      NSvgImage {
        Layout.fillHeight: true
        Layout.preferredWidth: 18
        source: svgSource
        elementId: "icon"
      }

      NFadeOut {
        Layout.fillWidth: true
        Layout.fillHeight: true
        Text {
          id: textItem
          text: oldMainWindow.windowTitle
          color: "#D1D8DB"
          font.bold: true
          font.pixelSize: 12
          anchors.centerIn: parent.width >= textItem.width ? parent : undefined
          anchors.left: parent.width >= textItem.width ? undefined : parent.left
          anchors.top: parent.top
          anchors.bottom: parent.bottom
          verticalAlignment: Text.AlignVCenter
        }

        layer.enabled: true
        layer.effect: DropShadow {
          verticalOffset: 1
          color: "#363b42"
          radius: 0
          samples: 0
        }
      }

      NSvgButton {
        Layout.preferredWidth: 20
        Layout.preferredHeight: 18
        onClicked: mainWindow.showMinimized()
        source: svgSource
        elementId: "minimize"
        elementIdHovered: "minimize-hover"
        elementIdPressed: "minimize-press"
      }

      NSvgButton {
        Layout.preferredWidth: 20
        Layout.preferredHeight: 18
        onClicked: mainWindow.close()
        source: svgSource
        elementId: "close"
        elementIdHovered: "close-hover"
        elementIdPressed: "close-press"
      }
    }

    NSplitter {
      id: splitter
      Layout.fillWidth: true
      Layout.fillHeight: true

      // FIXME: temporal adjustment for compatibility with the old skin:
      states: [parseInt(settings.value(settingsPrefix + "Splitter")[0]) + 4]

      handleDelegate: NSvgImage {
        height: 7
        source: svgSource
        elementId: "splitter"
        colorOverlay: "#2c3034"
      }

      Item {
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
          anchors.topMargin: 4
          anchors.leftMargin: 6
          anchors.rightMargin: 6
          spacing: 5

          Item {
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.minimumHeight: 30

            Rectangle {
              anchors.fill: parent
              color: "#3B4047"
              border.color: "#2c3034"
            }

            RowLayout {
              anchors.fill: parent
              spacing: 0

              NSvgButton {
                Layout.preferredWidth: 27
                Layout.preferredHeight: 27
                source: svgSource
                elementId: "prev"
                elementIdHovered: "prev-hover"
                elementIdPressed: "prev-press"
                //onClicked: playlistWidget.playPrevItem();
              }
              NSvgButton {
                Layout.preferredWidth: 27
                Layout.preferredHeight: 27
                source: svgSource
                elementId: playbackEngine.state == N.PlaybackPlaying ? "pause" : "play"
                elementIdHovered: playbackEngine.state == N.PlaybackPlaying ? "pause-hover" : "play-hover"
                elementIdPressed: playbackEngine.state == N.PlaybackPlaying ? "pause-press" : "play-press"
                onClicked: {
                  if (playbackEngine.state == N.PlaybackPlaying) {
                    playbackEngine.pause();
                  } else {
                    playbackEngine.play();
                  }
                }
              }
              NSvgButton {
                Layout.preferredWidth: 27
                Layout.preferredHeight: 27
                source: svgSource
                elementId: "next"
                elementIdHovered: "next-hover"
                elementIdPressed: "next-press"
                //onClicked: playlistWidget.playNextItem();
              }

              NCoverImage {
                margin: 1
                Layout.fillHeight: true
                growHorizontally: true

                Rectangle {
                  anchors.fill: parent
                  anchors.margins: 1
                  border.color: "#FFA519"
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
                  anchors.margins: 1
                  anchors.fill: parent

                  NWaveformBar {
                    id: waveformBar
                    anchors.fill: parent
                    visible: false
                    borderColor: "transparent"
                  }

                  Item {
                    id: leftSide
                    anchors.fill: parent
                    LinearGradient {
                      anchors {
                        top: parent.top
                        bottom: parent.bottom
                        left: parent.left
                      }
                      width: parent.width * waveformContainer.position
                      gradient: Gradient {
                        GradientStop {
                          position: 0.0
                          color: playbackEngine.state == N.PlaybackPlaying ? "#FFF657" : "#9BA0AA"
                        }
                        GradientStop {
                          position: 1.0
                          color: playbackEngine.state == N.PlaybackPlaying ? "#FFA519" : "#686E79"
                        }
                      }
                    }
                    visible: false
                  }

                  OpacityMask {
                    anchors.fill: parent
                    maskSource: waveformBar
                    source: leftSide
                  }

                  Item {
                    id: rightSide
                    anchors.fill: parent
                    LinearGradient {
                      anchors {
                        top: parent.top
                        bottom: parent.bottom
                        right: parent.right
                      }
                      width: parent.width * (1.0 - waveformContainer.position)
                      gradient: Gradient {
                        GradientStop {
                          position: 0.0
                          color: playbackEngine.state == N.PlaybackPlaying ? "#C0CACD" : "#6D717B"
                        }
                        GradientStop {
                          position: 1.0
                          color: playbackEngine.state == N.PlaybackPlaying ? "#919CA1" : "#545860"
                        }
                      }
                    }
                    visible: false
                  }

                  OpacityMask {
                    anchors.fill: parent
                    maskSource: waveformBar
                    source: rightSide
                  }

                  Rectangle {
                    id: groove
                    height: parent.height
                    width: 1
                    color: playbackEngine.state == N.PlaybackPlaying ? "#FF7B00" : "#B0B5BF"
                    x: parent.width * waveformContainer.position
                  }
                }

                NTrackInfo {
                  anchors.margins: 2
                  itemDelegate: Text {
                    color: "#D1D8DB"
                    leftPadding: 2
                    rightPadding: 2
                    font.bold: true
                    font.pixelSize: (rowIndex == 1 && columnIndex == 1) ? 12 : 11
                    text: itemText
                    Rectangle {
                      visible: itemText != ""
                      anchors.fill: parent
                      color: "#C8242B2F"
                      radius: 2
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

              Item {
                Layout.fillHeight: true
                Layout.preferredWidth: 7

                Slider {
                  id: volumeSlider
                  orientation: Qt.Vertical
                  anchors.fill: parent
                  anchors.margins: 1

                  stepSize: 0.02
                  value: playbackEngine.volume
                  onMoved: {
                    playbackEngine.volume = value;
                    player.showToolTip(player.volumeTooltipText(value));
                  }

                  background: Rectangle {
                    anchors.fill: parent
                    color: parent.hovered || parent.pressed ? "#F0B000" : "#636970"

                    Rectangle {
                      width: parent.width
                      height: volumeSlider.visualPosition * parent.height
                      color: "#3B4047"
                    }
                  }

                  handle: Rectangle {
                    x: 0
                    y: volumeSlider.visualPosition * (parent.height - height)
                    width: parent.width
                    height: parent.width
                    color: parent.hovered || parent.pressed ? "#ff7b00" : "#9C9EA0"
                  }
                }
              }
            }
          }
        }
      }

      NPlaylist {
        Layout.minimumHeight: 80
        anchors.margins: 6
        anchors.topMargin: 0
        clip: true

        model: playlistController.model()
        itemHeight: 20
        itemSpacing: -2
        dropIndicatorColor: "#b6c0c6"
        itemDelegate: Item {
          NFadeOut {
            anchors.fill: parent
            anchors.topMargin: 2
            anchors.leftMargin: 4
            Text {
              text: itemData.text
              font.bold: itemData.isCurrent
              font.pixelSize: 12
              color: itemData.isFailed ? "#555961" : (itemData.isFocused || itemData.isSelected ? (itemData.isCurrent ? "#FFFFFF" : "#e9f2f7") : (itemData.isCurrent ? "#D6DFE3" : "#B6C0C6"))
            }
          }

          Item {
            anchors.fill: parent
            anchors.margins: 1
            z: -1

            Rectangle {
              anchors.fill: parent
              visible: itemData.isSelected
              color: "#3E484F"
              opacity: 0.7
            }

            Rectangle {
              anchors.fill: parent
              visible: itemData.isHovered
              color: "#3e5268"
              opacity: 0.7
            }

            Rectangle {
              anchors.fill: parent
              visible: itemData.isFocused
              color: "transparent"
              border.color: "#6D7E8A"

              Rectangle {
                anchors.fill: parent
                anchors.margins: 1
                visible: itemData.isFocused
                color: "#315063"
                opacity: 0.3
              }
            }
          }
        }

        scrollbarWidth: 7
        scrollbarPadding: 1
        scrollbarSpacing: -1
        scrollbarContentItem: Rectangle {
          color: (scrollbarHoverHandler.hovered || scrollbar.pressed) ? "#E0A500" : "#636970"
          HoverHandler {
            id: scrollbarHoverHandler
            acceptedDevices: PointerDevice.Mouse
          }
        }

        Rectangle {
          anchors.fill: parent
          anchors.margins: 6
          anchors.topMargin: 0
          color: "#242B2F"
          z: -1
        }
        Rectangle {
          anchors.fill: parent
          anchors.margins: 6
          anchors.topMargin: 0
          color: "transparent"
          border.color: "#13181C"
          z: 1
        }
        Rectangle {
          anchors.bottom: parent.bottom
          anchors.left: parent.left
          anchors.right: parent.right
          anchors.margins: 1
          height: 5
          color: "#4F545B"
          z: 1
        }
      }
    }
  }
}
