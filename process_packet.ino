//
//============== PROCESSPACKET
//
// Called if data has been received from the RF network (i.e. Northbound) parses the contents and constructs northbound MQTT topic and value.
// If data is correct size, copies it to "mes" to work on.
// Then builds "buff_topic" e.g. "home/rfm_gw/nb/node03/dev043"
//           & "buff_mes"...Fills it with Integer, Real, String etc, depending on type of DevID.
// then calls "mqttClient.publish(buff_topic,buff_mess)" to send it north over MQTT/IP to broker.


void processPacket() {
  bool MesSizeOK = false; 
  typedef struct { // Radio packet structure max 66 bytes
    int32_t nodeID; // node identifier
    int32_t devID; // device identifier 0 is node; 31 is temperature, 32 is humidity
    int32_t cmd; // read or write
    int32_t intVal; // integer payload
    float fltVal; // floating payload
    char payLoad[32]; // char array payload
    } ARMMessage;
        
      ARMMessage ARMmes;
  
  #ifdef DEBUGPJ2
    Serial.println();
    Serial.println("<<<rf-rx<<< so Start processPacket()"); 
  #endif
  Rstat = true;                // set radio indicator flag 
  digitalWrite(R_LED, HIGH);  // turn on radio LED
  onMillis = millis();        // store timestamp

  if (radio.DATALEN == sizeof(mes)) // we got valid sized message from an AVR node.
    {
    mes = *(Message*)radio.DATA;  // copy radio packet
    MesSizeOK = true; 
    }
  
  if (radio.DATALEN == sizeof(ARMmes)) // we got valid sized message from an ARM node.
    {
    // translate the message from ARM Node into std msg before proceeding.
    ARMmes = *(ARMMessage*)radio.DATA;  // copy radio packet
    mes.nodeID = ARMmes.nodeID;
    mes.devID = ARMmes.devID;
    mes.cmd = ARMmes.cmd;
    mes.intVal = ARMmes.intVal;
    mes.fltVal = ARMmes.fltVal;
    for (int i=0; i<32; i++)
      {
      mes.payLoad[i] = ARMmes.payLoad[i];
      }
    MesSizeOK = true;  
    }
  
  if (MesSizeOK == false) // wrong message size means trouble
    {
    #ifdef DEBUGPJ
      Serial.println("<<<< RF msg received but had invalid message size.");
      Serial.print("radio.DATALEN:");
      Serial.println(radio.DATALEN);

      Serial.print("expected mes size:");
      Serial.println(sizeof(mes));
      
      // Serial.print('[');Serial.print(radio.SENDERID, DEC);Serial.print("] ");
      // for (byte i = 0; i < radio.DATALEN; i++)
      //   Serial.print((char)radio.DATA[i]);
      // Serial.print("   [RX_RSSI:");Serial.print(radio.RSSI);Serial.print("]");

      mes = *(Message*)radio.DATA;  // copy radio packet
      Serial.print("Inbound Message from Node:");Serial.print(radio.SENDERID);Serial.print("  with RSSI:");Serial.println(radio.RSSI);
      Serial.println("=========msg data===================");
      Serial.print("   devID:");Serial.println(mes.devID);
      Serial.print("     cmd:");Serial.println(mes.cmd);
      Serial.print("  intVal:");Serial.println(mes.intVal);
      Serial.print("  fltVal:");Serial.println(mes.fltVal);
      Serial.print("  NodeID:");Serial.println(mes.nodeID);
      Serial.print(" payLoad:");
            for (int i=0; i<32; i++) Serial.print(mes.payLoad[i]);
      Serial.println(":");
      Serial.println("====================================");
      
    #endif  
    }
    
  else    // message size OK...and mes is properly populated
    {
    // construct MQTT northbound topic ready to send
    sprintf(buff_topic, "home/rfm_gw/nb/node%02d/dev%03d", radio.SENDERID, mes.devID);  
    #ifdef DEBUGPJ
      //Serial.println("<<<< RF msg received, validated and published via MQTT to Mosquitto.");
      Serial.print("Inbound Message is - radio.senderID:");
      Serial.print(radio.SENDERID); Serial.print(", ");
      Serial.print(mes.devID);  Serial.print(", ");
      Serial.print(mes.cmd);  Serial.print(", ");
      Serial.print(mes.intVal); Serial.print(", ");
      Serial.print(mes.fltVal); Serial.print(", RSSI=");
      Serial.print(radio.RSSI); Serial.print(" Node:");
      Serial.print(mes.nodeID); Serial.print(" Str Payload:");
      for (int i=0; i<32; i++) Serial.print(mes.payLoad[i]);
      Serial.println();
    #endif  
    
    // reset flags before setting them...
    StatMess = false;
    RealMess = false;
    IntMess = false;
    StrMess = false;

    // construct MQTT message, according to incoming device ID
    DID = mes.devID;            
    IntMess = (DID==0 || DID==1 || DID==7 || (DID >=32 && DID <=39) || (DID>=64 && DID<=71) || (DID>=100 && DID<=116) || (DID>=201 && DID<=299));  // Integer in payload message
    RealMess = (DID==2 || DID==3 || DID==4 || (DID>=48 && DID<=63) || (DID>=400 && DID<=499));          // Float in payload message
    StatMess = (DID==5 || DID==6 || DID==8 || (DID>=16 && DID <32) || (DID>=40 && DID <47)  );    // Status in payload message
    StrMess = (DID==3 || DID==72 || DID==11 || DID==12);      // String in payload

    if (IntMess) {      // send integer value load
      sprintf(buff_mess, "%d",mes.intVal);
      }

    if (RealMess) {     // send decimal value
      dtostrf(mes.fltVal, 10,2, buff_mess);
      while (buff_mess[0] == 32) {        // remove any leading spaces
        for (int i =0; i<strlen(buff_mess); i++) {
          buff_mess[i] = buff_mess[i+1];
          }
        }
      }

    if (StatMess) {     // put status in payload
      if (mes.intVal == 1 )sprintf(buff_mess, "ON");
      if (mes.intVal == 0 )sprintf(buff_mess, "OFF");
      }

    if (StrMess) {
      int i; 
      for (i=0; i<32; i++){ 
        buff_mess[i] = (mes.payLoad[i]); 
        }
      } 

    switch (mes.devID)          
      {
      case (2):             // RSSI value
        { sprintf(buff_mess, "%d", radio.RSSI);
        }
        break;
        
      case (92):              // invalid device message
        { sprintf(buff_mess, "NODE %d invalid device %d", mes.nodeID, mes.intVal);
        }
        break;
        
      case (99):              // wakeup message
        { sprintf(buff_mess, "NODE %d WAKEUP", mes.nodeID);
        }
        break;
      } // end switch

    #ifdef DEBUGPJ
      Serial.print("^^^^ Northbound MQTT message to publish: ");
      Serial.print(buff_topic);
      Serial.print(": ");
      Serial.println(buff_mess);
    #endif

    mqttClient.publish(buff_topic,buff_mess);     // publish MQTT message in northbound topic

    if (radio.ACKRequested()) radio.sendACK();      // reply to any radio ACK requests
    }

  #ifdef DEBUGPJ2
    Serial.println("End processPacket()"); 
  #endif
} 
// ============= end processPacket
