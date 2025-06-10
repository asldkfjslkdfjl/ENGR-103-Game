#include <Adafruit_CircuitPlayground.h>

//variables for position indicator
float x, y, z, angle; 
int position; //if there were 12 equally spaced pixels around the board, position is the pixel number that is lowest down in space
int redAmt = 0; //the intensity of red that should be on the reddest pixel
int positionReds[] = {0,0,0,0,0,0,0,0,0,0,0,0}; //represents the amount of red color in each pixel if there were 12 pixels equally spaced around the board
int pixelReds[] = {0,0,0,0,0,0,0,0,0,0};//represents the amount of red color in the actual pixels 
const int indexer[10] = {1,2,3,4,5,7,8,9,10,11};//maps between the actual pixel positions and the imaginary 12 equally spaced positions. pixel n corresponds to position indexer[n]
const int rexedni[12] = {13,0,1,2,3,4,13,5,6,7,8,9}; //maps in opposite direction to indexer
int runningZ[20] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
float zSmooth = 0;

//variables for sequence
int correctSequence[] = {2,3,5,5,8,7,3,9,2,2};
const int specialSequence[] = {6,6,0,0,9,3,9,3,11,1};
int gameRound = 0;
int guess = 0;
int stage = 0; //0: not started, 1: display sequence, 2: guess, 3: lose, 4: win
int guessAmt = 0;

//misc variables
const int baseTone = 261.626;
bool secret = false;

//MIDI variables. Any strange formatting is a result of me constructing a song in a google sheet and pasting it into arduino.
float midi[127];
int A_four = 440; // a is 440 hz
int secretSound[][2] = {{	60	,	200	}	,
{	55	,	200	}	,
{	52	,	200	}	,
{	48	,	200	}	,
{	52	,	200	}	,
{	55	,	200	}	,
{	60	,	1200	}}	; //length 7
int secretSong[][2] = {	{	63	,	200	}	,	
	{	65	,	200	}	,	
	{	66	,	200	}	,	
	{	68	,	200	}	,	
	{	70	,	400	}	,	
	{	75	,	200	}	,	
	{	73	,	200	}	,	
	{	70	,	400	}	,	
	{	63	,	400	}	,	
	{	70	,	200	}	,	
	{	68	,	200	}	,	
	{	66	,	200	}	,	
	{	65	,	200	}		}; //length 13


//interrupt variables
const byte SlideSwitch = 7; //board pin number for the switch
volatile bool switchFlag = 0; //interrupt flag for the switch
bool switchState; //state of the switch
int switchCount = 0; //number of times the switch has been flipped
const byte LeftButton = 4;
const byte RightButton = 5;
volatile bool buttonFlag = 0;




//ANGLE RELATED


//initialize runningZ
void runningSetup() {
  z = CircuitPlayground.motionZ();
  for (int i = 0; i < 20; i++) {
    runningZ[i] = z;
  }
  updateZ();
}

//update zSmooth variable
void updateZ() {
  zSmooth = 0;
  for (int i = 0; i < 20; i++) {
    zSmooth += (runningZ[i]/20.0);
  }
}

//update runningZ[]
void updateZs(float z) {
  for (int i = 0; i < 19; i++) {
    runningZ[i] = runningZ[i+1];
  }
  runningZ[19] = z;
  updateZ();
}

//calculate the angle based on the x and y acceleration readings
float getAngle(float x, float y) {
  float nx, ny, retAngle;
  
  nx = x / 10.0;
  ny = y / 10.0;
  
  retAngle = atan((ny/nx)) * 180 / 3.14; // Figure out the angle of the accelerometer
  if(retAngle > 0.0)
  { // Adjust based on arctangent function (in degrees)
    if(nx < 0.0)
      retAngle += 180;
  }
  else
  { 
    if(ny > 0.0)
      retAngle += 180;
    else
      retAngle += 360;
  }
  if(retAngle == 360.0) // a 360 degree angle is the same as a zero degree angle
    retAngle = 0;

  return retAngle;
}

//uses redAmt, position, indexer[] to update positionReds[] and pixelReds[]
void updateReds(int redAmt) {
  //reset the reds array
  for (int i = 0; i < 12; i++) {
    positionReds[i] = 0;
  }

  //sets up the positionReds array so that the lowest position is the brightest red and red fades out two pixels to the left and right of it
  positionReds[position] = redAmt;
  positionReds[(position+1)%12] = redAmt*0.65; 
  positionReds[(position+11)%12] = redAmt*0.65;
  positionReds[(position+2)%12] = redAmt*0.25;
  positionReds[(position+10)%12] = redAmt*0.25;


  //sets up the pixelReds array based on the positions array
  for (int i = 0; i < 10; i++) {
    pixelReds[i] = positionReds[indexer[i]];
  }
}

