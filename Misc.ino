#if FEATURE_SPIFFS
void fileSystemCheck()
{
  if (SPIFFS.begin())
  {
    String log = F("SPIFFS Mount succesfull");
    addLog(LOG_LEVEL_INFO,log);
    File f = SPIFFS.open("config.txt", "r");
    if (!f)
    {
      log = F("formatting...");
      addLog(LOG_LEVEL_INFO,log);
      SPIFFS.format();
      log = F("format done!");
      addLog(LOG_LEVEL_INFO,log);
      File f = SPIFFS.open("config.txt", "w");
      if (f)
      {
        for (int x = 0; x < 32768; x++)
          f.write(0);
        f.close();
      }
      f = SPIFFS.open("security.txt", "w");
      if (f)
      {
        for (int x = 0; x < 512; x++)
          f.write(0);
        f.close();
      }
    }
  }
  else
  {
    String log = F("SPIFFS Mount failed");
    addLog(LOG_LEVEL_INFO,log);
  }
}
#endif


/********************************************************************************************\
* Find device index corresponding to task number setting
\*********************************************************************************************/
byte getDeviceIndex(byte Number)
{
  byte DeviceIndex = 0;
  for (byte x = 0; x <= deviceCount ; x++)
    if (Device[x].Number == Number)
      DeviceIndex = x;
  return DeviceIndex;
}


/********************************************************************************************\
* Find protocol index corresponding to protocol setting
\*********************************************************************************************/
byte getProtocolIndex(byte Number)
{
  byte ProtocolIndex = 0;
  for (byte x = 0; x <= protocolCount ; x++)
    if (Protocol[x].Number == Number)
      ProtocolIndex = x;
  return ProtocolIndex;
}


/********************************************************************************************\
* Find positional parameter in a char string
\*********************************************************************************************/
boolean GetArgv(const char *string, char *argv, int argc)
{
  int string_pos = 0, argv_pos = 0, argc_pos = 0;
  char c, d;

  while (string_pos < strlen(string))
  {
    c = string[string_pos];
    d = string[string_pos + 1];

    if       (c == ' ' && d == ' ') {}
    else if  (c == ' ' && d == ',') {}
    else if  (c == ',' && d == ' ') {}
    else if  (c == ' ' && d >= 33 && d <= 126) {}
    else if  (c == ',' && d >= 33 && d <= 126) {}
    else
    {
      argv[argv_pos++] = c;
      argv[argv_pos] = 0;

      if (d == ' ' || d == ',' || d == 0)
      {
        argv[argv_pos] = 0;
        argc_pos++;

        if (argc_pos == argc)
        {
          return true;
        }

        argv[0] = 0;
        argv_pos = 0;
        string_pos++;
      }
    }
    string_pos++;
  }
  return false;
}


/********************************************************************************************\
* Convert a char string to integer
\*********************************************************************************************/
unsigned long str2int(char *string)
{
  unsigned long temp = atof(string);
  return temp;
}


/********************************************************************************************\
* Convert a char string to IP byte array
\*********************************************************************************************/
boolean str2ip(char *string, byte* IP)
{
  byte c;
  byte part = 0;
  int value = 0;

  for (int x = 0; x <= strlen(string); x++)
  {
    c = string[x];
    if (isdigit(c))
    {
      value *= 10;
      value += c - '0';
    }

    else if (c == '.' || c == 0) // next octet from IP address
    {
      if (value <= 255)
        IP[part++] = value;
      else
        return false;
      value = 0;
    }
    else if (c == ' ') // ignore these
      ;
    else // invalid token
      return false;
  }
  if (part == 4) // correct number of octets
    return true;
  return false;
}


/********************************************************************************************\
* Save settings to SPIFFS
\*********************************************************************************************/
void SaveSettings(void)
{
#if FEATURE_SPIFFS
  SaveToFile((char*)"config.txt", 0, (byte*)&Settings, sizeof(struct SettingsStruct));
  SaveToFile((char*)"security.txt", 0, (byte*)&SecuritySettings, sizeof(struct SecurityStruct));
#else
  SaveToFlash(0, (byte*)&Settings, sizeof(struct SettingsStruct));
  SaveToFlash(32768, (byte*)&SecuritySettings, sizeof(struct SecurityStruct));
#endif
}


