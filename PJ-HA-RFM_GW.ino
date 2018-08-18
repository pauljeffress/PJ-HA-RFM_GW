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
// v21 04-09-16 - I noticed that for some reason this gw code in functions processRfPacket() and mqtt_subs() seemed to have dev000 (uptime) and dev003 (version string)
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
// v27 Oct 2017   When I expanded my Node Code so I could run it on SAMD21 boards like Adafruit Feather M0
//                I had to make a few changes here to the GW code too.
// v GitHub 17-10-2017 I migrated the v27 code as is to GitHub before I started to expand it to run on a SAMD21 too.

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

#include "PJ-HA-RFM_GW.h" // My global defines and extern variables to help multi file comilation.


//Note: The below function is here as it was the only way I could make the whole sketch compile!
//
//==============  MQTT_SUBS
//
//    receives Southbound MQTT/IP messages from subscribed topics
//    parses MQTT topic / message and constructs southbound RF message in "mes" struct.
//
//    The values in the MQTT topic/message are converted to corresponding values on the Radio network
//
void mqtt_subs(char* topic, byte* payload, unsigned int length) 
{ 
  // if we are here it's because the PubSub library received a 'topic' & 'payload' we are subscribed to and called this function for us.
  // so we have to check it and then do what needs to be done with it.

  // get payload into a format we can handle...
  payload[length] = '\0';                     // terminate the payload string we received with '0'
  String strPayload = String((char*)payload); // convert the payload to string, as it was a ptr to some bytes when we received it.


  #ifdef DEBUGPJ2
    Serial.println();
    Serial.println("<<<< IP/MQTT message received so starting mqtt_subs()"); 
    Serial.println("+++++++++++++++++++++++++++++++++++++++++++++++++++++");
    Serial.print("+   Topic:"); Serial.println(topic);
    Serial.print("+ Payload:"); Serial.println(strPayload);   
    Serial.println("+++++++++++++++++++++++++++++++++++++++++++++++++++++");
  #endif
  int i;
  String errStr = "";       // error string to be published back to MQTT broker, when a received IP/MQTT message has an issue.
  mes.nodeID = NODEID;      // start to build an new RF message to eventually send south. The device (the Gateway) nodeID will always be 1.
  mes.fltintVal = 0;        // zero out some of the message parameters
  mes.intVal = 0;
  mes.cmd = 0;
  rfToSend = false;       // not a valid request yet...
  error = 4;                // assume an error in processing the IP/MQTT data, until proven otherwise
  dest = 999;               // 999 is just a number that makes it easy to debug/see if no dest node was found and overwritten into this variable.


  if (strlen(topic) == 28)        // is the 'topic' we received the correct length ?
    {                             // originally was 27, now 28 as I have allowed for devID's > 99 hence three digit.
    // yes it is the correct length so lets process it...
    dest = (topic[19]-'0')*10 + topic[20]-'0';  // extract target node ID from MQTT topic and store it in global 'dest' for use later
    DID = (topic[25]-'0')*100 + (topic[26]-'0')*10 + topic[27]-'0';   // extract device ID from MQTT topic and stor it in global 'DID" for use later
    // xxxx - I really should be checking that 'dest and 'DID' are within acceptable ranges here and erroring out if not.
    // use what we have extracted to continue building our RF message
    mes.devID = DID;
    mes.cmd = 0;          // default command is '0/SET/WRITE' value, it gets changed to 1/GET/READ below if nesc.
    
    if (strPayload == "READ") mes.cmd = 1;    // if the payload was 'READ' then change the command to 1 (i.e. GET/READ)
    
    if (length == 0) // no payload sent in the southbound MQTT msg...thats not valid so error out.
      {
      error = 2;
      errStr = "Payload len=0";
      }                   
    else  // If there was a payload then classify what it should be (Int, Real, String etc) based on the DevID.
          // Only one of the below four lines should match.
      {
      #ifdef DEBUGPJ
        Serial.println("Classifying Payload");
      #endif
      // reset flags before setting them...
      StatMess = false;
      RealIntMess = false;
      IntMess = false;
      StrMess = false;
      
      StatMess = ( DID==5 || DID==6 || DID==8 || (DID>=16 && DID<=31) || (DID>=40 && DID <47)); // Set global 'StatMess' to true based on devID.
                                                                                                //    These special Integer DIDs can only be ON/OFF 
                                                                                                //    not any int value.
                                                                                                
      RealIntMess = ( DID==2 || DID==3 || DID==4 || (DID>=48 && DID<=63) || (DID>=400 && DID<=499));  // set global 'RealMess' to true based on devID.
                                                                                                
                                                                                                
      IntMess = (DID==0 || DID==1 || DID==7 || (DID >=32 && DID <=39) || (DID>=64 && DID<=71) || (DID>=100 && DID<=116) || (DID>=201 && DID<=299)  ); // set global 'IntMess' to true based on devID.

      StrMess = (DID==3 || DID==72 || DID==11 || DID==12);                                     // set global 'StrMess' to true based on devID.

      if (!(StatMess || RealIntMess || IntMess || StrMess)) // i.e we have a message that did not fit any of the above categories - and thats bad!
        {
          error = 5;
          errStr = "No Categ Match";
        }

      // Now that we have pulled the message apart, checked and classified it, lets action it...

      // Check if message is for Gateway itself and is it an Uptime request?
      if (dest == NODEID && DID == 0)          // gateway uptime wanted
        {                                                   // so construct MQTT topic and message/payload
        sprintf(buff_mess,  "%d", upTime);                  //    copy upTime into the buff_Mess (i.e. payload we will send north)
        sprintf(buff_topic, "home/sam_gw/nb/node%02d/dev000", NODEID);  //    copy the correct topic to publish this info out on into buff_topic 
        
        #ifdef DEBUGPJ2
        Serial.println();
        Serial.println("IP/MQTT message sending >>>>"); 
        Serial.println("+++++++++++++++++++++++++++++++++++++++++++++++++++++");
        Serial.print("+   Topic:"); Serial.println(buff_topic);
        Serial.print("+ Payload:"); Serial.println(buff_mess);   
        Serial.println("+++++++++++++++++++++++++++++++++++++++++++++++++++++");
        #endif
        
        mqttClient.publish(buff_topic,buff_mess);           // MQTT publish the topic & payload
        error =0;
        }

      // Check if message is for Gateway itself and is it a Version request?
      if (dest == NODEID && DID == 3)          // gateway version wanted
        {                                                   // so construct MQTT topic and message/payload
        for (i=0; i<sizeof(VERSION); i++){                  //    copy the VERSION string into the buff_Mess (i.e. payload we will send north)
          buff_mess[i] = (VERSION[i]); }
        mes.payLoad[i] = '\0';   // xxxx - check on this, looks odd???
        sprintf(buff_topic, "home/sam_gw/nb/node%02d/dev003", NODEID);  //    copy the correct topic to publish this info out on into buff_topic 
        
        #ifdef DEBUGPJ2
        Serial.println();
        Serial.println("IP/MQTT message sending >>>>"); 
        Serial.println("+++++++++++++++++++++++++++++++++++++++++++++++++++++");
        Serial.print("+   Topic:"); Serial.println(buff_topic);
        Serial.print("+ Payload:"); Serial.println(buff_mess);   
        Serial.println("+++++++++++++++++++++++++++++++++++++++++++++++++++++");
        #endif
        
        mqttClient.publish(buff_topic,buff_mess);           // MQTT publish the topic & payload
        error = 0;
        }

      // Check if message is for an RF Node (not this GW) and its a StatMess. 
      if (dest != NODEID && StatMess)              
        {
        #ifdef DEBUGPJ
          Serial.println("StatMess");
        #endif
        rfToSend = true;                          // flag that there is an RF message to send south. It will go when main loop() checks this flag.
        if (strPayload == "ON") mes.intVal = 1;     // convert MQTT payload string (ON>1, OFF>0) or leave it as READ in payload string 
        else if (strPayload == "OFF") mes.intVal = 0;
        else if (strPayload != "READ") // invalid payload; do not process
                { 
                rfToSend = false;
                error = 3;
                errStr = "invalid payload";
                }
        }

      // Message is for an RF Node and its a StatMess.
      if (dest != NODEID && (DID >=40 && DID <48))    // check if someone is trying to ON/OFF a DevID you can only READ from. i.e. an input.
                                              // I would have though this IF should also be confirming its a StatMess?????
        {
        #ifdef DEBUGPJ
          Serial.println("StatMess40-47");
        #endif
        if (strPayload == "READ") rfToSend = true; // flag that there is an RF message to send south. It will go when main loop() checks this flag.
        else // invalid payload; do not process
           { 
           rfToSend = false;
           error = 3;
           errStr = "invalid payload";
           }
        }
        
      // Message is for an RF Node and its a RealMess.
      if ( dest != NODEID && RealIntMess )          // It's a Real Message. Could be a READ or a WRITE.
        {
        if (mes.cmd == 0) // i.e its a WRITE
          {
          mes.fltintVal = strPayload.toFloat() * 100;   // If its a SET/WRITE then take the MQTT payload (which contains the 
                                                        // Float value to be written to RF Node DEVice) and convert it to 
                                                        // an Integer by multiplying it by 100 (assuming only two dec place)
                                                        // and then place it in the fltintVal ready for RF TX.
                                                        // Note: If its a GET/READ, we don't have to set anything else in the mess struct 
                                                        // because the mes.cmd will be interpreted at the RF end Node, and a READ will be performed.  
          }
          #ifdef DEBUGPJ
            Serial.println("RealIntMess");
          #endif
          rfToSend = true;  // flag that there is an RF message to send south. It will go when main loop() checks this flag.
        }

      // Message is for an RF Node and its an IntMess.
      if ( dest != NODEID && IntMess )          // It's an Integer Message. Could be a READ or a WRITE.
        {
        if (mes.cmd == 0) // i.e its a WRITE
          {
          mes.intVal = strPayload.toInt();  // If its a SET/WRITE then copy the MQTT payload across to the RF IntegerVal
                                            // Note: If its a GET/READ, we don't have to set anything else in the mess struct 
                                            // because the mes.cmd will be interpreted at the RF end Node, and a READ will be performed.  
          }
          #ifdef DEBUGPJ
            Serial.println("IntMess");
          #endif
          rfToSend = true;  // flag that there is an RF message to send south. It will go when main loop() checks this flag.
        }


      // Message is for an RF Node and its a StrMess.
      if ( dest != NODEID && StrMess )          // It's an String Message. Could be a READ or a WRITE.
        {
        if (mes.cmd == 0)               //    If its a SET/WRITE then copy the MQTT payload across to the RF Payload (there is no stringVal)
          {
          int i; 
          for (i=0; i<STRPAYLOADSIZE; i++)
            { 
            (mes.payLoad[i])=payload[i];
            }
          }
        rfToSend = true;              // flag that there is an RF message to send south. It will go when main loop() checks this flag.
        }

      // Check if any errors were found during the above processing, if not, set error to 0.
      if (rfToSend && (error == 4)) error = 0;    // valid device has been selected, hence error = 0
                                                    // Bit of an odd way to confirm that we can clear the error flag of its default 4 that it was set to at beginning of this function.
  
      //respNeeded = rfToSend;                      // valid request needs radio response
      

      } // end of the 'else' that got run if payload len != 0
    }  // end of the 'if' that checks topic is correct length
  else    // i.e The 'topic' we received was not of correct length
    {
    error = 1;
    errStr = "MQTT topic len bad";
    }

  // if we have an error, send an MQTT north with devID = 91 from the GW, with error details in the MQTT payload. Publish it.
  if ((error != 0) && verbose)          
    {
    sprintf(buff_mess, "err%d node%d %s", error,dest, errStr.c_str());  
    sprintf(buff_topic, "home/sam_gw/nb/node%02d/dev91", NODEID);             // construct MQTT topic and message
    mqttClient.publish(buff_topic,buff_mess);                       // publish ...
    #ifdef DEBUGPJ
      Serial.print("Syntax error code is: ");
      Serial.print(error);
      Serial.print("  Error description string is: ");
      Serial.println(errStr);
    #endif
  }

  #ifdef DEBUGx
    Serial.println("End mqtt_subs()"); 
  #endif
 
} // end mqttSubs()



