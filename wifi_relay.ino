#include <ESP8266WiFi.h>

//#define DEBUG

#define RELAY             1 // relay connected to  GPIO1
#define AUDIO             3 // audio sensor on GPIO3
#define BLINK_DELAY       1000
#define BLINKING_DURATION 3500
#define BLINKING_INTERVAL 500

const char* ssid = "2.4G_FERNANDES"; // fill in here your router or wifi SSID
const char* password = "francisca"; // fill in here your router or wifi password

WiFiServer server(80);

static uint32_t last_blink = 0;

static bool isBlinking = false;
static uint32_t blinking_time = 0;
static bool blinking_state = false;

static char relay_value = LOW;
static char last_audio = 0;
 
void setup() {
#ifdef DEBUG
  Serial.begin(115200); // must be same baudrate with the Serial Monitor
#endif

  pinMode(RELAY, OUTPUT);
  pinMode(AUDIO, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(RELAY, LOW);

#ifdef DEBUG
  // Connect to WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
#endif
 
  WiFi.begin(ssid, password);

  IPAddress ip(192,168,0,178);   
  IPAddress gateway(192,168,0,255);   
  IPAddress subnet(255,255,255,0);   
  WiFi.config(ip, gateway, subnet);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
#ifdef DEBUG
    Serial.print(".");
#endif
  }

#ifdef DEBUG
  Serial.println("");
  Serial.println("WiFi connected");
#endif

  // Start the server
  server.begin();

#ifdef DEBUG
  Serial.println("Server started");
 
  // Print the IP address
  Serial.print("Use this URL to connect: ");
  Serial.print(WiFi.localIP());
  Serial.println("/");
#endif
}
 
void loop() {
  // Check if a client has connected
  WiFiClient client = server.available();

  char audio_state = digitalRead(AUDIO);

  if (audio_state != last_audio) {
    last_audio = audio_state;
    isBlinking = true;
  }
  
  if (!client) {
    ShowAliveBlink();
    ShowBlinking();
    return;
  }

#ifdef DEBUG
  Serial.println("new client");
#endif
  
  // Read the first line of the request
  String request = client.readStringUntil('\r');

#ifdef DEBUG
  Serial.println(request);
#endif

  client.flush();
 
  // Match the request
  if (request.indexOf("/RELAY=ON") != -1) {
#ifdef DEBUG
    Serial.println("RELAY=ON");
#endif

    digitalWrite(RELAY, HIGH);
    relay_value = HIGH;
  }
  
  if (request.indexOf("/RELAY=OFF") != -1) {
#ifdef DEBUG
    Serial.println("RELAY=OFF");
#endif

    digitalWrite(RELAY, LOW);
    relay_value = LOW;
  }
  
  PrintPage(client, relay_value);
 
  delay(1);

#ifdef DEBUG
  Serial.println("Client disonnected");
  Serial.println("");
#endif
}

void PrintPage(WiFiClient &client, char value) {
  // Return the response
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println(""); //  this is a must
  client.println("<!DOCTYPE html>");
  client.println("<html><head>");
  client.println("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">");
  client.println("<style>");
  client.println("* { box-sizing: border-box; }");
  //client.println(".main { width: 100%; float: right; padding: 20px; overflow: hidden; background-color: rgb(197, 203, 211); }");
  client.println(".main1 { background-color:#353547; color:white; padding:15px; text-align:center; }");
  client.println(".button_style { text-decoration: none; color: white; }");
  client.println(".button_red { background-color: red; color: white; padding: 1em 1.5em; margin: 1em 1.5em; text-decoration: none; }");
  client.println(".button_green { background-color: green; color: white; padding: 1em 1.5em; margin: 1em 1.5em; text-decoration: none; }");
  client.println("body { justify-content: center; align-items: center; }");
  client.println("</style><title>Smart Central Home</title></head>");
  client.println("<body style=\"font-family:'Gill Sans', 'Gill Sans MT', Calibri, 'Trebuchet MS', sans-serif;\">");
  client.println("<div style=\"background-color:#100e2c;color:white;padding:15px;text-align:center;\">");
  client.println("<h1>Smart Central Home</h1><h3>The smartest way to control your home</h3></div>");
  client.println("<div class=\"main1\"> <h2>Lights</h2>");

  if (value == LOW) {
    client.println("<p>The <b>kitchen</b> light is now <b>OFF</b></p>");
    client.println("<div class=\"button_green\"><a href=\"/RELAY=ON\" class=\"button_style\">Turn ON</a></div>");
  }
  else {
    client.println("<p>The <b>kitchen</b> light is now <b>ON</b></p>");
    client.println("<div class=\"button_red\"><a href=\"/RELAY=OFF\" class=\"button_style\">Turn OFF</a></div>");
  }
  
  client.println("</div>");
  client.println("</div></body></html>");
}

void ShowAliveBlink() {
  if (!isBlinking) {
    if (millis() - last_blink > BLINK_DELAY) {
      digitalWrite(LED_BUILTIN, LOW);
      last_blink = millis();
      delay(3);
    }
    else {
      digitalWrite(LED_BUILTIN, HIGH);
    }
  }
}

void ShowBlinking() {
  static uint32_t last_change = 0;
  
  if (isBlinking) {
    if (blinking_time == 0) {
      blinking_time = millis();
      last_change = millis();
    }

    if (millis() - blinking_time > BLINKING_DURATION) {
      isBlinking = false;
      blinking_time = 0;

      if (relay_value == HIGH) {
        digitalWrite(RELAY, HIGH);        
      } else {
        digitalWrite(RELAY, LOW);        
      }

      return;
    }

    if (millis() - last_change > BLINKING_INTERVAL) {
      blinking_state = !blinking_state;
      last_change = millis();
    }

    if (blinking_state) {
      digitalWrite(RELAY, HIGH);
      digitalWrite(LED_BUILTIN, HIGH);
    } else {
      digitalWrite(RELAY, LOW);
      digitalWrite(LED_BUILTIN, LOW);
    }
  }
}
