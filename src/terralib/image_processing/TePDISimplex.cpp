#include "TePDISimplex.hpp"
#include "TePDIUtils.hpp"

#include <string>
#include <math.h>
#include <float.h>

TePDISimplex::TePDISimplex()
{
}


TePDISimplex::~TePDISimplex()
{
}


void TePDISimplex::ResetState( const TePDIParameters& )
{
}


bool TePDISimplex::CheckParameters( const TePDIParameters& parameters ) const
{
    /* checks parameters */
    double (*func)(double*);
    TEAGN_TRUE_OR_RETURN( parameters.GetParameter( "objective_function", func ), "Missing parameter: objective_function" );
    int n;
    TEAGN_TRUE_OR_RETURN( parameters.GetParameter( "num_variables", n ), "Missing parameter: num_variables" );
    double *x;
    TEAGN_TRUE_OR_RETURN( parameters.GetParameter( "x_array", x ), "Missing parameter: x_array" );
    int max_it;
    TEAGN_TRUE_OR_RETURN( parameters.GetParameter( "max_iterations", max_it ), "Missing parameter: max_iterations" );
    double epsilon;
    TEAGN_TRUE_OR_RETURN( parameters.GetParameter( "epsilon", epsilon ), "Missing parameter: epsilon" );

    return true;
}

/* 
 * Author : Michael F. Hutt
 * http://www.mikehutt.com
 * 11/3/97
 * 
 * An implementation of the Nelder-Mead simplex method applied to
 * Rosenbrock's function.
 *
 * Copyright (c) 1997-2007 <Michael F. Hutt>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *
 * Jan. 6, 1999 
 * Modified to conform to the algorithm presented
 * in Margaret H. Wright's paper on Direct Search Methods.
 *
 * Jul. 23, 2007
 * Fixed memory leak.
 *
 */
