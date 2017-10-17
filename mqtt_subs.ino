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

  #ifdef DEBUGPJ2
    Serial.println();
    Serial.println("IP/MQTT RX so starting mqtt_subs()"); 
  #endif
  int i;
  String errStr = "";       // error string to be published back to MQTT broker, when a received IP/MQTT message has an issue.
  mes.nodeID = NODEID;      // start to build an new RF message to eventually send south. The device (the Gateway) nodeID will always be 1.
  mes.fltVal = 0;           // zero out some of the message parameters
  mes.intVal = 0;
  mqttToSend = false;       // not a valid request yet...
  error = 4;                // assume an error is processing the IP/MQTT data, until proven otherwise
  dest = 999;               // 999 is just a number that makes it easy to debug/see if no dest node was found and overwritten into this variable.


  #ifdef DEBUGPJ2
    Serial.print("\\/  \\/  \\/ Southbound MQTT Topic received: ");
    Serial.println(topic);
  #endif

  if (strlen(topic) == 28)        // is the 'topic' we received the correct length ?
    {                             // originally was 27, now 28 as I have allowed for devID's > 99 hence three digit.
    // yes it is the correct length so lets process it...
    dest = (topic[19]-'0')*10 + topic[20]-'0';  // extract target node ID from MQTT topic and store it in global 'dest' for use later
    DID = (topic[25]-'0')*100 + (topic[26]-'0')*10 + topic[27]-'0';   // extract device ID from MQTT topic and stor it in global 'DID" for use later
    // xxxx - I really should be checking that 'dest and 'DID' are within acceptable ranges here and erroring out if not.
    payload[length] = '\0';                     // terminate the payload string we received with '0'
    String strPayload = String((char*)payload); // convert the payload to string, as it was a ptr to some bytes when we received it.
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
      RealMess = false;
      IntMess = false;
      StrMess = false;
      
      StatMess = ( DID==5 || DID==6 || DID==8 || (DID>=16 && DID<=31) || (DID>=40 && DID <47)); // Set global 'StatMess' to true based on devID.
                                                                                                //    These special Integer DIDs can only be ON/OFF 
                                                                                                //    not any int value.
                                                                                                
      RealMess = ( DID==2 || DID==3 || DID==4 || (DID>=48 && DID<=63) || (DID>=400 && DID<=499));  // set global 'RealMess' to true based on devID.
                                                                                                
                                                                                                
      IntMess = (DID==0 || DID==1 || DID==7 || (DID >=32 && DID <=39) || (DID>=64 && DID<=71) || (DID>=100 && DID<=116) || (DID>=201 && DID<=299)  ); // set global 'IntMess' to true based on devID.

      StrMess = (DID==3 || DID==72 || DID==11 || DID==12);                                     // set global 'StrMess' to true based on devID.

      if (!(StatMess || RealMess || IntMess || StrMess)) // i.e we have a message that did not fit any of the above categories - and thats bad!
        {
          error = 5;
          errStr = "No Categ Match";
        }

      // Now that we have pulled the message apart, checked and classified it, lets action it...

      // Check if message is for Gateway itself and is it an Uptime request?
      if (dest == 1 && DID == 0)          // gateway uptime wanted
        {                                                   // so construct MQTT topic and message/payload
        sprintf(buff_mess,  "%d", upTime);                  //    copy upTime into the buff_Mess (i.e. payload we will send north)
        sprintf(buff_topic, "home/rfm_gw/nb/node01/dev000");  //    copy the correct topic to publish this info out on into buff_topic 
        mqttClient.publish(buff_topic,buff_mess);           // MQTT publish the topic & payload
        error =0;
        }

      // Check if message is for Gateway itself and is it a Version request?
      if (dest == 1 && DID == 3)          // gateway version wanted
        {                                                   // so construct MQTT topic and message/payload
        for (i=0; i<sizeof(VERSION); i++){                  //    copy the VERSION string into the buff_Mess (i.e. payload we will send north)
          buff_mess[i] = (VERSION[i]); }
        mes.payLoad[i] = '\0';
        sprintf(buff_topic, "home/rfm_gw/nb/node01/dev003");  //    copy the correct topic to publish this info out on into buff_topic 
        mqttClient.publish(buff_topic,buff_mess);           // MQTT publish the topic & payload
        error =0;
        }

      // Message is for an RF Node and its a StatMess. 
      if (dest>1 && StatMess)              
        {
        #ifdef DEBUGPJ
          Serial.println("StatMess");
        #endif
        mqttToSend = true;                          // flag that there is an RF message to send south. It will go when main loop() checks this flag.
        if (strPayload == "ON") mes.intVal = 1;     // convert MQTT payload string (ON>1, OFF>0) or leave it as READ in payload string 
        else if (strPayload == "OFF") mes.intVal = 0;
        else if (strPayload != "READ") // invalid payload; do not process
                { 
                mqttToSend = false;
                error = 3;
                errStr = "invalid payload";
                }
        }

      // Message is for an RF Node and its a StatMess.
      if (dest>1 && (DID >=40 && DID <48))    // check if someone is trying to ON/OFF a DevID you can only READ from. i.e. an input.
                                              // I would have though this IF should also be confirming its a StatMess?????
        {
        #ifdef DEBUGPJ
          Serial.println("StatMess40-47");
        #endif
        if (strPayload == "READ") mqttToSend = true; // flag that there is an RF message to send south. It will go when main loop() checks this flag.
        else // invalid payload; do not process
           { 
           mqttToSend = false;
           error = 3;
           errStr = "invalid payload";
           }
        }
        
      // Message is for an RF Node and its a RealMess.
      if ( dest>1 && RealMess )          // It's a Real Message. Could be a READ or a WRITE.
        {
        if (mes.cmd == 0) // i.e its a WRITE
          {
          mes.fltVal = strPayload.toFloat();  // If its a SET/WRITE then copy the MQTT payload across to the RF FltVal
                                              // Note: If its a GET/READ, we don't have to set anything else in the mess struct 
                                              // because the mes.cmd will be interpreted at the RF end Node, and a READ will be performed.  
          }
          #ifdef DEBUGPJ
            Serial.println("RealMess");
          #endif
          mqttToSend = true;  // flag that there is an RF message to send south. It will go when main loop() checks this flag.
        }

      // Message is for an RF Node and its an IntMess.
      if ( dest>1 && IntMess )          // It's an Integer Message. Could be a READ or a WRITE.
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
          mqttToSend = true;  // flag that there is an RF message to send south. It will go when main loop() checks this flag.
        }


      // Message is for an RF Node and its a StrMess.
      if ( dest>1 && StrMess )          // It's an String Message. Could be a READ or a WRITE.
        {
        if (mes.cmd == 0)               //    If its a SET/WRITE then copy the MQTT payload across to the RF Payload (there is no stringVal)
          {
          int i; 
          for (i=0; i<32; i++)
            { 
            (mes.payLoad[i])=payload[i];
            }
          }
        mqttToSend = true;              // flag that there is an RF message to send south. It will go when main loop() checks this flag.
        }

      // Check if any errors were found during the above processing, if not, set error to 0.
      if (mqttToSend && (error == 4)) error = 0;    // valid device has been selected, hence error = 0
                                                    //     Bit of an odd way to confirm that we can clear the error flag of its default 4 that it was set to at beginning of this function.
  
      respNeeded = mqttToSend;                      // valid request needs radio response
      
      #ifdef DEBUGxx
        Serial.print("mes.payLoad:");
        Serial.println(mes.payLoad);
        Serial.print("intVal:  ");
        Serial.println(mes.intVal);
        Serial.print("fltVal:  ");
        Serial.println(mes.fltVal);
      #endif
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
    sprintf(buff_topic, "home/rfm_gw/nb/node01/dev91");             // construct MQTT topic and message
    mqttClient.publish(buff_topic,buff_mess);                       // publish ...
    #ifdef DEBUGPJ
      Serial.print("Syntax error code is: ");
      Serial.print(error);
      Serial.print("  Error description string is: ");
      Serial.println(errStr);
    #endif
  }

  #ifdef DEBUGPJ2
    Serial.println("End mqtt_subs()"); 
  #endif
 
} // end mqttSubs
