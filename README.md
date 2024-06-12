Battleships game in SDL 🛥️🚤🏴‍☠️🦜

Hubert Rutkowski && Mateusz Kazmierczak

![TheBlackPearlGIF](https://github.com/hubert-rutkowski/battleships/assets/90204209/5f1cb504-e545-44ea-b7e8-5b5191dd3c2b)

To play you must install the required libraries using the command "sudo apt-get install libsdl2-dev libsdl2-ttf-dev".

Then download the repo and navigate to the battleships directory.

To run the game, type the command "make run". The makefile will take care of everything.

Battleships Game Rules (Player vs Computer)
Game Setup
Board Setup:

The game is played on a 10x10 grid.
Both the player and the computer will have their own separate grids.
Ship Placement:

The player places a total of 10 ship segments on their grid as follows:
Two ships, each consisting of 3 segments.
One ship consisting of 4 segments.
Ships can be placed horizontally or vertically, but not diagonally.
Ships cannot overlap or be placed adjacent to each other; there must be at least one grid space between them.
Computer's Ships:

The computer will also have the same configuration of ships (two 3-segment ships and one 4-segment ship).
The computer will place its ships randomly on its grid, adhering to the same placement rules as the player.
Gameplay
Player's Turn:

The player goes first.
The player selects a grid coordinate (e.g., B4) to fire a shot.
Hit or Miss:

If the player's shot hits a part of any of the computer's ships, it is a "hit."
If the player's shot does not hit any ship segment, it is a "miss."
When a player hits, they get an additional turn and can fire another shot.
If the shot misses, it becomes the computer's turn.
Computer's Turn:

The computer fires at a randomly selected coordinate on the player's grid.
The computer follows the same hit/miss rules as the player.
If the computer hits a ship segment, it gets another turn.
If the computer misses, it becomes the player's turn again.
Winning the Game
The game continues until all segments of one side's ships are completely hit.
The first side (either the player or the computer) to have all their ship segments hit loses the game.
The remaining side with any ship segments still intact is the winner.
Additional Rules
Players must call out their coordinates clearly.
Once a shot is fired, the chosen coordinate cannot be fired upon again.
Players should keep track of their own shots and hits/misses to strategize effectively.
Ending the Game
The game ends when either the player or the computer has no remaining ship segments.
Congratulate the winner and optionally start a new game with ships rearranged.
Enjoy playing Battleships against the computer!
