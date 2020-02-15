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
Writes the new densities to the graphics card. Densities below the threshold set by DensityMap::setThreshold() are not written.  
This function should not be called too frequently, since it can take a while to complete (the time taken increases with the size of the array).

<b>void setThreshold(unsigned char value)</b>  
<b>unsigned char getThreshold()</b>  
These set and get the minimum threshold for writing a cell of the density map to the graphics card. 0 means that everything is written, and 255 means that only the brightest values are written.

## Movement

There are two movement options, controlled by setting ROTATE_GRID at the top of main.cpp to either true or false.  

If ROTATE_GRID is false, then the camera can be moved around using WASD plus Q and E for upwards and downwards movement. C zooms in the camera.  

If ROTATE_GRID is true, then the camera is stationary, and the grid can be rotated by clicking the left mouse button and dragging. Press R to reset the orientation.  

## To Do
 
Add GUI features.  
Speed optimizations !!!!!  

![The image is in the images folder](https://github.com/ethanlipson/DensityMap/raw/master/images/sphere.png "Sphere demo")
