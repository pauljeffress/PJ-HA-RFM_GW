//
//==============  LOOP
//

void loop() {
//  #ifdef DEBUGPJ2
//    Serial.print(freeRam());
//    Serial.print(".");
//    delay(1000);
//  #endif


  // CONTROL RADIO LED AND CALCULATE UPTIME 
  //
    if (Rstat) {            // turn off radio LED after 100 msec
      if (millis() - onMillis > 100) {
        Rstat = false;
        digitalWrite(R_LED, LOW);
        }
      }
    if (lastMinute != (millis()/60000))       // another minute passed ?
      {
      lastMinute = millis()/60000;
      upTime++;
      #ifdef DEBUGPJ2
        Serial.print("Uptime:");
        Serial.println(upTime);
      #endif
      }
  
  // RF SEND MESSAGE to node if there is one ready to go.
  //
  if (mqttToSend) {   // mqttToSend gets set to True via mqtt_subs() which is itself called/feed from the PubSub library.
                      // It is set to true when something subscribed to has been received from MQTT broker and it looks like 
                      // a valid message to send southbound to nodes via RF.
    #ifdef DEBUGPJ2
      Serial.println("loop() - mqttToSend was TRUE, calling sendMsg() to rf tx stuff" );
    #endif
    sendMsg(dest);    // send MQTT instruction packets south over the radio network
                      // 'dest' is the RF node ID, it must have been set by mqtt_Subs() at same time as mqttToSend was set to true perhaps?
    }   

  // RF RECEIVE MESSAGE if one has arrived and MQTT SEND MESSAGE if need be
  //
  if (radio.receiveDone()) { // check for received radio packets and construct MQTT message
    processPacket();
    } 

  // MQTT/IP - CHECK CONNECTION STILL UP - REESTABLISH IF NOT - RECEIVE/PROCESS MQTT SUBSCRIPTIONS
  //
  if (!mqttClient.loop()) {     // check connection MQTT server and process MQTT subscription input
    #ifdef DEBUGPJ2
      Serial.println("Loop() - mqtt not connected");
      //Serial.println(freeRam());
    #endif
    mqttCon = 0;                  // if you get to this line the MQTT/IP connection is down.
    digitalWrite(MQCON, LOW);     // switch off the MQTT/IP Connection LED, as we have lost connection.
  
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
    else Serial.println("No con with MQTT server");  // at this stage even if we have not been able to reconnect to
                                                      // mqtt broker, move on and let loop() keep running.
  }
} // ============== end LOOP
