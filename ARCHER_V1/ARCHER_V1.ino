#include <WiFi.h>
#include <HTTPClient.h>
//I'm changing someting here, next version 3

//String serial_number = "kdks;d";
char deviceData[10] = {0}; //default device Data
char tempDeviceData[5] = {0}; //response number
char responseData[10] = {0}; //response character, from string to character
char status = 0;
char numberOfLaunched = 0;
int count = 0; //number of character in serverPaht array.

const char* ssid = "ChinaNet-szA2";
const char* password = "ckzwfnns";

char serverPath[100] = "http://archer.scienceontheweb.net/nodes.php?serial_number=kdks;d&deviceData=";

//Your Domain name with URL path or IP address with path
//String serverName = "http://archer.scienceontheweb.net/nodes.php?serial_number="+serial_number+"&deviceData=";

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastTime = 0;
// Timer set to 10 minutes (600000)
//unsigned long timerDelay = 600000;
// Set timer to 5 seconds (5000)
unsigned long timerDelay = 5000;

String s_temp;

void setup() {
  Serial.begin(115200); 
  
  while(serverPath[count]!='\0')
  {
    count++;
  }

  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
 
  Serial.println("Timer set to 5 seconds (timerDelay variable), it will take 5 seconds before publishing the first reading.");
}

void loop() 
{
  //Send an HTTP POST request every 10 minutes
  if ((millis() - lastTime) > 500) 
  {
    //Check WiFi connection status
    if(WiFi.status()== WL_CONNECTED){
      HTTPClient http;

      //to change number array tempDeviceData to character array deviceData
      num2Char();
      //Serial.println(deviceData);
      // Your Domain name with URL path or IP address with path
      char temp = count; //last index of serverPaht array to temp to fill deviceData follow
      for(char i=0; i<10; i++)
      {
        serverPath[temp++] = deviceData[i];
      }
      //Serial.println(deviceData);
      //Serial.println(serverPath);
      http.begin(serverPath); // prepare http
      
      // If you need Node-RED/server authentication, insert user and password below
      //http.setAuthorization("REPLACE_WITH_SERVER_USERNAME", "REPLACE_WITH_SERVER_PASSWORD");
      
      // Send HTTP GET request

      int httpResponseCode = http.GET(); //http start and get response code, response code should be greater than 0, if less than 0 it's URL or internal problem
      
      if (httpResponseCode==200)  //get response from server
      {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);

        String payload = http.getString();//get data from server response

        //take only data from string like character array.
        for(char i=0; i<10; i++)
        {
          responseData[i] = payload.c_str()[i];
          //Serial.print(payload.c_str()[i]);
        }
        Serial.println();

        
        char2Num(); //change character to number, responseData is character and can't do manipulation, so change to number tempDeviceData

        if(status != tempDeviceData[4]) //check status register is change or not.
        {
          if((status & 1<<7) != (tempDeviceData[4] & 1<<7)) //if lunch bit is change
          {
             //launch code here;
             numberOfLaunched++;
          }
          
          if((status & 1<<5) != (tempDeviceData[4] & 1<<5)) //if reset bit is change
          {
             //reset all the 74hc595 here.
             numberOfLaunched = 0;
          }
        }
        tempDeviceData[3] = numberOfLaunched;
        status = tempDeviceData[4];

        /*for(char i=0; i<5; i++)
        {
          Serial.print((int)tempDeviceData[i]);
          Serial.print("  ");
        }
        Serial.println();*/

      }
      else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      }
      // Free resources
      http.end();
    }
    else 
    {
      //Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
  }
}


void num2Char (void)
{
    for(char i=0,c = 0; i<5; i++, c+=2)
    {
        char temp = tempDeviceData[i]>>4;
        if(temp<10)
        {
            deviceData[c] = 0x30+temp;
        }
        else 
        {
            deviceData[c] = 0x41 + (temp-10);
        }

        temp = tempDeviceData[i] & 0x0F;
        if(temp<10)
        {
            deviceData[c+1] = 0x30+temp;
        }
        else 
        {
            deviceData[c+1] = 0x41 + (temp-10);
        }
    }
}

void char2Num (void)//use atResponse;
{
    char temp = 0;
    for(char i=0, c=0; i<9; i+=2, c++)
    {
        if(responseData[i] <= 0x39)//for number
        { 
            temp = (responseData[i] - 0x30)<<4;
        }
        else
        {
            if(responseData[i] <= 0x46)//capital A,B,C
            {
                temp = ((responseData[i] - 0x41)+10)<<4;
            }
            else//small a, b, c
            {
                temp = ((responseData[i] - 0x61)+10)<<4;
            }
        }

        tempDeviceData[c] = temp;
        
        if(responseData[i+1] <= 0x39)//for number
        { 
            temp = (responseData[i+1] - 0x30);
        }
        else
        {
            if(responseData[i+1] <= 0x46)//capital A,B,C
            {
                temp = ((responseData[i+1] - 0x41)+10);
            }
            else//small a, b, c
            {
                temp = ((responseData[i+1] - 0x61)+10);
            }
        }
        tempDeviceData[c] |= temp;
        
    }
}