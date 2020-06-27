#ifdef USES_P178
//#######################################################################################################
//######################## Plugin 178: Air Quality sensor MICS VZ 89TE(I2C) ######################
//#######################################################################################################

#define PLUGIN_178
#define PLUGIN_ID_178         178
#define PLUGIN_NAME_178       "Gases - VZ89T (I2C) [Testing]"
#define PLUGIN_VALUENAME1_178 "VOC"
#define PLUGIN_VALUENAME2_178 "CO2"

#include "_Plugin_Helper.h"

float Plugin_178_VOC        = 0;
float Plugin_178_CO2        = 0;
uint32_t Plugin_178_RS      = 0;
uint8_t  Plugin_178_Status  = 0;

uint8_t  Plugin_178_addr;
#define MICS_I2C_ADDRESS      0x70 // I2C address for the sensor

boolean Plugin_178(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_178;
        Device[deviceCount].Type = DEVICE_TYPE_I2C;
        Device[deviceCount].VType = SENSOR_TYPE_DUAL;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = true;
        Device[deviceCount].ValueCount = 2;
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].TimerOption = true;
        Device[deviceCount].GlobalSyncOption = true;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_178);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_178));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_178));
        break;
      }

    case PLUGIN_READ:
      {
        Plugin_178_init_MICS(MICS_I2C_ADDRESS);
        
        success = Plugin_178_readSample();

        if (success) {
            UserVar[event->BaseVarIndex] = Plugin_178_VOC;
            String log = F("MICS: VOC: ");
            log += UserVar[event->BaseVarIndex];
            addLog(LOG_LEVEL_INFO, log);

            UserVar[event->BaseVarIndex + 1] = Plugin_178_CO2;
            log = F("MICS: CO2: ");
            log += UserVar[event->BaseVarIndex + 1];
            addLog(LOG_LEVEL_INFO, log);
        } else {
          String log = F("MICS: No reading!");
          addLog(LOG_LEVEL_INFO, log);
          UserVar[event->BaseVarIndex] = NAN;
          UserVar[event->BaseVarIndex + 1] = NAN;
        }
        break;
      }
  }
  return success;
}

void Plugin_178_init_MICS(uint8_t i2c_addr) {
  Plugin_178_addr = i2c_addr;
}

bool Plugin_178_readSample()
{
  uint8_t data[7];
  String log;
  uint8_t i =0;

  Wire.beginTransmission((uint8_t)Plugin_178_addr);
  Wire.write(0x0C); //This send the command to get data
  Wire.write(0x0);
  Wire.write(0x0);
  Wire.write(0x0);
  Wire.write(0x0);
  Wire.write(0xF3);
  Wire.endTransmission();
  
  Wire.beginTransmission(0xE1);
  Wire.endTransmission();
  
  Wire.requestFrom(Plugin_178_addr, uint8_t(7));
  if (Wire.available() == 7) {
      for (i=0; i<7; i++) {
      data[i] = Wire.read();
      log = F("MICS: data[");
      log += i;
      log += "] = ";
      log += data[i];
      addLog(LOG_LEVEL_DEBUG, log);
    }
  } else {
    log = F("MICS: Error data read ");
    addLog(LOG_LEVEL_INFO, log);
    return false;
  }
    
  // -- Important: this is assuming 2-byte data, 1-byte

  // check CRC
  if (Plugin_178_crc8(data, 6) != data[6]) {
      log = F("MICS: Error crc : ");
      log += Plugin_178_crc8(data, 6);
      addLog(LOG_LEVEL_INFO, log);
    return false;
  }

  // convert to VOC / CO2
  Plugin_178_VOC = (float(data[0]) - 13) * ( 1000 / 229 );
  Plugin_178_CO2 = (float(data[1]) - 13) * ( 1600 / 229 ) + 400;
  Plugin_178_RS = 10 * (data[4] + data[3] * 256 + data[2] * 65536);
  Plugin_178_Status = data[5];
  
  //log = F("MICS: Resistor = ");
  //log += Plugin_178_RS;
  //addLog(LOG_LEVEL_INFO, log);
  //log = F("MICS: Status = ");
  //log += Plugin_178_Status;
  //addLog(LOG_LEVEL_INFO, log);

  return true;
}


uint8_t Plugin_178_crc8(const uint8_t* data, uint8_t len)
{
  uint8_t crc = 0;
  uint8_t byteCtr;
  uint16_t sum = 0;
  for (byteCtr = 0; byteCtr < len; ++byteCtr) {
    sum = crc + data[byteCtr];
    crc = (uint8_t)sum;
    crc += (sum/0x100);
  }
  return 0xff - crc;
}
#endif
