// PJ Edits
// 26-06-15 - started with RFM_MQTT_GW_22.ino from https://github.com/computourist/RFM69-MQTT-client/ which is known as gateway 2.2
// 26-06-15 - configured for my environment
//              - configured my IP addresses
//              - set to 915MHz
//              - commented out the define for RFM69HW 
// 19-08-15 - lots of changes have been made in the prev months, but not documented at the time. 
//            Documenting them now as I try and put better comments in code to date as well.
//          - GREEN LED on Pin 'MQCON' indicates when a succesful connection to the MQTT server is established.
//          - RED LED on Pin 'R_LED' indicates radio activity.
// 22-09-15 - In order to be able to read and understand the "void mqtt_subs(char* topic, byte* payload, unsigned int length)" 
//              function, I had to indent it better etc.
// 25-09-15 - Added the Blue POWER/Startup LED
// 15-11-15 - Added in statements to allow my new Xmas Lights "deviceIDs" of 100-131 to be transmitted.
// v10 16-11-15 - Did lots of tab formatting and commenting to make code more readable
// v11 21-11-15 - Did some commenting, continued checking to see if my new DevID's of 100+ would be ok. 
//              - Updated code to accept 3 digit DevID's, i.e. "dev100"
//              - am now successfully sending values to dev100 on my XmasLights Moteino Node
// v12 29-01-15 - When I was starting to test my new Shed nodes, I noticed some of the code in this GW still responded with two digit
//                  devID's, not three digit.  So I updated the two lines that did it.  One of those two lines
//                  now looks like "sprintf(buff_topic, "home/rfm_gw/nb/node01/dev000");"
// v13 14-03-16 - When I was doing some testing and hit COMPILE on the v12 code with Arduino 1.6.8 IDE I got 
//                  "PJ_RFM_MQTT_GW_22v12:165: error: 'mqtt_subs' was not declared in this scope
//                       PubSubClient mqttClient(mqtt_server, 1883, mqtt_subs, ethClient );" so I had to create a prototype of that fn just above setup();
//              - Also added a few more PJDEBUG statements in mqtt_subs()
// v14 18-03-16 - the GW seemed to be locking up, I suspect it was the last few DEBUG statements I addewd used up too much memory.
//                  - So I have changed most of the recent DEBUGPJ's to DEBUGPJ2
// v15 03-04-16 - to try and get a handle on suspected memory issues I read https://learn.adafruit.com/memories-of-an-arduino/measuring-free-memory
//                    and added the freeRam() function to my code so I could check.
//              - broke into multiple tabs
// v16 16-04-16 - started porting to MEGA2560 board to get more mem etc. see Evernotes for details
// v17 19-05-16 - The RFM69 module seems to lock up within a few mins of powerup, doing some debugging to find out why.
// v18 28-05-16 - I experimented with connecting a pin from Arduino to RFM69 RESET so I could reset
//                it when the arduino did a soft reset (USB/Serial monitor triggered) my code would reset the RFM.
//                I never quite got it to work but then did not need it once I fixed w5100.h (see my notes in
//                Evernote)
// v19 04-08-16 - Now that I am working on dev4xx devices on my Ocean Mirror Node, I need to ensure that they are passed
//                in both directions by this RFM GW. They are FLOAT/REALS.
//                Quite alot of mods to mqtt_subs() and process_packet().
//                Tested it and it looks like I can now READ or WRITE to the new dev40x floats.
// v20 05-08-16 - As per the fix I did in v18, because I had recently updated the Arduino IDE on my PC, the w5100.h file that I had
//                previously customised (see my Evernotes) had been overwritten with the default un customised version. As a result
//                my GW started locking up again.  So I fixed my w5100.h again and all was well.
// v21 04-09-16 - I noticed that for some reason this gw code in functions processPacket() and mqtt_subs() seemed to have dev000 (uptime) and dev003 (version string)
//                as a RealMes!!! Dev000 (uptime) is an Integer and Dev003 (vesrion string) is a SringMes in all node code, so changed them here.
// v22 08/09-09-16 - Moved a bunch of Println statements out of DEBUG defs so they always run, tidied them up and made a number of them more descriptive.
//                   I did this while I was trying to troubleshoot what still feels like an unstable GW.
//                   I also changed the code in setup() so that if an mqtt connection could not be established at startup, it would not block there. It 
//                   would move on and the existing code that tries to reconnect later takes care of things.
// v23 12/09/16   This GW was unreliable at boot up, the give away was no lights ever came on on the Ethernet jack. None. If it booted up like that
//                it never managed to connect to mqtt broker. What I did find out was that it worked fine everytime when the GW was only plugged into
//                a PC USB port, NOT a USB charger, a PC USB port!.  So if it was plugged into power via the Vin DC Jack or USB to a USB charger, then
//                the Eth issue occured. It sounded like a reset issue, I suspected that the 16U2 on the Mega somehow did a better job of reseting the 
//                whole stack of boards than what occured when power was just applied. Anyway did some digging, turns out that newer Arduino Ethernet
//                Shields have a reset controller on them that provides the fickle reset timing the WizNet chip needs. Aparently its particularly
//                an issue on Megas.  Great thread here http://forum.arduino.cc/index.php?topic=102045.0 got me going in the right direction, and then by
//                looking at Ethernet Shield scematics I found the reset controller chip and then when I looked at my el cheapo "Mega compatible" Ethernet
//                Shields, you could see the pads, but no chip. Unreal!!!!!!! I check all of mine and all same, no reset controller chip.
//                So then in other reading I saw people were trying the Eth Shield reset to GPIO on the Mega and toggling it in thier setup() code.
//                Its also shown as an option on the diag at old.wiznet.co.kr/include_Files/Just_Download.asp?PK_NUM=101&file_local_path=ReferenceFiles&file_local_name=W5100_AN_SPI.pdf
//                So I soldered on a lead to the right place on the reset button on the dodgy Eth Shield and plugged it into D40 on the Mega when I had
//                assembled everything again.  Now in this ver of code I will toggle it in setup().
//                And it worked, the reset by pin toggle in setup() means the gw works fine everytime on Vin DC jack or usb charger.
// v24 28/09/16   Added DID's 11 & 12 (for the new timestamp and filename devices I recently added to my Node code) to mqtt_subs();
// v25 08-10-16   Moved DID's 40-47 from being IntMess's to StatMess's.  I realised that is where they should be when working with PIRs in Universal Node Code today.
// v26 08/04/17   As I've started using new DEVices in the 2xx range, I have had to modify mqtt_subs() and process_packet() to pass them in both dirs.
//                  - Code compiled and uploaded ok.  I did check C:\Program Files (x86)\Arduino\libraries\Ethernet\src\utility\w5100.h and I had not
//                    made the changes I had used previously to fix interupt issue in w5100.h see my evernote - 'w5100.h modifications for Wiznet Ethernet and RFM69 to co exist'

