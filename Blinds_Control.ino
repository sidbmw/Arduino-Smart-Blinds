/*
  Description: This program will help you control your blinds by opening, closing or opening half way.
               This can be done through the website (192.168.0.45) or you can set it to automatically close, open and open half way depending on the amount of light outside.
               Automatic control is setup so that the room remains cool. i.e Will close if it's very bright, will open if dark and open half way if there is some light.

  Author: Siddharth Natamai
  Date:   June 17
*/


#include <Stepper.h>
#include <SPI.h>
#include <Ethernet.h>


#define motorPin1  8     // IN1 on the ULN2003 driver 1
#define motorPin2  10     // IN2 on the ULN2003 driver 1
#define motorPin3  9     // IN3 on the ULN2003 driver 1
#define motorPin4  7   // IN4 on the ULN2003 driver 1

#define REQ_BUF_SZ 60
char HTTP_req[REQ_BUF_SZ] = {0}; // buffered HTTP request stored as null terminated string
char req_index = 0;              // index into HTTP_req buffer

const int pResistor = A0; // Photoresistor at Arduino analog pin A0
int value;          // Store value from photoresistor (0-1023)
int position = 0;

byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
IPAddress ip(192, 168, 0, 45); //Make sure this IP address can be assigned through the router

Stepper myStepper(4096, motorPin1, motorPin2, motorPin3, motorPin4);

EthernetServer server(80);


String readString;
boolean automaticControl = false; //Change to true if you would like Automatic Control to be enabled by default


void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }


  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());


  pinMode(pResistor, INPUT);// Set pResistor - A0 pin as an input

}//--(end setup )---

void loop() {

  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();

        if (readString.length() < 100) {
          //store characters to string
          readString += c;
          //Serial.print(c);
        }

        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {

          Serial.println(readString); //print to serial monitor for debuging

          if (readString.indexOf("?status=0") >= 0) { //don't send new page
            client.println("Blinds are Closed"); //Prints "Blinds are Closed on the webpage"

            if (position == 1) {
              for (int stepCount = 0; stepCount < 3072; stepCount++) {
                myStepper.step(-1);
                Serial.print("steps:");
                Serial.println(stepCount);
              }
              position = 0;
            }

            if (position == 2) {
              for (int stepCount = 0; stepCount < 7168; stepCount++) {
                myStepper.step(-1);
                Serial.print("steps:");
                Serial.println(stepCount);
              }
              position = 0;
            }

            automaticControl = false;
            break;
          }


          if (readString.indexOf("?status=1") >= 0) { //don't send new page


            client.println("Blinds are Half Open"); //Prints "Blinds are Half Open" to webpage

            if (position == 0) {
              for (int stepCount = 0; stepCount < 3072; stepCount++) {
                myStepper.step(1);
                Serial.print("steps:");
                Serial.println(stepCount);

              }
              position = 1;
            }

            if (position == 2) {
              for (int stepCount = 0; stepCount < 3072; stepCount++) {
                myStepper.step(-1);
                Serial.print("steps:");
                Serial.println(stepCount);

              }
              position = 1;
            }

            automaticControl = false;
            break;
          }



          if (readString.indexOf("?status=2") >= 0) { //don't send new page

            client.println("Blinds are Open"); //Prints Blinds are open



            if (position == 0) {
              for (int stepCount = 0; stepCount < 7168; stepCount++) {
                myStepper.step(1);

                Serial.print("steps:");
                Serial.println(stepCount);
              }
              position = 2;
            }

            if (position == 1) {
              for (int stepCount = 0; stepCount < 3072; stepCount++) {
                myStepper.step(1);
                Serial.print("steps:");
                Serial.println(stepCount);

              }
              position = 2;
            }

            automaticControl = false;
            break;
          }


          if (readString.indexOf("?status=3") >= 0) { //don't send new page

            client.println("AUTOMATIC CONTROL");
            automaticControl = true;
            break;
          }

          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println("Refresh: 5");  // refresh the page automatically every 5 sec
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          // output the value of each analog input pin


          if (position == 0) {
            client.print(" Blinds are  <font color='lime'>closed</font>");
            client.println("<br />");
          }

          if (position == 1) {
            client.print(" Blinds are  <font color='yellow'>half open</font>");
            client.println("<br />");
          }

          if (position == 2) {
            client.print(" Blinds are  <font color='red'>open</font>");
            client.println("<br />");
          }


          client.print("<FORM action=\"http://192.168.0.45/\" >");
          client.print("<P> <INPUT type=\"radio\" name=\"status\" value=\"0\">CLOSE");
          client.print("<P> <INPUT type=\"radio\" name=\"status\" value=\"1\">HALF OPEN");
          client.print("<P> <INPUT type=\"radio\" name=\"status\" value=\"2\">OPEN");
          client.print("<P> <INPUT type=\"radio\" name=\"status\" value=\"3\">AUTOMATIC CONTROL");
          client.print("<P> <INPUT type=\"submit\" value=\"Submit\"> </FORM>");
          client.println("</html>");
          break;
        }

        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;

        }
      }
    }


    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println("client disconnected");

    //clearing string for next read
    readString = "";
  }



  /////////////////////////////////////////
  //Stepper motor control


  value = analogRead(pResistor); //Read value put out by photoresistor
  Serial.println(value);
  delay(1000);


  if (automaticControl == true) {

    if (value > 400 && position != 0) {

      if (position == 1) {
        for (int stepCount = 0; stepCount < 3072; stepCount++) {
          myStepper.step(-1);
          Serial.print("steps:");
          Serial.println(stepCount);
        }
        position = 0;
      }

      if (position == 2) {
        for (int stepCount = 0; stepCount < 7168; stepCount++) {
          myStepper.step(-1);
          Serial.print("steps:");
          Serial.println(stepCount);
        }
        position = 0;
      }

    }


    if (value >= 300 && value <= 400 && position != 1) {

      if (position == 0) {
        for (int stepCount = 0; stepCount < 3072; stepCount++) {
          myStepper.step(1);
          Serial.print("steps:");
          Serial.println(stepCount);

        }
        position = 1;
      }

      if (position == 2) {
        for (int stepCount = 0; stepCount < 3072; stepCount++) {
          myStepper.step(-1);
          Serial.print("steps:");
          Serial.println(stepCount);

        }
        position = 1;
      }
    }


    if (value < 300 && position != 2) {

      if (position == 0) {
        for (int stepCount = 0; stepCount < 7168; stepCount++) {
          myStepper.step(1);

          Serial.print("steps:");
          Serial.println(stepCount);
        }
        position = 2;
      }

      if (position == 1) {
        for (int stepCount = 0; stepCount < 3072; stepCount++) {
          myStepper.step(1);
          Serial.print("steps:");
          Serial.println(stepCount);

        }
        position = 2;
      }
    }
  }
}
