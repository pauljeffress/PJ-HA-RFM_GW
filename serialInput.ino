//
//  SERIALINPUT, only used in debugging mode
//
//  some commands can be given thru the terminal to simulate actions
// 'l' will toggle LED
// 's' will toggle tranmsission interval between 4 states
// 
//

#ifdef DEBUGPJ
bool serialInput() {        // get and process manual input
bool msgAvail = false;
char input = Serial.read();
dest = 2;         // debug against fixed node 2
            // comment out to debug against node that last sent respons
mes.nodeID = radio.SENDERID;      // initialize
mes.devID = 0;
mes.intVal = 0;
mes.fltVal = 0;
mes.cmd = 0;
if (input == 'l')           // toggle LED
{
  msgAvail = true;
  respNeeded = true;
  act1Stat = !act1Stat;
  mes.devID = 16;
  if (act1Stat) mes.intVal = 1; else mes.intVal =0;
  Serial.print("Current LED status is ");
  Serial.println(act1Stat);
}

if (input == 's')           // set polling interval
{
  msgAvail = true;
  respNeeded = true;
  curstat++;
  if (curstat == 4) curstat = 0;
  mes.devID = 1;
  mes.intVal = stat[curstat];
  Serial.print("Current polling interval is ");
  Serial.println(mes.intVal);

}
return msgAvail;
} // end serialInput
#endif
