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
long exponential(double lamda,int factor,int shift)
{
	long result;
	double x;
	x=((double) rand() / (RAND_MAX));
	result=(shift+(factor*((-1)*(1/lamda)*log(x))));
	return result;

}
int poissonRandom(double expectedValue,int factor,int shift)
{
	int n;
	double x;
	n=1;
	x=1.0;
	x=(double) rand()/INT_MAX;
	while(x>=exp((-1)*expectedValue)){
		x=(x*((double) rand()/INT_MAX));
		n=n+1;
	}
	n=shift+(factor*n);
	return n;
}

int	paretoII_lomax(int factor,int shift, double alfa,double sigm,double mmi)
{
	/*Pareto I distribution.*/
	double r;
	double out;
	r	=((double) rand() / (RAND_MAX));
	out	=(1- ( pow ( ( ( r-mmi) / sigm ),( (-1)*alfa) ) ) );
	out	=shift+(factor*out);
	return (int)out;
}
int	paretoI(int factor,int shift, double alfa,double sigm)
{
	/*Pareto I distribution.*/
	double r;
	double out;
	r	=((double) rand() / (RAND_MAX));
	out	=(pow((r/sigm),((-1)*alfa)));
	out	=shift+(factor*out);
	return (int)out;
}

unsigned long uniform(void *data)
{
    struct p_emu_uniform_delay *p = (struct p_emu_uniform_delay*) data;

    double scaled = (double)rand()/RAND_MAX;

    return ( ( ( p->max - p->min +1 ) * scaled ) + p->min );
}

