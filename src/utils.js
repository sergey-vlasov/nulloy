function calculateMinimumHeight(item) {
  if (typeof ColumnLayout === 'undefined') {
    console.error('utils.js: QtQuick.Layouts definition is missing');
  }
  return _calculateMinimumHeight(item);
}

function _calculateMinimumHeight(item) {
  if (item.Layout.minimumHeight > 0) {
    return item.Layout.minimumHeight;
  } else if (item instanceof ColumnLayout) {
    return Object.keys(item.children).reduce((acc, key) => {
      return acc + item.children[key].Layout.minimumHeight;
    }, 0) + item.spacing * (item.children.length - 1) + item.anchors.topMargin + item.anchors.bottomMargin;
  } else {
    return Object.keys(item.children).reduce((acc, key) => {
      return acc + _calculateMinimumHeight(item.children[key]);
    }, 0);
  }
}

function calculateMinimumWidth(item) {
  if (typeof RowLayout === 'undefined') {
    console.error('utils.js: QtQuick.Layouts definition is missing');
  }
  return _calculateMinimumWidth(item);
}

function _calculateMinimumWidth(item) {
  if (item.Layout.minimumWidth > 0) {
    return item.Layout.minimumWidth;
  } else if (item instanceof RowLayout) {
    return Object.keys(item.children).reduce((acc, key) => {
      return acc + item.children[key].Layout.minimumWidth;
    }, 0) + item.spacing * (item.children.length - 1) + item.anchors.leftMargin + item.anchors.rightMargin;
  } else {
    return Object.keys(item.children).reduce((acc, key) => {
      return acc + _calculateMinimumWidth(item.children[key]);
    }, 0);
  }
}

function bound(min, val, max) {
  return Math.min(Math.max(min, val), max);
}
