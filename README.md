# Checkers Server
"Object-oriented programming languages" C++ project. In this project I used SFML and ASIO library (https://www.youtube.com/@javidx9 - Link to the channel I learnt about ASIO library).
The Server is required to play. Server can handle only one game and after that you have to restart it. The first player which connects to the server is white. After winning the clients are closed.
The server uses uses tcp/ip protocol, and is used only for forwarding game messages between clients. (The server was tested only in the local network with ipv4 addresses)
The program requires some upgrades, but the game works (you can play with another player xD). I will try to finish and it in the future. 
\nUpgrade ideas:
* multiple games at the time
* registering users (database)
* Users ranking and point's for each game
* more advanced gui (without terminal)
  
Checkers Client link - https://github.com/olek-dot-com/CheckersClient
