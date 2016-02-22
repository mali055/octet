
# Assignment: Intro to game programming. Deadline midnight Sun Nov 8th

*Name : Mohammad Ali*

My modification converted it into a top down shump (shoot em up). Below I will detail how and why 
I made the choices I have for the project.

I started with modifying the controls of the ship make it slower harder to dodge, and making it move up and down, 
as well as adding alternate WASD controls. Space was still used to shoot. (app_common and 

In the root of example_invaderers there is a test.csv file which contain information about the enemies that spawn
in game, including in the specific order enemy type (0 = normal, 1 = boss), HP - how much damage they can take 
(ship now deals 10 damage), spawning grid position, average screen space position of where the invaders will spawn 
5 is roughly the far right of the screen, travelling speed of the invader, lastly spawning interval from the start 
of the game instance.

All these stats are stored in internal vectors during init to have a local copy.

All of the mechanics above are implemented, among other things like modifying the AI and limitations such that,
Invaders spawn based on the timers, move down the screen and despawn when reaching the bottom, They take 10 
damage from missiles (so 5 hits to the boss), boss invader is larger and does not go down the screen so you 
have to defeat it to end the game, invaders move in different speeds from the stats and bounce of the 
sides like normal. Those invaders that you ignore are not counted in the score but do count as dead invaders.

The shooting pattern system I wanted to be like the Touhou Project, example below
[Youtube](https://www.youtube.com/watch?v=p7Kc-r4Evuc)

But an implementaion like that is too complex for the scale of this project so I decided to do a 
basic sine curve when spawning bullets, I did attempt to make invidual pattern system class but I had to
revert back to the basic one because of time restrictions. 

Lastly I modified the Fragment shader only slightly, to render a different shade of RGB. I really tried to do something more 
fancy and failed to make it work.

Youtube video of the application is here:
[Youtube](https://www.youtube.com/watch?v=W1yLgegORds)




============================================================


# octet

Octet is a framework for teaching OpenGL and the rudiments of game programming such
as Geometry construction, Shaders, Matrices, Rigid body Physics and Fluid dynamics.

It has a number of examples in the src/examples directory.

To use with visual studio, fork this repository into your own account and then
"Clone Into Desktop" using the GitHub tool and open one of the .sln files in src/examples.

There is a python script for generating your own projects from a template.

From the octet directory run:

packaging\make_example.py my_example

To create your own project in src/examples

Examples should also work with Xcode, although testing is a lot less thorough. If it does not work, send
me a pull request with fixes, please...

Octet is a bit unusual in that it does not use external libraries such as libjpeg or zlib.
These are implemented in source form in the framework so that you can understand the code.
The source of most academic libraries is almost unreadble, so we aim to help your understanding
of coding codecs such as GIF, JPEG, ZIP and so on.
