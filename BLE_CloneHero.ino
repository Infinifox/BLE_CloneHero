#include <Arduino.h>
#include <BleGamepad.h>

BleGamepad bleGamepad("Wireless Guitar", "Infinifox", 100);

#define numOfButtons 10
#define numOfHats 1
#define zAxisSamples 10

// Button arrays
byte previousButtonStates[numOfButtons];
byte currentButtonStates[numOfButtons];

// Button pin definitions
byte buttonPins[numOfButtons] = {12, 14, 27, 26, 25, 19, 21};
byte physicalButtons[numOfButtons] = {1, 2, 4, 3, 5, 7, 8};

// D-Pad array
byte previousHatStates[numOfHats * 4];
byte currentHatStates[numOfHats * 4];

// D-Pad pin definitions
byte hatPins[numOfHats * 4] = {1, 23, 22, 3};

// Whammy bar pin definition
byte zAxisPin = 2;
short currentZAxis;
short previousZAxis;

// Tilt pin definition
byte sliderPin = 32;
short currentSlider;
short previousSlider;

byte tilted = false;
byte tiltDuration = 18;
byte currentTiltDuration = tiltDuration;

// LED stuff

byte led1Pin = 16;
byte led2Pin = 17;
byte led3Pin = 5;
byte led4Pin = 18;

byte currentLED = 1;
byte forwards = true;
const unsigned long blinkRate = 100;
unsigned long startMillis;
unsigned long currentMillis;

void setup()
{
  // Setup Buttons
  for (byte currentPinIndex = 0; currentPinIndex < numOfButtons; currentPinIndex++)
  {
    pinMode(buttonPins[currentPinIndex], INPUT_PULLUP);
    previousButtonStates[currentPinIndex] = HIGH;
    currentButtonStates[currentPinIndex] = HIGH;
  }

  // Setup Hats
  for (byte currentPinIndex = 0; currentPinIndex < numOfHats * 4; currentPinIndex++)
  {
    pinMode(hatPins[currentPinIndex], INPUT_PULLUP);
    previousHatStates[currentPinIndex] = HIGH;
    currentHatStates[currentPinIndex] = HIGH;
  }

  // Setup LEDs

  pinMode(led1Pin, OUTPUT);
  pinMode(led2Pin, OUTPUT);
  pinMode(led3Pin, OUTPUT);
  pinMode(led4Pin, OUTPUT);

  digitalWrite(led1Pin, HIGH);
  digitalWrite(led2Pin, HIGH);
  digitalWrite(led3Pin, HIGH);
  digitalWrite(led4Pin, HIGH);

  // I don't remember why this is here
  bleGamepad.setZ(0);

  Serial.begin(115200);
  Serial.println("Starting BLE work!");

  BleGamepadConfiguration bleGamepadConfig;

  // This was an attempt to get Clone Hero to identify the controller as an official device.
  // The goal was to remove the need to setup and bind buttons within CH.
  // It didn't work... :'(

  //bleGamepadConfig.setVid(0x1430);
  //bleGamepadConfig.setPid(0x0719);
  //bleGamepadConfig.setHidReportId(uint8_t value)

  //bleGamepadConfig.setAxesMin(1935); // GUITAR HERO
  //bleGamepadConfig.setAxesMax(4095); // GUITAR HERO

  bleGamepadConfig.setAxesMin(100);
  bleGamepadConfig.setAxesMax(3090);

  bleGamepadConfig.setAutoReport(false);
  bleGamepadConfig.setButtonCount(numOfButtons);
  bleGamepadConfig.setHatSwitchCount(numOfHats);
  bleGamepadConfig.setWhichAxes(false, false, true, false, false, false, true, false);
  bleGamepad.begin(&bleGamepadConfig);

  startMillis = millis();
}

void setLED(byte i)
{
  digitalWrite(led1Pin, HIGH);
  digitalWrite(led2Pin, HIGH);
  digitalWrite(led3Pin, HIGH);
  digitalWrite(led4Pin, HIGH);

  switch(i)
  {
    case 1:
      digitalWrite(led1Pin, LOW);
      break;
    case 2:
      digitalWrite(led2Pin, LOW);
      break;
    case 3:
      digitalWrite(led3Pin, LOW);
      break;
    case 4:
      digitalWrite(led4Pin, LOW);
      break;
  }
}

