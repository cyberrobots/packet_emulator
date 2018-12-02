#include "p_emu_libs.h"
#include "p_emu_dbg.h"
#include "p_emu_core.h"
#include "p_emu_help.h"
#include "p_emu_math.h"

#include <math.h>
#include <limits.h>


unsigned long rand_normal(void *data){
    struct p_emu_gaussian_delay* p = (struct p_emu_gaussian_delay*)data;
     static double n2 = 0.0;
     static int n2_cached = 0;

    if (!n2_cached) {
        double x, y, r;
        do {
            x = 2.0*rand()/RAND_MAX - 1;
            y = 2.0*rand()/RAND_MAX - 1;
            r = x*x + y*y;
        } while (r == 0.0 || r > 1.0);
        {
            double d = sqrt(-2.0*log(r)/r);
            double n1 = x*d;
            n2 = y*d;
            long result = n1* p->stddev + p->mean;
            n2_cached = 1;
            return (long)(result + p->shift);
        }
     } else {
        n2_cached = 0;
        return (long)(n2* p->stddev + p->mean + p->shift );
     }
}
unsigned long exponential(void* data)
{
	struct p_emu_exponential_delay* p = (struct p_emu_exponential_delay*)data;
#if 0
	struct p_emu_exponential_delay* p = (struct p_emu_exponential_delay*)data;
	
	long result;
	double x;
	x=((double) rand() / (RAND_MAX));
	result=(p->shift+(p->factor*((-1)*(1/p->lamda)*log(x))));
	return result;
#endif
	long result = -1;
	double U = ((double) rand() / (RAND_MAX));
	
	result = ((-1)*p->lamda) * log ( 1 - U );
	return result;
}
unsigned long poisson(void* data)
{
	struct p_emu_poisson_delay* p = (struct p_emu_poisson_delay*)data;
	
	int n; 
	double x;
	n = 1;
	x = 1.0;
	x = (double) rand()/INT_MAX;
	while( x >= exp((-1) * p->expected ) ){
		x = (x*((double) rand() / INT_MAX));
		n = n + 1;
	}
	n = p->shift + (p->factor * n);
	return n;
}


unsigned long paretoI(void* data)
{
	struct p_emu_pareto_delay* p = (struct p_emu_pareto_delay*)data;

	/*Pareto I distribution. as described http://www.ntrand.com/pareto-distribution */

	/*
		pow(x,y)
		This function returns the result of raising x to the power y.
	*/

	double U 			= ((double) rand() / (RAND_MAX));
	double out 			= (1 / (1 - U));
	double inv_shape 	= (1 / p->shape);
	out 				= pow(out,inv_shape);
	out 				= out * p->scale;
	
	return (unsigned long) out;
#if 0	
	/*double r;
	double out;
	r	= 
	out	= (pow((r / p->sigm),((-1) * p->alfa)));
	out	= p->shift + (p->factor * out);
	*/
#endif	
}

unsigned long paretoII_lomax(void* data)
{
	struct p_emu_paretoII_delay* p = (struct p_emu_paretoII_delay*)data;
	/*Pareto II distribution.*/
	double r;
	double out;
	r	=((double) rand() / (RAND_MAX));
	out	=(1- ( pow ( ( ( r - p->mmi) / p->sigm ),( (-1)* p->alfa) ) ) );
	out	= p->shift + (p->factor * out);
	return (int)out;
}


unsigned long uniform(void *data)
{
    struct p_emu_uniform_delay *p = (struct p_emu_uniform_delay*) data;

    double scaled = (double)rand()/RAND_MAX;

    return ( ( ( p->max - p->min +1 ) * scaled ) + p->min );
}

