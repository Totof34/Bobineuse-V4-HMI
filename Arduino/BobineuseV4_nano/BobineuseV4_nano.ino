/*
This sketch shows how to send data from the Nextion display to arduino, and vice versa.

I didn't find a reliable way to receive data from the nextion display without using the library 
so I am going to use the official library to receive data, but I am not going to use it to send data to the display
because it can create problems with touch events when we send data in the loop.
I think it's easier to send data to the display without the library, anyway.

Connection with Arduino Uno/Nano:
* +5V = 5V
* TX  = pin 0 (RX)
* RX  = pin 1 (TX)
* GND = GND

If you are going to use an Arduino Mega, you have to edit everything on this sketch that says "Serial"
and replace it with "Serial1" (or whatever number you are using). Also define the Serial port on NexConfig.h
inside the nextion library.


Nextion library: https://github.com/itead/ITEADLIB_Arduino_Nextion


This sketch was made for my new coilwinder with two stepper motor driver

Made by Totof
Last update: 26/04/2025
*/

#include <Arduino.h>
#include <avr/wdt.h>
#include "BasicStepperDriver.h"
#include "MultiDriver.h"
#include "SyncDriver.h"
#include <Nextion.h>  // Include the nextion library (the official one) https://github.com/itead/ITEADLIB_Arduino_Nextion
                      // Make sure you edit the NexConfig.h file on the library folder to set the correct serial port for the display.
                      // By default it's set to Serial1, which most arduino boards don't have.
                      // Change "#define nexSerial Serial1" to "#define nexSerial Serial" if you are using arduino uno, nano, etc.


bool  State = 0; // Create a variable to have a state of the coilwinder to operate a standby
bool  StateMotor = 0; // Create a variable to have a state of the Enable/Disable of the motor
int variable1 = 0;  // Create a variable to have a counter going up by one on each cycle
int counter = 0;  // Create a variable to have a counter for the + and - buttons
int CurrentPage = 0;  // Create a variable to store which page is currently loaded
uint32_t A = 0;  // Create variable to store value we are going to get the width of the coil in mm
uint32_t B = 0;  // Create variable to store value we are going to get the wire diametre in mm
uint32_t C = 0;  // Create variable to store value we are going to get the amount of turns
uint32_t D = 0;  // Create variable to store value we are going to get the type of winding process
uint32_t E = 0;  // Create variable to store value we are going to get the sense of the guidewire
uint32_t F = 0;  // Create variable to store value we are going to get the step for jogging
uint32_t G = 0;  // Create variable to store value we are going to get the step for jogging
uint32_t H = 0;  // Create variable to store value we are going to get the step for jogging
uint32_t I = 0;  // Create variable to store value we are going to get the Enable or Disable state of the motor
uint32_t ST = 0;  // Create variable to store value we are going to get the amount of stepping for jogging
uint32_t K = 0;  // Create variable to store value we are going to get the Speed rotation
int L = 0; // Number of wire for the width of the coil
int PPT = 0; // Pulse per turns for the motor
int SW = -1 ; // sense of winding
int M = 0; // Mode of winding
int EN = 8; // the pin to control the Enable or Disable state of the motor
int TPGW = 450; // The thread pitch of the guide wire 360/pitch because it's décimal value, calcul = (1/0.8)*360 where 0.8 is the pitch
int Xrpm = 10; // Create variable to store value we are going to get the x-axis rpm
int Yrpm = 10; // Create variable to store value we are going to get the y-axis rpm
int Value = 0; // Create variable to store analog reading value
int RpmValue = 0; // Create variable to store value we are going to get the speed rotation

#define DIR_X 2
#define STEP_X 5
#define DIR_Y 4
#define STEP_Y 7
/*
     * Microstepping mode: 1, 2, 4, 8, 16 or 32 (where supported by driver)
     * Mode 1 is full speed.
     * Mode 32 is 32 microsteps per step.
     * The motor should rotate just as fast (at the set RPM),
     * but movement precision is increased, which may become visually apparent at lower RPMs.
     *
     *stepper.setMicrostep(8);   // Set microstep mode to 1:8
     */