//print angle, position, z, redAmt
void printVals() {
  //print accelerometer and map data to check that everything is working properly
  Serial.print("angle, position, z, redAmt: ");
  if (0<=angle && angle<10)
    Serial.print("  ");
  if (10<=angle && angle<100)
    Serial.print(" ");
  Serial.print(angle);
  Serial.print(", ");
  Serial.print(position);
  Serial.print(", ");
  if (0<=z && z<10)
    Serial.print("  ");
  Serial.print(z);
  Serial.print(", ");
  if (0<=redAmt && redAmt<10)
    Serial.print("  ");
  if (10<=redAmt && redAmt<100)
    Serial.print(" ");
  Serial.println(redAmt);

}

//get position based on angle
int getPosition(float angle) {
  int position = map(angle, 0, 180, 3, 9);
  position = position%12;
  return position;
}

//set the brightness of red based on how tilted the board is in the z direction
int getRedAmt(float z) {
  updateZs(z);
  int redAmt = map(abs(zSmooth), 5, 9, 255, 0);
  if (redAmt<0)
  {
    redAmt = 0;
  } else if (redAmt>255) {
    redAmt = 255;
  }
  return redAmt;
}




//MISC


void gameSetup() {
  //correctSequence = {0,0,0,0,0,0,0,0,0,0};
  int toAdd = 1;
  for (int i = 0; i < 10; i++) {
    toAdd = random(0,12);
    if (toAdd == 0) {
      toAdd = 1;
    } else if (toAdd == 6) {
      toAdd = 7;
    }
    correctSequence[i] = toAdd;
  }
  guess = 0;
  gameRound = 0;
}

// This function calculates frequency values for MIDI pitch numbers 0 - 127.
// These values are stored in an array where the index at the MIDI pitch value will retrieve the corresponding frequency to set a speaker
// MIDI 60 is C4 on the piano, and corresponds to 261.62 Hz
// Each digit up or down corresponds to playing the key that many steps above or below on the piano
// adapted from https://subsynth.sourceforge.net/midinote2freq.html
void generateMIDI()
{
  for (int x = 0; x < 127; ++x)
  {
    midi[x] = (A_four / 32.0) * pow(2.0, ((x - 9.0) / 12.0));
    //Serial.println(midi[x]);
  }
}

void play(int song[][2], int length) {
  for (int i = 0; i < length; i++) {
    CircuitPlayground.playTone(midi[song[i][0]], song[i][1]);
  }
}



//SOUNDS AND COLORS


void playGoodSound() {
  CircuitPlayground.playTone(baseTone, 100);
  CircuitPlayground.playTone(baseTone*2,200);
  Serial.println("good sound");
} 

void playBadSound() {
  CircuitPlayground.playTone(baseTone, 100);
  CircuitPlayground.playTone(baseTone/2,200);
  Serial.println("bad sound");
} 

void playSecretSound() {
  Serial.println("secret sound");
  play(secretSound, 7);
} 

void playSecretSong() {
  Serial.print("secret song oooo");
  play(secretSong, 13);
} 

//flashing pattern. color input is used via colorWheel().
void flasher(int color) {
  CircuitPlayground.clearPixels();
  for (int i = 0; i < 10; i+=2) {
    CircuitPlayground.setPixelColor(i, CircuitPlayground.colorWheel(color));
  }
  delay(200);
  CircuitPlayground.clearPixels();
  for (int i = 1; i < 10; i+=2) {
    CircuitPlayground.setPixelColor(i, CircuitPlayground.colorWheel(color));
  }
  delay(200);
}

void allBlue() {
  for (int i = 0; i < 10; i++) {
    CircuitPlayground.setPixelColor(i, 0, 0, 255);
  }
}




//STAGES


//stage 0
void startingStage() {
  //modified rainbow cycle from concept 5
  // Make an offset based on the current millisecond count scaled by the current speed.
  int offset = millis() / 100;
  // Loop through each pixel and set it to an incremental color wheel value.
  for(int i=0; i<10; ++i) {
    CircuitPlayground.setPixelColor(i, CircuitPlayground.colorWheel(((i * 256 / 10) + offset) & 255));
  }

  if (buttonFlag) {
    playGoodSound();
    gameSetup();
    stage = 1; //display stage, start game
    Serial.println("starting game");
    buttonFlag = 0;
  }

}

