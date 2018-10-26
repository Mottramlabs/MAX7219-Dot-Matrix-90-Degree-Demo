

/*
      Name    Arduino   ESP8266       MAX7219 7 Segment display
      -----   -------   -------       -------------------------
              +5V       5V            VCC
              GND       GND           GND
      DIN     D11/MOSI  GPIO13 (D7)   DIN
      CS      *D7/SS    *GPIO15 (D8)   CS
      CLK     D13/SCK   GPIO14 (D5)   CLK

       Select in software
*/

// --------------------------------------------------------------------------------------
// Global variables
// --------------------------------------------------------------------------------------

// MAX7219 items
#define Number_MAX7219 8                         // Number of MAX7219 chips present
#define MAX7219_CS  7                             // Pin used for CS. 7=UNO 15=ESP8266

// 90 Deg Font library
#include "Font_90_Degrees.h"

// MAX7219 register name list
#define MAX7219_Noop          0x0
#define MAX7219_DecodeMode    0x9
#define MAX7219_Intensity     0xA
#define MAX7219_ScanLimit     0xB
#define MAX7219_Shutdown      0xC
#define MAX7219_Displaytest   0xF

// Display items
#define Space_Width 4                             // How wide the space should be 0-8
int Column = 0;                                   // Column address for character
uint8_t Frame_Buffer[Number_MAX7219 * 8];         // Frame buffer array
bool Scrolling = false;
int Scroll_Position = 0;
String Message_Text = "";                         // Message to display
int Scroll_Speed = 0;
// --------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------------------------
// Write a single byte of data to a single register within the MAX7219. used for all MAX7219 Communications
// Address, MAX7219 register
// Data, data to write to register
// --------------------------------------------------------------------------------------------------------
void MAX7219_Register_Write(uint8_t Address, uint8_t Data) {

  // Send the register address
  SPI.transfer(Address);

  // Send the value
  SPI.transfer(Data);

} // end of function


// -------------------------------------------------------------------------------------
// Write data to all MAX7219 chips present, used to initialize display or set brightness
// Writes one byte at a time to each of the MAX7219 chips present
// -------------------------------------------------------------------------------------
void MAX7219_All_Chips(uint8_t Address, uint8_t Data) {

  digitalWrite(MAX7219_CS, LOW);  // Take CS Low

  for (uint8_t i = 0; i < Number_MAX7219; ++i) {

    SPI.transfer(Address);    // Send the register address
    SPI.transfer(Data);    // Send the value

  } // end of loop

  digitalWrite(MAX7219_CS, HIGH);  // Take CS High

} // end Function


// --------------------------------------------------------
// Initialise the display, writing to all present MAX7219's
// --------------------------------------------------------
void MAX7219_Init() {

  // Initialize all MAX7219 chips write to each present MAX7219 chip present
  MAX7219_All_Chips(MAX7219_DecodeMode, 0x00);
  MAX7219_All_Chips(MAX7219_Intensity, 0x02);
  MAX7219_All_Chips(MAX7219_ScanLimit, 0x07);
  MAX7219_All_Chips(MAX7219_Displaytest, 0x00);
  MAX7219_All_Chips(MAX7219_Shutdown, 0x01);

} // end of function


// ------------------------------------------------------------------------------------------------------------------------
// Write data to display row (as display is rotated 90, a row is in fact a MAX7219 data register.
// If the row number requested is greater than possible this function will do nothing.
// Register_Address = Address
// Register_Data = Data
// So for a 12 chip display (range 0-95) Register_Address 95 (last address) is the last chips (far right) first register #1
// and Address 0 is first chips (far left) last register #8. Got that?
// ------------------------------------------------------------------------------------------------------------------------
void Write_Row(uint8_t Register_Address, uint8_t Register_Data) {



  // Range check the address value
  if (Register_Address >= 0 && Register_Address <= ((Number_MAX7219 * 8) - 1)) {

    // Range check passed. Which MAX7219 chip to write data to, write NOOP's to all others
    // Chip_Number range 0 to number of chips, for a 12 chip display thats 0 to 11
    int Chip_Number = Register_Address / 8;

    // Find to chips data register to write data too, result is Row_Address. Range = 1 to 8 (data registers of the MAX7219
    int xx = (Chip_Number * 8); Register_Address = 8 - (Register_Address - xx);

    // Chip_Number = Calculated chip ID to write the data too (Number of MAX chips - 1) to 0
    // Register_Address = Register within above chip (1-8)

    // Start the transfer to the Max chips, loop through all MAX chips, writing either the data or NOOP
    digitalWrite(MAX7219_CS, LOW);

    // Test which chip to write data too and write noop to all others, that is register address 0x00
    // Loop from the last to the first chip (far right to far left)
    for (int xx = Number_MAX7219; xx-- > 0; ) {
      // test if it the chip we are to write data to else write a NOOP
      if (xx == Chip_Number) {
        // This is the chip required, now select the column
        MAX7219_Register_Write(Register_Address, Register_Data);
      }
      // else will write noop to other MAX7219 chips
      else {
        MAX7219_Register_Write(MAX7219_Noop, 0x00);
      }
    } // end of loop

    // CS high
    digitalWrite(MAX7219_CS, HIGH);

  } // end of range check

} // end of function


// ---------------------------------------------------------------------------------------
// Update Display. Writes all the data from the display array into the MAX7219's registers
// Starts at the far right, the last MAX7219 chip and at the last data register
// Loops down through the array to the first byte in the array
// With a 12 chip display the range for i is 95 to 0
// ---------------------------------------------------------------------------------------
void Display_Update() {

  for (int i = ((Number_MAX7219 * 8) - 1); i > -1; i--) {

    Write_Row(i, Frame_Buffer[i]); // Write to column, with data

  } // end of loop

} // end of function


