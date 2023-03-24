/* @Copyright University of Essex 2017
 * @Author: NicoleYu wy19403@essex.ac.uk
 * @Date: 2023-03-03 22:20:33
 * @LastEditors: NicoleYu wy19403@essex.ac.uk
 * @LastEditTime: 2023-03-22 16:11:55
 * @FilePath: \mbed-os-example-blinky\assignment.cpp
 * @FILE - main.c
        mbed LPC1768 application.
 * @DESCRIPTION
        hardware: mbed LPC1768, an extension board.
        Use a terminal program such as 'HyperTerminal' to communicate with the application.
        The serial port configuration is shown below.
            9600 Baud 
            8 data bits 
            No parity bits 
            One stop bit 
            No hardware flow control    
==============================================================================*/
//Assignment提交的版本
#include "mbed.h"
#include "TextLCD.h"

//---------Board initialization declaration----------
TextLCD lcd(p15, p16, p17, p18, p19, p20); // Create a TextLCD object with pins for rs, e, d4-d7
BusOut rows(p26,p25,p24);// Create a TextLCD object with pins for rs, e, d4-d7
BusIn cols(p14,p13,p12,p11);// Create a BusIn object with pins for columns
SPI sw(p5, p6, p7);// Create an SPI object with pins for mosi, miso and sclk
DigitalOut cs(p8); // Create a DigitalOut object with pin for cs
DigitalOut boardLed(LED1);// Create a DigitalOut object with pin for boardLed

Ticker myTicker;// Create Ticker objects for light, monitorZones, blink, blinkk and bright 
Timer exitPeriod, entryPeriod, alarmPeriod;// Create Timer objects for exitPeriod, entryPeriod and alarmPeriod

char Keytable[] = { 'F', 'E', 'D', 'C',   
                    '3', '6', '9', 'B',   
                    '2', '5', '8', '0',   
                    '1', '4', '7', 'A'    
                   }; 

//----------State declaration--------------------------                   
enum State {UNSET, EXIT, SET, ENTRY, ALARM, REPORT} securityState; // State machine setting
const char* StateNames[] = {"UNSET", "EXIT", "SET", "ENTRY", "ALARM", "REPORT"}; // In order to display the current status on the lcd screen conveniently

//----------Personalization declaration------------------
uint8_t keyCode[4] = {'1','2','3','4'}; // Correct password
uint8_t keyCode_u[4] = {'1','2','3','4'}; // User input password

//----------Intermediate variable declaration-----------------
int judge=0;//'judge' variable used to judge the state. 0=unset, 1=other, 2=unset, 3=alarm.
int gobalLed = 0;// gobalLed is ALARMLED on the development board LED 
int counter = 0; // counter is 4-digit code attempt times, and can not more than 3.

//-----------Function declaration-----------------
void initLeds();
char getKey();
int getSwitch();
void readKeypad();
bool compareArrays(const uint8_t* keyCode, const uint8_t* keyCode_u);
void ledBit(int order);
void checkSwitches();
void setKey(void);
void stateLed();
void ledCall();
void unsetState();
void exitState();
void setState();
void entryState();
void alarmState();
void reportState();
int main();

//-------------------Code begining----------------------------

// Initialize the LED
void initLeds() {

    cs = 0;// latch must start low
    sw.format(16,0);// SPI 16 bit data, low state, high going clock
    sw.frequency(1000000);// 1MHz clock rate

}

char getKey(){
    int i,j;
    char ch=' ';
    
    for (i = 0; i <= 3; i++) {
        rows = i; 
        for (j = 0; j <= 3; j++) {           
            if (((cols ^ 0x00FF)  & (0x0001<<j)) != 0) {
                ch = Keytable[(i * 4) + j];
            }            
        }        
    }
    return ch;
}

void readKeypad(){
    char b = ' ';
    b=getKey();
    wait_ns(10000);
    if (b != ' '){
        setKey();
    }
}

