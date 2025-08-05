# Simons-Game
Simon’s Game is like a fun test of memory where you watch a pattern of lights and sounds, then try to copy it without messing up. The sequence increases in length after every round. 

1. This project implements simon's game on LPC2378 using 4 different colored LEDs and a push button for each LED.
2. Each LED also has a unique sound associated with it which is generated using the DAC.
3. The random sequence is generates using the noise generated in the ADC channels.
4. The user neeeds to repeat the generated random sequence with 4 seconds for each push, else the game ends (this is handled using a timer interrupt).
5. The user inputs act as an external interrupt. When a wrong button is pushed the game ends.
6. When the game ends, "Game Over" is displayed on the inbuilt LCD module. The game restarts with a new random sequence.
7. The current score and the high score is displayed on the inbuilt LCD module.