/********************************************************************************************\
* Load settings from SPIFFS
\*********************************************************************************************/
boolean LoadSettings()
{
#if FEATURE_SPIFFS
  LoadFromFile((char*)"config.txt", 0, (byte*)&Settings, sizeof(struct SettingsStruct));
  LoadFromFile((char*)"security.txt", 0, (byte*)&SecuritySettings, sizeof(struct SecurityStruct));
#else
  LoadFromFlash(0, (byte*)&Settings, sizeof(struct SettingsStruct));
  LoadFromFlash(32768, (byte*)&SecuritySettings, sizeof(struct SecurityStruct));
#endif
}


/********************************************************************************************\
* Save Task settings to SPIFFS
\*********************************************************************************************/
void SaveTaskSettings(byte TaskIndex)
{
#if FEATURE_SPIFFS
  SaveToFile((char*)"config.txt", 4096 + (TaskIndex * 1024), (byte*)&ExtraTaskSettings, sizeof(struct ExtraTaskSettingsStruct));
#else
  SaveToFlash(4096 + (TaskIndex * 1024), (byte*)&ExtraTaskSettings, sizeof(struct ExtraTaskSettingsStruct));
#endif
}


/********************************************************************************************\
* Load Task settings from SPIFFS
\*********************************************************************************************/
void LoadTaskSettings(byte TaskIndex)
{
#if FEATURE_SPIFFS
  if(ExtraTaskSettings.TaskIndex == TaskIndex)
    return;
  LoadFromFile((char*)"config.txt", 4096 + (TaskIndex * 1024), (byte*)&ExtraTaskSettings, sizeof(struct ExtraTaskSettingsStruct));
  ExtraTaskSettings.TaskIndex = TaskIndex; // store active index
#else
  LoadFromFlash(4096 + (TaskIndex * 1024), (byte*)&ExtraTaskSettings, sizeof(struct ExtraTaskSettingsStruct));
#endif
}


/********************************************************************************************\
* Save Custom Task settings to SPIFFS
\*********************************************************************************************/
void SaveCustomTaskSettings(int TaskIndex, byte* memAddress, int datasize)
{
if (datasize > 512)
  return;
#if FEATURE_SPIFFS
  SaveToFile((char*)"config.txt", 4096 + (TaskIndex * 1024) + 512, memAddress, datasize);
#else
  SaveToFlash(4096 + (TaskIndex * 1024) + 512, memAddress, datasize);
#endif
}


/********************************************************************************************\
* Save Custom Task settings to SPIFFS
\*********************************************************************************************/
void LoadCustomTaskSettings(int TaskIndex, byte* memAddress, int datasize)
{
if (datasize > 512)
  return;
#if FEATURE_SPIFFS
  LoadFromFile((char*)"config.txt", 4096 + (TaskIndex * 1024) + 512, memAddress, datasize);
#else
  LoadFromFlash(4096 + (TaskIndex * 1024) + 512, memAddress, datasize);
#endif
}


#if FEATURE_SPIFFS
/********************************************************************************************\
* Save data into config file on SPIFFS
\*********************************************************************************************/
void SaveToFile(char* fname, int index, byte* memAddress, int datasize)
{
  File f = SPIFFS.open(fname, "r+");
  if (f)
  {
    f.seek(index, SeekSet);
    byte *pointerToByteToSave = memAddress;
    for (int x = 0; x < datasize ; x++)
    {
      f.write(*pointerToByteToSave);
      pointerToByteToSave++;
    }
    f.close();
    String log = F("FILE : File saved");
    addLog(LOG_LEVEL_INFO,log);
  }
}


/********************************************************************************************\
* Load data from config file on SPIFFS
\*********************************************************************************************/
void LoadFromFile(char* fname, int index, byte* memAddress, int datasize)
{
  File f = SPIFFS.open(fname, "r+");
  if (f)
  {
    f.seek(index, SeekSet);
    byte *pointerToByteToRead = memAddress;
    for (int x = 0; x < datasize; x++)
    {
      *pointerToByteToRead = f.read();
      pointerToByteToRead++;// next byte
    }
    f.close();
  }
}
#endif