// Verify that the password is correct
bool compareArrays(const uint8_t* keyCode, const uint8_t* keyCode_u) {
    for (int i = 0; i < 3; i++) {
        if (keyCode[i] != keyCode_u[i]) {
            return 0;
        }
    }
    return 1;
}
//controlling the state of one or more LEDs using a shift register. 
void ledBit(int order) {

    order = (((8-order) & 0x0007) + 1) * 2;             //offset of led state in 'gobalLed'
    gobalLed = gobalLed & ((0x0003 << order) ^ 0xffff); // clear and set led state
    sw.write((gobalLed & 0x03ff) | ((gobalLed & 0xa800) >> 1) | ((gobalLed & 0x5400) << 1));
    cs = 1;                                        // latch pulse start 
    cs = 0;
}

void checkSwitches(){
    initLeds();
    int switches = getSwitch();
    if ((securityState != UNSET)&& (securityState != ALARM) && (securityState != REPORT)&&(num != 0x80)&&(num !=0x00)){
        
        judge = 1;
        counter = 0;
        securityState = ALARM;
        exitPeriod.stop();
        exitPeriod.reset();
        lcd.cls();
        alarmState();
    }
}

int getSwitch()
{
    int switches = cols;
    switches = switches*17 
    for(int i=0;i<=7;i++){
        if ((switches & 0x0001<<i)!=0){                    
            gobalLed  = gobalLed | (0x0003 << i*2); }
        else {                                            
            gobalLed  = gobalLed & ((0x0003 << i*2) ^ 0xffff); }
    }
    sw.write(gobalLed); 
    cs = 1;
    cs = 0;
    return switches;
    //([1])
}

void setKey(void){
    int a;
    char b = ' ';
    judge =0;

    b=getKey();
    wait_ns(10000);
        
    lcd.cls();
    lcd.locate(0,0);
    lcd.printf("STATE: %s", StateNames[securityState]); 
    lcd.locate(0,1);
    lcd.printf("Code:____");
    
    for(a=0;a<4;a++){
        
        b=getKey();
        wait_ns(10000);
    
        if (judge != 0){
            judge = 0;
            lcd.cls();
            return;
        }

        switch(b){
            case ' ':
                a--;
                break;
            case 'C':
                if(a>0){
                    a=a-2;
                    lcd.locate(6+a,1);
                    lcd.putc('_');
                }
                else if(a==0){
                    a--;
                }
                break;
            case 'B':
                default:
                lcd.locate(5+a,1);
                lcd.putc('*');
                keyCode_u[a]=b;
                break;
        }    // [3]
    }

    b=getKey();
    wait_ns(10000);

    while (b != 'B'){

        if (judge != 0){
            judge = 0;
            lcd.cls();
            return;
        }
        lcd.locate(0,1);
        lcd.printf("Press B to set");
        b=getKey();
        wait_ns(100);
    }

    if(bool isEqual = compareArrays(keyCode, keyCode_u)){

        if(securityState == UNSET){
            securityState = EXIT;
            lcd.cls();
        }

        else if(securityState == EXIT){
            securityState = UNSET;
            exitPeriod.stop();
            exitPeriod.reset();
            boardLed = 0;
            lcd.cls();
        }

        else if(securityState == ENTRY){
            securityState = UNSET;
            entryPeriod.stop();
            entryPeriod.reset();
            boardLed = 0;
            lcd.cls();
        }

        else if(securityState == ALARM){
            securityState = REPORT;
            alarmPeriod.stop();
            alarmPeriod.reset();
            lcd.cls();
            reportState();
        }
        counter = 0;
    }

    else {
        counter = counter++;
        if(counter >= 3){

            counter = 0;
            securityState = ALARM;
            lcd.cls();
            alarmState();
        }
        else{

            lcd.locate(0,1);
            lcd.printf("Remain attempt:%d", 3-counter);
            wait_ns(10000);
        }
    }
}

