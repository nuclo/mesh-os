// Mesh imports
#include "mesh_.h"
#include "mesh_sensor_.h"

//  General imports
#include <Wire.h>

// Bluetooth imports
#include <SoftwareSerial.h>

// Clock imports
#include <DS3232RTC.h>

// Storage imports
#include <SD.h>
#include <SPI.h>

uint8_t MeshOS::_CONTROL_VERSION = 1;
String MeshOS::_SAMPLE_SPLITTER = "\n";
String MeshOS::_DATA_CHUNK_SPLITTER = "||";
String MeshOS::_INTRADATA_SPLITTER = "<>";

#define _TIMESTAMP_FORMAT "YYYY-MM-DDTHH:MM:SS"

#define STATE_INITIALIZING -1
#define STATE_SLEEPING 0
#define STATE_AWAKE 1

#define MILLISECONDS_TO_SECONDS 1000
#define SECONDS_TO_MINUTES 60
#define MINUTES_TO_HOURS 60
#define HOURS_DAYS 24

#define _BAUD_RATE 9600
#define _BAUD_RATE_COMMS 9600

#define MINUTES_BETWEEN_TRANSMISSIONS 5
#define TRANSMISSION_DELAY 300000 // 5 mins * 60 sec * 1000 milli secs;
#define MAXIMUM_TRANSMISSIONS 1000

/* * * * * * * * * * * * * * * * * * * * * * * *
  Configuration: Node Set up
 * * * * * * * * * * * * * * * * * * * * * * * */
MeshOS::MeshOS(
  String NODE_NAME,
  uint8_t NODE_VERSION,
  uint8_t BLT_RXD_PIN,
  uint8_t BLT_TXD_PIN,
  uint8_t BLT_LED_PING_PIN,
  uint8_t SD_CS_PIN,
  float SECONDS_BETWEEN_SAMPLES
) :
  _nodeCommsPort(BLT_RXD_PIN, BLT_TXD_PIN)
{
  _NODE_NAME = NODE_NAME;
  _NODE_VERSION = NODE_VERSION;
  _BLT_LED_PING_PIN = BLT_LED_PING_PIN;
  _SD_CS_PIN = SD_CS_PIN;
  _SAMPLE_DELAY = SECONDS_BETWEEN_SAMPLES * MILLISECONDS_TO_SECONDS;
}

/* * * * * * * * * * * * * * * * * * * * * * * *
  SETUP: INITIALIZE
 * * * * * * * * * * * * * * * * * * * * * * * */

void MeshOS::initialize(MeshSensorCtrl** sensorRelay) {
  Serial.begin(_BAUD_RATE);
  Wire.begin(_BAUD_RATE);

  Serial.println('*');

  _sensorRelay = sensorRelay;
  _controlState = STATE_INITIALIZING;

  // Initialize connected hardware
  _initializeClock();
  _initializeStorage();
  _initializeWirelessCommunication();

  // Initialize all the sensors
  String currentSensorName;
  String currentFilename;
  for (int sensorRelayIndex = 0; sensorRelayIndex < sizeof _sensorRelay/sizeof _sensorRelay[0]; sensorRelayIndex++) {
    _sensorRelay[sensorRelayIndex]->initialize();
  }

  // Start sample times
  _startTimeStamp = getCurrentTimestamp();
  _currentTimeStamp = getCurrentTimestamp();

  _lastSensorRelaySample = 0;
  _lastTransmission = 0;
  _hasStartedSampling = false;

  _sleep();
}

/* * * * * * * * * * * * * * * * * * * * * * * *
  SETUP: Set up the clock
 * * * * * * * * * * * * * * * * * * * * * * * */

void MeshOS::_initializeClock() {
  // setTime(7, 22, 50, 11, 2, 2019);
  // RTC.set(now());

  setSyncProvider(RTC.get);
}

/* * * * * * * * * * * * * * * * * * * * * * * *
  SETUP: Set up the SD storage system
 * * * * * * * * * * * * * * * * * * * * * * * */

void MeshOS::_initializeStorage() {
  pinMode(_SD_CS_PIN, OUTPUT);
  bool sdCardStatus = SD.begin();

  if (!sdCardStatus) {
    Serial.println('x');
  }
}

/* * * * * * * * * * * * * * * * * * * * * * * *
  SETUP: Set up the bluetooth communication
 * * * * * * * * * * * * * * * * * * * * * * * */

void MeshOS::_initializeWirelessCommunication() {
  _nodeCommsPort.begin(_BAUD_RATE_COMMS);
}

/* * * * * * * * * * * * * * * * * * * * * * * *
  COMMAND: Looping node function
 * * * * * * * * * * * * * * * * * * * * * * * */