/********************************************************************************************\
* Save data to flash
\*********************************************************************************************/
#define FLASH_EEPROM_SIZE 4096
extern "C" {
#include "spi_flash.h"
}
extern "C" uint32_t _SPIFFS_start;
extern "C" uint32_t _SPIFFS_end;
extern "C" uint32_t _SPIFFS_page;
extern "C" uint32_t _SPIFFS_block;

void SaveToFlash(int index, byte* memAddress, int datasize)
{
  if (index > 33791) // Limit usable flash area to 32+1k size
    {
      return;
    }
  uint32_t _sector = ((uint32_t)&_SPIFFS_start - 0x40200000) / SPI_FLASH_SEC_SIZE;
  uint8_t* data = new uint8_t[FLASH_EEPROM_SIZE];
  int sectorOffset = index / SPI_FLASH_SEC_SIZE;
  int sectorIndex = index % SPI_FLASH_SEC_SIZE;
  uint8_t* dataIndex = data + sectorIndex;
  _sector += sectorOffset;

  // load entire sector from flash into memory
  noInterrupts();
  spi_flash_read(_sector * SPI_FLASH_SEC_SIZE, reinterpret_cast<uint32_t*>(data), FLASH_EEPROM_SIZE);
  interrupts();

  // store struct into this block
  memcpy(dataIndex,memAddress,datasize);

  noInterrupts();
  // write sector back to flash
  if(spi_flash_erase_sector(_sector) == SPI_FLASH_RESULT_OK)
    if(spi_flash_write(_sector * SPI_FLASH_SEC_SIZE, reinterpret_cast<uint32_t*>(data), FLASH_EEPROM_SIZE) == SPI_FLASH_RESULT_OK)
      {
        //Serial.println("flash save ok");
      }
  interrupts();
  delete [] data;
  String log = F("FLASH: Settings saved");
  addLog(LOG_LEVEL_INFO,log);
}


/********************************************************************************************\
* Load data from flash
\*********************************************************************************************/
void LoadFromFlash(int index, byte* memAddress, int datasize)
{
  uint32_t _sector = ((uint32_t)&_SPIFFS_start - 0x40200000) / SPI_FLASH_SEC_SIZE;
  uint8_t* data = new uint8_t[FLASH_EEPROM_SIZE];
  int sectorOffset = index / SPI_FLASH_SEC_SIZE;
  int sectorIndex = index % SPI_FLASH_SEC_SIZE;
  uint8_t* dataIndex = data + sectorIndex;
  _sector += sectorOffset;

  // load entire sector from flash into memory
  noInterrupts();
  spi_flash_read(_sector * SPI_FLASH_SEC_SIZE, reinterpret_cast<uint32_t*>(data), FLASH_EEPROM_SIZE);
  interrupts();

  // load struct from this block
  memcpy(memAddress,dataIndex,datasize);
  delete [] data;
}


/********************************************************************************************\
* Erase data on flash
\*********************************************************************************************/
void EraseFlash()
{
  uint32_t _sectorStart = ((uint32_t)&_SPIFFS_start - 0x40200000) / SPI_FLASH_SEC_SIZE;
  uint32_t _sectorEnd = _sectorStart + 32+1; //((uint32_t)&_SPIFFS_end - 0x40200000) / SPI_FLASH_SEC_SIZE;
  uint8_t* data = new uint8_t[FLASH_EEPROM_SIZE];

  uint8_t* tmpdata = data;
  for (int x=0; x < FLASH_EEPROM_SIZE; x++)
  {
    *tmpdata = 0;
    tmpdata++;
  }

  noInterrupts();
  for (uint32_t _sector=_sectorStart; _sector < _sectorEnd; _sector++)
  {
  // write sector to flash
  if(spi_flash_erase_sector(_sector) == SPI_FLASH_RESULT_OK)
    if(spi_flash_write(_sector * SPI_FLASH_SEC_SIZE, reinterpret_cast<uint32_t*>(data), FLASH_EEPROM_SIZE) == SPI_FLASH_RESULT_OK)
      {
        String log = F("FLASH: Erase Sector: ");
        log += _sector;
        addLog(LOG_LEVEL_INFO,log);
      }
  }
  interrupts();
  delete [] data;
}


