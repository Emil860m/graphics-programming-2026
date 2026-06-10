# Lecture 0:
## Understand the main applications of computer graphics.
Movies, games, design documents, architecture etc.
## List the basic elements of a virtual scene.
Objects:
    Models, particle systems, volumes


Camera:
    Viewer of the scene
    
Lights:
    makes objects visible
## List advantages and disadvantages of raster graphics and ray tracing.
rasterized graphics: WebGL, OpenGL, DirectX

Each geometry is projected to the camera. Then process visible pixels of that geometry and compute its color. Meaning, each pixel on the screen that contains the geometry is the color of the geometry at that point. Light is estimated rather than taken directly from the source, making it look less realistic. (This is what we have focused on for the course)

raytracing graphics: OptiX, Vulkan, DirectX12

Process is kinda the inverse. A ray is cast from each pixel to find the intersection with the geometry. Then it can bounce to find a light source. This creates more accurate light simulation, but takes more computing power.

## Describe what VBO, EBO, VAO and handles stand for when developing with modern OpenGL.

Rendering pipeline: 

Vertex specification -> Vertex Processing -> Vertex Post-Processing -> Primitive Assembly -> Raserization -> Fragment Processing -> Per-Sample Operations

### VBO: Vertex Buffer Object

Stores vertex data (attributes), such as positions, colors, texture coordinates, normals etc

    layout (location = 0) in vec3 VertexPosition;
    layout(location = 1) in vec2 VertexUV;

### VAO: Vertex Array Object

Stores which VBOs are used for attributes, attribute layout and the EBO

### EBO: Element Buffer Object

Stores indicies that specify which vertices should be used to form primitives. (If no EBO, )

Can share vertices to reduce memory

Example in my project: 

    indices.insert(indices.end(), {
        i0, i2, i1,
        i1, i2, i3
    });

### Handles: 

Integer identifier that refers to GPU resource.




# Lecture 1

## Understand the different steps that conform the rendering pipeline.

Vertex specification -> Vertex Processing -> Vertex Post-Processing -> Primitive Assembly -> Raserization -> Fragment Processing -> Per-Sample Operations

VS: Takes input data and builds list of vertices to process, like building the mesh/model

VP: Process each vertex individually, in parallel (vertex shader code)

VPP: Send vertex stream to Rasterization stage. Can also be used to store transformed vertices into buffer objects

PA: Build the primitives for the Rasterization. Build from the EBO, if it exists

R: Transform primitives to screen. Break into fragments. Interpolates if needed

FP: Process each fragment individually, in parallel (fragment shader code)

PSO: Post-Processing. Writes the pixel color, blending the fragment color with previous from draw buffer.


## List the basic primitives used for rendering. Explain why we almost exclusively render triangles.

points, line, triangle, patches?

Triangles: Smallest number of edges to create a 2D object. From that everything else can be created.

## List common attributes of vertices, such as position, color or normals.

position, color, normals, uv (texture coords), tangent

## Explain how the depth-buffer works, why we need a depth-buffer, and what kind of problems we can have without it.

The depth buffer is a depth value per pixel. When rendering geometry compare each pixel depth value of each geometry. Render the closest one.

If we did not have a depth buffer, we would have a render order that made it so that objects would appear as though they are always in front of others even when not supposed to.

## Explain how linear interpolation is used in rendering.

Used to estimated each fragment from the complete primitive. Color of each vertex multiplied by the area of the opposite triangle. Then devide by total area.

    F = ( A*Area(BCF) + B*Area(CAF) + C*Area(ABF) )/ Area(ABC)

Used in the Rasterization step in the pipeline



# Lecture 2

## Describe what are shaders and list the types of shaders.

Shaders are computer programs that run on the CPU. Single instruction, multiple data (SIMD)

Vertex shaders, fragment shaders, compute shaders, optionaly geometry shaders and tessellation shaders

## Explain the difference between vertex and fragment shaders.
The vertex shader controls the position of the primitives, and requires a gl_Position output. This is used in the vertex processing step of the pipeline.

The fragment shader controls the color of each pixel. The pixel is interpolated from the primitive. This is used in the fragment processing step in the pipeline.
## List basic uses of vertex and fragment shaders.
Vertex: Position, Tessellation, Geometry, normals, tangents, texture coordinates

