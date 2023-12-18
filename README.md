# Snake-Game

The Snake Game, an iconic and timeless classic in the world of video games, originated in the mid-1970s. The simple yet addictive gameplay involves controlling a snake that grows in length as it consumes food, while avoiding collisions with itself. Over the years, this game has seen numerous adaptations and remains a popular choice for developers and hobbyists exploring programming and game design on platforms like Arduino. This enduring game continues to captivate players with its straightforward mechanics and nostalgic charm.

I selected the Snake Game for its sentimental value—it reminds me of my childhood when I spent countless hours immersed in this classic on my trusty Nokia 3310. The decision to implement this game was made because I wanted to relive those nostalgic moments. This project not only pays homage to a beloved pastime but also serves as a personal journey down memory lane, rekindling the joy and simplicity of early mobile gaming.

# How to play

On the matrix display, the snake moves around the screen without stopping, eating food and getting longer. The user can steer the snake in any direction using the joystick. If the snake collides into itself, that means the game is over, in which case the game stops and a message is displayed on the LCD Display. It is a simple yet entertaining game that can be played by anyone, no matter the age. 

# Components
| Component  | Link |
| ------------- | ------------- |
| LCD Display  | https://docs.arduino.cc/learn/electronics/lcd-displays|
| 8x8 Matrix Display  | https://docs.arduino.cc/built-in-examples/display/RowColumnScanning  |
| Joystick  | https://arduinogetstarted.com/tutorials/arduino-joystick  |
| Buzzer  | https://www.ardumotive.com/how-to-use-a-buzzer-en.html  |
| Potentiometer  | https://docs.arduino.cc/learn/electronics/potentiometer-basics  |

# Menu display

Upon powering up the game, a greeting message briefly welcomes the player.
The menu in the game is designed for a seamless and user-friendly experience. Here's how it works:

* Start Game
  * Initiates the game, beginning at the initial level for an exciting gaming session.
* Settings
  * LCD Brightness Control
    * Adjusts the brightness of the LCD display. The player can control the brightness using the joystick. Changes are saved to EEPROM for a persistent setting.
  * Matrix Brightness Control
    * Allows adjustment of matrix brightness using the joystick. The changes are reflected in real-time on the matrix display. The selected brightness level is saved to EEPROM.
  * Sounds On/Off
    * Enables or disables game sounds using the joystick. The preference is saved to EEPROM.
* About
  * Displays details about the creator(s) of the game, including the game name, author, and a link to the GitHub profile. Scrolling text ensures a dynamic and engaging presentation.

In-Game Display:
        While playing the game, the menu provides continuous updates on key information such as lives, level, score, and potentially time, creating an immersive gaming environment.

Game Ending Menu:
        Upon completion of the game, a menu appears with relevant game information, including the final score, time played, and lives remaining. Players are informed if they have beaten the high score. This menu can only be closed by the player, ensuring a deliberate action to conclude the gaming session.