/********************************************************************************************\
* Reset all settings to factory defaults
\*********************************************************************************************/
void ResetFactory(void)
{
  // Direct Serial is allowed here, since this is only an emergency task.

  byte bootCount = 0;
  if (readFromRTC(&bootCount))
  {
    Serial.print(F("RESET: Reboot count: "));
    Serial.println(bootCount);
    if (bootCount > 3)
      {
        Serial.println(F("RESET: To many reset attempts"));
        return;
      }
  }
  else
      Serial.println(F("RESET: Cold boot"));

  bootCount++;
  saveToRTC(bootCount);

  #if FEATURE_SPIFFS
  File f = SPIFFS.open("config.txt", "w");
  if (f)
  {
    for (int x = 0; x < 32768; x++)
      f.write(0);
    f.close();

  }
  f = SPIFFS.open("security.txt", "w");
  if (f)
  {
    for (int x = 0; x < 512; x++)
      f.write(0);
    f.close();
  }
  #else
    EraseFlash();
  #endif

  LoadSettings();
  // now we set all parameters that need to be non-zero as default value
  Settings.PID             = ESP_PROJECT_PID;
  Settings.Version         = VERSION;
  Settings.Unit            = UNIT;
  strcpy_P(SecuritySettings.WifiSSID, PSTR(DEFAULT_SSID));
  strcpy_P(SecuritySettings.WifiKey, PSTR(DEFAULT_KEY));
  strcpy_P(SecuritySettings.WifiAPKey, PSTR(DEFAULT_AP_KEY));
  SecuritySettings.Password[0] =0;
  str2ip((char*)DEFAULT_SERVER, Settings.Controller_IP);
  Settings.ControllerPort      = DEFAULT_PORT;
  Settings.Delay           = DEFAULT_DELAY;
  Settings.Pin_i2c_sda     = 4;
  Settings.Pin_i2c_scl     = 5;
  Settings.Protocol        = DEFAULT_PROTOCOL;
  strcpy_P(Settings.Name, PSTR(DEFAULT_NAME));
  Settings.SerialLogLevel  = 2;
  Settings.WebLogLevel     = 2;
  Settings.BaudRate        = 115200;
  Settings.MessageDelay = 1000;
  Settings.deepSleep = false;
  Settings.CustomCSS = false;
  for (byte x = 0; x < TASKS_MAX; x++)
  {
    Settings.TaskDevicePin1[x] = -1;
    Settings.TaskDevicePin2[x] = -1;
    Settings.TaskDevicePin1PullUp[x] = true;
    Settings.TaskDevicePin1Inversed[x] = false;
  }
  SaveSettings();
  delay(1000);
  WifiDisconnect();
  ESP.reset();
}


/********************************************************************************************\
* If RX and TX tied together, perform emergency reset to get the system out of boot loops
\*********************************************************************************************/

void emergencyReset()
{
  // Direct Serial is allowed here, since this is only an emergency task.
  Serial.begin(115200);
  Serial.write(0xAA);
  Serial.write(0x55);
  delay(1);
  if (Serial.available() == 2)
    if (Serial.read() == 0xAA && Serial.read() == 0x55)
    {
      Serial.println(F("System will reset in 10 seconds..."));
      delay(10000);
      ResetFactory();
    }
}


/********************************************************************************************\
* Get free system mem
\*********************************************************************************************/
extern "C" {
#include "user_interface.h"
}

unsigned long FreeMem(void)
{
  return system_get_free_heap_size();
}


/********************************************************************************************\
* In memory convert float to long
\*********************************************************************************************/
unsigned long float2ul(float f)
{
  unsigned long ul;
  memcpy(&ul, &f, 4);
  return ul;
}


/********************************************************************************************\
* In memory convert long to float
\*********************************************************************************************/
float ul2float(unsigned long ul)
{
  float f;
  memcpy(&f, &ul, 4);
  return f;
}


