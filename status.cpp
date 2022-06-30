#include "status.hpp"

void poll_stat(UTFT myDisplay, char* command, char* buffer, uint16_t time_out) {

    char head[20] = "AT+";                  // 
    strcat(head, command);                  // concatenate strings to assemble a complete AT command
    myDisplay.setColor(255,255,255);
    myDisplay.print(head,0,0);
    strcat(head, "\r");                     // AT commands must be terminated with a carriage return
    Serial.write(head);
    delay(550);

    char reply[40]={0};
    uint8_t i=0;
    
    uint32_t time_start = millis();
    while(1) {
        uint32_t time_now = millis();
        if (time_now < time_start + time_out) {
            if (Serial.available()) {
                char u = Serial.read();
                if (u == '\r') {
                    break;
                }
                reply[i++] = u;
            }
        }
        else {
            break;
        }
    }

    if (strlen(reply) == 0) {
        // myDisplay.setColor(0, 0, 255);
        // myDisplay.print("No Response",CENTER,40);
        // myDisplay.setColor(125,125,125);
        // myDisplay.print("Press to restart",CENTER,70);
        // while (1) {
        //     //
        // };
        // 
    }
    else {
        myDisplay.setColor(244, 81, 30);
        myDisplay.print(reply,0,40);
        Serial.flush();
        delay(500);                             // retain display for some time so that we can see it
        myDisplay.clrScr();

        char* status = strtok(reply, "=");      // get left part of "="
        status = strtok(NULL, "=");             // get right part of "="
        strcpy(buffer, status);
    }
}