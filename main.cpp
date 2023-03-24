#include "mbed.h"
#include "TextLCD.h"
//可以跑通的版本
// TextLCD 
TextLCD lcd(p15, p16, p17, p18, p19, p20);//, TextLCD::LCD20x4); // rs, e, d4-d7

// Read from switches and keypad
BusOut rows(p26,p25,p24);
BusIn cols(p14,p13,p12,p11);

// Control external LEDs
SPI sw(p5, p6, p7);
DigitalOut cs(p8); 

DigitalOut Alarm_LED(LED1);

Ticker light, monitorZones, blink, blinkk, bright;
Timer exit_s, entry_s, alarm_s;
int led_bits = 0;   // global LED status used for readback
int fail = 0; // number of failed attempts
int error_fail = 0; // what error
int attempts = 0; // number of remaind attempts
int s = 0; // Is the alarm LED still on in alarm mode?
int kk = 0;
int cc = 0;
int dd = 0;
int ee = 0;
enum State {UNSET, EXIT, SET, ENTRY, ALARM, REPORT} securityState; // State machine setting
const char* StateNames[] = {"UNSET", "EXIT", "SET", "ENTRY", "ALARM", "REPORT"}; // In order to display the current status on the lcd screen conveniently
char Keytable[] = {'F','E','D','C','3','6','9','B','2','5','8','0','1','4','7','A'}; //Keyboard content
char getKey(){
    int i,j;
    char ch=' ';

    for(i=0;i<=3;i++){
        rows=i;
        for(j=0;j<=3;j++){
            if (((cols^0x00FF)&(0x0001<<j))!=0){
                ch = Keytable[(i*4)+j];
            }
        }
    }
    return ch;
}
char keyCode[4] = {'1','2','3','4'}; // Correct password
char keyCode_u[4] = {'1','2','3','4'}; // User input password
// Check which switch the user is currently using
int read_switch()
{
    rows = 4;
    int switches = cols;
    rows = 5;
    switches = switches*16 + cols;
    
    return switches;
}
// Connect 8 switches and 8 LEDs
int switchleds()
{
    int switches = read_switch();
        
    for(int i=0;i<=7;i++){
        if ((switches & 0x0001<<i)!=0){                         // 1, then turn on 
            led_bits  = led_bits | (0x0003 << i*2); }
        else {                                                  // 0, then turn off
            led_bits  = led_bits & ((0x0003 << i*2) ^ 0xffff); }
    }
    sw.write(led_bits); 
    cs = 1;
    cs = 0;

    return switches;
}
// Initialize keyboard-related functions
void readKeypad();
void setKey();
// Initialize the LED
void initleds() {
    cs = 0;                                        // latch must start low
    sw.format(16,0);                               // SPI 16 bit data, low state, high going clock
    sw.frequency(1000000);                         // 1MHz clock rate
}
// Together, these two functions connect the current state to the LED
void setleds(int ledall) {
    led_bits = ledall;                              // update global LED status
    sw.write((led_bits & 0x03ff) | ((led_bits & 0xa800) >> 1) | ((led_bits & 0x5400) << 1));
    cs = 1;                                        // latch pulse start 
    cs = 0;                                        // latch pulse end
}

