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

--------------------------------------------------------- Required attributes end ----------------------------------------------------------

#### Bellow are optional settings

A **‘material’ color** (components defined on a scale from 0-1). 

>**mtlcolor**   Odr Odg Odb Osr Osg Osb ka kd ks n

A **sphere**

>**sphere**   cx  cy  cz  r

**parallel projection**

>**projection**  parallel


 **point light**
>**light** x y z w r g b

**point light with attenuation**

>**attlight** x y z w r g b c1 c2 c3

**depth cueing**

>**depthcueing** dcr dcg dcb amax amin distmax distmin

**shadow**    enable soft shadow
>**shadow** soft
