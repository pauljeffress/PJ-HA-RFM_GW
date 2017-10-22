//
//==============  SENDMSG
//
//  sends messages over the radio network

#include "PJ-HA-RFM_GW.h" // My global defines and extern variables to help multi file comilation.

void sendMsg(int target) {
  #ifdef DEBUGPJ2
    Serial.println();
    Serial.println("RF msg about to be sent >>>>");
    Serial.print("Outbound Message to Node:");Serial.print(radio.SENDERID);Serial.print("  with RSSI:");Serial.println(radio.RSSI);
    Serial.println("=========RF msg data===================");
    Serial.print("From devID:");Serial.println(mes.devID);
    Serial.print("       cmd:");Serial.println(mes.cmd);
    Serial.print("    intVal:");Serial.println(mes.intVal);
    Serial.print(" fltintVal:");Serial.println(mes.fltintVal);
    Serial.print("To  NodeID:");Serial.println(mes.nodeID);
    Serial.print("   payLoad:");
          for (int i=0; i<32; i++) Serial.print(mes.payLoad[i]);
    Serial.println(":");
    Serial.println("=======================================");
  #endif  // DEBUGPJ2
  
  Rstat = true;               // radio indicator on
  digitalWrite(R_LED, HIGH);  // turn on radio LED
  onMillis = millis();        // store timestamp, so loop() can turn it off when the time is up.

  int i = RFTXRETRIES;                  // number of RF transmission retries

  while (respNeeded && i>0) {       // first try to send packets
    #ifdef DEBUGPJ
      Serial.print(">>>rf-tx>>> to node:" );
      Serial.println(target);
    #endif
    if (radio.sendWithRetry(target, (const void*)(&mes), sizeof(mes),RFTXRETRIES)) {
      respNeeded = false;
       } 
    else delay(500);        // half a second delay between retries
    i--;
    } // end of while loop

  if (respNeeded && verbose) {          // if not succeeded in sending packets after 5 retries
    sprintf(buff_topic, "home/sam_gw/nb/node%02d/dev90", NODEID); // construct MQTT topic and message
    sprintf(buff_mess, "connection lost to node %d", target);    // for radio loss (device 90)
    mqttClient.publish(buff_topic,buff_mess);     // publish ...
    respNeeded = false;           // reset response needed flag
    #ifdef DEBUGPJ2
      Serial.print("No connection with RF node:");
      Serial.println(target);
    #endif
    }
 
  mqttToSend = false;       // reset send trigger

  Serial.println("sendMsg()- Done"); 

} // ======== end sendMsg

