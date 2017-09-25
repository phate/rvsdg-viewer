/******************************************************************************
 *
 * Various configuration defines
 *
 * Asbjørn Djupdal 2017
 *
 *****************************************************************************/

#ifndef RVSDG_VIEWER_H
#define RVSDG_VIEWER_H

///////////////////////////////////////////////////////////////////////////////
// XML defines

#define TAG_NODE     "node"
#define TAG_REGION   "region"
#define TAG_EDGE     "edge"
#define TAG_INPUT    "input"
#define TAG_OUTPUT   "output"
#define TAG_ARGUMENT "argument"
#define TAG_RESULT   "result"

#define ATTR_ID      "id"
#define ATTR_SOURCE  "source"
#define ATTR_TARGET  "target"
#define ATTR_NAME    "name"
#define ATTR_TYPE    "type"

///////////////////////////////////////////////////////////////////////////////
// layout defines

#define INPUTOUTPUT_SIZE       10
#define INPUTOUTPUT_CLEARANCE  10
#define TEXT_CLEARANCE         10
#define LINE_CLEARANCE         10
#define REGION_CLEARANCE       10

#define NODE_COLOR        Qt::gray
#define GAMMA_NODE_COLOR  Qt::green
#define LAMBDA_NODE_COLOR Qt::blue
#define THETA_NODE_COLOR  Qt::red
#define PHI_NODE_COLOR    QColor(255,165,0)

#define EDGE_COLORS { \
  Qt::red, \
  Qt::green, \
  Qt::blue, \
  Qt::yellow, \
  Qt::cyan \
}

#define COLOR_NAMES { \
  "Red", \
  "Green", \
  "Blue", \
  "Yellow", \
  "Cyan" \
}

///////////////////////////////////////////////////////////////////////////////
// gui defines

#define SCALE_IN_FACTOR 1.25
#define SCALE_OUT_FACTOR 0.8

#endif