#define MICROSTEPS 16
// Motor steps per revolution. Most steppers are 200 steps or 1.8 degrees/step
#define MOTOR_STEPS 200
// Target RPM for X axis motor
#define MOTOR_X_RPM Xrpm*10
// Target RPM for Y axis motor
#define MOTOR_Y_RPM Yrpm*10

int count = 0;
BasicStepperDriver stepperX(MOTOR_STEPS, DIR_X, STEP_X);
BasicStepperDriver stepperY(MOTOR_STEPS, DIR_Y, STEP_Y);
SyncDriver controller(stepperX, stepperY);

// Declare objects that we are going to read from the display. This includes buttons, sliders, text boxes, etc:
// Format: <type of object> <object name> = <type of object>(<page id>, <object id>, "<object name>");
/* ***** Types of objects:
 * NexButton - Button
 * NexDSButton - Dual-state Button
 * NexHotspot - Hotspot, that is like an invisible button
 * NexCheckbox - Checkbox
 * NexRadio - "Radio" checkbox, that it's exactly like the checkbox but with a rounded shape
 * NexSlider - Slider
 * NexGauge - Gauge
 * NexProgressBar - Progress Bar
 * NexText - Text box
 * NexScrolltext - Scroll text box
 * NexNumber - Number box
 * NexVariable - Variable inside the nextion display
 * NexPage - Page touch event
 * NexGpio - To use the Expansion Board add-on for Enhanced Nextion displays
 * NexRtc - To use the real time clock for Enhanced Nextion displays
 * *****
 */

NexButton b0 = NexButton(1, 17, "b0");  // Button added
NexButton b1 = NexButton(1, 18, "b1");  // Button added
NexButton b2 = NexButton(1, 19, "b2");  // Button added
NexButton b3 = NexButton(1, 20, "b3");  // Button added
NexButton b4 = NexButton(1, 21, "b4");  // Button added
NexButton b5 = NexButton(1, 25, "b5");  // Button added
NexButton b20 = NexButton(2, 1, "b20");  // Button added
NexButton b21 = NexButton(2, 4, "b21");  // Button added
NexButton b22 = NexButton(2, 5, "b22");  // Button added
NexButton b23 = NexButton(2, 6, "b23");  // Button added
NexButton b24 = NexButton(2, 7, "b24");  // Button added
NexDSButton bt0 = NexDSButton(1, 33, "bt0");  // Dual state button added
NexDSButton bt1 = NexDSButton(1, 37, "bt1");  // Dual state button added
NexCheckbox c0 = NexCheckbox(1, 11, "c0");  // Checkbox added
NexCheckbox c20 = NexCheckbox(2, 11, "c20");  // Checkbox added
NexCheckbox c21 = NexCheckbox(2, 18, "c21");  // Checkbox added
NexCheckbox c22 = NexCheckbox(2, 20, "c22");  // Checkbox added
NexNumber n0 = NexNumber(1, 4, "n0"); // Number added
NexNumber n2 = NexNumber(1, 8, "n2"); // Number added
NexNumber x0 = NexNumber(1, 14, "x0"); // Number added
NexNumber x3 = NexNumber(1, 22, "x3"); // Number added
NexNumber x1 = NexNumber(1, 15, "x1"); // Number added
NexNumber n1 = NexNumber(1, 27, "n1"); // Number added
NexNumber n3 = NexNumber(1, 30, "n3"); // Number added
NexNumber n4 = NexNumber(1, 40, "n4"); // Number added


// Declare pages:
// Sending data to the display to nonexistent objects on the current page creates an error code sent by the display.
// Any error sent by the display creates lag on the arduino loop because arduino tries to read it, thinking it's a touch event.
// So to avoid this, I am only going to send data depending on the page the display is on.
// That's the reason I want the arduino to know which page is loaded on the display.
// To let arduino know what page is currently loaded, we are creating a touch event for each page.
// On the nextion project, each page most send a simulated "Touch Press Event" in the "Preinitialize Event" section so
// we can register that a new page was loaded.
NexPage page0 = NexPage(0, 0, "page0");  // Page added as a touch event
NexPage page1 = NexPage(1, 0, "page1");  // Page added as a touch event
NexPage page2 = NexPage(2, 0, "page2");  // Page added as a touch event

