#include "mesh_sensor_.h"
#include "mesh_.h"

/* * * * * * * * * * * * * * * * * * * * * * * *
  VIRTUAL: Functions defined by child
 * * * * * * * * * * * * * * * * * * * * * * * */

MeshSensorCtrl::MeshSensorCtrl() {}

void MeshSensorCtrl::initialize() {}

String MeshSensorCtrl::sample() {}

/* * * * * * * * * * * * * * * * * * * * * * * *
  COMMAND: Awake mode
 * * * * * * * * * * * * * * * * * * * * * * * */

void MeshSensorCtrl::awake() {
  _controlState = _STATE_AWAKE;
}

/* * * * * * * * * * * * * * * * * * * * * * * *
  COMMAND: Sleep mode - power conservation
 * * * * * * * * * * * * * * * * * * * * * * * */

void MeshSensorCtrl::sleep() {
  _controlState = _STATE_SLEEPING;
}

/* * * * * * * * * * * * * * * * * * * * * * * *
  COMMAND: Read state
 * * * * * * * * * * * * * * * * * * * * * * * */

int MeshSensorCtrl::getControlState() {
  return _controlState;
}

/* * * * * * * * * * * * * * * * * * * * * * * *
  COMMAND: Sensor data reading format
 * * * * * * * * * * * * * * * * * * * * * * * */

String MeshSensorCtrl::getDataFormat() {
  return _DATA_FRAME_FORMAT;
}

/* * * * * * * * * * * * * * * * * * * * * * * *
  COMMAND: Control version
 * * * * * * * * * * * * * * * * * * * * * * * */

String MeshSensorCtrl::getSensorName() {
  return _SENSOR_NAME;
}

/* * * * * * * * * * * * * * * * * * * * * * * *
  COMMAND: Control version
 * * * * * * * * * * * * * * * * * * * * * * * */

uint8_t MeshSensorCtrl::getVersion() {
  return _SENSOR_VERSION;
}

// /* * * * * * * * * * * * * * * * * * * * * * * *
//   UTIL: Labels for sensors
//  * * * * * * * * * * * * * * * * * * * * * * * */

static String MeshSensorCtrl::getSensorLabels(String sensorName) {
  if (sensorName == "ac") return "Ax"+MeshOS::getIntraDataSplitter()+"Ay"+MeshOS::getIntraDataSplitter()+"Az";
  else if (sensorName == "gy") return  "Gx"+MeshOS::getIntraDataSplitter()+"Gy"+MeshOS::getIntraDataSplitter()+"Gz";
  else if (sensorName == "hu") return "H";
  else if (sensorName == "li") return "L";
  else if (sensorName == "rf") return "R";
  else if (sensorName == "te") return "T";
  else return "-";
}

// /* * * * * * * * * * * * * * * * * * * * * * * *
//   UTIL: Given a set of floats conver to a string
//  * * * * * * * * * * * * * * * * * * * * * * * */

static String MeshSensorCtrl::stringifyFloatDataset(float* dataset) {
  uint8_t lengthOfDataSet = sizeof(dataset);

  String valueString = "";
  String intraDataSplitter = MeshOS::getIntraDataSplitter();

  for (int i = 0; i < lengthOfDataSet; i++) {
    String buff = "-";

    if (!isnan(dataset[i])) {
      buff = MeshSensorCtrl::_floatToStringConversion(dataset[i]);
    }

    valueString += buff;
    if(i < lengthOfDataSet-1) {
      valueString += intraDataSplitter;
    }
  }

  return valueString;
}

// /* * * * * * * * * * * * * * * * * * * * * * * *
//   UTIL: convert the float to the string
//  * * * * * * * * * * * * * * * * * * * * * * * */

static String MeshSensorCtrl::_floatToStringConversion(float tempVar) {
  char buf[10];
  dtostrf(tempVar, 4, 2, buf);

  return buf;
}