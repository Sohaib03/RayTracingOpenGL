# RayTracing Project

## Overview
This project implements a simple Ray Tracer capable of rendering scenes with spheres, cubes, pyramids, and light sources. The application uses OpenGL for visualization and includes features such as reflections, shading, and texture mapping.

## Features
- **Geometry**: Support for rendering spheres, cubes, pyramids, and a textured floor.
- **Materials**: Different objects can have varying reflective and refractive properties.
- **Lighting**: Handles both point lights and spotlights, simulating realistic lighting effects.
- **Texture Mapping**: Optional texture mode for objects using bitmap images.

## Getting Started
1. Clone the repository.
2. Ensure you have the necessary dependencies installed, including OpenGL and GLUT.
3. Compile the code using a C++ compiler.
4. Run the executable.

## Usage
- Use the keyboard to control the camera and toggle features.
  - `W`, `A`, `S`, `D`: Move the camera forward, left, backward, and right.
  - Arrow keys: Adjust the camera's view direction.
  - `Space`: Toggle texture mode.
  - `0`: Capture and save the current scene.

## Configuration
- The scene and rendering parameters are loaded from the `input.txt` file.
- Customize object properties, lights, and camera settings in the input file.

## Dependencies
- Ensure OpenGL and GLUT are installed on your system.

## Examples
![Objects Before RT](https://github.com/Sohaib03/RayTracingOpenGL/blob/main/Screenshot%202024-02-11%20203718.png)
![Objects After RT](https://github.com/Sohaib03/RayTracingOpenGL/blob/main/out1.png)
## License
This project is licensed under the [MIT License](LICENSE).

Feel free to explore and modify the code to suit your needs. If you encounter any issues or have suggestions, please open an [issue](https://github.com/yourusername/raytracing-project/issues). Happy rendering!