// End of declaring objects


char buffer[200] = {0};  // This is needed only if you are going to receive a text from the display. You can remove it otherwise.
                         // Further on this sketch I do receive text so that's why I created this buffer.


// Declare touch event objects to the touch event list: 
// You just need to add the names of the objects that send a touch event.
// Format: &<object name>,

NexTouch *nex_listen_list[] = 
{
  &b0,  // Button added
  &b1,  // Button added
  &b2,  // Button added
  &b3,  // Button added
  &b4,  // Button added
  &b5,  // Button added
  &b20,  // Button added
  &b21,  // Button added
  &b22,  // Button added
  &b23,  // Button added
  &b24,  // Button added
  &bt0,  // Dual state button added
  &bt1,  // Dual state button added
  &c0,  // Checkbox added
  &c20,  // Checkbox added
  &c21,  // Checkbox added
  &c22,  // Checkbox added
  &page0,  // Page added as a touch event
  &page1,  // Page added as a touch event
  &page2,  // Page added as a touch event
  NULL  // String terminated
};  // End of touch event list



////////////////////////// Touch events:
// Each of the following sections are going to run everytime the touch event happens:
// Is going to run the code inside each section only ones for each touch event.

void(* reboot) (void) = 0;//déclarer une fonction reboot qui pointe vers l'adresse 0 de la flash


void b0PopCallback(void *ptr)  // Release event for button b1
{
  
  n0.getValue(&A);
  x0.getValue(&B);
  n2.getValue(&C);

  TransformData();
    
}  // End of release event

void b1PopCallback(void *ptr)  // Release event for button b1
{
  
  n0.getValue(&A);
  x0.getValue(&B);
  n2.getValue(&C);

  TransformData();
    
}  // End of release event

void b2PopCallback(void *ptr)  // Release event for button b1
{
  
  n0.getValue(&A);
  x0.getValue(&B);
  n2.getValue(&C);
  n4.getValue(&K);
  bt0.getValue(&D);
  c0.getValue(&E);

  TransformData();
  ChoiceRpm();
      
}  // End of release event




void b4PopCallback(void *ptr)  // Release event for button b1
{
  State = !State;
}  // End of release event





void b3PopCallback(void *ptr)  // Release event for button b0
{
  n0.setValue(0);
  x0.setValue(0);
  n2.setValue(0);
  bt0.setValue(0);
  c0.setValue(0);
  x3.setValue(0);
  x1.setValue(0);
  n1.setValue(0);
  n3.setValue(0);
  n4.setValue(200);

  reboot();
}  // End of press event





void bt0PopCallback(void *ptr)  // Release event for dual state button bt0
{
  
  bt0.getValue(&D);  // Read value of dual state button to know the state (0 or 1)
  
  if(D == 1){  // If dual state button is equal to 1 (meaning is ON)...
    SW = 1;
    Serial.print("n8.val=");
    Serial.print(SW);
    Serial.write(0xff);
    Serial.write(0xff);
    Serial.write(0xff);
  }else{  // Since the dual state button is OFF...
    SW = -1;
    Serial.print("n8.val=");
    Serial.print(SW);
    Serial.write(0xff);
    Serial.write(0xff);
    Serial.write(0xff);
  }
}  // End of release event


void bt1PopCallback(void *ptr)  // Release event for dual state button bt1
{
  
  bt1.getValue(&I);  // Read value of dual state button to know the state (0 or 1)
  
  if(I == 1){  // If dual state button is equal to 1 (meaning is ON)...
    digitalWrite(EN,HIGH);
  }else{  // Since the dual state button is OFF...
    digitalWrite(EN,LOW);
  }
}  // End of release event

void c0PopCallback(void *ptr)  // Press event for checkbox
{
  
  c0.getValue(&E);  // Read the state of the checkbox
  
  if(E == 1){  // If dual state button is equal to 1 (meaning is ON)...
    M = 1;
    Serial.print("n7.val=");
    Serial.print(M);
    Serial.write(0xff);
    Serial.write(0xff);
    Serial.write(0xff);
  }else{  // Since the dual state button is OFF...
    M = 0;
    Serial.print("n7.val=");
    Serial.print(M);
    Serial.write(0xff);
    Serial.write(0xff);
    Serial.write(0xff);
  }
}  // End of press event






