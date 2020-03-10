# Descrition
Windows only task manager

## Compile
g++ -o main main.cpp -lPdh -lPsapi

## Control
If press sort button 2 times - will reverse sort
* R - sort by RAM
* C - sort by CPU
* N - sort by Name
* I - sort by PID
* M - B -> KB -> MB (After MB - goes back to bytes)
* T - show as tree
* Arrow UP - scroll up
* Arrow DOWN - scroll down