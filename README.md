# WhittedStyle_Raytracer
a renderer takes input objects configuration file and render a Whitted-Style Raytracing ppm img


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

A **‘material’ color** (in terms of r, g, b components defined on a scale from 0-1). The material color should be treated as a state variable, meaning that all subsequently-defined objects should use the immediately-preceding material color

>**mtlcolor**   r  g  b

A **sphere**

>**sphere**   cx  cy  cz  r

**parallel projection**

>**projection**  parallel