void b5PopCallback(void *ptr)  // Press event for "Home" button on page 2
{
  Serial.print("page 2");  // Sending this it's going to tell the nextion display to go to page 2.
  Serial.write(0xff);  // We always have to send this three lines after each command sent to the nextion display.
  Serial.write(0xff);
  Serial.write(0xff);
}  // End of press event


void b20PopCallback(void *ptr)  // Press event for "Send" button on page 2
{
  Serial.print("page 1");  // Sending this it's going to tell the nextion display to go to page 2.
  Serial.write(0xff);  // We always have to send this three lines after each command sent to the nextion display.
  Serial.write(0xff);
  Serial.write(0xff);
  digitalWrite(EN,LOW); // Reactivate the motor by default
}  // End of press event

void b21PopCallback(void *ptr)  // Release event for button b1
{
 stepchoice(); 
 controller.rotate(36*ST,0);
    
}  // End of release event

void b22PopCallback(void *ptr)  // Release event for button b1
{
 stepchoice();
 controller.rotate(-36*ST,0);
    
}  // End of release event

void b23PopCallback(void *ptr)  // Release event for button b1
{
 stepchoice(); 
 controller.rotate(0,45*ST); //36 step * pitch of the thread, here 0.8 so 45
    
}  // End of release event

void b24PopCallback(void *ptr)  // Release event for button b1
{
 stepchoice(); 
 controller.rotate(0,-45*ST); //36 step * pitch of the thread, here 0.8 so 45
    
}  // End of release event

void stepchoice(){

  c20.getValue(&F);
  c21.getValue(&G);
  c22.getValue(&H);

  if(F==1)
{
  ST = 1;
}else if(G==1)
{
  ST = 10;
}else if(H==1)
{
  ST = 100;
}else
{
  ST = 1;
}
}

// Page change event:
void page0PushCallback(void *ptr)  // If page 0 is loaded on the display, the following is going to execute:
{
  CurrentPage = 0;  // Set variable as 0 so from now on arduino knows page 0 is loaded on the display
}  // End of press event


// Page change event:
void page1PushCallback(void *ptr)  // If page 1 is loaded on the display, the following is going to execute:
{
  CurrentPage = 1;  // Set variable as 1 so from now on arduino knows page 1 is loaded on the display
}  // End of press event


// Page change event:
void page2PushCallback(void *ptr)  // If page 2 is loaded on the display, the following is going to execute:
{
  CurrentPage = 2;  // Set variable as 2 so from now on arduino knows page 2 is loaded on the display
}  // End of press event

void TransformData(){
  L = (A*1000)/B;
  PPT = (A*TPGW)/L;
}

void ChoiceRpmPot(){
  Value = analogRead(A7);
  RpmValue = map(Value,0,1023,1,20);
  Serial.print("n4.val=");  // This is sent to the nextion display to set what object name (before the dot) and what atribute (after the dot) are you going to change.
  Serial.print(RpmValue*10);  // This is the value you want to send to that object and atribute mentioned before.
  Serial.write(0xff);  // We always have to send this three lines after each command sent to the nextion display.
  Serial.write(0xff);
  Serial.write(0xff);
}

void ChoiceRpm(){
  Xrpm = int(K/5);
  Yrpm = Xrpm*2;
}


