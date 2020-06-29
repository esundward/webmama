/*********
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp32-esp8266-input-data-html-form/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*********/

#include <Arduino.h>
#ifdef ESP32
  #include <WiFi.h>
  #include <AsyncTCP.h>
  #include <SPIFFS.h>
#else
  #include <ESP8266WiFi.h>
  #include <ESPAsyncTCP.h>
  #include <Hash.h>
  #include <FS.h>
#endif
#include <ESPAsyncWebServer.h>

AsyncWebServer server(80);

// REPLACE WITH YOUR NETWORK CREDENTIALS
const char* ssid = "XXXwebinterface";
const char* password = "YYY";

const char* PARAM_STRING = "inputString";
const char* COCKTAILNAME = "cocktailname";
uint8_t pour_volume[]={10, 11, 12, 13, 14};
uint8_t dispenser_volume[]={20, 20, 20, 40, 40};
String ingredient_name[]={"bottle0","bottle1","bottle2","bottle3","bottle4"};
//
// POURVOLUME
//
//storage files for the active pour volume
#define STORAGE_POURVOLUME0_TXT "pv0.txt"
#define STORAGE_POURVOLUME1_TXT "pv1.txt"
#define STORAGE_POURVOLUME2_TXT "pv2.txt" 
#define STORAGE_POURVOLUME3_TXT "pv3.txt"
#define STORAGE_POURVOLUME4_TXT "pv4.txt" 

#define NAME_GET_POURVOLUME0 "pv0"
#define NAME_GET_POURVOLUME1 "pv1"
#define NAME_GET_POURVOLUME2 "pv2"
#define NAME_GET_POURVOLUME3 "pv3"
#define NAME_GET_POURVOLUME4 "pv4"
//storage files for the active dispenser volume
#define DV0TXT "dv0.txt" 
#define DV1TXT "dv1.txt"
#define DV2TXT "dv2.txt"
#define DV3TXT "dv3.txt"
#define DV4TXT "dv4.txt"

#define NAME_GET_DISPENSERVOLUME0 "dv0"
#define NAME_GET_DISPENSERVOLUME1 "dv1"
#define NAME_GET_DISPENSERVOLUME2 "dv2"
#define NAME_GET_DISPENSERVOLUME3 "dv3"
#define NAME_GET_DISPENSERVOLUME4 "dv4"

#define CHECKBOXCHECKED "checked"
#define CHECKBOXUNCHECKED "non-checked"

//storage files for the  igrendient names

#define NAME_GET_INGREDIENTNAME0 "in0"
#define NAME_GET_INGREDIENTNAME1 "in1"
#define NAME_GET_INGREDIENTNAME2 "in2"
#define NAME_GET_INGREDIENTNAME3 "in3"
#define NAME_GET_INGREDIENTNAME4 "in4"



#define IN0TXT in0.txt 
#define IN1TXT in1.txt 
#define IN2TXT in2.txt 
#define IN3TXT in3.txt 
#define IN4TXT in4.txt 


void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

String readFile(fs::FS &fs, const char * path){
  Serial.printf("Reading file: %s\r\n", path);
  File file = fs.open(path, "r");
  if(!file || file.isDirectory()){
    Serial.println("- empty file or failed to open file");
    return String();
  }
  Serial.println("- read from file:");
  String fileContent;
  while(file.available()){
    fileContent+=String((char)file.read());
  }
  Serial.println(fileContent);
  return fileContent;
}

void writeFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Writing file: %s\r\n", path);
  File file = fs.open(path, "w");
  if(!file){
    Serial.println("- failed to open file for writing");
    return;
  }
  if(file.print(message)){
    Serial.println("- file written");
  } else {
    Serial.println("- write failed");
  }
}

// 
// Placholder Processor
//
// replaces placeholder with stored values

int bottle_num_from_placeholder(const String& var){
  return(var.charAt(2)-'0');
}

String processor(const String& var){
if(var.charAt(0)=='d')
  return(processor_dispenser_checkbox(var));
else if (var.charAt(0)=='i')
  return(processor_ingredientname(var));
else
  return(processor_pourvolume(var));
}
String processor_pourvolume(const String& var){
  // placholder pvX
  // x number of botlle
  int bottle_num;
  bottle_num=bottle_num_from_placeholder(var);
  return(String(pour_volume[bottle_num]));
 
}

