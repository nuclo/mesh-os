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
  if (sensorName == "accelerometer") return "Ax"+MeshOS::getIntraDataSplitter()+"Ay"+MeshOS::getIntraDataSplitter()+"Az";
  else if (sensorName == "gyroscope") return  "Gx"+MeshOS::getIntraDataSplitter()+"Gy"+MeshOS::getIntraDataSplitter()+"Gz";
  else if (sensorName == "humidity") return "H";
  else if (sensorName == "light") return "L";
  else if (sensorName == "rfid") return "R";
  else if (sensorName == "temperature") return "T";
  else return "-";
}

String MeshSensorCtrl::_floatToStringConversion(float tempVar) {
  char buf[10];
  dtostrf(tempVar, 4, 2, buf);

  return buf;
}