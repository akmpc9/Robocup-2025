The Code on our robots worked with two separate components: the esp32 and the camera. The esp32 controlled the robot's movements through the motor controllers while the camera, being the only sensor on board, made of all the stategic decisions. The esp32 was slave to the camera and just recieved simple instructions to move, which it then translated into the individual motor speeds and sent the right instructions to the motor controllers.

abcdefg
