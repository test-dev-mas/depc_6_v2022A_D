#include "test.hpp"

void run_test(UTFT myDisplay, char* command, uint16_t time_out) {

    char head[20] = "AT+";                  //
    strcat(head, command);
    strcat(head, "=1");
    myDisplay.setColor(255,255,255);
    myDisplay.print(head,0,0);
    strcat(head, "\r");
    Serial.write(head);
    delay(650);                             // if delay is not long enough(<550ms), response gets printed on next screen
    
    char reply[20] = {0};
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
    
    myDisplay.setColor(244, 81, 30);
    myDisplay.print(reply,0,40);
    Serial.flush();
    myDisplay.clrScr();
    
    if (strstr(reply, "ok") == 0) {
        // myDisplay.setColor(0, 0, 255);
        // myDisplay.print("No Response",CENTER,40);
        // myDisplay.setColor(125,125,125);
        // myDisplay.print("Press to restart",CENTER,70);
        // while (1) {
        //     //
        // };
        //
        ok_index++;
    }
}