String processor_dispenser_checkbox(const String& var){
  // placeholder dvXYY
  // X - num of botlle
  // YY - dispenser volume
  int bottle_num;
  bottle_num=bottle_num_from_placeholder(var);
  int dispenservolumehtmlvar=var.substring(3,5).toInt();
    if (dispenser_volume[bottle_num]== dispenservolumehtmlvar){
      return(String(CHECKBOXCHECKED));
    }
    else{
      return(String(CHECKBOXUNCHECKED));
    }
 
}
String processor_ingredientname(const String& var){
  // placeholder inX[s]
  // s = placeholder from submit form
  int bottle_num;
  bottle_num=bottle_num_from_placeholder(var);
  // return ingredientname with quotes, necessary for html formating inside the submit form
  if(var.charAt(3)=='v'){
    String igredientnamewithquotes="\""+ingredient_name[bottle_num]+"\"";
    return(igredientnamewithquotes);
  }
  // return ingredientname without quotes
  else{
    return(ingredient_name[bottle_num]);
  }
  
}

  
//
// Store values  
//
void read_values(String prefix,uint8_t * volumearray){
	for (int i=0; i<5;i++){
	String filename=prefix+i+".txt";
	volumearray[i]=readFile(SPIFFS,filename.c_str()).toInt();
	}
}

void write_values(String prefix,uint8_t *valuearray){
	for (int i=0; i<5;i++){
	String filename=prefix+i+".txt";
	writeFile(SPIFFS,filename.c_str(),String(valuearray[i]).c_str());
	}
}


void setup() {
  Serial.begin(115200);
  // Initialize SPIFFS
  #ifdef ESP32
    if(!SPIFFS.begin(true)){
      Serial.println("An Error has occurred while mounting SPIFFS");
      return;
    }
  #else
    if(!SPIFFS.begin()){
      Serial.println("An Error has occurred while mounting SPIFFS");
      return;
    }
  #endif

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("WiFi Failed!");
    return;
  }
  Serial.println();
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // index.html
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
  server.on("/index.html", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
  // configure.html
  server.on("/configure.html", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/configure.html", String(), false, processor);
  });
  // receips.html
  server.on("/receips.html", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/receips.html", "text/html");
  });
  // css - style.css
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/style.css", "text/css");
  });
  // javascript - script.js 
  server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/script.js", "text/javascript");
  });


  // Send a GET request to <ESP_IP>/get?inputString=<inputMessage>

 //
 // DISPENSER VOLUME
 //
  server.on("/updatedispenservolume", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
      //inputMessage = request->getParam(COCKTAILNAME)->value();
      // bottle 1
      int paramsNr = request->params();
      for(int i=0;i<paramsNr;i++){
        inputMessage = request->getParam(i)->value();
        dispenser_volume[i]=inputMessage.toInt();
      }
      write_values("dv",dispenser_volume);

    request->send(SPIFFS, "/configure.html", String(), false, processor);
  });
   //
 // POUR VOLUME
 //
   server.on("/updatepourvolume", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
      //inputMessage = request->getParam(COCKTAILNAME)->value();
      // bottle 1
      inputMessage = request->getParam(NAME_GET_POURVOLUME0)->value();
      pour_volume[0]=inputMessage.toInt();
      inputMessage = request->getParam(NAME_GET_POURVOLUME1)->value();
      pour_volume[1]=inputMessage.toInt();
      inputMessage = request->getParam(NAME_GET_POURVOLUME2)->value();
      pour_volume[2]=inputMessage.toInt();
      inputMessage = request->getParam(NAME_GET_POURVOLUME3)->value();
      pour_volume[3]=inputMessage.toInt();
      inputMessage = request->getParam(NAME_GET_POURVOLUME4)->value();
      pour_volume[4]=inputMessage.toInt();
      
      write_values("pv",pour_volume);

    request->send(SPIFFS, "/configure.html", String(), false, processor);
    //request->send(200);
  });
  //
 // INGREDIENTNAME
 //
   server.on("/updateingredientname", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
      //inputMessage = request->getParam(COCKTAILNAME)->value();
      // bottle 1
      inputMessage = request->getParam(NAME_GET_INGREDIENTNAME0)->value();
      ingredient_name[0]=inputMessage;
      inputMessage = request->getParam(NAME_GET_INGREDIENTNAME1)->value();
      ingredient_name[1]=inputMessage;
      inputMessage = request->getParam(NAME_GET_INGREDIENTNAME2)->value();
      ingredient_name[2]=inputMessage;
      inputMessage = request->getParam(NAME_GET_INGREDIENTNAME3)->value();
      ingredient_name[3]=inputMessage;
      inputMessage = request->getParam(NAME_GET_INGREDIENTNAME4)->value();
      ingredient_name[4]=inputMessage;
//      write_values("in",ingredient_name);

    request->send(SPIFFS, "/configure.html", String(), false, processor);
  });

  server.onNotFound(notFound);
  server.begin();    
  
  read_values("pv",pour_volume);
  read_values("dv",dispenser_volume);
  //read_values("in",ingredient_name);
}

void loop() {

}
