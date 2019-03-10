#include "mesh_sensor_light.h"


#include "mesh_.h"
#include "mesh_sensor_.h"

MeshSensorLight::MeshSensorLight(uint8_t LIGHT_SENSOR_PIN) : MeshSensorCtrl() {
  MeshSensorCtrl::_SENSOR_NAME = "L1";
  MeshSensorCtrl::_SENSOR_VERSION = 1;
  _LIGHT_SENSOR_PIN = LIGHT_SENSOR_PIN;

  String lightLabel = MeshSensorCtrl::getSensorLabels("li");
  MeshSensorCtrl::_DATA_FRAME_FORMAT = lightLabel;
}

/* * * * * * * * * * * * * * * * * * * * * * * *
  COMMAND: BEGIN
 * * * * * * * * * * * * * * * * * * * * * * * */

void MeshSensorLight::initialize() {}

/* * * * * * * * * * * * * * * * * * * * * * * *
  Utility: Read the light
 * * * * * * * * * * * * * * * * * * * * * * * */

String MeshSensorLight::sample() {
  int lightSensorValue = analogRead(_LIGHT_SENSOR_PIN);
  return String(lightSensorValue);
}