void loop()
{
  // Prints the values output from the whammy
  //Serial.println(analogRead(zAxisPin));


  // If the guitar is not connected, cycle the 4 LEDs back and fourth
  if (!bleGamepad.isConnected())
  {
    currentMillis = millis();

    if (currentMillis - startMillis >= blinkRate)
    {
      startMillis = currentMillis;

      if (forwards) // LEDs moving forward
      {
        if (currentLED < 4) // Continue forward
        {
          currentLED += 1;
          setLED(currentLED);
        }
        else // Reached end, change direction
        {
          forwards = false;
          currentLED -= 1;
          setLED(currentLED);
        }
      }
      else // LEDs moving backwards
      {
        if (currentLED > 1) //Continue backwards
        {
          currentLED -= 1;
          setLED(currentLED);
        }
        else // Reached end, change direction
        {
          forwards = true;
          currentLED += 1;
          setLED(currentLED);
        }
      }
    }
  }

  if (bleGamepad.isConnected())
  {
    setLED(1);

    // Handle button states
    for (byte currentIndex = 0; currentIndex < numOfButtons; currentIndex++)
    {
      currentButtonStates[currentIndex] = digitalRead(buttonPins[currentIndex]);

      if (currentButtonStates[currentIndex] != previousButtonStates[currentIndex])
      {
        if (currentButtonStates[currentIndex] == LOW)
        {
          bleGamepad.press(physicalButtons[currentIndex]);
          //if (currentIndex == 5)
          //{
          //  currentTiltDuration = 0;
          //}
        }
        else
        {
          bleGamepad.release(physicalButtons[currentIndex]);
        }
      }
    }

    //if (currentTiltDuration < tiltDuration)
    //{
    //  currentTiltDuration += 1;
    //  bleGamepad.press(physicalButtons[5]);
    //}

    // Handle hat states
    for (byte currentHatPinsIndex = 0; currentHatPinsIndex< numOfHats * 4; currentHatPinsIndex++)
    {
      currentHatStates[currentHatPinsIndex] = digitalRead(hatPins[currentHatPinsIndex]);
    }

    signed char hatValues[4] = {0, 0, 0, 0};

    for (byte currentHatIndex = 0; currentHatIndex < 4; currentHatIndex++)
    {
       signed char hatValueToSend = 0;

            for (byte currentHatPin = 0; currentHatPin < 4; currentHatPin++)
            {
                // Check for direction
                if (currentHatStates[currentHatPin + currentHatIndex * 4] == LOW)
                {
                    hatValueToSend = currentHatPin * 2 + 1;

                    // Account for last diagonal
                    if (currentHatPin == 0)
                    {
                        if (currentHatStates[currentHatIndex * 4 + 3] == LOW)
                        {
                            hatValueToSend = 8;
                            break;
                        }
                    }

                    // Account for first 3 diagonals
                    if (currentHatPin < 3)
                    {
                        if (currentHatStates[currentHatPin + currentHatIndex * 4 + 1] == LOW)
                        {
                            hatValueToSend += 1;
                            break;
                        }
                    }
                }
            }
      hatValues[currentHatIndex] = hatValueToSend;
    }

    bleGamepad.setHats(hatValues[0], hatValues[1], hatValues[2], hatValues[3]);

    // Here I attempted to implement a deadzone for the bottom end of the whammy bar.
    // It only sort of works. For some reason the value likes to overflow backwards
    // to the highest value.
    if (analogRead(zAxisPin) > 220)
    {
      currentZAxis = map(analogRead(zAxisPin), 220, 3150, 0, 4096);
      //currentZAxis = analogRead(zAxisPin);
    }

    if (analogRead(sliderPin) > 0)
    {
      currentSlider = map(analogRead(sliderPin), 4095, 0, 0, 4095);
    }

    // Only report states that have changed?
    // I think that's what this does... I don't know if this is neccessary though.
    if (currentButtonStates != previousButtonStates || currentHatStates != previousHatStates || currentZAxis != previousZAxis || currentSlider != previousSlider)
    {
      for (byte currentIndex = 0; currentIndex < numOfButtons; currentIndex++)
      {
        previousButtonStates[currentIndex] = currentButtonStates[currentIndex];
      }

      for (byte currentIndex = 0; currentIndex < numOfHats * 4; currentIndex++)
      {
        previousHatStates[currentIndex] = currentHatStates[currentIndex];
      }

      previousZAxis = currentZAxis;
      previousSlider = currentSlider;
      
      bleGamepad.setZ(currentZAxis);
      bleGamepad.setSlider(currentSlider);
      bleGamepad.sendReport();
    }

    //Serial.println(analogRead(zAxisPin));

    // Pause for x ms
    // Could probably do with being lower than 15ms for higher responsivitiy
    // I expect this would reduce battery life though
    delay(15);
  }
}