# WhittedStyle_Raytracer
## a renderer takes input objects configuration file and render a Whitted-Style Raytracing ppm img


input file format:

**eye position** 'camera position' or 'center of projection' (a 3D point in space)

>**eye**   eyex eyey eyez

The **viewing** direction (a 3D vector)

>**viewdir**   vdirx  vdiry  vdirz

The **'up'** direction (a 3D vector)

>**updir**   upx  upy  upz

The **horizontal field of view** (in degrees, please)

>**hfov**   fovh

The **size** of the output image (in pixel units)

>**imsize**   width  height

The **‘background’ color** (using r, g, b components defined on a scale from 0-1)

>**bkgcolor**   r  g  b

--------------------------------------------------------- Required attributes ends ----------------------------------------------------------

#### Bellow are optional settings

**‘material’ color** (components defined on a scale from 0-1). 

>**mtlcolor**   Odr Odg Odb Osr Osg Osb ka kd ks n

**sphere**

>**sphere**   cx  cy  cz  r

**parallel projection**

>**projection**  parallel

**textrue**
all following object will use texture data as diffuse term
>** textrue**  filename

 **point light**
>**light** x y z w r g b

**point light with attenuation**

>**attlight** x y z w r g b c1 c2 c3

**depth cueing**

>**depthcueing** dcr dcg dcb amax amin distmax distmin

**shadow**    enable soft shadow
>**shadow** soft

**Vertex** 
# list of vertex positions

v   x1 y1 z1

v   x2 y2 z2

v   x3 y3 z3

v   x4 y4 z4

# list of vertex normal vectors

vn   nx1 ny1 nz1

vn   nx2 ny2 nz2

# list of texture coordinates

vt   u1 v1

vt   u2 v2

# list of triangle definitions, consisting of appropriate indices into the vertex array, starting at 1 (not 0)





<pre>
f   v1 v2 v3                             // flat-shaded triangle
f   v1//vn1    v2//vn2    v3//vn3        // smooth-shaded triangle 
f   v1/vt1     v2/vt2     v3/vt3         // flat-shaded textured triangle 
f   v1/vt1/vn1  v2/vt2/vn2   v3/vt3/vn3  // smooth-shaded textured triangle 
</pre>


### usage:
1. root dir to use cmake
2. when you get executable file, call it with the name of configuration file as the argument.
3. output img will be named after the config file. e.g.  calling with config.txt generates config.ppm


# sample img:

![alt text](https://github.com/bobhansky/WhittedStyle_Raytracer/blob/main/out/water_bunny_tex.png)
