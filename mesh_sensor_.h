#ifndef mesh_sensor_H

#define mesh_sensor_H

#if (ARDUINO >= 100)
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

class MeshSensorCtrl {
  public:
    // Constructor
    MeshSensorCtrl();

    // Command Methods 
    void awake();
    void sleep();
    int getControlState();
    String getDataFormat();
    String getSensorName();
    uint8_t getVersion();
    
    // Util
    static String getSensorLabels(String sensorName);
    static String stringifyFloatDataset(float* dataset);

    // Functions that must be defined
    virtual void initialize();
    virtual String sample();

  protected:
    String _SENSOR_NAME;
    uint8_t _SENSOR_VERSION;
    int _controlState;
    String _DATA_FRAME_FORMAT = "";

    const static int _STATE_INITIALIZING = -1;
    const static int _STATE_SLEEPING = 0;
    const static int _STATE_AWAKE = 1;
    const static int _STATE_READING = 2;

  private:
    static String _floatToStringConversion(float tempVar);
};

#endif