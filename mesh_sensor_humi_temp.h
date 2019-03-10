#ifndef mesh_sensor_humi_temp_H

#define mesh_sensor_humi_temp_H

#if (ARDUINO >= 100)
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

#include "mesh_sensor_.h"

// Sensor Device
#include <DHT.h>


class MeshSensorHumiTemp : public MeshSensorCtrl {
  public:
    // Constructor
    MeshSensorHumiTemp(uint8_t HUMI_TEMP_PIN);

    // Mesh Functions
    void initialize();
    String sample();

  private:
    DHT _sensorDHT;

    float _humiditySample, _temperatureSample;
};

#endif