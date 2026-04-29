**F** to activate physics (necessary)  
**Left Click** to **fire** a particle repellor

# Volumes of Particle Attraction - Worksheet 4
In this final iteration, I implemented the following:
 - Projectile repellors (fired with Left Click)
 - Improved collision physics combining both strict and fluid collision
 - Improved optimisation through culling of objects and rendering a sphere instead of using a model
 - Final tweaks to improve the experience of the simulation

With this iteration, I was able to fully implement and polish the primary elements of my effect, with a hybrid approach to collision allowing the particle attractor volumes to shape in response to repelling forces.

![image](https://github.falmouth.ac.uk/GA-Undergrad-Student-Work-24-25/COMP305-2200066/assets/1434/abbde33d-89f6-48a0-ad51-d07ba0abf96b)

However, I was not able to implement the creation of particle attractor volumes in the shape of meshes provided by loaded models, which would be my goal in further development. My effect currently fulfils the requirements for my effect regardless, allowing a user to use particle repellors to shape, interact with and destroy a volume composed of particle attractors, with the shape of the volume determined by the physics and forces of the attractors.

## Profiling Results
For these profiling tests, I generated a 10x10x10 cube of particle attractors (1000 in total) and shaped/destroyed the cube with repellors until it was fully removed.

![image](https://github.falmouth.ac.uk/GA-Undergrad-Student-Work-24-25/COMP305-2200066/assets/1434/f351543a-8b3d-4c4e-b9c0-0f5e6e4c27b0)

### CPU
The performance, though low in FPS at first with 1000 particles, was comparable to the performance in Worksheet 3 with 216 particles, with larger amounts of particles now performing far better than they did in previous iterations, likely due to the removal of model loading and replacement with manual rendering. Notably, the performance quickly increased during the simulation as no-longer-used particles were culled. These results are reflected in the CPU profiling, showing that the previous bottlenecks of simulating repelling forces and creating the particle attractor volume now have an extremely minimal cost on performance, instead replaced by the simulation of Collisions and Attraction forces at 43.85% and 43.50% respectively.

As shown by the image, these is mostly due to length calculations used to measure collision and attraction forces. Though attempts were made to optimise this, I could not find a workaround to these bottlenecks within this iteration, especially due to the specific order the functions needed to be performed in, preventing the respective mathematical calculations of the functions from being combined. Altogether however, I am pleased by the progress made in optimisation, and believe that further optimisation would be possible, but is not required to effectively use and interact with the simulation in its current state.

![image](https://github.falmouth.ac.uk/GA-Undergrad-Student-Work-24-25/COMP305-2200066/assets/1434/80917e8c-6265-4808-8b1d-e04e0cb92892)

### Process Memory
Memory usage was very consistent throughout every test, always settling at approximately 80MB. This is an improvement on both Worksheet 1 and 2, likely in part to the removel of unnecessary model loading.

![image](https://github.falmouth.ac.uk/GA-Undergrad-Student-Work-24-25/COMP305-2200066/assets/1434/9ee40762-f34c-488c-9de0-ef7c4a137047)

Notably, there was a large spike in GPU utilisation at the beginning of these tests, which was not found in previous iterations. This could be due to the large number of particles rendered at the beginning of the simulation. Regardless, GPU usage quickly reduced to similar levels as before.

![image](https://github.falmouth.ac.uk/GA-Undergrad-Student-Work-24-25/COMP305-2200066/assets/1434/e0dd8ace-abef-4c8c-9d41-9126137ce7b2)

## Next Iteration
Though my digital effect is finished, if I were to continue development, I would plan to implement:
  - The replacement of loaded 3D models with particle attractors filling the mesh
  - Plane-based repellor (split volumes in half)

To futher improve performance, I would plan to:
  - Explore physics libraries and alternative designs to optimise my physics and/or distance calculations.

## Project Resources
https://natureofcode.com/particles/ - Particle systems involving attractors and repellers  
https://open.clemson.edu/all_theses/2002/ - Particle simulation, with reference to the mechanics of particle attractors  
https://doi.org/10.1002/vis.263 - Very helpful/interesting stuff on attractors (and cutters, though irrelevant for this)  
https://github.com/nothings/stb/blob/master/stb_image.h  
https://free3d.com/3d-model/bird-v1--282209.html (Personal Use License)

Sphere created using Blender
