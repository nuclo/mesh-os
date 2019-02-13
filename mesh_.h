#ifndef mesh_H

#define mesh_H

#if (ARDUINO >= 100)
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

// Mesh imports
#include "mesh_sensor_.h"

// Bluetooth imports
#include <SoftwareSerial.h>

// Clock imports
// #include <DS3232RTC.h>

// Storage imports
#include <SD.h>
#include <SPI.h>

class MeshOS {
  public:
    // Constructor
    MeshOS(
      String NODE_NAME,
      uint8_t NODE_VERSION,
      uint8_t BLT_RXD_PIN,
      uint8_t BLT_TXD_PIN,
      uint8_t BLT_LED_PING_PIN,
      uint8_t SD_CS_PIN,
      float SECONDS_BETWEEN_SAMPLES
    );

    // Command Methods 
    void initialize(MeshSensorCtrl** sensorRelay);
    void loop();

    // Mesh control
    static String getDataChunkSplitter();
    static String getIntraDataSplitter();

    // Node Specific
    int getControlState();
    String getDataFormat();
    uint8_t getNodeVersion();

    // Communication Methods 
    int getDataFrameSize();

    // Clock Methods
    String getCurrentTimestamp();

  private:
    static uint8_t _CONTROL_VERSION;
    static String _SAMPLE_SPLITTER;
    static String _DATA_CHUNK_SPLITTER;
    static String _INTRADATA_SPLITTER;

    String _NODE_NAME;
    uint8_t _NODE_VERSION;

    // Pins
    uint8_t _BLT_LED_PING_PIN;
    uint8_t _SD_CS_PIN;

    // Connected hardware on node
    File _currentStorageFile;
    SoftwareSerial _nodeCommsPort;
    MeshSensorCtrl** _sensorRelay;

    // Node loop Variables
    int _controlState;

    String _startTimeStamp;
    String _currentTimeStamp;

    unsigned int _timeStartSync;
    unsigned long _lastSensorRelaySample;
    unsigned long _lastTransmission;

    bool _hasStartedSampling;

    unsigned long _SAMPLE_DELAY;

    // Set up functions
    void _initializeClock();
    void _initializeStorage();
    void _initializeWirelessCommunication();

    // Internal node function
    void _awake();
    void _sleep();
    void _sampleSensorRelay();

    // Storage
    void _addReadingToDataset(String dataTimestamp, String dataReading);
    void _createSensorRelayDataFiles();
    String _getCurrentFilename(int sensorRelayIndex);
    void _storeSensorTimestamp(int sensorRelayIndex);
    void _storeSensorReading(int sensorRelayIndex);  

    void _openStorageFile(String storageFilename);
    void _writeToStorageFile(String textToWrite);
    void _closeStorageFile();
    String _readLineFromOpenStorageFile();

    // Transmission
    void _receivedIncomingCommunication();
    void _transmitSensorRelayFiles();
};

#endif