/********************************************************************************************\
* Add to log
\*********************************************************************************************/
void addLog(byte loglevel, String& string)
{
  addLog(loglevel, string.c_str());
}

void addLog(byte loglevel, const char *line)
{
  if (loglevel <= Settings.SerialLogLevel)
    Serial.println(line);

  if (loglevel <= Settings.SyslogLevel)
    syslog(line);

  if (loglevel <= Settings.WebLogLevel)
  {
    logcount++;
    if (logcount > 9)
      logcount = 0;
    Logging[logcount].timeStamp = millis();
    Logging[logcount].Message = line;
  }
}


/********************************************************************************************\
* Delayed reboot, in case of issues, do not reboot with high frequency as it might not help...
\*********************************************************************************************/
void delayedReboot(int rebootDelay)
{
  // Direct Serial is allowed here, since this is only an emergency task.
  while (rebootDelay != 0 )
  {
    Serial.print(F("Delayed Reset "));
    Serial.println(rebootDelay);
    rebootDelay--;
    delay(1000);
  }
  ESP.reset();
}


/********************************************************************************************\
* Save a byte to RTC memory
\*********************************************************************************************/
#define RTC_BASE 28 // 64
void saveToRTC(byte Par1)
{
  byte buf[3] = {0xAA, 0x55, 0};
  buf[2] = Par1;
  system_rtc_mem_write(RTC_BASE, buf, 3);
}


/********************************************************************************************\
* Read a byte from RTC memory
\*********************************************************************************************/
boolean readFromRTC(byte* data)
{
  byte buf[3] = {0, 0, 0};
  system_rtc_mem_read(RTC_BASE, buf, 3);
  if (buf[0] == 0xAA && buf[1] == 0x55)
  {
    *data = buf[2];
    return true;
  }
  return false;
}


/********************************************************************************************\
* Calculate function for simple expressions
\*********************************************************************************************/
#define CALCULATE_OK                            0
#define CALCULATE_ERROR_STACK_OVERFLOW          1
#define CALCULATE_ERROR_BAD_OPERATOR            2
#define CALCULATE_ERROR_PARENTHESES_MISMATCHED  3
#define CALCULATE_ERROR_UNKNOWN_TOKEN           4
#define STACK_SIZE 10 // was 50
#define TOKEN_MAX 20

float globalstack[STACK_SIZE];
float *sp = globalstack - 1;
float *sp_max = &globalstack[STACK_SIZE - 1];

#define is_operator(c)  (c == '+' || c == '-' || c == '*' || c == '/' )

int push(float value)
{
  if (sp != sp_max) // Full
  {
    *(++sp) = value;
    return 0;
  }
  else
    return CALCULATE_ERROR_STACK_OVERFLOW;
}

float pop()
{
  if (sp != (globalstack - 1)) // empty
    return *(sp--);
}

float apply_operator(char op, float first, float second)
{
  switch (op)
  {
    case '+':
      return first + second;
    case '-':
      return first - second;
    case '*':
      return first * second;
    case '/':
      return first / second;
      return 0;
  }
}

char *next_token(char *linep)
{
  while (isspace(*(linep++)));
  while (*linep && !isspace(*(linep++)));
  return linep;
}

int RPNCalculate(char* token)
{
  if (token[0] == 0)
    return 0; // geen moeite doen voor een lege string

  if (is_operator(token[0]))
  {
    float second = pop();
    float first = pop();

    if (push(apply_operator(token[0], first, second)))
      return CALCULATE_ERROR_STACK_OVERFLOW;
  }
  else // Als er nog een is, dan deze ophalen
    if (push(atof(token))) // is het een waarde, dan op de stack plaatsen
      return CALCULATE_ERROR_STACK_OVERFLOW;

  return 0;
}

// operators
// precedence   operators         associativity
// 3            !                 right to left
// 2            * / %             left to right
// 1            + - ^             left to right
int op_preced(const char c)
{
  switch (c)
  {
    case '*':
    case '/':
      return 2;
    case '+':
    case '-':
      return 1;
  }
  return 0;
}