//stage 1
void displayStage() {
  for (int i = 0; i <= gameRound; i++) {
    allBlue();
    CircuitPlayground.setPixelColor(rexedni[correctSequence[i]], 255, 0, 0);
    Serial.print(i);
    Serial.print(", ");
    Serial.print(correctSequence[i]);
    Serial.print(", ");
    Serial.println(indexer[correctSequence[i]]-2);
    delay(200);
  }
  allBlue();
  delay(1000);
  if (buttonFlag == 1) {
    playGoodSound();
    stage = 2;
    Serial.println("ready for guess");
    buttonFlag = 0;
  }
}

//stage 2
void pickerStage() {
  x = CircuitPlayground.motionX(); // Get the CP accelerometer x and y positions  
  y = CircuitPlayground.motionY(); 
  z = CircuitPlayground.motionZ();
  angle = getAngle(x,y);
  position = getPosition(angle);
  redAmt = getRedAmt(z);
  //printVals(); 
  Serial.print("correct, actual: ");
  Serial.print(correctSequence[guessAmt]);
  Serial.print(", ");
  Serial.println(position);
  updateReds(redAmt);
  //sets the pixel colors based on the pixelReds array
  for (int i = 0; i < 10; i++) {
    CircuitPlayground.setPixelColor(i, pixelReds[i], 0, 255-pixelReds[i]);
  }

  //button pressed
  if (buttonFlag == 1 && !secret) {
    delay(10);
    buttonFlag = 0;
    guess = position;
    if (guess == correctSequence[guessAmt]) //correct guess
    {
      guessAmt++;
      if (guessAmt>gameRound) { //full sequence correct
        gameRound++;
        guessAmt = 0;
        playGoodSound();
        stage = 1; //next round
        if (gameRound == 10) {
          gameRound = 0;
          stage = 4; //win
          Serial.println("win");
          playGoodSound();
          playGoodSound();
          playGoodSound();
        }
      }
    } else { //incorrect guess
      gameRound = 0;
      stage = 3; //loss
      Serial.println("loss");
      playBadSound();
    }
    
  } else if (buttonFlag == 1 && secret) {
    delay(10);
    buttonFlag = 0;
    guess = position;
    gameRound = 9;
    
    if (guess == correctSequence[guessAmt]) //correct guess
    {
      guessAmt++;
      if (guessAmt>gameRound) //full sequence correct
      {
        stage = 6;
      }
    }
  }
}

//stage 3
void lossStage() {
  flasher(0);
  if (buttonFlag == 1) {
    stage = 0;
    buttonFlag = 0;
  }

}

//stage 4
void winStage() {
  flasher(70);
  if (buttonFlag == 1) {
    stage = 0;
    buttonFlag = 0;
  }
}

//stage 5
void testerStage() {
  for (int i = 0; i < 10; i++) {
    correctSequence[i] = specialSequence[i];
  }
  playSecretSound();
  stage = 2;
  guessAmt = 0;
}

//stage 6
void yippeeStage() {
  playSecretSong(); 
  secret = false;
  stage = 0;
}


//RUN FUNCTIONS


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  CircuitPlayground.begin();

  
  //set up the interrupts
  attachInterrupt(digitalPinToInterrupt(SlideSwitch), slideSwitch, CHANGE); 
  attachInterrupt(digitalPinToInterrupt(LeftButton), leftButton, RISING);
  attachInterrupt(digitalPinToInterrupt(RightButton), rightButton, RISING);
  bool switchState = CircuitPlayground.slideSwitch();

  //initializing
  runningSetup();
  generateMIDI();

}

void loop() {
  if (switchFlag && switchCount > 10) {
    if (!secret)
      stage = 5;
    else
      stage = 0;
    secret = !secret;
    switchCount = 0;
    switchFlag = 0;
  } else if (switchFlag) {
    switchFlag = 0;
  }
  switch (stage) {
    case 0:  
      startingStage();
      break;
    case 1:
      displayStage();
      break;
    case 2: 
      pickerStage();
      break;
    case 3:
      lossStage();
      break;
    case 4:
      winStage();
      break;
    case 5:
      testerStage();
      break;
    case 6:
      yippeeStage();
      break;
  }
  

  //delay(500);

}




//INTERRUPT SERVICE ROUTINES


void slideSwitch() {
  delay(10);
  switchFlag = 1;
  switchCount++;
}

void leftButton() {
  delay(10);
  buttonFlag = 1;
}

void rightButton() {
  delay(10);
  buttonFlag = 1;
}

