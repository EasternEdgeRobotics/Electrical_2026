/*
 Main.  execute top level calls
*/

// pin definitions
 // TODO


/*
seperate all initialization functions in their respective tabs
*/
void setup() {
  
  WifiSetup();

}

/*
separate the loop code into functions in their respective tabs.  the functions themselves should not loop since loop() is an infinite loop
*/
void loop() {

  WifiLoop();
}