void WindingWostop(){
  if (A > 0 && B > 0 && C > 0) {
    
if (count <= C){

  
for (int i = 0; i <= L; i++){
  if (count > C){
break;
  }
 nexLoop(nex_listen_list);  // Check for any touch event
 while (State == 1)  {
  nexLoop(nex_listen_list);  // Check for any touch event
  }
if (i == 0){
   stepperX.setRPM(Xrpm);
   stepperY.setRPM(Yrpm);
  }
  else if (i == 1){
   stepperX.setRPM(Xrpm*2);
   stepperY.setRPM(Yrpm*2);
  }
  else if (i == 2){
   stepperX.setRPM(Xrpm*3);
   stepperY.setRPM(Yrpm*3);
  }
  else if (i == 3){
   stepperX.setRPM(Xrpm*4);
   stepperY.setRPM(Yrpm*4);
  }
  else if (i == 4){
   stepperX.setRPM(Xrpm*5);
   stepperY.setRPM(Yrpm*5);
  }
  else if (i == 5){
   stepperX.setRPM(Xrpm*6);
   stepperY.setRPM(Yrpm*6);
  }
  else if (i == 6){
   stepperX.setRPM(Xrpm*7);
   stepperY.setRPM(Yrpm*7);
  }
  else if (i == 7){
   stepperX.setRPM(Xrpm*8);
   stepperY.setRPM(Yrpm*8);
  }
  else if (i == 8){
   stepperX.setRPM(Xrpm*9);
   stepperY.setRPM(Yrpm*9);
  }
  else if (i == (L-8)){
   stepperX.setRPM(Xrpm*9);
   stepperY.setRPM(Yrpm*9);
  }
  else if (i == (L-7)){
   stepperX.setRPM(Xrpm*8);
   stepperY.setRPM(Yrpm*8);
  }
  else if (i == (L-6)){
   stepperX.setRPM(Xrpm*7);
   stepperY.setRPM(Yrpm*7);
  }
  else if (i == (L-5)){
   stepperX.setRPM(Xrpm*6);
   stepperY.setRPM(Yrpm*6);
  }
  else if (i == (L-4)){
   stepperX.setRPM(Xrpm*5);
   stepperY.setRPM(Yrpm*5);
  }
  else if (i == (L-3)){
   stepperX.setRPM(Xrpm*4);
   stepperY.setRPM(Yrpm*4);
  }
  else if (i == (L-2)){
   stepperX.setRPM(Xrpm*3);
   stepperY.setRPM(Yrpm*3);
  }
  else if (i == (L-1)){
   stepperX.setRPM(Xrpm*2);
   stepperY.setRPM(Yrpm*2);
  }
  else if (i == L){
   stepperX.setRPM(Xrpm);
   stepperY.setRPM(Yrpm);
  }
  else{
   stepperX.setRPM(Xrpm*10);
   stepperY.setRPM(Yrpm*10);
  }  
controller.rotate(360,PPT*SW);
Serial.print("x3.val=");
Serial.print(count*10);
Serial.write(0xff);
Serial.write(0xff);
Serial.write(0xff);
count++;  
}


for (int i = 0; i <= L; i++){
if (count > C){
break;
  }
nexLoop(nex_listen_list);  // Check for any touch event  
while (State == 1)  {
  nexLoop(nex_listen_list);  // Check for any touch event
  }
if (i == 0){
   stepperX.setRPM(Xrpm);
   stepperY.setRPM(Yrpm);
  }
  else if (i == 1){
   stepperX.setRPM(Xrpm*2);
   stepperY.setRPM(Yrpm*2);
  }
  else if (i == 2){
   stepperX.setRPM(Xrpm*3);
   stepperY.setRPM(Yrpm*3);
  }
  else if (i == 3){
   stepperX.setRPM(Xrpm*4);
   stepperY.setRPM(Yrpm*4);
  }
  else if (i == 4){
   stepperX.setRPM(Xrpm*5);
   stepperY.setRPM(Yrpm*5);
  }
  else if (i == 5){
   stepperX.setRPM(Xrpm*6);
   stepperY.setRPM(Yrpm*6);
  }
  else if (i == 6){
   stepperX.setRPM(Xrpm*7);
   stepperY.setRPM(Yrpm*7);
  }
  else if (i == 7){
   stepperX.setRPM(Xrpm*8);
   stepperY.setRPM(Yrpm*8);
  }
  else if (i == 8){
   stepperX.setRPM(Xrpm*9);
   stepperY.setRPM(Yrpm*9);
  }
  else if (i == (L-8)){
   stepperX.setRPM(Xrpm*9);
   stepperY.setRPM(Yrpm*9);
  }
  else if (i == (L-7)){
   stepperX.setRPM(Xrpm*8);
   stepperY.setRPM(Yrpm*8);
  }
  else if (i == (L-6)){
   stepperX.setRPM(Xrpm*7);
   stepperY.setRPM(Yrpm*7);
  }
  else if (i == (L-5)){
   stepperX.setRPM(Xrpm*6);
   stepperY.setRPM(Yrpm*6);
  }
  else if (i == (L-4)){
   stepperX.setRPM(Xrpm*5);
   stepperY.setRPM(Yrpm*5);
  }
  else if (i == (L-3)){
   stepperX.setRPM(Xrpm*4);
   stepperY.setRPM(Yrpm*4);
  }
  else if (i == (L-2)){
   stepperX.setRPM(Xrpm*3);
   stepperY.setRPM(Yrpm*3);
  }
  else if (i == (L-1)){
   stepperX.setRPM(Xrpm*2);
   stepperY.setRPM(Yrpm*2);
  }
  else if (i == L){
   stepperX.setRPM(Xrpm);
   stepperY.setRPM(Yrpm);
  }
  else{
   stepperX.setRPM(Xrpm*10);
   stepperY.setRPM(Yrpm*10);
  }  
controller.rotate(360,-PPT*SW);  
Serial.print("x3.val=");
Serial.print(count*10);
Serial.write(0xff);
Serial.write(0xff);
Serial.write(0xff);
count++;
}

} 
 
}
}

