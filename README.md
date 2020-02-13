# DensityMap

DensityMap is a class that stores a 3D array of unsigned bytes between 0 and 255 (inclusive) and allows them to be displayed using OpenGL.

## Public Members

<b>std::vector&lt;std::vector&lt;std::vector&lt;unsigned char&gt;&gt;&gt; cells</b>  
The 3D array containing the density at each position, consisting of unsigned bytes.

## Methods

<b>DensityMap(int dim)</b>  
Initializes the DensityMap with a cubic array of side length dim.

<b>void addLine(glm::vec3 p1, glm::vec3 p2, std::vector&lt;unsigned char&gt; vals)</b>  
Adds a line of data to the array along the line segment defined by p1 and p2.
The more values there are in vals, the smoother the line will be.

<b>void clear(int value = 0)</b>  
Fills the whole array with a given value. Defaults to 0.

<b>std::vector&lt;float&gt; getVertexPositions()</b>  
Returns a vector containing the positions of vertices used to display the density map.

<b>std::vector&lt;unsigned char&gt; getVertexDensities()</b>  
Returns a vector containing the densities of each cell (used to display the density map).

<b>int getDim()</b>  
Returns the side length of the cube.

<b>void draw(glm::dmat4 projection, glm::dmat4 view, glm::dmat4 model)</b>  
Draws the density map and a white box around it to the screen.

<b>void updateVertexBuffer()</b>
Writes the new densities to the grahpics card. This function should not be called too frequently, since it takes ~100 milliseconds to complete (the time taken increases with the size of the array).

## Movement

Right now, the camera is controlled using WASD, plus Q and E for vertical movement. This will be changed to a stationary camera with a rotating display in the future.

## To Do

Add cube rotation and a static camera.  
Add GUI features.  
Speed optimizations !!!!!  
Dynamic resolution scaling (for the density display)  

![The image is in the images folder](https://github.com/ethanlipson/DensityMap/raw/master/images/sphere.png "Sphere demo")
