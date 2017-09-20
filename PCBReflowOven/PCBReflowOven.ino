#include "Adafruit_MAX31855.h"
#include <LiquidCrystal.h>

int thermoCS  = 10; //SS
int thermoDO  = 12; //MISO
int thermoCLK = 13; //SCLK

int mode      = 0;
int startstop = 1;
int relay     = 2;

int Est_Time = 121;

LiquidCrystal lcd(A0, A1, A2, A3, A4, A5);
Adafruit_MAX31855 thermocouple(thermoCLK, thermoCS, thermoDO);

int reflow_State = 0;
String StateNames[6] = {
  " Begin", " Ramp", " Soak", "Reflow", " Cool", " Done"
  };

int displayed_Profile = 0;
String ProfileNames[3] = {
  "Pb Solder       ", "RoHS            ", "Profile 3       "
  };
int Profile[3][6] = {
  {75,140,45,125,205,20},
  {75,140,45,125,205,20},
  {75,140,45,125,205,20},
  };

boolean toaster_running = 0;
boolean pressed_StartorStop = 0;
boolean pressed_Mode = 0;

// Variables changed within interuppt
volatile float  seconds  = 0;  // this will get incremented once a second
volatile long   count    = 0;
volatile float  temp    = 0;

void setup() {  
  Serial.begin(9600); 
  lcd.begin(16, 2);
  lcd.print("PCB Reflow Oven");
  lcd.setCursor(0,1);
  lcd.print("  Stride Tech  ");
  delay(1000);
  
  lcd.clear();
  lcd.print("Select Profile:");
  lcd.setCursor(0,1);
  lcd.print("   ");
  lcd.print(ProfileNames[displayed_Profile]);

  pinMode(relay, OUTPUT);
  digitalWrite(relay, LOW);

  pinMode(startstop, INPUT);
  digitalWrite(startstop, HIGH);

  pinMode(mode, INPUT);
  digitalWrite(mode,HIGH);

  //16b Timer1 - CTC Mode w/ 1 Hz
  TCCR1A = 0;                           
  TCCR1B = _BV(WGM12) | _BV(CS10) | _BV(CS12);   
  OCR1A = 3902;                                
  TIMSK1 = _BV(OCIE1A); 
}


SIGNAL(TIMER1_COMPA_vect) { 
  float new_temp;
  float prev_temp = temp;

  count ++;
  seconds = (float) count / 4;

  new_temp = thermocouple.readCelsius();
  if(isnan(new_temp)){
    temp = prev_temp; 
    }
  else{
    temp = new_temp;
    } 

  //line 1
  lcd.setCursor(0,0);
  lcd.print("Temp:");  
  lcd.setCursor(5,0);
  lcd.print(temp);
    if(temp < 99)
      lcd.setCursor(7,0);
    else
      lcd.setCursor(8,0);
  lcd.print((char)223); // deg symbol
  lcd.print("C"); 
  lcd.print(StateNames[reflow_State]);
  
  //line 2
  lcd.setCursor(0,1);
  lcd.print("Targ:");  
  lcd.setCursor(5,0);
  lcd.print(Profile[displayed_Profile][reflow_State]);
    if(Profile[displayed_Profile][reflow_State] < 99)
      lcd.setCursor(7,0);
    else
      lcd.setCursor(8,0);
  lcd.print((char)223);
  lcd.print("C  "); 
  Est_Time--;
  lcd.print(Est_Time); 


    
  switch(reflow_State){
    case 0:
      digitalWrite(relay, LOW);
     break;  

    case 1:
      analogWrite(relay, Profile[displayed_Profile][0]);
        if(temp >= Profile[displayed_Profile][1]){
         reflow_State = 2;
          count = 0;  
          }
      break;  

    case 2:
      if(temp < Profile[displayed_Profile][1])
        digitalWrite(relay, HIGH);
      else
        digitalWrite(relay, LOW);
      if(seconds >= Profile[displayed_Profile][2]){
        reflow_State = 3;  
         }
      break;  

    case 3:
      analogWrite(relay,Profile[displayed_Profile][3]);
         if(temp >= Profile[displayed_Profile][4]){
           reflow_State = 4;
           count = 0;  
           }
      break;  

     case 4:
       if(temp < Profile[displayed_Profile][4])
         digitalWrite(relay, HIGH);
       else
         digitalWrite(relay, LOW);
       if(seconds >= Profile[displayed_Profile][5]){
          reflow_State = 0;  
           digitalWrite(relay, LOW);
         }
        break;  

      default:
          digitalWrite(relay, LOW);
          reflow_State = 0;
          count = 0;
         break;  
     }   

  // Start/Stop the Profile 
  if(digitalRead(startstop) == LOW && !pressed_StartorStop){
    pressed_StartorStop = 1;
    
    if(toaster_running == 1){
      toaster_running = 0;
      reflow_State = 0;
      digitalWrite(relay, LOW);
     }
    else{
      toaster_running = 1; 
      reflow_State = 1;
      } 
    }

  
  if(digitalRead(startstop) == HIGH)
    pressed_StartorStop = 0;

  //Cycles Profile Menu
  if(digitalRead(mode) == LOW && !pressed_Mode){
    pressed_Mode = 1;
    if(displayed_Profile < 2)
      displayed_Profile++;
    else
      displayed_Profile = 0;

     lcd.setCursor(0,1);
     lcd.print("   ");
     lcd.print(ProfileNames[displayed_Profile]);
     }  
  
  if(digitalRead(mode) == HIGH)
    pressed_Mode = 0;
    }

void loop(){}