void WindingWithstop(){

  if (A > 0 && B > 0 && C > 0) {
 
  if (count <= C){

  
for (int i = 0; i <= L; i++){
  if (count > C){
break;
  }
  nexLoop(nex_listen_list);  // Check for any touch event
 while (State == 1)  {
  nexLoop(nex_listen_list);  // Check for any touch event
  }
  if (i == 0){
   stepperX.setRPM(Xrpm);
   stepperY.setRPM(Yrpm);
  }
  else if (i == 1){
   stepperX.setRPM(Xrpm*2);
   stepperY.setRPM(Yrpm*2);
  }
  else if (i == 2){
   stepperX.setRPM(Xrpm*3);
   stepperY.setRPM(Yrpm*3);
  }
  else if (i == 3){
   stepperX.setRPM(Xrpm*4);
   stepperY.setRPM(Yrpm*4);
  }
  else if (i == 4){
   stepperX.setRPM(Xrpm*5);
   stepperY.setRPM(Yrpm*5);
  }
  else if (i == 5){
   stepperX.setRPM(Xrpm*6);
   stepperY.setRPM(Yrpm*6);
  }
  else if (i == 6){
   stepperX.setRPM(Xrpm*7);
   stepperY.setRPM(Yrpm*7);
  }
  else if (i == 7){
   stepperX.setRPM(Xrpm*8);
   stepperY.setRPM(Yrpm*8);
  }
  else if (i == 8){
   stepperX.setRPM(Xrpm*9);
   stepperY.setRPM(Yrpm*9);
  }
  else if (i == (L-8)){
   stepperX.setRPM(Xrpm*9);
   stepperY.setRPM(Yrpm*9);
  }
  else if (i == (L-7)){
   stepperX.setRPM(Xrpm*8);
   stepperY.setRPM(Yrpm*8);
  }
  else if (i == (L-6)){
   stepperX.setRPM(Xrpm*7);
   stepperY.setRPM(Yrpm*7);
  }
  else if (i == (L-5)){
   stepperX.setRPM(Xrpm*6);
   stepperY.setRPM(Yrpm*6);
  }
  else if (i == (L-4)){
   stepperX.setRPM(Xrpm*5);
   stepperY.setRPM(Yrpm*5);
  }
  else if (i == (L-3)){
   stepperX.setRPM(Xrpm*4);
   stepperY.setRPM(Yrpm*4);
  }
  else if (i == (L-2)){
   stepperX.setRPM(Xrpm*3);
   stepperY.setRPM(Yrpm*3);
  }
  else if (i == (L-1)){
   stepperX.setRPM(Xrpm*2);
   stepperY.setRPM(Yrpm*2);
  }
  else if (i == L){
   stepperX.setRPM(Xrpm);
   stepperY.setRPM(Yrpm);
  }
  else{
   stepperX.setRPM(Xrpm*10);
   stepperY.setRPM(Yrpm*10);
  }  
controller.rotate(360,PPT*SW);
Serial.print("x3.val=");
Serial.print(count*10);
Serial.write(0xff);
Serial.write(0xff);
Serial.write(0xff);
count++;  
}

if (count < C){
delay(10000);
  }

for (int i = 0; i <= L; i++){
if (count > C){
break;
  }
  nexLoop(nex_listen_list);  // Check for any touch event
 while (State == 1)  {
  nexLoop(nex_listen_list);  // Check for any touch event
  }
 if (i == 0){
   stepperX.setRPM(Xrpm);
   stepperY.setRPM(Yrpm);
  }
  else if (i == 1){
   stepperX.setRPM(Xrpm*2);
   stepperY.setRPM(Yrpm*2);
  }
  else if (i == 2){
   stepperX.setRPM(Xrpm*3);
   stepperY.setRPM(Yrpm*3);
  }
  else if (i == 3){
   stepperX.setRPM(Xrpm*4);
   stepperY.setRPM(Yrpm*4);
  }
  else if (i == 4){
   stepperX.setRPM(Xrpm*5);
   stepperY.setRPM(Yrpm*5);
  }
  else if (i == 5){
   stepperX.setRPM(Xrpm*6);
   stepperY.setRPM(Yrpm*6);
  }
  else if (i == 6){
   stepperX.setRPM(Xrpm*7);
   stepperY.setRPM(Yrpm*7);
  }
  else if (i == 7){
   stepperX.setRPM(Xrpm*8);
   stepperY.setRPM(Yrpm*8);
  }
  else if (i == 8){
   stepperX.setRPM(Xrpm*9);
   stepperY.setRPM(Yrpm*9);
  }
  else if (i == (L-8)){
   stepperX.setRPM(Xrpm*9);
   stepperY.setRPM(Yrpm*9);
  }
  else if (i == (L-7)){
   stepperX.setRPM(Xrpm*8);
   stepperY.setRPM(Yrpm*8);
  }
  else if (i == (L-6)){
   stepperX.setRPM(Xrpm*7);
   stepperY.setRPM(Yrpm*7);
  }
  else if (i == (L-5)){
   stepperX.setRPM(Xrpm*6);
   stepperY.setRPM(Yrpm*6);
  }
  else if (i == (L-4)){
   stepperX.setRPM(Xrpm*5);
   stepperY.setRPM(Yrpm*5);
  }
  else if (i == (L-3)){
   stepperX.setRPM(Xrpm*4);
   stepperY.setRPM(Yrpm*4);
  }
  else if (i == (L-2)){
   stepperX.setRPM(Xrpm*3);
   stepperY.setRPM(Yrpm*3);
  }
  else if (i == (L-1)){
   stepperX.setRPM(Xrpm*2);
   stepperY.setRPM(Yrpm*2);
  }
  else if (i == L){
   stepperX.setRPM(Xrpm);
   stepperY.setRPM(Yrpm);
  }
  else{
   stepperX.setRPM(Xrpm*10);
   stepperY.setRPM(Yrpm*10);
  }  
controller.rotate(360,-PPT*SW);  
Serial.print("x3.val=");
Serial.print(count*10);
Serial.write(0xff);
Serial.write(0xff);
Serial.write(0xff);
count++;
}
if (count < C){
delay(10000);
  }

}  
   
}
}