void stateLed(){

    switch(securityState)
        {
            case UNSET :
                ledBit(1);
                break;

            case EXIT :
                ledBit(2);
                break;

            case SET :
                ledBit(3);
                break;

            case ENTRY :
                ledBit(4);
                break;

            case ALARM :
                ledBit(5);
                break;
                
            case REPORT :
                ledBit(6);
                break;
      
        }
}
//controlling the sort of security system with LED lights and an LCD screen. 
void ledCall(){
    if ((exitPeriod.read()<=10) && (securityState == EXIT) &&(securityState == ENTRY)){
        boardLed = !boardLed;
    }

    else if((exitPeriod.read()>10)){

        exitPeriod.stop();
        exitPeriod.reset();
        counter = 0;

        if (securityState == EXIT){
            judge = 2;
            securityState = SET;
            boardLed = 0;
            lcd.cls();
            setState();
        }
        else if(securityState == ENTRY){
            judge = 3;
            securityState = ALARM;
            lcd.cls();
            alarmState();
        }
        
    }
    else if (securityState == ALARM){
        alarmPeriod.start();
        boardLed = 1;
        if(alarmPeriod.read()>20){
            boardLed = 0;
            alarmPeriod.stop();
            alarmPeriod.reset();
        }
    }
}
// unset state function
void unsetState(){
    lcd.locate(0,0);
    lcd.printf("STATE: UNSET"); 
    ledBit(1);
    readKeypad();
}
// exit state function
void exitState(){
    exitPeriod.start();
    lcd.locate(0,0);
    lcd.printf("STATE: EXIT"); 
    ledBit(2);
    readKeypad();
}
// set state function
void setState(){
    lcd.locate(0,0);
    lcd.printf("STATE: SET");
    ledBit(3);
    if(num == 0x80){
        counter = 0;
        securityState = ENTRY;
        lcd.cls();  
    }

}
// entry state function
void entryState(){
    entryPeriod.start();
    lcd.locate(0,0);
    lcd.printf("STATE: ENTRY");
    ledBit(4);
    readKeypad();
}
//alarm state function
void alarmState(){

    alarmPeriod.start();
    lcd.locate(0,0);
    lcd.printf("STATE: ALARM");
    ledBit(5);
    readKeypad();
}
//report state function
void reportState(){
    char cls=' ';
    if ( (checkSwitches() & 0x03) != 0 )     
         lcd.printf("full set error");

	if ( (checkSwitches() & 0x0c) != 0 )     
          lcd.printf("set error"); 

	if ( (checkSwitches() & 0x30) != 0 )     
          lcd.printf("entry error");

	if ( (checkSwitches() & 0xc0) != 0 )     
          lcd.printf("exit error");
    else     
          lcd.printf("code error");

    lcd.locate(0,1);
    lcd.printf("C key to clear");
    cls = getKey();
    wait_ns(10000);

    if (cls == 'C'){
        securityState = UNSET;
        lcd.cls();
    }

    ledBit(6);
}

int main() {
 
    initLeds(); //initialize leds
    lcdRefresh(); // initial lcd clear
    securityState = UNSET; //initial state
    monitorZones.attach_us(&checkSwitches, 0.1); //check zones every 100ms    
    myTicker.attach(&ledCall, 0.1);// check LED 

    while(1)
    {
        // State machine judgment
        switch(securityState)
        {
            case UNSET :
                unsetState();
                break;

            case EXIT :
                exitState();
                break;

            case SET :
                setState();
                break;

            case ENTRY :
                entryState();
                break;

            case ALARM :
                alarmState();
                break;
                
            case REPORT :
                reportState();
                break;
            //[4]
        }
    }
        
}

// reference
// [1]"home_alarm_simple - This is a simplified version of home alarm system… | Mbed," mbed import http://os.mbed.com/users/liruihao2008/code/home_alarm_simple/.
// [2]"CE323_Lab4"
// [3]"CE323_Lab6"
// [4]"CE323_Lab7"