// Adapted from nmsimplex.c, Author: Michael F. Hutt (http://www.mikehutt.com).
//#define MAX_IT      300       // maximum number of iterations
//int MAX_IT = 300;
//double EPSILON = 1.0E-05;
#define ALPHA       1.0       // reflection coefficient
#define BETA        0.5       // contraction coefficient
#define GAMMA       2.0       // expansion coefficient
double simplex(double (*func)(double[]), double start[],int n, double EPSILON, double scale, int MAX_IT) {
    int vs, vh, vg; // vertex with smallest, next smallest, and largest value
    int k, itr;   	 // track the number of function evaluations and iterations
    double **v;     // simplex vertices
    double pn,qn;   // values for initial simplex 
    double *f, fr, fe, fc;      // function value at each vertex, at reflection point, at expansion point, at contraction point
    double *vr, *ve, *vc, *vm;  // reflection/expansion/contraction/centroid - coordinates 
    int i, j, m, row;
    double min, fsum, favg, s, cent;
    v =  (double **) malloc ((n+1) * sizeof(double *));
    f =  (double *) malloc ((n+1) * sizeof(double));
    vr = (double *) malloc (n * sizeof(double));
    ve = (double *) malloc (n * sizeof(double));  
    vc = (double *) malloc (n * sizeof(double));  
    vm = (double *) malloc (n * sizeof(double));  
    for (i = 0; i <= n; i++)
        v[i] = (double *) malloc (n * sizeof(double));
    // create the initial simplex 
    // assume one of the vertices is 0,0 
    pn = scale*(sqrt((double)(n+1))-1+n)/(n*sqrt(2.0));
    qn = scale*(sqrt((double)(n+1))-1)/(n*sqrt(2.0));
    for (i = 0; i < n; i++)
        v[0][i] = start[i];
    for (i = 1; i <= n; i++)
        for (j = 0; j < n; j++)
            if (i-1 == j)
                v[i][j] = pn + start[j];
            else
                v[i][j] = qn + start[j];
    // find the initial function values 
    for (j = 0; j <= n; j++)
        f[j] = func(v[j]);
    k = n+1;
    // print out the initial values 
    printf("Initial Values\n");
    for (j = 0; j <= n; j++)
        printf("%f %f %f\n",v[j][0],v[j][1],f[j]);
    // begin the main loop of the minimization 
    for (itr = 1; itr <= MAX_IT; itr++) {
        // find the index of the largest value 
        vg = 0;
        for (j = 0; j <= n; j++)
            if (f[j] > f[vg])
                vg = j;
        // find the index of the smallest value 
        vs = 0;
        for (j = 0; j <= n; j++)
            if (f[j] < f[vs])
                vs = j;
        // find the index of the second largest value 
        vh = vs;
        for (j = 0; j <= n; j++)
            if (f[j] > f[vh] && f[j] < f[vg])
                vh = j;
        // calculate the centroid 
        for (j = 0; j <= n-1; j++) {
            cent = 0.0;
            for (m = 0; m <= n; m++)
                if (m != vg)
                    cent += v[m][j];
            vm[j] = cent/n;
        }
        // reflect vg to new vertex vr 
        for (j = 0; j <= n-1; j++)
            //vr[j] = (1+ALPHA)*vm[j] - ALPHA*v[vg][j];
            vr[j] = vm[j]+ALPHA*(vm[j]-v[vg][j]);
        fr = func(vr);
        k++;
        if (fr < f[vh] && fr >= f[vs]) {
            for (j = 0; j <= n-1; j++)
                v[vg][j] = vr[j];
            f[vg] = fr;
        }
        // investigate a step further in this direction 
        if ( fr <  f[vs]) {
            for (j = 0; j <= n-1; j++)
                //ve[j] = GAMMA*vr[j] + (1-GAMMA)*vm[j];
                ve[j] = vm[j]+GAMMA*(vr[j]-vm[j]);
            fe = func(ve);
            k++;
            // by making fe < fr as opposed to fe < f[vs], Rosenbrocks function takes 63 iterations as opposed to 64 when using double variables
            if (fe < fr) {
                for (j = 0; j <= n-1; j++)
                    v[vg][j] = ve[j];
                f[vg] = fe;
            }
            else {
                for (j = 0; j <= n-1; j++)
                    v[vg][j] = vr[j];
                f[vg] = fr;
            }
        }
        // check to see if a contraction is necessary 
        if (fr >= f[vh]) {
            if (fr < f[vg] && fr >= f[vh]) {
                // perform outside contraction 
                for (j = 0; j <= n-1; j++)
                    //vc[j] = BETA*v[vg][j] + (1-BETA)*vm[j];
                    vc[j] = vm[j]+BETA*(vr[j]-vm[j]);
                fc = func(vc);
                k++;
            }
            else {
                // perform inside contraction 
                for (j = 0; j <= n-1; j++)
                    //vc[j] = BETA*v[vg][j] + (1-BETA)*vm[j];
                    vc[j] = vm[j]-BETA*(vm[j]-v[vg][j]);
                fc = func(vc);
                k++;
            }
            if (fc < f[vg]) {
                for (j = 0; j <= n-1; j++)
                    v[vg][j] = vc[j];
                f[vg] = fc;
            }
            // at this point the contraction is not successful, we must halve the distance from vs to all the vertices of the simplex and then continue
            else {
                for (row = 0; row <= n; row++)
                    if (row != vs)
                        for (j = 0; j <= n-1; j++)
                            v[row][j] = v[vs][j]+(v[row][j]-v[vs][j])/2.0;
                f[vg] = func(v[vg]);
                k++;
                f[vh] = func(v[vh]);
                k++;
            }
        }
        // print out the value at each iteration 
        printf("Iteration %d\n",itr);
        for (j = 0; j <= n; j++)
            printf("%f %f %f\n",v[j][0],v[j][1],f[j]);
        // test for convergence 
        fsum = 0.0;
        for (j = 0; j <= n; j++)
            fsum += f[j];
        favg = fsum/(n+1);
        s = 0.0;
        for (j = 0; j <= n; j++)
            s += pow((f[j]-favg),2.0)/(n);
        s = sqrt(s);
        if (s < EPSILON)
            break;
    } // end main loop of the minimization 
    // find the index of the smallest value 
    vs = 0;
    for (j = 0; j <= n; j++)
        if (f[j] < f[vs])
            vs = j;
    printf("Minimum found at\n"); 
    for (j = 0;j < n; j++) {
        printf("%e\n",v[vs][j]);
        start[j] = v[vs][j];
    }
    min=func(v[vs]);
    k++;
    printf("%d function evals\n",k);
    printf("%d iterations\n",itr);
    free(f);
    free(vr);
    free(ve);
    free(vc);
    free(vm);
    for (i = 0; i <= n; i++)
        free (v[i]);
    free(v);
    return min;
}

bool TePDISimplex::RunImplementation()
{
    /* retrieves parameters */
    double (*func)(double*);
    params_.GetParameter( "objective_function", func );
    int n;
    params_.GetParameter( "num_variables", n );
    double *x;
    params_.GetParameter( "x_array", x );
    /* convergence parameters */
    int max_it;
    params_.GetParameter( "max_iterations", max_it );
    double epsilon;
    params_.GetParameter( "epsilon", epsilon );
    double scale = 1.0;
    double tmp;
    if (params_.GetParameter( "scale", tmp ))
        scale = tmp;

    simplex(func, x, n, epsilon, scale, max_it);
    
    return true;
}