////////////////////////// End of touch events


void setup() {  // Put your setup code here, to run once:
  
  Serial.begin(9600);  // Start serial comunication at baud=9600


  // I am going to change the Serial baud to a faster rate.
  // The reason is that the slider have a glitch when we try to read it's value.
  // One way to solve it was to increase the speed of the serial port.
  delay(500);  // This dalay is just in case the nextion display didn't start yet, to be sure it will receive the following command.
  Serial.print("baud=115200");  // Set new baud rate of nextion to 115200, but it's temporal. Next time nextion is power on,
                                // it will retore to default baud of 9600.
                                // To take effect, make sure to reboot the arduino (reseting arduino is not enough).
                                // If you want to change the default baud, send the command as "bauds=115200", instead of "baud=115200".
                                // If you change the default baud, everytime the nextion is power ON is going to have that baud rate, and
                                // would not be necessery to set the baud on the setup anymore.
  Serial.write(0xff);  // We always have to send this three lines after each command sent to nextion.
  Serial.write(0xff);
  Serial.write(0xff);

  Serial.end();  // End the serial comunication of baud=9600

  Serial.begin(115200);  // Start serial comunication at baud=115200


  // Register the event callback functions of each touch event:
  // You need to register press events and release events seperatly.
  // Format for press events: <object name>.attachPush(<object name>PushCallback);
  // Format for release events: <object name>.attachPop(<object name>PopCallback);
  b0.attachPop(b0PopCallback);  // Button release
  b1.attachPop(b1PopCallback);  // Button release
  b2.attachPop(b2PopCallback);  // Button release
  b3.attachPop(b3PopCallback);  // Button release
  b4.attachPop(b4PopCallback);  // Button release
  b5.attachPop(b5PopCallback);  // Button release
  b20.attachPop(b20PopCallback);  // Button release
  b21.attachPop(b21PopCallback);  // Button release
  b22.attachPop(b22PopCallback);  // Button release
  b23.attachPop(b23PopCallback);  // Button release
  b24.attachPop(b24PopCallback);  // Button release
  bt0.attachPop(bt0PopCallback);  // Dual state button bt0 release
  bt1.attachPop(bt1PopCallback);  // Dual state button bt1 release
  c0.attachPop(c0PopCallback);  // Radio checkbox release
  page0.attachPush(page0PushCallback);  // Page press event
  page1.attachPush(page1PushCallback);  // Page press event
  page2.attachPush(page2PushCallback);  // Page press event
  stepperX.begin(MOTOR_X_RPM, MICROSTEPS);
  stepperY.begin(MOTOR_Y_RPM, MICROSTEPS);
  // End of registering the event callback functions
  
    
  pinMode(13, OUTPUT);
  pinMode(EN,OUTPUT);
  digitalWrite(EN,LOW);
    
}  // End of setup




