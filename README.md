# PJ-HA-RFM_GW
My HA RFM_GW code.
Up until 17-10-2017 I hadn't used GitHub to manage my RFM_GW code. Now that I am about to convert it to run on a SAMD21 (SenseBender board) I thought I'd get it onto GitHub so I can manage the required changes.
The last version I had before bringing it to GitHub was PJ_RFM_MQTT_GW_22v27. I had just recently made a few changes from v26 when I updated my Node code to support a SAMD21 (Adafruit Feather M0 RFM69) and the gateway code had to be slightly modified to accomodate that new Node code.  Prior to that the RFM_GW v26 code had been unchanged for maybe 6mths.
.
16-11-17
Almost a month ago I finished updating this GW code to run on my Sensebender (SAMD21) board.  See notes against various commits and also in my Evernote called "RFM_GW conversion...." etc.
I'm thinking I'll leave my old AVR based RFM_GW sevice in place and run this new SenseBender GW in parallel, on a different RF Network ID.  That way I can move existing AVR Nodes across to the corresponding new Node Code that is compatible with this SAMD21 GW code, one at a time.  Then when all of my Nodes are migrated I can switch off the old AVR RFM_GW.
Likewise I can start deploying SAMD21 Nodes straight away and put them straight onto this new GW and Network.
