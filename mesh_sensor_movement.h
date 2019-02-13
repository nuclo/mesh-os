#ifndef mesh_sensor_movement_H

#define mesh_sensor_movement_H

#if (ARDUINO >= 100)
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

#include "mesh_sensor_.h"

class MeshSensorMovement : public MeshSensorCtrl {
  public:
    // Constructor
    MeshSensorMovement(bool IS_AD0_PIN_HIGH=false);

    // Mesh Functions
    void initialize();
    String sample();

  private:
    uint8_t _IC2_ADDR;
    bool _IS_AD0_PIN_HIGH;

    long accelX, accelY, accelZ;
    float gForceX, gForceY, gForceZ;

    long gyroX, gyroY, gyroZ;
    float rotX, rotY, rotZ;

    // Functions to process data
    void _configureMPU();
    void _processAccelData();
    void _processGyroData();
    void _recordAccelRegisters();
    void _recordGyroRegisters();
};

#endif