void loop() {  // Put your main code here, to run repeatedly:
    
  delay(30);  // This is the only delay on this loop.
              // I put this delay because without it, the timer on the display would stop running.
              // The timer I am talking about is the one called tm0 on page 0 (of my example nextion project).
              // Aparently we shouldn't send data to the display too often.
  
  ChoiceRpmPot();
  
  nexLoop(nex_listen_list);  // Check for any touch event
   

  // I created the following variable to have a dynamic number to send to the display:
  variable1++;  // Add 1 to the variable1
  Serial.print("z0.val=");  // This is sent to the nextion display to set what object name (before the dot) and what atribute (after the dot) are you going to change.
  Serial.print(variable1);  // This is the value you want to send to that object and atribute mentioned before.
  Serial.write(0xff);  // We always have to send this three lines after each command sent to the nextion display.
  Serial.write(0xff);
  Serial.write(0xff);
  if(variable1 > 360){  // If the variable is above 240
    variable1 = 0;  // Set variable to 0 to start over
  }

   
  if (M == 1){
    WindingWithstop();
  }else{
    WindingWostop();
  }


  // the current page creates an error code sent by the display.
  // Any error sent by the display creates lag on the arduino loop because arduino tries to read it, thinking it's a touch event.
  // So to avoid this, I am only going to send data depending on the page the display is on.
  // That's the reason I want the arduino to know which page is loaded on the display.

  if(CurrentPage == 0){  // If the display is on page 0, do the following:
  
    
  }



  if(CurrentPage == 1){  // If the display is on page 1, do the following:
  
   
  }





  if(CurrentPage == 2){  // If the display is on page 2, do the following:
  
   
  }


  // We are going to check the list of touch events we enter previously to
  // know if any touch event just happened, and excecute the corresponding instructions:
  


  //nexLoop(nex_listen_list);  // Check for any touch event
  

}  // End of loop

