#include <config.h>
#include <string>
#include <TimeLib.h>
#include <WiFi.h>

// Add your WiFi information to reset the 
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

// Setup default NTP server for time synchronisation
const char* ntpServer = "pool.ntp.org";

// Initialize watch class
TTGOClass *watch;

/**
 * Define the logo.
 * To generate a bitmap logo from your PNG, use the following tool:
 * https://marlinfw.org/tools/u8glib/converter.html
 * 
 * Make sure your images are max. 128x64.
 * 
 * When you define your colors, make use of the following tool (for quick conversion):
 * https://rgbcolorpicker.com/565
*/
#define LOGO_WIDTH            (64)
#define LOGO_HEIGHT           (64)

#define BACKGROUND_COLOR      0x1a68
#define PRIMARY_COLOR         TFT_WHITE
#define SECONDARY_COLOR       0xd7e0

const unsigned char logo_bmp[] PROGMEM = {
  // Paste your marlin logo code here
};

/**
 * Resets the internal Real Time Clock module, using an NTP Server and a WiFi connection.
 * Pass `offsetHours` as a way to update your offset from GMT. Pass 2 for Central European Time.
 * Also needs `ssid` and `password` to connect to the WiFi network.
*/
struct tm* resetRTC(int offsetHours, const char* ssid, const char* password){

  Serial.println("\n\nResetting the RTC module. Please remove calling of this function on next flash, as the RTC Module will have received the most recent time.\n");

  // Setup wifi
  WiFi.begin(ssid, password);
  Serial.print("\nConnecting to WiFi network...");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }  
  
  Serial.print("Connected with IP address: ");
  Serial.print(WiFi.localIP());

  // Time check will be done, if the time is incorrect, it will be set to compile time
  configTime(0, 0, ntpServer);

  // Wait until time is retrieved by NTP server
  while (time(nullptr) < 1600000000) {
    delay(100);
  }

  // Get current time
  time_t now = time(nullptr);

  // Add custom offset
  now += offsetHours * SECS_PER_HOUR;

  struct tm* newTime = gmtime(&now);
  return newTime;
}

void setup() {

  Serial.begin(921600);

  watch = TTGOClass::getWatch();
  watch->begin();
  watch->openBL(); // Turn on backlight

  // Reset RTC once, uncomment next 2 lines when needed.
  // struct tm* newTime = resetRTC(2, ssid, password);
  // watch->rtc->setDateTime(newTime->tm_year + 1900, newTime->tm_mon + 1, newTime->tm_mday, newTime->tm_hour, newTime->tm_min, newTime->tm_sec);

  // TFT display init
  watch->tft->setTextFont(2);
  watch->tft->setTextColor(PRIMARY_COLOR);
  watch->tft->fillScreen(BACKGROUND_COLOR);

  // Bitmap logo
  watch->tft->drawBitmap(88, 40, logo_bmp, LOGO_WIDTH, LOGO_HEIGHT, PRIMARY_COLOR);

  // Custom message at the bottom
  watch->tft->drawString("Your custom message here!", 48, 200);

  // Reset for loop code
  watch->tft->setTextColor(SECONDARY_COLOR);
}

void loop() {

  // Reset from last time
  watch->tft->fillRect(52, 120, watch->tft->width(), 50, BACKGROUND_COLOR); 

  // Get telemetry
  const char* datetime = watch->rtc->formatDateTime(PCF_TIMEFORMAT_HMS);
  std::string battery = "Battery: " + std::to_string(watch->power->getBattPercentage()) + "%";

  // Draw datetime
  watch->tft->setTextFont(1);
  watch->tft->setTextSize(3);

  watch->tft->drawString(datetime, 50, 120); 

  // Draw battery info
  watch->tft->setTextFont(2);
  watch->tft->setTextSize(1);

  watch->tft->drawString(battery.c_str(), 78, 140 + watch->tft->fontHeight(1));

  // cleanup
  watch->tft->endWrite();
  watch->tft->flush();

  delay(1000);
}