#ifndef __FFT__
#define __FFT__

#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#ifdef _WIN_
#define PI	3.14159
#elif _MAC_
#define PI	M_PI
#endif

#define TWOPI	(2.0*PI)   

void FFT(std::vector<double> &data,int nn,int isign);

#endif
