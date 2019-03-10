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
    void toggleSettingsComms(bool componentSettings=false);
    void toggleSettingsClock(bool componentSettings=false);
    void toggleSettingsStorage(bool componentSettings=false);
    void toggleSettingsSampleTimeTransmission(bool componentSettings=false);
    int getControlState();
    String getDataFormat();
    uint8_t getNodeVersion();

    // Communication Methods 
    int getDataFrameSize();

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

    // Toggling hardware connected
    bool _IS_ENABLED_COMMS = true;
    bool _IS_ENABLED_CLOCK = true;
    bool _IS_ENABLED_STORAGE = true;
    bool _IS_ENABLED_SAMPLE_TIME_TRANSMISSION = false;

    // Node loop Variables
    int _controlState;

    unsigned long _startTimeStamp;
    unsigned long _currentTimeStamp;

    uint8_t _timeStartSync;
    unsigned long _lastSensorRelaySample;
    unsigned long _lastTransmission;

    bool _hasStartedSampling;

    unsigned long _SAMPLE_DELAY;
    unsigned long _TRANSMISSION_DELAY;

    // Set up functions
    void _initializeClock();
    void _initializeStorage();
    void _initializeWirelessCommunication();

    // Internal node function
    void _awake();
    void _sleep();
    void _sampleSensorRelay();

    // Storage
    void _createSensorRelayDataFiles(int sensorRelayIndex);
    String _getCurrentFilename(int sensorRelayIndex);
    void _storeCurrentSensorReading(int sensorRelayIndex);  

    void _closeStorageFile();
    bool _openStorageFile(int sensorRelayIndex, bool toWrite=false);
    String _readLineFromOpenStorageFile();
    void _writeToStorageFile(String textToWrite);

    // Transmission
    void _receivedIncomingCommunication();
    void _transmitSensorRelayFiles();
    void _transmitMessage(String currentMessage);

    // Clock
    uint8_t _calculateReadingTimestamp();
};

#endif