void MeshOS::loop() {
  _currentTimeStamp = getCurrentTimestamp();
  unsigned long _millisecondsDelta = millis() - _timeStartSync;

  if (!_hasStartedSampling) {
    // Check frequently to sync with the turn over of a second to start sampling
    _hasStartedSampling = _startTimeStamp != _currentTimeStamp;

    // Set delta
    if (_hasStartedSampling) {
      Serial.println('o');

      _startTimeStamp = _currentTimeStamp;
      _timeStartSync = millis();

      // Create new files for all the sensors
      _createSensorRelayDataFiles();
    }

  } else {
    // Sample the sensors at a set frequency
    if ((_millisecondsDelta - _lastSensorRelaySample) >= _SAMPLE_DELAY) {
      _sampleSensorRelay();
      _lastSensorRelaySample = _millisecondsDelta;
    }

    // Send data at a set freqency
    if ((_millisecondsDelta - _lastTransmission) >= TRANSMISSION_DELAY) {
      // Stop sampling when in transmission mode
      _hasStartedSampling = false;

      // Begin transmission
      _transmitSensorRelayFiles();
      _lastTransmission = _millisecondsDelta;
    }
  }
}

// /* * * * * * * * * * * * * * * * * * * * * * * *
//   MESH CONTROL: Data chunk splitter
//  * * * * * * * * * * * * * * * * * * * * * * * */

static String MeshOS::getDataChunkSplitter() {
  return MeshOS::_DATA_CHUNK_SPLITTER;
}

// /* * * * * * * * * * * * * * * * * * * * * * * *
//   MESH CONTROL: Intradata Splitter
//  * * * * * * * * * * * * * * * * * * * * * * * */

static String MeshOS::getIntraDataSplitter() {
  return MeshOS::_INTRADATA_SPLITTER;
}

/* * * * * * * * * * * * * * * * * * * * * * * *
  NODE SPECIFIC: Read state
 * * * * * * * * * * * * * * * * * * * * * * * */

int MeshOS::getControlState() {
  return _controlState;
}

/* * * * * * * * * * * * * * * * * * * * * * * *
  INTERNAL: Awake mode
 * * * * * * * * * * * * * * * * * * * * * * * */

void MeshOS::_awake() {
  // Serial.println('+');
  _controlState = STATE_AWAKE;

  // TODO: Wake up all sensors and periphery connections

  // Check for any imcoming messages
  if (_nodeCommsPort.available() > 0) {
    _receivedIncomingCommunication();
  }
}

/* * * * * * * * * * * * * * * * * * * * * * * *
  INTERNAL: Sleep mode - power conservation
 * * * * * * * * * * * * * * * * * * * * * * * */

void MeshOS::_sleep() {
  _controlState = STATE_SLEEPING;

  // TODO: Put to sleep all periphery connections

  // Serial.println('-');
}

/* * * * * * * * * * * * * * * * * * * * * * * *
  INTERNAL: Sample all sensors
 * * * * * * * * * * * * * * * * * * * * * * * */

void MeshOS::_sampleSensorRelay() {
  _awake();

  for (int sensorRelayIndex = 0; sensorRelayIndex < sizeof _sensorRelay/sizeof _sensorRelay[0]; sensorRelayIndex++) {
    // Saving separately because memory issues timestamps getting written
    _storeSensorTimestamp(sensorRelayIndex);
    _storeSensorReading(sensorRelayIndex);
  }

  _sleep();
}

/* * * * * * * * * * * * * * * * * * * * * * * *
  STORAGE: Get current file name
 * * * * * * * * * * * * * * * * * * * * * * * */

String MeshOS::_getCurrentFilename(int sensorRelayIndex) {
  // Create file and save the format for the data storing
  String sensorName = _sensorRelay[sensorRelayIndex]->getSensorName();
  String filename = _NODE_NAME + '-' + sensorName + '-' + _startTimeStamp + ".000Z.txt";
  return filename;
}

/* * * * * * * * * * * * * * * * * * * * * * * *
  STORAGE: Creating the files
 * * * * * * * * * * * * * * * * * * * * * * * */

void MeshOS::_createSensorRelayDataFiles() {
  // Create file and save the format for the data storing
  String filename;

  // Create new files for all the sensors 
  for (int sensorRelayIndex = 0; sensorRelayIndex < sizeof _sensorRelay/sizeof _sensorRelay[0]; sensorRelayIndex++) {
    filename = _getCurrentFilename(sensorRelayIndex);
    _currentStorageFile = SD.open(filename, FILE_WRITE);

    _closeStorageFile();
  }
}

/* * * * * * * * * * * * * * * * * * * * * * * *
  STORAGE: Store Timestamps
 * * * * * * * * * * * * * * * * * * * * * * * */