Fragment: Colors, Texture mapping, Lighting, transparency, normal mapping without adding geometry, 
## Describe what swizzling is and how it is useful.
The swizzling operator is used to combine and rearrange the vector. It is useful as it saves on temporary variables and makes code more readable


# Lecture 3
## Describe the properties of an orthonormal matrix.
All axis are Orthogonal and each axis is normalized
## Describe how you can recognize if a transformation matrix is a rotation, scale, sheer or translation.
Scale: has 0 on all other spaces

        [sx, 0, 0]
    S = [0, sy, 0]
        [0, 0, sz]

Rotation: euler angles/axis angles/quaternions

translation: A displacement vector. Is always (x, y, z)
## Explain what we mean with basis vectors, and how it relates to coordinate spaces.
Basis vectors are vectors that are linearly independent (They cannot be written as combinations of the others) and span the space (All vectors in the space can be written as a linear combination of them)

for R3 the standard basis is: 

    [1]     [0]
    [0]     [1]

Meaning that if we apply the vector to the standard vector:

    [3]
    [2]

Then it means we move 3 in the direction of the first vector and 2 in the direction of the second. If we used other vectors than the standard basis. 

In short, it means that the coordinate representation of a vector depends on the chosen basis.

## Describe how we can transform from one coordinate space to another.
Apply each matrix to a new basis

In short: Add the offset between the origin of the 2 spaces to one to get the coordinate in another.
## Explain the difference between orthogonal and perspective projections.

Orthographic: No deformation, preserves parallel lines 

Perspective: Objects look larger the closer they are (like eyes)

## Describe what are the elements in a projection matrix.

Horizontal scaling. It controls the scale of the x coordinate

Vertical scaling. Same with the y coordinate

2 depth mapping terms. They map depth to the near and far clipping planes. Also depth information and ensures points outside the viewing can be clipped.

Lastly, Perspective term: -1 

    w_clip = -z_view

w_clip is what is used in the perspective divide

Everything else is 0, since no skewing and coordinates are transformed independently

## Explain what is the role of the “perspective divide” step in the graphics pipeline.

How objects that are farther away looks smaller. This is done by dividing with the depth information.


# Lecture 4
## Describe what is a texture and how it can be used.
OpenGL Object
Contains one or more images

is either a source of a texture or used as a render target
## List the different types of textures
Texture ND: grid with N dimensions (N= 1 2 or 3)

Cubemap: 6 2D textures organized as a cube

Texture arrays: group of several textures of the same type and dimensions

## Explain what are mipmaps and their purpose
mipmaps are smaller versions of the texture in power of 2 size. it can be used for minification 
## Describe what is a material and the relationship to textures
Materials do not exist in OpenGL

Rather we use it as a way to group shader uniforms and other properties that affect rendering of an object. Materials can take textures as uniform samplers to use in the shader code

## Explain how blending works and list some common blending combinations

Combining the fragment color with the destination color. Usually used for adding colors together or for transparency. Or multiplicative


# Lecture 5
## List the variables required to compute the Phong reflection model and describe their roles.
Ambient: Intensity * Reflectence constant * color

This results mostly in just the color of the object and how visible it is. One single constant color across the entire object

Diffuse: Intensity * Reflectence constant * color * max(light direction - normal, 0)

This is the directional light. The object is darker on the opposite side of the object from the lightsource

Specular: Intensity * Reflectence constant * color * max(normal - half vector of light and view , 0)^specular exponent

This is the "mirror" style of directional reflection. This creates the spots that looks like the object is shining.

These are all added together to get the full effect
## Explain the difference between flat shading, Gouraud shading, and Phong shading, and how these relate to the graphics pipeline.
Flat shading: Only one normal per triangle. Gives a polygonal look

Gouraud shading: Vertices share the average normal. This is computed in the vertex shader. Usually fast

Blinn-Phong shading: Vertices share average normal, but here the normal is passed to the fragment shader, and the lighting is computed in fragment shader

## List the shadow techniques seen in class.

Shadow volumes: Hard shadows

Essentially, each object has volumes drawn away from the lightsource, and objects inside this volume is in shadow

Shadow mapping:

Creates textures that is stored in framebuffers, for then later to used that to apply shadows. This is done from the lightsource rather than the camera. 

## Describe the shadow mapping algorithm.

Render a pass from the light source. Render to a texture, storing depth values. Later, use that texture and compare fragment depth to see if each fragment should be in shadow.


