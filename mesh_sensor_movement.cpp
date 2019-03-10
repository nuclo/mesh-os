
#include "mesh_sensor_movement.h"

#include "mesh_.h"
#include "mesh_sensor_.h"

#if ARDUINO >= 100
#include <Wire.h>
#define i2cAvailable Wire.available
#define i2cBegin Wire.begin
#define i2cBeginTransmission Wire.beginTransmission
#define i2cEndTransmission Wire.endTransmission
#define i2cRequestFrom Wire.requestFrom
#define i2cRead Wire.read
#define i2cWrite Wire.write
#else
#include <Wire.h>
#define i2cAvailable Wire.available
#define i2cBegin Wire.begin
#define i2cBeginTransmission Wire.beginTransmission
#define i2cEndTransmission Wire.endTransmission
#define i2cRequestFrom Wire.requestFrom
#define i2cRead Wire.receive
#define i2cWrite Wire.send
#endif

#define IC2_LOW_ADDR 0b1101000
#define IC2_HIGH_ADDR 0b1101001

#define PWR_ADDR 0x6B
#define PWR_SLP 0b00000000

#define ACCEL_CONFIG_REG 0x1C
#define ACCEL_CONFIG_2 0x1B //Setting the accel to +/- 2g
#define ACCEL_DATA_REG 0x3B
#define ACCEL_DATA_FULL 0b1101001

#define GYRO_CONFIG_REG 0x1B
#define GYRO_CONFIG_FULL 0x00000000 //Setting the gyro to full scale +/- 250deg./s 
#define GYRO_DATA_REG 0x43
#define GYRO_DATA_FULL 0b1101001

const long GYRO_CONFIG_SENSITIVITY = 131.0;
const long ACCEL_CONFIG_SENSITIVITY = 16384.0;

/* * * * * * * * * * * * * * * * * * * * * * * *
  Configuration: Sensor Set up
 * * * * * * * * * * * * * * * * * * * * * * * */

MeshSensorMovement::MeshSensorMovement(bool IS_AD0_PIN_HIGH) : MeshSensorCtrl() {
  _IS_AD0_PIN_HIGH = IS_AD0_PIN_HIGH;

  if (_IS_AD0_PIN_HIGH) {
    _IC2_ADDR = IC2_HIGH_ADDR;
  } else {
    _IC2_ADDR = IC2_LOW_ADDR;
  }
  
  MeshSensorCtrl::_SENSOR_NAME = "M1";
  MeshSensorCtrl::_SENSOR_VERSION = 1;

  String accelerometerLabel = MeshSensorCtrl::getSensorLabels("ac");
  String intraDataSplitter = MeshOS::getIntraDataSplitter();
  String gyroscope = MeshSensorCtrl::getSensorLabels("gy");
  MeshSensorCtrl::_DATA_FRAME_FORMAT = "" + accelerometerLabel + intraDataSplitter + gyroscope;
}

/* * * * * * * * * * * * * * * * * * * * * * * *
  MESH: BEGIN
 * * * * * * * * * * * * * * * * * * * * * * * */

void MeshSensorMovement::initialize() {
  _controlState = MeshSensorCtrl::_STATE_INITIALIZING;

  i2cBegin();
  _configureMPU();

  _controlState = MeshSensorCtrl::_STATE_AWAKE;
}

/* * * * * * * * * * * * * * * * * * * * * * * *
  MESH: Read the Movement
 * * * * * * * * * * * * * * * * * * * * * * * */

String MeshSensorMovement::sample() {
  _recordAccelRegisters();
  _recordGyroRegisters();

  float dataset[6] = { rotX, rotY, rotZ, gForceX, gForceY, gForceZ };
  String valueString = MeshSensorCtrl::stringifyFloatDataset(dataset);

  return valueString;
}


/* * * * * * * * * * * * * * * * * * * * * * * *
  Utility: Set up the movement sensor 
 * * * * * * * * * * * * * * * * * * * * * * * */

void MeshSensorMovement::_configureMPU(){
  // Set power management into on mode
  i2cBeginTransmission(_IC2_ADDR);
  i2cWrite(PWR_ADDR);
  i2cWrite(PWR_SLP);
  i2cEndTransmission();  

  // Set up gyroscope params
  i2cBeginTransmission(_IC2_ADDR);
  i2cWrite(GYRO_CONFIG_REG);
  i2cWrite(GYRO_CONFIG_FULL);
  i2cEndTransmission(); 
  
  // Set up accelerometer params
  i2cBeginTransmission(_IC2_ADDR);
  i2cWrite(ACCEL_CONFIG_REG);
  i2cWrite(ACCEL_CONFIG_2);
  i2cEndTransmission(); 
}

/* * * * * * * * * * * * * * * * * * * * * * * *
  Accelerometer: Record the accelerometer data to registers
 * * * * * * * * * * * * * * * * * * * * * * * */

void MeshSensorMovement::_recordAccelRegisters() {
  i2cBeginTransmission(_IC2_ADDR);
  i2cWrite(ACCEL_DATA_REG);
  i2cEndTransmission();
  i2cRequestFrom(ACCEL_DATA_FULL, 6);
  while(i2cAvailable() < 6);
  accelX = i2cRead() << 8 | i2cRead();
  accelY = i2cRead() << 8 | i2cRead();
  accelZ = i2cRead() << 8 | i2cRead();
  _processAccelData();
}

/* * * * * * * * * * * * * * * * * * * * * * * *
  Accelerometer: Post recording filtering
 * * * * * * * * * * * * * * * * * * * * * * * */

void MeshSensorMovement::_processAccelData(){
  gForceX = accelX / ACCEL_CONFIG_SENSITIVITY;
  gForceY = accelY / ACCEL_CONFIG_SENSITIVITY; 
  gForceZ = accelZ / ACCEL_CONFIG_SENSITIVITY;
}
/* * * * * * * * * * * * * * * * * * * * * * * *
  Gyroscope: Record the gyroscope data to registers
 * * * * * * * * * * * * * * * * * * * * * * * */

void MeshSensorMovement::_recordGyroRegisters() {
  i2cBeginTransmission(_IC2_ADDR);
  i2cWrite(GYRO_DATA_REG);
  i2cEndTransmission();
  i2cRequestFrom(GYRO_DATA_FULL, 6);
  while(i2cAvailable() < 6);
  gyroX = i2cRead() << 8 | i2cRead();
  gyroY = i2cRead() << 8 | i2cRead();
  gyroZ = i2cRead() << 8 | i2cRead();
  _processGyroData();
}

/* * * * * * * * * * * * * * * * * * * * * * * *
  Gyroscope: Post recording filtering
 * * * * * * * * * * * * * * * * * * * * * * * */

void MeshSensorMovement::_processGyroData() {
  rotX = gyroX / GYRO_CONFIG_SENSITIVITY;
  rotY = gyroY / GYRO_CONFIG_SENSITIVITY; 
  rotZ = gyroZ / GYRO_CONFIG_SENSITIVITY;
}