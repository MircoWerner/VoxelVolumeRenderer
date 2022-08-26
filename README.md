# VoxelVolumeRenderer

Real time volume rendering of large voxel data sets (from cell growth simulations) with support for global ambient occlusion (caused by near and distant occluders) and emission from many voxels.
This program was created as part of my bachelor thesis at KIT (Karlsruhe Institute of Technology, Germany), Computer Graphics Group in summer term 2022.

> **Abstract**
> To support the visual perception of the spatial structure when rendering large three-dimensional voxel data sets in real time, approximations of global lighting effects like ambient occlusion or emission are essential. 
> Our renderer uses accelerated ray marching by employing a signed distance field to render the static voxel volume. 
> We present Voxel Distance Field Cone Traced Ambient Occlusion, which exploits the presence of the signed distance field to transform the distance values into occlusion values that are sampled front to back by multiple cones. 
> In contrast to the original Voxel Cone Traced Ambient Occlusion, our method achieves higher framerates and does not require an additional three-dimensional occlusion texture or sparse voxel octree. 
> Additionally, we combine the result with a cheap, local approximation of ambient occlusion, designed especially for voxel rendering, to increase the overall quality of the occlusion caused by near and distant occluders. 
> Moreover, this [program ~~thesis~~] introduces an approximation for light emitted by voxels by assigning light levels to solid voxels and propagating the light iteratively through the scene while keeping track of a limited number of directions the light can travel to limit cases of physically incorrect propagation. 
> Combining the ambient occlusion and emission techniques, the visualization of the voxel data supports both the perception of the spatial structure, including large features like cave structures, and the highlighting of important parts of the data set that are missed otherwise.

<img src="https://user-images.githubusercontent.com/34870366/186930969-4989a5b9-4331-4419-a861-d9c49266fd6f.jpg" alt="img: vdcao + ddfe" style="max-width: 960px; max-height: 960px">
<br />
Scene rendered with constant ambient lighting only (left) and with ambient occlusion and emission (right). The spatial structure is just perceptible on the right. Some colorful cells are emphasized by letting them emit light. Rendered with VDCAO + DDFE.

## Table of contents
1. [ Features ](#features)
2. [ Build ](#build)
3. [ Controls ](#controls)
4. [ Results ](#results)
5. [ References ](#references)

<a name="features"></a>
## Features
- Accelerated ray marching of large voxel volumes using a global signed distance field.
- Various ambient occlusion techniques (shader code .
    * (Accumulated) Ray Traced Ambient Occlusion (RTAO) (ground truth)
    * Voxel Distance Field Cone Traced Ambient Occlusion (VDCAO) (ours)
    * Voxel Cone Traced Ambient Occlusion (VCTAO)
    * Distance Field Ambient Occlusion (DFAO)
    * Local Voxel Ambient Occlusion (LVAO)
    * Horizon-Based Ambient Occlusion (HBAO)
- Techniques to gather emission from many voxels.
    * Directional RGB Distance Field Emission (DDFE) (ours)
    * Voxel Cone Traced Emission (VCTE)

<a name="build"></a>
## Build
Linux
1. Clone the repository recursively: `git clone --recursive git@github.com:MircoWerner/VoxelVolumeRenderer.git`
2. Change to the working directory: `cd VoxelVolumeRenderer`
3. Copy additional required GLEW sources to the GLEW submodule: `./pre-build.sh`
4. Create build folder: `mkdir build`, `cd build`
5. Generate makefile: `cmake ..`
6. Build: `make`

Requires C++ 17 and OpenGL 4.3 support.

<a name="execute"></a>
## Execute
1. Place the volume data and csv file under `resources/volume/[400|1000]/`. **IMPORTANT: The resources cannot be published at this time.** Test data may be provided the future.
2. Execute: `./VoxelVolumeRenderer [--v400|--v800]`. `--v400` loads the 400^3 data set, `--v800` the 1000^3 data set that size is reduced to 800^3.

<a name="controls"></a>
## Controls
- W,A,S,D,Shift,Space: Move the camera forward,left,backward,right,down,up.
- Right mouse button (press and hold) and dragging the mouse: Rotate the camera.
- Mouse wheel: Zoom the camera.

<a name="results"></a>
## Results
<img src="https://user-images.githubusercontent.com/34870366/186929418-12478344-2444-4173-bbe4-e0d33427cdcb.jpg" alt="img: vdcao + ddfe">
<br> 800^3, far. VDCAO + DDFE.
<br>
<br>
<img src="https://user-images.githubusercontent.com/34870366/186929383-ae2201dd-0f70-4ab2-88f0-095490e551d2.jpg" alt="img: vctao + vcte">
<br> 800^3, far. VCTAO + VCTE (diffuse and specular cones).
<br>
<br>
<img src="https://user-images.githubusercontent.com/34870366/186929443-fe8538c0-0a36-4455-82c6-ea4a99ba7596.jpg" alt="img: vdcao + ddfe">
<br> 800^3, near. VDCAO + DDFE.
<br>
<br>
<img src="https://user-images.githubusercontent.com/34870366/186929432-56d66ce9-30c6-417e-947a-202e16abf148.jpg" alt="img: vctao + vcte">
<br> 800^3, far. VCTAO + VCTE (diffuse and specular cones).
<br>
<br>
<img src="https://user-images.githubusercontent.com/34870366/186929511-9e9f5230-b3d0-46c3-b962-00a5d4748d86.jpg" alt="img: vdcao">
<br> 400^3, near. VDCAO.


<a name="references"></a>
## References
*Main* references for the implementation.
- Accelerated ray marching: 
  - DDA: https://www.researchgate.net/publication/2611491_A_Fast_Voxel_Traversal_Algorithm_for_Ray_Tracing
  - Distance field acceleration: https://doi.org/10.1007/BF01900697
- Ambient occlusion:
    - RTAO: https://doi.org/10.1201/b22086
    - VCTAO: https://doi.org/10.1111/j.1467-8659.2011.02063.x
    - DFAO: https://doi.org/10.1145/1185657.1185834
    - LVAO: https://iquilezles.org/articles/voxellines/
    - HBAO: https://doi.org/10.1145/1401032.1401061
- Emission:
    - DFE: https://0fps.net/2018/02/21/voxel-lighting/ (without keeping track of some directions)
    - VCTE: https://doi.org/10.1111/j.1467-8659.2011.02063.x