# BLE Clone Hero
Code for an ESP32 powered Bluetooth Clone Hero guitar controller.

I wrote this code about a year ago now I think, maybe longer. I don't fully remember all of the descisions I made when writing this, so some of the comments are me guessing what's happening and why I chose to do things the way I did.

It's a bit messy, and could definitly use some cleaining up.

My initial intention was to attempt to "spoof" the device ID's and layout of a genuine Guitar Hero controller, in the hopes that Clone Hero would automagically set up all of the buttons and inputs correctly, creating a plug and play device with minimal setup from the user.

This, however, didn't work out as planned. I chose to leave the code unchanged from this approach though, since everything was in a functional state and I didn't fancy gutting it. This is why there are 10 buttons defined in the BLE Gamepad configuration, and why there are some Vid and Pid values defined (they're commented out).

This uses the (BLE Gamepad)[https://github.com/lemmingDev/ESP32-BLE-Gamepad] library, so make sure you have it installed.