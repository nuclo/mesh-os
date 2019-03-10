#ifndef mesh_sensor_light_H

#define mesh_sensor_light_H

#if (ARDUINO >= 100)
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

#include "mesh_sensor_.h"

class MeshSensorLight : public MeshSensorCtrl {
  public:
    // Constructor
    MeshSensorLight(uint8_t LIGHT_SENSOR_PIN);

    // Mesh Functions
    void initialize();
    String sample();

  private:
    uint8_t _LIGHT_SENSOR_PIN;
};

#endif