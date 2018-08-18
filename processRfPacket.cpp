//
//============== processRfPacket
//
// Called if data has been received from the RF network (i.e. Northbound) parses the contents and constructs northbound MQTT topic and value.
// If data is correct size, copies it to "mes" to work on.
// Then builds "buff_topic" e.g. "home/rfm_gw/nb/node03/dev043"
//           & "buff_mes"...Fills it with Integer, Real, String etc, depending on type of DevID.
// then calls "mqttClient.publish(buff_topic,buff_mess)" to send it north over MQTT/IP to broker.

#include "PJ-HA-RFM_GW.h" // My global defines and extern variables to help multi file comilation.

void processRfPacket(int rfPackLen, int rfPackFrom) {
  bool MesSizeOK = false; 
  
  //#ifdef DEBUGx
  //  Serial.println();
  //  Serial.println("<<<rf-rx<<< so Start processRfPacket()"); 
  //#endif
  
  Rstat = true;               // set radio indicator flag 
  digitalWrite(R_LED, HIGH);  // turn on radio LED
  onMillis = millis();        // store timestamp, so loop() can turn it off when the time is up.

  if (rfPackLen == HARFPACKSIZE) // we got valid sized home automation message from a node.
    {
    // XXXX populate mes from radioDataBuf, maybe use something like mes = *(Message*)radio.DATA;
    MesSizeOK = true; 
    }
     
  if (!MesSizeOK) // Bad message size - wrong message size means trouble
    {
    #ifdef DEBUGPJ2
      Serial.println();
      Serial.println("<<<< RF msg received but had invalid home automation message size.");
      Serial.print("rfPackLen:");
      Serial.println(rfPackLen);
      Serial.print("expected mes size:");
      Serial.println(HARFPACKSIZE);
      
      // mes = *(Message*)radio.DATA;  // copy radio packet

      // Serial.print("Inbound Message from Node:");Serial.print(radio.SENDERID);Serial.print(" with RSSI:");Serial.println(radio.RSSI);
      // Serial.println("=========RF msg data===================");
      // Serial.print("From devID:");Serial.println(mes.devID);
      // Serial.print("       cmd:");Serial.println(mes.cmd);
      // Serial.print("    intVal:");Serial.println(mes.intVal);
      // Serial.print(" fltintVal:");Serial.println(mes.fltintVal);
      // Serial.print("To  NodeID:");Serial.println(mes.nodeID);
      // Serial.print("   payLoad:");
      //       for (int i=0; i<32; i++) Serial.print(mes.payLoad[i]);
      // Serial.println(":");
      // Serial.println("=======================================");
    #endif  // DEBUGPJ2
    }
    
  else    // message size OK...
    {
    #ifdef DEBUGPJ2
    Serial.println();
    Serial.println("<<<< RF msg received with correct size.");
    Serial.print("Inbound Message from Node:");Serial.println(rfPackFrom);
    Serial.println("=========RF msg data===================");
    Serial.print("To  NodeID:");Serial.println(mes.nodeID);
    Serial.print("     devID:");Serial.println(mes.devID);
    Serial.print("       cmd:");Serial.println(mes.cmd);
    Serial.print("    intVal:");Serial.println(mes.intVal);
    Serial.print(" fltintVal:");Serial.println(mes.fltintVal);
    Serial.print("   payLoad:");
          for (int i=0; i<32; i++) Serial.print(mes.payLoad[i]);
    Serial.println(":");
    Serial.println("=======================================");
    #endif  // DEBUGPJ2
    
    // reset flags before setting them...
    StatMess = false;
    RealIntMess = false;
    IntMess = false;
    StrMess = false;

    // construct MQTT northbound topic ready to send
    sprintf(buff_topic, "home/sam_gw/nb/node%02d/dev%03d", rfPackFrom, mes.devID); 

    // construct MQTT Payload, according to incoming device ID
    DID = mes.devID;            
    IntMess = (DID==0 || DID==1 || DID==7 || (DID >=32 && DID <=39) || (DID>=64 && DID<=71) || (DID>=100 && DID<=116) || (DID>=201 && DID<=299));  // Integer in payload message
    RealIntMess = (DID==2 || DID==3 || DID==4 || (DID>=48 && DID<=63) || (DID>=400 && DID<=499));          // Float in payload message
    StatMess = (DID==5 || DID==6 || DID==8 || (DID>=16 && DID <32) || (DID>=40 && DID <47)  );    // Status in payload message
    StrMess = (DID==3 || DID==72 || DID==11 || DID==12);      // String in payload

    if (IntMess) {      // send integer value load
      sprintf(buff_mess, "%d",mes.intVal);
      }

    if (RealIntMess) {     // send float as integer value
      sprintf(buff_mess, "%d",mes.fltintVal);
      }

    if (StatMess) {     // put status in payload
      if (mes.intVal == 1 )sprintf(buff_mess, "ON");
      if (mes.intVal == 0 )sprintf(buff_mess, "OFF");
      }

    if (StrMess) {
      int i; 
      for (i=0; i<STRPAYLOADSIZE; i++){ 
        buff_mess[i] = (mes.payLoad[i]); 
        }
      } 

    switch (mes.devID)          
      {
      case (2):             // RSSI value
        { sprintf(buff_mess, "%d", 999);    // pre RadioHead it was radio.RSSI
        }
        break;
        
      case (92):              // invalid device message
        { sprintf(buff_mess, "NODE%d invalid DEVice %d", mes.nodeID, mes.intVal);
        }
        break;
        
      case (99):              // wakeup message
        { sprintf(buff_mess, "NODE %d WAKEUP", mes.nodeID);
        }
        break;
      } // end switch

        
      #ifdef DEBUGPJ2
      Serial.println();
      Serial.println("IP/MQTT message sending >>>>"); 
      Serial.println("+++++++++++++++++++++++++++++++++++++++++++++++++++++");
      Serial.print("+   Topic:"); Serial.println(buff_topic);
      Serial.print("+ Payload:"); Serial.println(buff_mess);   
      Serial.println("+++++++++++++++++++++++++++++++++++++++++++++++++++++");
      Serial.println();
      #endif
      

    mqttClient.publish(buff_topic,buff_mess);     // publish MQTT message in northbound topic

    // if (radio.ACKRequested()) radio.sendACK();      // reply to any radio ACK requests // Not required with RadioHead
    }

  #ifdef DEBUGx
    Serial.println("End processRfPacket()"); 
  #endif
} 
// ============= end processRfPacket
