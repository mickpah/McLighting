// Neopixel
#define PIN 5         // PIN where neopixel / WS2811 strip is attached
#define NUMLEDS 24    // Number of leds in the strip


#define HOSTNAME "ESP8266_VORONOI"   // Friedly hostname

#define ENABLE_OTA    // If defined, enable Arduino OTA code.

#define ENABLE_MQTT   // If defined, enable MQTT client code.
#ifdef ENABLE_MQTT
  const char mqtt_intopic[] = "inTopic";
  const char mqtt_outtopic[] = "outTopic";
  const char mqtt_server[] = "raspberrypi2";
#endif

// ***************************************************************************
// Global variables / definitions
// ***************************************************************************
#define DBG_OUTPUT_PORT Serial  // Set debug output port

// List of all color modes
enum MODE { SET_MODE, HOLD, OFF, ALL, WIPE, RAINBOW, RAINBOWCYCLE, THEATERCHASE, THEATERCHASERAINBOW, TV };

MODE mode = RAINBOW;        // Standard mode that is active when software starts

int ws2812fx_speed = 10;    // Global variable for storing the delay between color changes --> smaller == faster
int brightness = 192;       // Global variable for storing the brightness (255 == 100%)

int ws2812fx_mode = 0;      // Helper variable to set WS2812FX modes

bool exit_func = false;     // Global helper variable to get out of the color modes when mode changes

struct ledstate             // Data structure to store a state of a single led
{
   uint8_t red;
   uint8_t green;
   uint8_t blue;
};

typedef struct ledstate LEDState;    // Define the datatype LEDState
LEDState ledstates[NUMLEDS];         // Get an array of led states to store the state of the whole strip
LEDState main_color = { 255, 0, 0 }; // Store the "main color" of the strip used in single color modes 