// RFM69 MQTT gateway sketch
//
// This gateway relays messages between a MQTT-broker and several wireless nodes and will:
// - receive sensor data from several nodes periodically and on-demand
// - send/receive commands from the broker to control actuators and node parameters
//
//	Connection to the MQTT broker is over a fixed ethernet connection:
//
//		The MQTT topic is /home/rfm_gw/direction/nodeid/devid
//		where direction is: southbound (sb) towards the remote node and northbound (nb) towards MQTT broker
//
//	Connection to the nodes is over a closed radio network:
//
//		RFM Message format is: nodeID/deviceID/command/integer/float/string
//		where Command = 1 for a read request and 0 for a write request
//
//	Current defined gateway devices are:
//	0	uptime:			gateway uptime in minutes 
//	3	Version:		read version gateway software
//	
//	Reserved ranges for node devices, as implemented in the gateway are:
//	0  - 15				Node system devices
//	16 - 31				Binary output (LED, relay)
//	32 - 39				Integer output (pwm, dimmer)
//	40 - 47				Binary input (button, switch, PIR-sensor)
//	48 - 63				Real input (temperature, humidity)
//	64 - 71				Integer input (light intensity)
//
//	72	string:			transparant string transport
//
//	73 - 89		Special devices not implemented in gateway (yet)
//
//	Currently defined error messages are:
//	90	error:			Tx only: error message if no wireless connection
//	91	error:			Tx only: syntax error
//	92	error:			Tx only: invalid device type
//  93  unused error codes
//	99	wakeup:			Tx only: sends a message on node startup
//
//  XX  PJ additional modes see my EverNote titled "RFM Message Structure & Device Functions - PJ Notes"
//
//
//	==> Note: 
//		- Interrupts are disabled during ethernet transactions in w5100.h (ethernet library)
//		  (See http://harizanov.com/2012/04/rfm12b-and-arduino-ethernet-with-wiznet5100-chip/)
//		- Ethernet card and RFM68 board default use the same Slave Select pin (10) on the SPI bus;
//		  To avoid conflict the RFM module is controlled by another SS pin (8).
//
//
// RFM69 Library by Felix Rusu - felix@lowpowerlab.com
// Get the RFM69 library at: https://github.com/LowPowerLab/s
//
// version 1.8 - by Computourist@gmail.com december 2014
// version 1.9 - fixed resubscription after network outage  Jan 2015
// version 2.0 - increased payload size; standard device types; trim float values; uptime & version function gateway;	Jan 2015
// version 2.1 - implemented string device 72; devices 40-48 handled uniformly		Feb 2015
// version 2.2 - changed handling of binary inputs to accomodate Openhab: message for ON and OFF on statechange; 
//			   - RSSI value changed to reception strength in the gateway giving a more accurate and uptodate value ; March 2015
//	


