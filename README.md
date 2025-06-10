# ENGR-103-Game
Final project for ENGR 103. Memory game.

Game: This is a memory game. In it, you will be shown various sequences of lights, and then you will input them by tilting the circuit board and pressing a button.
Rules: First, press either button to start the game. Now, the board will begin flashing one light red. This is the sequence (of length 1) that you need to remember. Press any button when you are ready to enter the sequence. To enter the sequence, tilt the board so that the desired light is red, then press a button to enter your guess. For the first sequence of length 1, you only need to do this once. If you enter the full sequence correctly, the board will flash the next sequence but with increased length. If you get to round 10 (sequence of length 10) and correctly input it, you win and you will go to the win screen. Press any button from there to return to the start screen. If at any point you enter an incorrect guess, you will go to the loss screen. Press any button from there to go to the start screen and restart the game.
There is an easter egg in this game.

Inputs:  their function, their raw ranges, what they will be assigned to control, any threshold or map() functions applied.
  Accelerometer:
    function: to input guesses
    raw range: x, y, z values ranging from -10 to 10
    control: where your guess appears on the board
    mapping: trigonometry is used to convert x and y into an angle from 0 to 360  degrees, which is converted to a position from 0 to 11. A running average is taken of z, called zSmooth, which is then mapped: map(abs(zSmooth), 5, 9, 255, 0) to produce a value for an amount of red in a color.
  Buttons:
    function: to advance through screens and confirm guesses
    signal used: RISING
    control: movement through the game
    mapping: N/A. Implemented via interrupts.
Outputs: 
  speaker
  neopixels
