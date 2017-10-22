#ifndef PJ-HA-RFM_GW_H
#define PJ-HA-RFM_GW_H

#include <arduino.h>

#include <RFM69.h>
#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>

/* DEBUG CONFIGURATION PARAMETERS */
#define DEBUG // uncomment for debugging
//#define DEBUGPJ1 // uncomment for debugging
#define DEBUGPJ2 // uncomment for debugging
#ifdef DEBUG
  #define DEBUGPJ1 // if DEBUG thern ensure DEBUGPJ1 is enabled as it includes things DEBUG needs.
  #define DEBUGPJ2 // if DEBUG thern ensure DEBUGPJ2 is enabled as it includes things DEBUG needs.
#endif

#define SERIAL_BAUD 115200

#define VERSION "SAMD-RFM_GWvGitHub"   // this value can be queried as device 3

#define STRPAYLOADSIZE 32   // How many chars in the String Payload we send?
#define RFTXRETRIES 5       // RF TX reties.

/* RFM_GW CORE CONFIGURATION PARAMETERS 
****************************************************/
// Wireless settings
#define NODEID 50				// unique node ID in the closed radio network;
//PJ WAS HERE - RFM_SS is very specific per hardware platform.
//              I changed RFM_SS to 10 when testing some of this on a Moteino
//              I used pin 8 on a real Uno (native SPI SS pin on this board is 10 but Eth shield hardwired to it
//                                          so as per forums etc, go with pin 8)
//              I used pin 53 for Mega2560 (native SPI SS pin on this board...see mega pinout diag)
//                  Note: Mega has hw SPI port on 50,51,52,53...so need to make sure all wiring to the RFM69 is correct.
#define PJ_RFM_SS      MY_RF69_SPI_CS				// Slave Select RFM69 is connected to.  See comments above, as this varies. MY_RF69_SPI_CS is a SenseBender pin name.
#define PJ_RFM_RESET   MY_RF69_RESET
#define PJ_RFM_IRQ_PIN MY_RF69_IRQ_PIN
#define PJ_RFM_IRQ_NUM MY_RF69_IRQ_NUM
#define PJ_RFM_IS_RFM69HCW false

#define NETWORKID 111				// closed radio network ID

//Match frequency to the hardware version of the radio (uncomment one):
//#define FREQUENCY RF69_433MHZ
//define FREQUENCY RF69_868MHZ
#define FREQUENCY RF69_915MHZ

#define ENCRYPTKEY "xxxxxxxxxxxxxxxx" 		// shared 16-char encryption key is equal on Gateway and nodes
//define IS_RFM69HW 				// uncomment only for RFM69HW! Leave out if you have RFM69W!
#define ACK_TIME 50 				// max # of ms to wait for an ack

// PIN settings
#define MQCON LED_GREEN					// GREEN LED - MQTT Connection indicator.  LED_GREEN is a SenseBender pin name.
#define R_LED LED_RED	    			// RED LED - Radio activity indicator
#define P_LED LED_BLUE          // BLUE LED - Power/Startup LED

// pj samd stuff     #define PJETHRESET NET_RST   // see notes above re v23 12-09-16.  NET_RST is a SenseBender pin name.

// ==============================================
// 'struct's that I had to place here not in main ino file, to assist compilation.
//
typedef struct {			// Radio packet structure max 66 bytes (only transmitted between RF GW <> Nodes, not over IP)
    long  nodeID;			// FROM node. Using a long as its 32bits on AVR and ARM. 
    long	devID;			// device identifier 0 is node; 31 is temperature, 32 is humidity
    long	cmd;			  // read or write
    long	intVal;			// integer payload
    long	fltintVal;	// floating point payload, but multiplied by 100 and transported as a long.  Works on ARM and AVR the same.
    char	payLoad[STRPAYLOADSIZE];	// char array, String payload
    } Message;

// ============================================
// Function prototypes when required;
int freeRam ();
void mqtt_subs();
void processPacket();
void sendMsg(int target);


// =============================================
// Global variables as 'externs' so individual files can compile if they use them.
extern byte mac[];
extern byte mqtt_server[];
extern byte ip[];
extern Message mes;
extern int dest;
extern int DID;
extern int 	error;
extern long	lastMinute;
extern long	upTime;
extern bool	Rstat;
extern bool	mqttCon;
extern bool	respNeeded;
extern bool	mqttToSend;
extern bool	promiscuousMode;
extern bool	verbose;
extern bool	IntMess, RealIntMess, StatMess, StrMess;
extern long	onMillis;
extern char	*subTopic;
extern char	*clientName;
extern char	buff_topic[30];
extern char	buff_mess[32];
extern RFM69 radio;
extern EthernetClient ethClient;
extern PubSubClient mqttClient;

#endif // PJ-HA-RFM_GW_H