// ----------------------
// Clear the frame buffer
// ----------------------
void Clear_Buffer() {

  for (int a = 0; a < (Number_MAX7219 * 8); a++) {
    Frame_Buffer[a] = 0;
  } // end of loop

}  // end function


// ----------------------------------------------
// Flush the display array and update the display
// ----------------------------------------------
void Clear_All() {

  // Clear the buffer
  Clear_Buffer();

  // Now update the display
  Display_Update();

} // end Function


// ------------------------------------------------------------------------------------------------------------------------------
// Write a charater to display buffer starting at the location found in column. Column is incrument by the width of the character
// ------------------------------------------------------------------------------------------------------------------------------
void Write_Character(int Ascii_Value) {

  for (int Digit_Colummn = 7; Digit_Colummn > -1; Digit_Colummn--) {

    int Character_Value = Ascii_Value;

    // Range check for the ascii value
    if (Character_Value > 0x1F && Character_Value < 0x7F) {

      // Apply offset as the font table starts at 0 in memory
      Character_Value = Character_Value - 0x20;

      int Array_Data = Column;

      // Range check, don't write to array if less than 0 or greater that the array size
      if (Array_Data > -1 && Array_Data < (Number_MAX7219 * 8)) {

        // Calculate the MAX7219 chip ID (Number of MAX Chip – INT(Column/8))
        int MAX_ID = Array_Data / 8;
        int Row_Bit = Array_Data - (MAX_ID * 8);

        Array_Data = Array_Data - Row_Bit;

        // Loop 8 times through the character fonts data
        for (int ii = 0; ii < 8; ii++) {

          // Read font data based on ASII value
          int Font_Row = pgm_read_byte(&Text_Font[Character_Value] [ii]);

          // Range check before writing to array
          if (Array_Data < 0 || Array_Data < (Number_MAX7219 * 8)) {

            // Set or clear bit within the display array
            if (bitRead(Font_Row, Digit_Colummn) == 1) { // Read bit 7
              bitSet(Frame_Buffer[Array_Data], Row_Bit); // Set bit
            } // end of bit set

            else {
              bitClear(Frame_Buffer[Array_Data], Row_Bit); // Clear bit
            } // end of bit clear

          } // end of loop

          Array_Data++;

        } // end of character loop (ii)

      } //end of column range check

      // Inc Column position ready for next run
      // Don't allow the column to increase above the buffers maximum, does this fix bad wrap? Already fixed by column range check
      //    if (Column < (Number_MAX7219 * 8)) {
      Column++;
      //   } // end of column check

    } // end of range check

  } // end of loop

} // end of function


// --------------------------------------------------------
// Display the text message starting at the location column
// --------------------------------------------------------
void Display_Text(String Text_Value, int Location) {

  Column = Location;

  Clear_Buffer();       // Clear the frame buffer

  // Write text message to buffer
  for (int ll = 0; ll < Text_Value.length(); ll++) {
    Write_Character(Text_Value.charAt(ll));
  } // end of loop

  // Write the frame buffer to the MAX7219 display
  Display_Update();

} // end of function


// ------------------------------------------
// Wipe the text message starting to the left
// ------------------------------------------
void Wipe_Text_Left(String Text_Value, int Speed) {

  for (int ii = (Number_MAX7219 * 8) + 1; ii > -1; ii--) {

    Column = ii;

    // Write text message to buffer
    for (int ll = 0; ll < Text_Value.length(); ll++) {
      Write_Character(Text_Value.charAt(ll));
    } // end of loop

    // Write the frame buffer to the MAX7219 display
    Display_Update();

    delay(Speed);

  } // end of loop

} // end of function

// -------------------------------------------
// Wipe the text message starting to the right
// -------------------------------------------
void Wipe_Text_Right(String Text_Value, int Speed) {

  // Write the text message
  for (int ii = (0 - (Text_Value.length() * 8)); ii < 1; ii++) {

    Column = ii;

    // Write text message to buffer
    for (int ll = 0; ll < Number_MAX7219 * 8; ll++) {
      Write_Character(Text_Value.charAt(ll));             // Write character at location column
    } // end of loop

    // Write the frame buffer to the MAX7219 display
    Display_Update();

    delay(Speed);

  } // end of loop


  // Fill the rest of the buffer with spaces
  while (Column < (Number_MAX7219 * 8)) {

    Write_Character(0x20);
    Display_Update();
    delay(Speed * 30);    // Needs to be slower the the text above

  } // end of loop

} // end of function


// ------------------------------------
// Update the xtext at current position
// ------------------------------------
void Scroll_Update() {

  //Serial.print("Scrolling at "); Serial.println(Scroll_Position);

  Column = Scroll_Position;

  // Write text message to buffer
  for (int ll = 0; ll < Message_Text.length(); ll++) {
    Write_Character(Message_Text.charAt(ll));
  } // end of loop

  // Write the frame buffer to the MAX7219 display
  Display_Update();

  delay(Scroll_Speed);
  Scroll_Position--;

  int zz = (0 - (Message_Text.length() * 8));
  if (Scroll_Position < zz) {
    Scrolling = false;
  }

} // end function


// -------------------------
// Load and start the scroll
// -------------------------
void Scroll_Text_Start(String Text, int a) {

  Scrolling = true;

  // Position the marker far right
  Scroll_Position = (Number_MAX7219 * 8) + 1;
  Message_Text = Text;
  Scroll_Speed = a;

  Serial.print("Starting scroll at "); Serial.println(Scroll_Position);

} // end of function

