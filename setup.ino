//
//==============  SETUP
//

void setup() {

  Serial.begin(SERIAL_BAUD);
  Serial.println("PJ RFM Gateway");
  Serial.print("N ");
  Serial.print(NODEID);
  Serial.print(" ");
  Serial.println(VERSION);
  
  Serial.println("RF Config");
  Serial.print("Freq ");
  Serial.print(FREQUENCY==RF69_433MHZ ? 433 : FREQUENCY==RF69_868MHZ ? 868 : 915);
  Serial.println(" Mhz");
  
  Serial.println("ETH Config");
  Serial.print("MAC ");
    for (int i = 0; i < 6; i++)
      {
      Serial.print(mac[i], HEX);
      Serial.print(":");
      }
  Serial.println();  
  Serial.print("Node IP Addr: ");
    for (int i = 0; i < 4; i++)
      {
      Serial.print(ip[i]);
      Serial.print(".");
      }
  Serial.println();  
  Serial.print("MQTT Broker: ");
    for (int i = 0; i < 4; i++)
      {
      Serial.print(mqtt_server[i]);
      Serial.print(".");
      }
  Serial.println();
  
  #ifdef DEBUGPJ2
    //Serial.begin(SERIAL_BAUD);
    Serial.println("DEBUGPJ2 ON");
    Serial.println(freeRam());
  #endif
  
  #ifdef DEBUGPJ
    //Serial.begin(SERIAL_BAUD);
    Serial.println("DEBUGPJ ON");
  #endif
  
  #ifdef DEBUG
    Serial.println("DEBUG ON");
  #endif

  // manually toggle reset on dodgy Arduino Ethernet Shield from China with no reset conrtoller. 
  // See notes in PJ_RFM_MQTT_GW_22v23.ino re v23 12-09-16.
  pinMode(PJETHRESET, OUTPUT);      // setup the pin I have jumpered to Eth Shield reset track.
  digitalWrite(PJETHRESET, LOW);    // reset the Eth Shield (active low reset)
  delay(300);                       // keep it in reset for X mSec.
  digitalWrite(PJETHRESET, HIGH);   // allow it to come out of reset
  delay(5000);                      // Give Eth Shield time to settle after reset.

  // setup LED Pins etc.
  pinMode(R_LED, OUTPUT);          // set pin of radio indicator
  pinMode(MQCON, OUTPUT);         // set pin for MQTT connection indicator
  pinMode(P_LED, OUTPUT);         // set pin for Power/Startup indicator
  digitalWrite(MQCON, LOW);         // switch off MQTT connection indicator
  digitalWrite(R_LED, LOW);       // switch off radio indicator
  digitalWrite(P_LED, HIGH);        // switch OFF Power/Startup indicator

  // test all LEDS - flash all LEDS 3 times. Then leave just PWR LED on.
  delay(1000);
  digitalWrite(MQCON, HIGH);         // switch all LEDS ON
  digitalWrite(R_LED, HIGH);       
  digitalWrite(P_LED, LOW);        
  delay(1000);
  digitalWrite(MQCON, LOW);         // switch all LEDS OFF
  digitalWrite(R_LED, LOW);       
  digitalWrite(P_LED, HIGH);        
  delay(1000);
  digitalWrite(MQCON, HIGH);         // switch all LEDS ON
  digitalWrite(R_LED, HIGH);       
  digitalWrite(P_LED, LOW);        
  delay(1000);
  digitalWrite(MQCON, LOW);         // switch all LEDS OFF
  digitalWrite(R_LED, LOW);       
  digitalWrite(P_LED, HIGH);        
  delay(1000);
  digitalWrite(MQCON, HIGH);         // switch all LEDS ON
  digitalWrite(R_LED, HIGH);       
  digitalWrite(P_LED, LOW);        
  delay(1000);
  digitalWrite(MQCON, LOW);         // switch all LEDS OFF but Power
  digitalWrite(R_LED, LOW);             
  delay(1000);

  Serial.println("LED Flashes done");
    
  radio.setCS(RFM_SS);          // change default Slave Select pin for RFM
  
  #ifdef DEBUGPJ2
    Serial.println("radio.setCS is done");
  #endif
   
  radio.initialize(FREQUENCY,NODEID,NETWORKID);   // initialise radio module

  #ifdef DEBUGPJ2
    Serial.println("radio.initialise is done");
  #endif
  
  #ifdef IS_RFM69HW
  radio.setHighPower();           // only for RFM69HW!
  #endif
  
  radio.encrypt(ENCRYPTKEY);        // encrypt with shared key
  radio.promiscuous(promiscuousMode);     // listen only to nodes in closed network

  #ifdef DEBUGPJ2
    Serial.println("radio.promiscuous is done");
  #endif
  
  digitalWrite(P_LED, HIGH);     // flash P_LED twice to say we have completed radio setup.      
  delay(1000);
  digitalWrite(P_LED, LOW);        
  delay(1000);
  digitalWrite(P_LED, HIGH);        
  delay(1000);
  digitalWrite(P_LED, LOW);        
  delay(1000);
  

  //PJ - I commented out the section a few lines below as it seemed to want to do DHCP, and only if that failed 
  // would it just static assign the IP address we defined earlier. I then added the single line below that just 
  // straight out statics the eth port to then nominated IP.
  Ethernet.begin(mac, ip); // start the Ethernet connection with static IP
  Serial.println("Ethernet.begin is done");
    
  digitalWrite(P_LED, HIGH);     // flash P_LED three times to say Ethernet setup statements done.      
  delay(1000);
  digitalWrite(P_LED, LOW);        
  delay(1000);
  digitalWrite(P_LED, HIGH);        
  delay(1000);
  digitalWrite(P_LED, LOW);        
  delay(1000);
  digitalWrite(P_LED, HIGH);        
  delay(1000);
  digitalWrite(P_LED, LOW);        
  delay(1000);
  
  mqttCon = 0;          // reset connection flag
  int numtries = 5;
  while((mqttCon != 1) && (numtries > 0))        // retry MQTT connection 'numtries' times if nescesary
      {
      Serial.println("Try mqtt connect...");
      mqttCon = mqttClient.connect(clientName); // retry connection to broker
      delay(2000);          // wait 2 secs between retries
      }

  if(mqttCon){          // Connected !
    Serial.println("Connected with MQTT server");
    digitalWrite(MQCON, HIGH);      // switch on MQTT connection indicator LED
    mqttClient.subscribe(subTopic);     // subscribe to all southbound messages
    }
  else Serial.println("No con with MQTT server");


  #ifdef DEBUGPJ2
    Serial.println("Setup Done");
    Serial.println(freeRam());
  #endif
  
} // end setup