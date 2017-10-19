//
//==============  SENDMSG
//
//  sends messages over the radio network

#include "PJ-HA-RFM_GW.h" // My global defines and extern variables to help multi file comilation.

void sendMsg(int target) {
  #ifdef DEBUGPJ2
    Serial.println("sendMsg() - Start RF TX"); 
  #endif
  Rstat = true;               // radio indicator on
  digitalWrite(R_LED, HIGH);  // turn on radio LED
  onMillis = millis();        // store timestamp
  int i = 5;                  // number of transmission retries

  while (respNeeded && i>0) {       // first try to send packets
    #ifdef DEBUGPJ
      Serial.print(">>>rf-tx>>> to node:" );
      Serial.println(target);
    #endif
    if (radio.sendWithRetry(target, (const void*)(&mes), sizeof(mes),5)) {
      respNeeded = false;
       } 
    else delay(500);        // half a second delay between retries
    i--;
    } // end of while loop

  if (respNeeded && verbose) {          // if not succeeded in sending packets after 5 retries
    sprintf(buff_topic, "home/rfm_gw/nb/node%02d/dev90", NODEID); // construct MQTT topic and message
    sprintf(buff_mess, "connection lost node %d", target);    // for radio loss (device 90)
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

