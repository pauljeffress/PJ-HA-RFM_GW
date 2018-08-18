//
//==============  rfSendMsg
//
//  sends messages over the radio network

#include "PJ-HA-RFM_GW.h" // My global defines and extern variables to help multi file comilation.

void rfSendMsg(int rfPackTo) {
  #ifdef DEBUGPJ2
    Serial.println();
    Serial.println("RF msg about to be sent >>>>");
    Serial.print("Outbound Message to Node:");Serial.println(rfPackTo);
    Serial.println("=========RF msg data===================");
    Serial.print("Src NodeID:");Serial.println(mes.nodeID);
    Serial.print("     devID:");Serial.println(mes.devID);
    Serial.print("       cmd:");Serial.println(mes.cmd);
    Serial.print("    intVal:");Serial.println(mes.intVal);
    Serial.print(" fltintVal:");Serial.println(mes.fltintVal);
    Serial.print("   payLoad:");
          for (int i=0; i<32; i++) Serial.print(mes.payLoad[i]);
    Serial.println(":");
    Serial.println("=======================================");
  #endif  // DEBUGPJ2
  
  Rstat = true;               // radio indicator on
  digitalWrite(R_LED, HIGH);  // turn on radio LED
  onMillis = millis();        // store timestamp, so loop() can turn it off when the time is up.

  //RadioHead based sender code
  Serial.print("Sending to Mesh Addr: ");
  Serial.println(rfPackTo);  

  // XXXX - copy mes into toSendData here XXXXXX
  //uint8_t toSendData[] = "ABCDEFGHIJ";
  //Serial.print("temp dummy toSendData =");
  //Serial.println((char*)toSendData);
  // Send a message to an rf69 mesh node
  // A route to the destination will be automatically discovered.
  if (manager.sendtoWait((uint8_t*)&mes, sizeof(mes), rfPackTo) == RH_ROUTER_ERROR_NONE)
  {
    // It has been reliably delivered to the next node.
    Serial.println("Message reliably delivered.");
    rfToSend = false;
  }
  else
  {   
    //The send failed...
    Serial.println("sendtoWait failed. Are the intermediate mesh servers running?");
    sprintf(buff_topic, "home/sam_gw/nb/node%02d/dev90", NODEID); // construct MQTT topic and message
    sprintf(buff_mess, "connection lost to node %d", rfPackTo);    // for radio loss (device 90)
    mqttClient.publish(buff_topic,buff_mess);     // publish ...
    rfToSend = false;           // reset response needed flag
    #ifdef DEBUGPJ2
      Serial.print("No connection with RF node:");
      Serial.println(rfPackTo);
    #endif
  }
  
  // code below is the old pre RadioHead rf send code.
  
  // int i = RFTXRETRIES;                  // number of RF transmission retries

  // while (rfToSend) && i>0) {       // first try to send packets
  //   #ifdef DEBUGPJ
  //     Serial.print(">>>rf-tx>>> to node:" );
  //     Serial.println(target);
  //   #endif
  //   if (radio.sendWithRetry(target, (const void*)(&mes), sizeof(mes),RFTXRETRIES)) {
  //     rfToSend = false;
  //      } 
  //   else delay(500);        // half a second delay between retries
  //   i--;
  //   } // end of while loop

  // if (rfToSend && verbose) {          // if not succeeded in sending packets after 5 retries
  //   sprintf(buff_topic, "home/sam_gw/nb/node%02d/dev90", NODEID); // construct MQTT topic and message
  //   sprintf(buff_mess, "connection lost to node %d", target);    // for radio loss (device 90)
  //   mqttClient.publish(buff_topic,buff_mess);     // publish ...
  //   rfToSend = false;           // reset response needed flag
  //   #ifdef DEBUGPJ2
  //     Serial.print("No connection with RF node:");
  //     Serial.println(target);
  //   #endif
  //   }
 
  rfToSend = false;       // reset send trigger

  Serial.println("rfSendMsg()- Done"); 

} // ======== end rfSendMsg