bool op_left_assoc(const char c)
{
  switch (c)
  {
    case '*':
    case '/':
    case '+':
    case '-':
      return true;     // left to right
      //case '!': return false;    // right to left
  }
  return false;
}

unsigned int op_arg_count(const char c)
{
  switch (c)
  {
    case '*':
    case '/':
    case '+':
    case '-':
      return 2;
      //case '!': return 1;
  }
  return 0;
}


int Calculate(const char *input, float* result)
{
  const char *strpos = input, *strend = input + strlen(input);
  char token[25];
  char c, *TokenPos = token;
  char stack[32];       // operator stack
  unsigned int sl = 0;  // stack length
  char     sc;          // used for record stack element
  int error = 0;

  //*sp=0; // bug, it stops calculating after 50 times
  sp = globalstack - 1;

  while (strpos < strend)
  {
    // read one token from the input stream
    c = *strpos;
    if (c != ' ')
    {
      // If the token is a number (identifier), then add it to the token queue.
      if ((c >= '0' && c <= '9') || c == '.')
      {
        *TokenPos = c;
        ++TokenPos;
      }

      // If the token is an operator, op1, then:
      else if (is_operator(c))
      {
        *(TokenPos) = 0;
        error = RPNCalculate(token);
        TokenPos = token;
        if (error)return error;
        while (sl > 0)
        {
          sc = stack[sl - 1];
          // While there is an operator token, op2, at the top of the stack
          // op1 is left-associative and its precedence is less than or equal to that of op2,
          // or op1 has precedence less than that of op2,
          // The differing operator priority decides pop / push
          // If 2 operators have equal priority then associativity decides.
          if (is_operator(sc) && ((op_left_assoc(c) && (op_preced(c) <= op_preced(sc))) || (op_preced(c) < op_preced(sc))))
          {
            // Pop op2 off the stack, onto the token queue;
            *TokenPos = sc;
            ++TokenPos;
            *(TokenPos) = 0;
            error = RPNCalculate(token);
            TokenPos = token;
            if (error)return error;
            sl--;
          }
          else
            break;
        }
        // push op1 onto the stack.
        stack[sl] = c;
        ++sl;
      }
      // If the token is a left parenthesis, then push it onto the stack.
      else if (c == '(')
      {
        stack[sl] = c;
        ++sl;
      }
      // If the token is a right parenthesis:
      else if (c == ')')
      {
        bool pe = false;
        // Until the token at the top of the stack is a left parenthesis,
        // pop operators off the stack onto the token queue
        while (sl > 0)
        {
          *(TokenPos) = 0;
          error = RPNCalculate(token);
          TokenPos = token;
          if (error)return error;
          sc = stack[sl - 1];
          if (sc == '(')
          {
            pe = true;
            break;
          }
          else
          {
            *TokenPos = sc;
            ++TokenPos;
            sl--;
          }
        }
        // If the stack runs out without finding a left parenthesis, then there are mismatched parentheses.
        if (!pe)
          return CALCULATE_ERROR_PARENTHESES_MISMATCHED;

        // Pop the left parenthesis from the stack, but not onto the token queue.
        sl--;

        // If the token at the top of the stack is a function token, pop it onto the token queue.
        if (sl > 0)
          sc = stack[sl - 1];

      }
      else
        return CALCULATE_ERROR_UNKNOWN_TOKEN;
    }
    ++strpos;
  }
  // When there are no more tokens to read:
  // While there are still operator tokens in the stack:
  while (sl > 0)
  {
    sc = stack[sl - 1];
    if (sc == '(' || sc == ')')
      return CALCULATE_ERROR_PARENTHESES_MISMATCHED;

    *(TokenPos) = 0;
    error = RPNCalculate(token);
    TokenPos = token;
    if (error)return error;
    *TokenPos = sc;
    ++TokenPos;
    --sl;
  }

  *(TokenPos) = 0;
  error = RPNCalculate(token);
  TokenPos = token;
  if (error)
  {
    *result = 0;
    return error;
  }
  *result = *sp;
  return CALCULATE_OK;
}