# Lecture 6
## Describe what is normal mapping, what are its strengths and weaknesses.

A normal map is a "black-and-white" (mostly blue) texture that can be used to added to a surface to either rough up or smooth out a surface. 

It can be used to add "fake geometry" to a flat surface, meaning that the flat surface now can be treated as if the normals would have a geometry.

I used it for both terrain for my beach, and my water surface disturbance. I used it in my terrain by loading in the image file, and for each pixel, use the brightness to set a height.
## Explain what tangent space is, and how it is used in normal mapping.
Tangent space (and binormal) is the coordinate system of the texture. When using a normal map, the normals are all in relation to the tangent space. To compute the normals in world space (or model), we compute the TBN, and from that we can use the normals.

## Explain what environment mapping is, how it is used to produce reflection and refraction, and what are the limitations.
This is how an object is affected by the environment. Usually modelled as reflection/refraction or indirect lighting.

The way (also the way i did it) it is used in reflection/refraction is by fx. baking the environment into a cubemap. This fakes reflection and refraction.

As my project, it does not give perfect real reflections/refractions as it uses a texture rather than using a framebuffer to apply the effects. It also does not react to lighting changes or moving objects

## Explain why reflection and refraction are impractical in the raster graphics pipeline.
It is all faked, unlike raytracing. Rasterization only sees the visible objects from the cameras perspective. Both reflection and refraction requires additional rays to be sent out, to find out which objects or surfaces should be reflected/refracted.

# Lecture 7
## Explain how forward rendering works
Forward rendering renders objects and computes light immediately during fragment shading. Each visible fragment computes the scene lights and writes is color directly to the framebuffer.

Bad with many lights, as it renders all objects for each light.
## Explain how deferred rendering works, and how it compares to forward rendering.
Deffered rendering method "defers" lighting to a later stage. Meaning it renders all objects without light to g-buffers, then does another pass and renders the lighting after.

Compared to forward rendering, it is far it is additative big O notation rather than multiplicative, but it requires far more to be stored in memory
## Explain what is a G-Buffer and list the most common content stored in them.
Textures that store geometry data. This could be any values that could be needed for later passes. Depth values, Albedo, Normals, lighting values (fx. for Blinn-Phong)


## Describe the problems of transparency in deferred rendering.
Transparent objects require the objects behind the transparent object to be rendered with the light. Otherwise the opaque object might look like it is receiving the light that the transparent object is getting, since there will only be one depth when lighting is rendered.

# Lecture 8
## List the conditions required for physically based rendering.
Energy conserving

Physically based BRDF

microfacet surface model
## Explain what the term "energy conserving" means.
It means that energy cannot be created. Fx. an object cannot be both perfectly diffuse and perfectly reflective.
## Describe the microfacet model and the use of the halfway vector.
This model is driven by a roughness parameter, describing how rough a surface is. Meaning that the surface has imperfactions (microfacets) that are randomly alligned to scatter the light.

Light will only reflect of a microfacet if the normal is oriented halfway between the light direction and the view direction. Otherwise it will be reflecting another direction and not towards the camera.

## List the functions that compose the Cook-Torrance BRDF.
Distribution function (aka normal distribution function). Ratio of microfacets that are aligned to a specific half vector

Geometry function. This takes into account the microfacets that are shadowed or masked

Fresnel function. ratio of light that get specular reflected. (more angle = more reflect)

# Lecture 9
## Explain what aliasing is and what techniques can be used to soften it.
Aliasing is the visual pixelated borders that occurs when an image resolution is to low.

Super sampling: Render at a higher resolution and scale down to target resolution.

Multi sampling: Similar to super sampling, but the color buffer is normal size, and the depth buffer is high resolution instead. This uses less memeory, while preserving the benefits of super sampling.

Morphological: Tries to detect high contrast edges and blur them. This has different techniques: MLAA, FXAA, SMAA. This will smooth anything, even intentional high contrast areas.

Temporal (TAA): Similar to MSAA, but small random offsets within the pixel. but one each frame and blend with previous frame.

AI: using deep learning. Probably overhyped
## Describe what color grading is and how it can be applied to an image.
Techniques that aim to improve quality of an image by adjusting the color. It can be used to achieve a realistic or stylized look.

Blending with the framebuffer, idk
## List some screen space effects that can be used to improve rendering.
ambient occlusion

Screen space reflections

Screen space global illumination

Screen space shadows, or contact shadows