//-------------------------------------------------------------------------
// GENERAL STARTUP VARIABLES & DEFAULTS 
//-------------------------------------------------------------------------
// Ethernet settings
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0x01, 0x01 };	// My MAC address for ethernet match it to NodeID to ease troubleshooting.
byte mqtt_server[] = { 192, 168, 200, 241};		        // MQTT broker address
byte ip[] = { 192, 168, 200 , 248 };			            // My IP address

// Others
Message mes;
int	  dest;				      // destination node for radio packet, when we send one.
int   DID;              // Device ID
int 	error;					  // Syntax error code
long	lastMinute = -1;	// timestamp last minute
long	upTime = 0;				// uptime in minutes
bool	Rstat = false;		// radio indicator flag
bool	mqttCon = false;	// MQTT broker connection flag
//bool	respNeeded = false;			  // MQTT message flag in case of radio connection failure
bool	rfToSend = false;			  // Flag that is set when we have a southbound RF message to send out to remote Node during main loop() run through.
bool	promiscuousMode = false;	// only receive closed network nodes
bool	verbose = true;				    // generate error messages
bool	IntMess, RealIntMess, StatMess, StrMess;	// types of messages
long	onMillis;				// timestamp when radio LED was turned on. Used to keep track of when to turn it off.
char	*subTopic = "home/sam_gw/sb/#";		// MQTT subscription topic ; direction is southbound
char	*clientName = "SAMD_RFM_gateway";		  // MQTT system name of gateway
char	buff_topic[30];				// MQTT publish topic string
char	buff_mess[32];				// MQTT publish message string

uint8_t radioDataBuf[RH_MESH_MAX_MESSAGE_LEN]; //RadioHead tx/rx data


// Define our RFM69 RadioHead 'driver'.
RH_RF69 driver(PJ_RFM_SS, PJ_RFM_IRQ_PIN);  

// RadioHead 'manager' to manage message delivery and receipt, using the driver declared above
RHMesh manager(driver, CLIENT_ADDRESS);


//old pre RadioHead radio definition.
//RFM69 radio = RFM69(PJ_RFM_SS, PJ_RFM_IRQ_PIN, PJ_RFM_IS_RFM69HCW, PJ_RFM_IRQ_NUM);  // define all pins if they are not the standard ones the RFM69 lib uses
                                                                    // in this case define them for the SenseBender GW board.

EthernetClient ethClient;
PubSubClient mqttClient(mqtt_server, 1883, mqtt_subs, ethClient );


// Add prototypes as what compiled ok before now seems to want prototypes
void mqtt_subs(char* topic, byte* payload, unsigned int length);
void rfSendMsg(int target);
void processRfPacket();




