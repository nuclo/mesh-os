
#include "mesh_sensor_humi_temp.h"

#include "mesh_.h"
#include "mesh_sensor_.h"

// Sensor Device
#include <DHT.h>

#define DHTTYPE DHT11

/* * * * * * * * * * * * * * * * * * * * * * * *
  Configuration: Sensor Set up
 * * * * * * * * * * * * * * * * * * * * * * * */

MeshSensorHumiTemp::MeshSensorHumiTemp(uint8_t HUMI_TEMP_PIN) :
MeshSensorCtrl(),
_sensorDHT(HUMI_TEMP_PIN, DHTTYPE) {
  MeshSensorCtrl::_SENSOR_NAME = "HT1";
  MeshSensorCtrl::_SENSOR_VERSION = 1;

  String humidityLabel = MeshSensorCtrl::getSensorLabels("hu");
  String intraDataSplitter = MeshOS::getIntraDataSplitter();
  String temperatureLabel = MeshSensorCtrl::getSensorLabels("te");
  MeshSensorCtrl::_DATA_FRAME_FORMAT = "" + humidityLabel + intraDataSplitter + temperatureLabel;
}

/* * * * * * * * * * * * * * * * * * * * * * * *
  MESH: BEGIN
 * * * * * * * * * * * * * * * * * * * * * * * */

void MeshSensorHumiTemp::initialize() {
  _controlState = MeshSensorCtrl::_STATE_INITIALIZING;

  _sensorDHT.begin();

  _controlState = MeshSensorCtrl::_STATE_AWAKE;
}

/* * * * * * * * * * * * * * * * * * * * * * * *
  MESH: Read the humidity and temp
 * * * * * * * * * * * * * * * * * * * * * * * */

String MeshSensorHumiTemp::sample() {
  _humiditySample = _sensorDHT.readHumidity();
  _temperatureSample = _sensorDHT.readTemperature();

  float dataset[2] = { _humiditySample, _temperatureSample };
  String valueString = MeshSensorCtrl::stringifyFloatDataset(dataset);

  return valueString;
}
