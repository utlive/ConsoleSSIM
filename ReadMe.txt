Arthur : Ming-Jun Chen
email: mjchen@utexas.edu
GUI version of this implementation is available at https://sites.google.com/site/mingjunchen/Home/Research

For console mode operations, you need to first install MS VC++ 2010 redistributable package if you haven't installed Visual studio 
2010 or  Visual studio express 2010 in your Windows system 
 
To operate the console mode, For modes, 0- SSIM, 1 - MS-SSIM, 2- PSNR, 3 - Correlation 
example: conSSIM.exe -m 2 -r bikes.bmp -t img29.bmp 
See ConSSIM.cpp for details 

The project can be compiled and edited by Visual studio express 2010(free-download in MS website) 
Read conSSIM_Architecture.pdf to understand the architexture of this package before tracing the codes. 
 

***********************************************************************

Copyright Notice

-----------COPYRIGHT NOTICE STARTS WITH THIS LINE------------
Copyright (c) 2012 The University of Texas at Austin
All rights reserved.

Permission is hereby granted, without written agreement and without license or royalty fees, 
to use, copy, modify, and distribute the software provided and its documentation for research purpose only,
provided that this copyright notice and the original authors' names appear on all copies and supporting documentation.
The software provided may not be commercially distributed.


The following papers are to be cited in the bibliography whenever the software is used as:

Ming-Jun Chen and Alan C. Bovik, �Fast structural similarity index algorithm�, Journal of Real-Time Image Processing, pp. 1-7. August, 2010


IN NO EVENT SHALL THE UNIVERSITY OF TEXAS AT AUSTIN BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, 
SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OF THIS DATABASE AND ITS 
DOCUMENTATION, EVEN IF THE UNIVERSITY OF TEXAS AT AUSTIN HAS BEEN ADVISED OF THE POSSIBILITY OF
SUCH DAMAGE.

THE UNIVERSITY OF TEXAS AT AUSTIN SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED
TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE DATABASE 
PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND THE UNIVERSITY OF TEXAS AT AUSTIN HAS NO OBLIGATION
TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

-----------COPYRIGHT NOTICE ENDS WITH THIS LINE------------

