# 3D Surface Particle Simulation System

A real-time 3D particle system built with **C** and **OpenGL/GLUT**. The application simulates dynamic physical phenomena (fire, smoke/steam, and rain) emitted directly from the polygonal faces of 3D mesh objects.


## Key Features

* **Mesh Surface Emission:** Calculates face centroids and surface normals to emit particles directly from polygonal faces (cubes, pyramids, prisms, octahedrons).
* **Multiple Particle Behaviors:**
  * **Fire:** Additive blending (`GL_ONE`), velocity decay, and color shifting over time.
  * **Smoke / Steam:** Alpha-blended fading with procedural turbulence.
  * **Rain:** Cloud height offset, gravity acceleration, and **plane collision detection** with surface splashing.
* **Environmental Forces:** Real-time wind vector controls ($X, Y, Z$ axes).


## Tech Stack

* **Language:** C / C++
* **Graphics Library:** OpenGL, GLUT
* **Concepts:** 3D Vector Math, Linear Algebra, Particle Physics, Lighting & Materials
