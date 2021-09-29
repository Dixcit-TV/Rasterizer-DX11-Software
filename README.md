# Rasterizer-DX11-Software
Merge between software and hardware rasterizer - Allowing to "seemlessly" switch between both at runtime

Sofwtare rasterizer

![Software Rasterizer](https://user-images.githubusercontent.com/76394390/135262746-8e964c6e-a392-44fe-82e4-4bf4209ed650.jpg)

Hardware rasterizer

![Hardware rasterizer](https://user-images.githubusercontent.com/76394390/135262625-118b643d-02e7-471e-bc36-f2fae4467ebc.png)

## Introduction
Deeper in Graphics Programming 1, we were introduced to the rasterization technique for rendering. First I implemented a righ-handed software rasterizer to get a hand of the concept and understand the complete rasterization pipeline. Then came DX11 and the implementation of a hardware rasterizer using the Effect Framework from DirectX.

As final project, I then merged both software and hardware rasterizer into one single project with the ability to switch between both setups at runtime (and "seamlessly").

## The challenges
First of all the main, requirement was to have only 1 renderer, calling the project draw calls according to the chosen rasterizers, so having one renderer for each was out of the question.
Then another big challenge was that both rasterizers shares quite of lot of data and structures; cameras, textures, meshes, etc... But also need different resources to make use of it, especially the hardware rasterizer requiring all the DX11 objects and handles. I didn't need to extra resources eating up memory if they were not going to be used at all with the current rasterizer. I then created a resource (un-)loading functions that are called on the render, meshes and textures when the switch occurs, and will either load or unload all the extra resources.
Another aspect to solve was that the sofware rasterizer is built as a Right-Handed system but DX11 uses a Left-Handed system. To use the same camera for both cases required to convert between system at the moment of the switch. It was done by re-calculating the correct projection matrix and flipping the camera controls and ONB.

Transparency on a software rasterizer was also something that we didn't cover but I found it a shame to have it on one side and not the other. Re-analysing my course content on how transparency work theoretically (and on DX11) I tried to integrate it to my software rasterizer. It turned out great and required a surpisingly low amount of code change.

## What can these rasterizers do?
The rasterizers provide the following features:

OBJ file loading (removes duplicate vertices).
Triangle meshes rasterization.
Directional lights.
Transparency
Front-, Back- & No-Culling modes
Fustrum culling
Filtering (Point, Linear & Anisotropic) - Hardware Only
Dynamic camera (UE4-like controls)
