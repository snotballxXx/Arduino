// Present a "Will be back soon web page", as stand-in webserver.
// 2011-01-30 <jc@wippler.nl> http://opensource.org/licenses/mit-license.php
 
#include <EtherCard.h>

#define STATIC 1  // set to 1 to disable DHCP (adjust myip/gwip values below)

// Pin 9 has an LED connected on most Arduino boards.
// give it a name:
int led = 9;

#if STATIC
// ethernet interface ip address
static byte myip[] = { 192,168,1,200 };
// gateway ip address
static byte gwip[] = { 192,168,1,10 };
#endif

// ethernet mac address - must be unique on your network
static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x31,0x31 };

byte Ethernet::buffer[500]; // tcp/ip send and receive buffer

const char page[] PROGMEM =
"HTTP/1.0 503 Service Unavailable\r\n"
"Content-Type: text/html\r\n"
"Retry-After: 600\r\n"
"\r\n"
"<html>"
  "<head><title>"
    "Service Temporarily Unavailable"
  "</title></head>"
  "<body>"
    "<h3>This service is currently unavailable</h3>"
    "<p><em>"
      "The main server is currently off-line.<br />"
      "Please try again later."
    "</em></p>"
  "</body>"
"</html>"
;

void setup(){
  pinMode(led, OUTPUT);   
  Serial.begin(57600);
  Serial.println("\n[backSoon]");
  multiFlash(500, 2);
  if (ether.begin(sizeof Ethernet::buffer, mymac) == 0) 
  {
    Serial.println( "Failed to access Ethernet controller");
    multiFlash(500, 10);
  }
#if STATIC
  ether.staticSetup(myip, gwip);
#else
  if (!ether.dhcpSetup())
  {
    Serial.println("DHCP failed");

multiFlash(500, 5);
  }
  #endif

  ether.printIp("IP:  ", ether.myip);
  ether.printIp("GW:  ", ether.gwip);  
  ether.printIp("DNS: ", ether.dnsip);  
  
   multiFlash(1000, 2);
}

void multiFlash(int frequency, int times)
{
  for (int i = 0; i < times;++i)
  {
   digitalWrite(led, HIGH);   // turn the LED on (HIGH is the voltage level)
   delay(frequency);               // wait for a second
   digitalWrite(led, LOW);    // turn the LED off by making the voltage LOW
   delay(frequency);               // wait for a second
  }
}

void printBuffer(int offset, int length)
{
  Serial.println("Buffer Print");
  Serial.println("============");
  
  for (int i = 0; i < length;++i)
  {
    if ((i % 8) == 0)
      Serial.println("");
    
    if (Ethernet::buffer[offset + i] < 16)
    Serial.print(" 0x0");
    else
    Serial.print(" 0x");
    
    Serial.print(Ethernet::buffer[offset + i], HEX);
  }
  
  Serial.println("");
    Serial.println("****** Buffer Print End *******");

}

void loop(){
  // wait for an incoming TCP packet, but ignore its contents
  word length = ether.packetReceive();
  word offset = ether.packetLoop(length);
  
  if (offset) 
  {
      printBuffer(offset, length - offset);
    
    memcpy_P(ether.tcpOffset(), page, sizeof(page));
    ether.httpServerReply(sizeof(page) - 1);
  }
}
