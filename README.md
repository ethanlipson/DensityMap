# DensityMap

DensityMap is a class that stores a 3D array of unsigned bytes between 0 and 255 (inclusive) and allows them to be displayed using OpenGL.

## Methods

<b>DensityMap(int dim)</b>  
Initializes the DensityMap with a cubic array of side length dim.

<b>void clear(int value = 0)</b>  
Fills the whole array with a given value. Defaults to 0.

<b>void writeLine(glm::vec3 p1, glm::vec3 p2, std::vector&lt;unsigned char&gt; vals, WriteMode writeMode = DensityMap::WriteMode::Avg)</b>  
Adds a line of data to the array along the line segment defined by p1 and p2. The more values there are in vals, the smoother the line will be.  
The value written to the cell will be the weighted average of the new and old value. The coefficient used in this formula is determined by setUpdateCoefficient().

<b>void writeCell(unsigned int x, unsigned int y, unsigned int z, unsigned char value)</b>  
Writes to one cell of the buffer on the graphics card.

Multiple data values can fall into the same cell (especially if there is a lot of data), so there are multiple ways to combine them.  
If writeMode is equal to DensityMap::WriteMode::Avg, then all values in the same cell will be averaged, and that value will be written to the cell.  
If writeMode is equal to DensityMap::WriteMode::Max, then the maximum of all the values in the cell will be written (this can be better if your data is sparse).

<b>int getDim()</b>  
Returns the side length of the cube.

<b>void draw(glm::dmat4 projection, glm::dmat4 view, glm::dmat4 model)</b>  
Draws the density map and a white box around it to the screen.

<b>void setThreshold(unsigned char value)</b>  
<b>unsigned char getThreshold()</b>  
These set and get the minimum value needed to draw a cell. The fewer cells are drawn, the faster your program will run.

<b>void setUpdateCoefficient(float value)</b>  
<b>float getUpdateCoefficient()</b>  
These set and get the update coefficient used for the weighted average in writeLine().  
If it is 1, then the new value completely overwrites the old value. If it is 0.5, then the mean of the new and old values is taken. If it is 0, then writing new values has no effect (not recommended for obvious reasons).

<b>void setBrightness(float value)</b>  
<b>float getBrightness()</b>  
<b>void setContrast(float value)</b>  
<b>float getContrast()</b>  

<b>unsigned char readCell(int x, int y, int z)</b>  
Gets the value at a specific index in the array. Blocks when drawing is happening.

<b>unsigned char readCellInterpolated(float x, float y, float z)</b>  
Gets the interpolated value at a position in the cube. Blocks when drawing is happening.  
x, y, and z must all be on the half-open range [0, 1)

These set and get the image brightness and contrast.  
After the cell shades are mapped onto the closed interval [0, 1], the following formula is applied:

```
shade = contrast * (shade - 0.5) + 0.5 + brightness
```

When the brightness is 1, all cells will be white, and when the brightness is -1, all cells will be invisible.  
When the contrast is between 0 and 1, cells will appear less contrasted with their neighbors, and when it is more than 1, they will appear more contrasted with their neighbors.

## Movement

There are two movement options, controlled by setting ROTATE_GRID at the top of main.cpp to either true or false.  

If ROTATE_GRID is false, then the camera can be moved around using WASD plus Q and E for upwards and downwards movement. C zooms in the camera.  

If ROTATE_GRID is true, then the camera is stationary, and the grid can be rotated by clicking the left mouse button and dragging. Press R to reset the orientation.  

![The image is in the images folder](https://github.com/ethanlipson/DensityMap/raw/master/images/sphere.png "Sphere demo")