void MeshOS::_storeSensorTimestamp(int sensorRelayIndex) {
  unsigned int millisecondDelta = (millis() - _timeStartSync) % 1000;

  // Adding leading zeros, organized to preserve global memory space
  String timestampPadding = ".";
  if (millisecondDelta < 100) timestampPadding += "0";
  if (millisecondDelta < 10) timestampPadding += "0";

  String currentReadingTimestamp = _currentTimeStamp + timestampPadding + String(millisecondDelta) + "Z";

  _openStorageFile(_getCurrentFilename(sensorRelayIndex));
  _writeToStorageFile(currentReadingTimestamp);
  _closeStorageFile();
}

/* * * * * * * * * * * * * * * * * * * * * * * *
  STORAGE: Add readings
 * * * * * * * * * * * * * * * * * * * * * * * */

void MeshOS::_storeSensorReading(int sensorRelayIndex) {
  String currentSensorReading = _sensorRelay[sensorRelayIndex]->sample();

  _openStorageFile(_getCurrentFilename(sensorRelayIndex));
  _writeToStorageFile(currentSensorReading);
  _closeStorageFile();
}
/* * * * * * * * * * * * * * * * * * * * * * * *
  FILES: Open a file
 * * * * * * * * * * * * * * * * * * * * * * * */

void MeshOS::_openStorageFile(String storageFilename) {
  _currentStorageFile = SD.open(storageFilename);
}

/* * * * * * * * * * * * * * * * * * * * * * * *
  FILES: Write data to an opened file
 * * * * * * * * * * * * * * * * * * * * * * * */

void MeshOS::_writeToStorageFile(String textToWrite) {
  if (_currentStorageFile) {
    _currentStorageFile.println(textToWrite);
  }
}

/* * * * * * * * * * * * * * * * * * * * * * * *
  FILES: Close an opened file
 * * * * * * * * * * * * * * * * * * * * * * * */

void MeshOS::_closeStorageFile() {
  if (_currentStorageFile) {
    _currentStorageFile.close();
  }
}

/* * * * * * * * * * * * * * * * * * * * * * * *
  FILES: Close an opened file
 * * * * * * * * * * * * * * * * * * * * * * * */

String MeshOS::_readLineFromOpenStorageFile() {
  String received = "";
  char ch;
  while (_currentStorageFile.available()){
    ch = _currentStorageFile.read();
    if (ch == '\n') {
      return String(received);
    } else {
      received += ch;
    }
  }
  return received;
}
/* * * * * * * * * * * * * * * * * * * * * * * *
  TRANSMISSION: Received incoming
 * * * * * * * * * * * * * * * * * * * * * * * */

void MeshOS::_receivedIncomingCommunication() {
  Serial.println('~');

  if (_nodeCommsPort.available()) {
    // Serial.write(_nodeCommsPort.read());
  }
}

/* * * * * * * * * * * * * * * * * * * * * * * *
  TRANSMISSION: Send the data that is saved 
 * * * * * * * * * * * * * * * * * * * * * * * */

void MeshOS::_transmitSensorRelayFiles() {
  // Start light
  digitalWrite(_BLT_LED_PING_PIN, HIGH);

  // Open the file to save to
  String storageFilename;

  for (int sensorRelayIndex = 0; sensorRelayIndex < sizeof _sensorRelay/sizeof _sensorRelay[0]; sensorRelayIndex++) {
    storageFilename = _getCurrentFilename(sensorRelayIndex);
    _openStorageFile(storageFilename);

    // Loop through each line and transmit to gateway
    String currentLine = ".";
    uint8_t lineIndex = 0;

    // Check the length of the line to see if we are at the end of the file or max transmissions
    while ((currentLine.length() > 0) || (lineIndex >= MAXIMUM_TRANSMISSIONS)) {
      // Transmit line
      currentLine = _readLineFromOpenStorageFile();
      _nodeCommsPort.println(currentLine);
      lineIndex++;
    }

    // Close file
    _closeStorageFile();
  }

  // Turn off light
  digitalWrite(_BLT_LED_PING_PIN, LOW);
}


/* * * * * * * * * * * * * * * * * * * * * * * *
  CLOCK: Request the time
 * * * * * * * * * * * * * * * * * * * * * * * */

String MeshOS::getCurrentTimestamp() {
  // Format the time and date and insert into the temporary buffer.
  char buf[25];
  snprintf(buf, sizeof(buf), "%04d-%02d-%02dT%02d:%02d:%02d",
      year(), month(), day(),
      hour(), minute(), second());

  // Print the formatted string to serial so we can see the time.
  return buf;
}