#include <RFM69.h>
#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>


//#define DEBUG					// uncomment for debugging
#define DEBUGPJ         // uncomment for my selective debugging rather than all.
#define DEBUGPJ2         // allows me to turn on just afew of my debugs...keeps memory used down.

#ifdef DEBUG
  #define DEBUGPJ // if DEBUG thern ensure DEBUGPJ is enabled as it includes things DEBUG needs.
#endif

#define VERSION "GWV2.2PJv26"

// Ethernet settings
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xBA, 0xBE };	// MAC address for ethernet
byte mqtt_server[] = { 192, 168, 200, 241};		// MQTT broker address
byte ip[] = { 192, 168, 200 , 242 };			// Gateway address (if DHCP fails)


// Wireless settings
#define NODEID 1				// unique node ID in the closed radio network; gateway is 1
//PJ WAS HERE - RFM_SS is very specific per hardware platform.
//              I changed RFM_SS to 10 when testing some of this on a Moteino
//              I used pin 8 on a real Uno (native SPI SS pin on this board is 10 but Eth shield hardwired to it
//                                          so as per forums etc, go with pin 8)
//              I used pin 53 for Mega2560 (native SPI SS pin on this board...see mega pinout diag)
//                  Note: Mega has hw SPI port on 50,51,52,53...so need to make sure all wiring to the RFM69 is correct.
#define RFM_SS 53				// Slave Select RFM69 is connected to.  See comments above, as this varies.
#define NETWORKID 100				// closed radio network ID

//Match frequency to the hardware version of the radio (uncomment one):
//#define FREQUENCY RF69_433MHZ
//define FREQUENCY RF69_868MHZ
#define FREQUENCY RF69_915MHZ

#define ENCRYPTKEY "xxxxxxxxxxxxxxxx" 		// shared 16-char encryption key is equal on Gateway and nodes
//define IS_RFM69HW 				// uncomment only for RFM69HW! Leave out if you have RFM69W!
#define ACK_TIME 50 				// max # of ms to wait for an ack

// PIN settings
#define MQCON 7					// MQTT Connection indicator
#define R_LED 9					// Radio activity indicator
#define P_LED A5        // Power/Startup LED

#define PJETHRESET 40   // see notes above re v23 12-09-16.

#define SERIAL_BAUD 115200

typedef struct {				// Radio packet structure max 66 bytes (only transmitted between RF GW <> Nodes, not over IP)
  int		  nodeID;				// node identifier (PJ - represents the FROM Node, not the TO Node.) 
  int		  devID;				// device identifier 0 is node; 31 is temperature, 32 is humidity
  int		  cmd;				  // read or write
  long		intVal;				// integer payload
  float		fltVal;				// floating payload
  char		payLoad[32];	// char array payload
  } Message;

Message mes;

#ifdef DEBUGPJ
  bool	act1Stat = false;			  // remember LED state in debug mode
  bool	msgToSend = false;			// message request by debug action
  int	curstat = 0;				      // current polling interval in debug mode
  int	stat[] = {0,10,30,120};		// status 0 means no polling, 1, 6, 20 seconds interval
#endif							

int	  dest;				      // destination node for radio packet, when we send one.
int   DID;              // Device ID
int 	error;					  // Syntax error code
long	lastMinute = -1;	// timestamp last minute
long	upTime = 0;				// uptime in minutes
bool	Rstat = false;		// radio indicator flag
bool	mqttCon = false;	// MQTT broker connection flag
bool	respNeeded = false;			  // MQTT message flag in case of radio connection failure
bool	mqttToSend = false;			  // Flag that is set when we have a southbound RF message to send out to remote Node during main loop() run through.
bool	promiscuousMode = false;	// only receive closed network nodes
bool	verbose = true;				    // generate error messages
bool	IntMess, RealMess, StatMess, StrMess;	// types of messages
long	onMillis;				// timestamp when radio LED was turned on. Used to keep track of when to turn it off.
char	*subTopic = "home/rfm_gw/sb/#";		// MQTT subscription topic ; direction is southbound
char	*clientName = "RFM_gateway";		  // MQTT system name of gateway
char	buff_topic[30];				// MQTT publish topic string
char	buff_mess[32];				// MQTT publish message string

// Add prototypes as what compiled ok before now seems to want prototypes
void mqtt_subs(char* topic, byte* payload, unsigned int length);
void sendMsg(int target);
void processPacket();

RFM69 radio;
EthernetClient ethClient;
PubSubClient mqttClient(mqtt_server, 1883, mqtt_subs, ethClient );






