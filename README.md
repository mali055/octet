# Assignment: Tools and Middleware. Deadline midnight Sun Nov 8th.

My project modifies the shapes examples into a basketball simulation, use space to shoot upto 5 balls into a hoop. below I describe all the steps I did so far.

I started with adding controls to the game, using the draw world update to call a function that checks for inputs and moves the camera (W A S D controls) Shift / Ctrl for up and down or shoots a ball (Space bar) WASD was added in app_common aswell. As well as the mouselook for mouse look controls.

Next I started setting up the scene objects like backboard and hoop and net (which would eventually use hinge constraints). Starting with the back board I used a large cube mesh and gave it a fixed constraint.

The hoop is made of four cubes in kind of a net shape attached to the back board / wall with static placement. Originally i tried to us a torus shape generated in a modified file of mesh_cylinder & custom math\torus.h . I tried to make my own scene\mesh_torus.h but the macros defined in octet would not accept the new class (even tried classes.h / math.h etc). This is something I would like to discuss and understand at some point.

The net is just some long cubes attached with spring contraints. Which seem to behave too erradically. I tried numerous stuffness and dampening values, locking the angular rotations, and the x and z axis so it only springs on the y axis but nothing got it behave in a stable way. Besides the springs the 4 cubes are attached to the hoop above them with a spring constraint, and attached to each other with a hinge constraint in a circular way. Then another pair of cubes are attached below them with hinge constraints. All this to simulate a basketball net like scenario.
Hinges seem to work its just the springs dont come to rest (dampening does not seem to work on any axis properly)

Then I spawn the main balls that the player will shoot out and disabled its redering and physics simulation, until space is clicked to shoot them forward from the camera.

For collision callback I set up a check_collision function which iterates through the bt Collision Manifolds to find if any of the balls collides with the wall and changes the color of the ball when it does. 

Youtube video of the application is here:
[Youtube](https://www.youtube.com/watch?v=PkUuEWeBuyk)


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
