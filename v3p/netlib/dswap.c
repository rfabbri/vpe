/* dswap.f -- translated by f2c (version of 23 April 1993  18:34:30).
   You must link the resulting object file with the libraries:
        -lf2c -lm   (in that order)
*/

#include "f2c.h"

/* Subroutine */ void dswap_(n, dx, incx, dy, incy)
integer *n;
doublereal *dx;
integer *incx;
doublereal *dy;
integer *incy;
{
    /* Local variables */
    static integer i, m;
    static doublereal dtemp;
    static integer ix, iy;

/*     interchanges two vectors.                                    */
/*     uses unrolled loops for increments equal one.                */
/*     jack dongarra, linpack, 3/11/78.                             */
/*     modified 12/3/93, array(1) declarations changed to array(*)  */

    if (*n <= 0) {
        return;
    }
    if (*incx == 1 && *incy == 1) {
        m = *n % 3;
        for (i = 0; i < m; ++i) {
            dtemp = dx[i];
            dx[i] = dy[i];
            dy[i] = dtemp;
        }
        for (i = m; i < *n; i += 3) {
            dtemp = dx[i];
            dx[i] = dy[i];
            dy[i] = dtemp;
            dtemp = dx[i + 1];
            dx[i + 1] = dy[i + 1];
            dy[i + 1] = dtemp;
            dtemp = dx[i + 2];
            dx[i + 2] = dy[i + 2];
            dy[i + 2] = dtemp;
        }
    }
    else {
        ix = 0; iy = 0;
        if (*incx < 0) {
            ix = (1-(*n)) * *incx;
        }
        if (*incy < 0) {
            iy = (1-(*n)) * *incy;
        }
        for (i = 0; i < *n; ++i) {
            dtemp = dx[ix];
            dx[ix] = dy[iy];
            dy[iy] = dtemp;
            ix += *incx; iy += *incy;
        }
    }
} /* dswap_ */