void setled(int ledno, int ledstate) {
    ledno = 9 - ledno;
    ledno = ((ledno - 1) & 0x0007) + 1;             // limit led number
    ledno = (8 - ledno) * 2;                        // offset of led state in 'led_bits'
    ledstate = ledstate & 0x0003;                   // limit led state
    ledstate = ledstate << ledno;
    int statemask = ((0x0003 << ledno) ^ 0xffff);   // mask used to clear led state
    led_bits = ((led_bits & statemask) | ledstate); // clear and set led state
    setleds(led_bits);
}
// Initialize the alarm stste function
void alarmState();
// Loop through the user's input to the switch
void checkSwitches(){
    initleds();
    int switches = switchleds();
    kk = switchleds();
    //if ((securityState != ALARM) && (securityState != UNSET)&&(securityState != REPORT)&&(switches == 64)){
    if ((securityState != ALARM) && (securityState != UNSET)&&(securityState != REPORT)&&(switches != 128)&&(switches !=0)){
        error_fail = 4;
        cc = 1;
        fail = 0;
        securityState = ALARM;
        exit_s.stop();
        exit_s.reset();
        entry_s.stop();
        entry_s.reset();
        lcd.cls();
        alarmState();
        
    }
    // else if ((securityState == SET)&&(switches == 128)){
    //     securityState = ENTRY;
    //     lcd.cls();
    //     lcd.locate(0,0);
    //     lcd.printf("STATE: ENTRY"); 
    // }
}
// Determine which LED should be lit in the current state
void which_light(){
    switch(securityState)
        {
            case UNSET :
                setled(1,3);
                break;

            case EXIT :
                setled(2,3);
                break;

            case SET :
                setled(3,3);
                break;

            case ENTRY :
                setled(4,3);
                break;

            case ALARM :
                setled(5,3);
                break;
                
            case REPORT :
                setled(6,3);
                break;
      
        }
}
// unset state function
void unsetState(){
    //dd = 0;
    lcd.locate(0,0);
    lcd.printf("STATE: UNSET"); 
    setled(1,3);
    readKeypad();
}
// exit state function
void exitState(){
    exit_s.start();
    lcd.locate(0,0);
    lcd.printf("STATE: EXIT"); 
    setled(2,3);
    readKeypad();

}
// set state function
void setState(){
    lcd.locate(0,0);
    lcd.printf("STATE: SET");
    setled(3,3);
    if(kk == 128){
        fail = 0;
        error_fail = 4;
        securityState = ENTRY;
        lcd.cls();  
    }

}
// entry state function
void entryState(){
    entry_s.start();
    lcd.locate(0,0);
    lcd.printf("STATE: ENTRY");
    setled(4,3);
    readKeypad();

}
//alarm state function
void alarmState(){
    //ee = 0;
    if (s ==0){
        alarm_s.start();
    }
    
    lcd.locate(0,0);
    lcd.printf("STATE: ALARM");
    setled(5,3);
    readKeypad();
}
//report state function
void reportState(){
    int report_clear;
    if (error_fail == 3){
        lcd.locate(0,0);
        lcd.printf("Error: CODE");
        lcd.locate(0, 1);
        lcd.printf("C key to clear");
        report_clear = getKey();
        wait_ms(100);
        if (report_clear == 'C'){
            securityState = UNSET;
            lcd.cls();
        }
    }
    else if (error_fail == 5) {
        lcd.locate(0,0);
        lcd.printf("Error: TIMEOUT");
        lcd.locate(0, 1);
        lcd.printf("C key to clear");
        report_clear = getKey();
        wait_ms(100);
        if (report_clear == 'C'){
            securityState = UNSET;
            lcd.cls();
        }
    }
    else if (error_fail == 4){
        lcd.locate(0,0);
        lcd.printf("Error: FULLSET");
        lcd.locate(0, 1);
        lcd.printf("C key to clear");
        report_clear = getKey();
        wait_ms(100);
        if (report_clear == 'C'){
            securityState = UNSET;
            lcd.cls();
        }
    }
    setled(6,3);
}
// The entire function of keyboard input
void setKey(void){
    int a;
    char b = ' '; 
    cc = 0;
    dd = 0;
    ee = 0;
    b=getKey();
    wait_ms(100);
    lcd.cls();
    lcd.locate(0,0);
    lcd.printf("STATE: %s", StateNames[securityState]); 
    lcd.locate(0,1);
    lcd.printf("Code:____");
    for(a=0;a<4;a++){
        b=getKey();
        wait_ms(100);
        if(dd == 1){
            dd = 0;
            lcd.cls();
            return;
        }
        else if(ee == 1){
            ee = 0;
            lcd.cls();
            return;
        }
        else if(cc == 1){
            cc = 0;
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
        }
    }
    b = '1';
    b=getKey();
    wait_ms(100);
    while (b != 'B'){
        if(dd == 1){
            dd = 0;
            lcd.cls();
            return;
        }
        else if(ee == 1){
            ee = 0;
            lcd.cls();
            return;
        }
        else if(cc == 1){
            cc = 0;
            lcd.cls();
            return;
        }
        lcd.locate(0,1);
        lcd.printf("Press B to set");
        b=getKey();
        wait_ms(100);
    }

    if((keyCode[0] == keyCode_u[0])&&(keyCode[1] == keyCode_u[1])&&(keyCode[2] == keyCode_u[2])&&(keyCode[3] == keyCode_u[3])){
        fail = 0;
        if(securityState == UNSET){
            securityState = EXIT;
            lcd.cls();
        }
        else if(securityState == EXIT){
            securityState = UNSET;
            exit_s.stop();
            exit_s.reset();
            Alarm_LED = 0;
            lcd.cls();
        }
        else if(securityState == ENTRY){
            securityState = UNSET;
            entry_s.stop();
            entry_s.reset();
            Alarm_LED = 0;
            lcd.cls();
        }
        else if(securityState == ALARM){
            securityState = REPORT;
            s = 0;
            alarm_s.stop();
            alarm_s.reset();
            lcd.cls();
            reportState();
        }
    }
    else {
        fail = fail+1;
        attempts = 3-fail;
        if(fail == 3){
            error_fail = fail;
            fail = 0;
            securityState = ALARM;
            lcd.cls();
            alarmState();
        }
        else{
            //error_fail = 0;
            lcd.locate(0,1);
            lcd.printf("Remain attempt:%d", attempts);
            wait(1);
            lcd.cls();
        }
    }
}
// Determine whether the current user is using the keyboard
void readKeypad(){
    char b = ' ';
    b=getKey();
    wait_ms(100);
    if (b != ' '){
        setKey();
    }
}
// LED blinking function for exit state
void blinking(){
    if ((exit_s.read()<=10) && (securityState == EXIT)){
        Alarm_LED = !Alarm_LED;
    }
    else if((exit_s.read()>10) && (securityState == EXIT)){
        dd = 1;
        fail = 0;
        exit_s.stop();
        exit_s.reset();
        securityState = SET;
        Alarm_LED = 0;
        lcd.cls();
        setState();
    }
    
}
// LED blinking function for entry state
void blinking_2(){
    if ((entry_s.read()<=10) && (securityState == ENTRY)){
        Alarm_LED = !Alarm_LED;
    }
    else if((entry_s.read()>10) && (securityState == ENTRY)){
        ee = 1;
        fail = 0;
        error_fail = 5;
        entry_s.stop();
        entry_s.reset();
        securityState = ALARM;
        lcd.cls();
        alarmState();
    }
    
}
// The alarm status LED lights up function
void brighting(){
    if ((s ==0)&&(securityState == ALARM)){
        alarm_s.start();
        Alarm_LED = 1;
        if(alarm_s.read()>20){
            Alarm_LED = 0;
            alarm_s.stop();
            alarm_s.reset();
            s = 1;
        }
    }
}
// main function
int main(){
    lcd.cls();
    securityState = UNSET; //first state
    monitorZones.attach(&checkSwitches, 0.1); //check zones every 100ms    
    light.attach(&which_light, 0.05);
    blink.attach(&blinking, 0.5);
    blinkk.attach(&blinking_2, 0.5);
    bright.attach(&brighting, 0.1);
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
      
        }
    }
        

}