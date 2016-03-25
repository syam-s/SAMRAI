/*************************************************************************
 *
 * This file is part of the SAMRAI distribution.  For full copyright
 * information, see COPYRIGHT and COPYING.LESSER.
 *
 * Copyright:     (c) 1997-2011 Lawrence Livermore National Security, LLC
 * Description:   F77 external declarations for SAMRAI linear advection example.
 *
 ************************************************************************/

#include <math.h>
#include <signal.h>

extern "C" {

void F77_FUNC(linadvinit2d, LINADVINIT2D) (
   const int&, const double *, const double *, const double *,
   const int&, const int&,
   const int&, const int&,
   const int&,
   const int&,
   double *,
   const int&,
   const double *, const double *);

void F77_FUNC(linadvinit3d, LINADVINIT3D) (
   const int&, const double *, const double *, const double *,
   const int&, const int&,
   const int&, const int&,
   const int&, const int&,
   const int&,
   const int&,
   const int&,
   double *,
   const int&,
   const double *, const double *);

void F77_FUNC(linadvinitsine2d, LINADVINITSINE2D) (
   const int&, const double *, const double *,
   const double *, const double *,
   const int&, const int&,
   const int&, const int&,
   const int&,
   const int&,
   double *,
   const int&,
   const double *, const double *,
   const double&, const double *);

void F77_FUNC(linadvinitsine3d, LINADVINITSINE3D) (
   const int&, const double *, const double *,
   const double *, const double *,
   const int&, const int&,
   const int&, const int&,
   const int&, const int&,
   const int&,
   const int&,
   const int&,
   double *,
   const int&,
   const double *, const double *,
   const double&, const double *);

void F77_FUNC(initsphere2d, INITSPHERE2D) (
   const int&, const double *, const double *, const double *,
   const int&, const int&,
   const int&, const int&,
   const int&,
   const int&,
   double *,
   const double&, const double&,
   const double *, const double&);

void F77_FUNC(initsphere3d, INITSPHERE3D) (
   const int&, const double *, const double *, const double *,
   const int&, const int&,
   const int&, const int&,
   const int&, const int&,
   const int&,
   const int&,
   const int&,
   double *,
   const double&, const double&,
   const double *, const double&);

void F77_FUNC(stabledt2d, STABLEDT2D) (
   const double *,
   const int&, const int&,
   const int&, const int&,
   const int&,
   const int&,
   const double *,
   const double *,
   double&);

void F77_FUNC(stabledt3d, STABLEDT3D) (
   const double *,
   const int&, const int&,
   const int&, const int&,
   const int&, const int&,
   const int&,
   const int&,
   const int&,
   const double *,
   const double *,
   double&);

void F77_FUNC(inittraceflux1d, INITTRACEFLUX1D) (
   const int&, const int&,
   const double *,
   double *, double *, double *);

void F77_FUNC(inittraceflux2d, INITTRACEFLUX2D) (
   const int&, const int&,
   const int&, const int&,
   const double *,
   double *, double *, double *,
   double *, double *, double *);

void F77_FUNC(inittraceflux3d, INITTRACEFLUX3D) (
   const int&, const int&,
   const int&, const int&,
   const int&, const int&,
   const double *,
   double *, double *, double *,
   double *, double *, double *,
   double *, double *, double *);

void F77_FUNC(chartracing1d0, CHARTRACING1D0) (
   const double&, const int&, const int&,
   const int&, const double&, const double&, const int&,
   const double *,
   double *, double *,
   double *, double *,
   double *, double *);

void F77_FUNC(chartracing2d0, CHARTRACING2D0) (
   const double&, const int&, const int&,
   const int&, const int&,
   const int&, const double&, const double&, const int&,
   const double *,
   double *, double *,
   double *, double *,
   double *, double *);

void F77_FUNC(chartracing2d1, CHARTRACING2D1) (
   const double&, const int&, const int&, const int&, const int&,
   const int&, const double&, const double&, const int&,
   const double *,
   double *, double *,
   double *, double *,
   double *, double *);

void F77_FUNC(chartracing3d0, CHARTRACING3D0) (
   const double&, const int&, const int&,
   const int&, const int&,
   const int&, const int&,
   const int&, const double&, const double&, const int&,
   const double *,
   double *, double *,
   double *, double *,
   double *, double *);

void F77_FUNC(chartracing3d1, CHARTRACING3D1) (
   const double&, const int&, const int&, const int&, const int&,
   const int&, const int&,
   const int&, const double&, const double&, const int&,
   const double *,
   double *, double *,
   double *, double *,
   double *, double *);

void F77_FUNC(chartracing3d2, CHARTRACING3D2) (
   const double&, const int&, const int&, const int&, const int&,
   const int&, const int&,
   const int&, const double&, const double&, const int&,
   const double *,
   double *, double *,
   double *, double *,
   double *, double *);

void F77_FUNC(fluxcalculation2d, FLUXCALCULATION2d) (
   const double&, const int&, const int&,
   const double *,
   const int&, const int&,
   const int&, const int&,
   const double *,
   const double *,
   double *, double *, double *,
   double *, double *, double *);

void F77_FUNC(fluxcalculation3d, FLUXCALCULATION3d) (
   const double&, const int&, const int&,
   const int&,
   const double *,
   const int&, const int&,
   const int&, const int&,
   const int&, const int&,
   const double *,
   const double *,
   double *, double *, double *,
   double *, double *, double *,
   double *, double *, double *);

void F77_FUNC(fluxcorrec2d, FLUXCORREC2D) (
   const double&, const int&, const int&, const int&, const int&,
   const int&, const int&,
   const double *, const double *, const int&,
   const double *,
   const double *, const double *, const double *,
   const double *, const double *, const double *,
   const double *, const double *, const double *,
   double *, double *, double *,
   double *, double *, double *);

void F77_FUNC(fluxcorrec3d, FLUXCORREC3D) (
   const double&, const int&, const int&, const int&, const int&,
   const int&, const int&,
   const double *, const double *,
   const double *,
   const double *, const double *, const double *,
   const double *, const double *, const double *,
   double *, double *, double *,
   double *, double *, double *);

void F77_FUNC(fluxcorrec, FLUXCORREC) (
   const double&, const int&, const int&, const int&, const int&,
   const double *,
   const double *, const double *,
   double *, double *,
   double *, double *,
   double *, double *);

void F77_FUNC(consdiff2d, CONSDIFF2D) (
   const int&, const int&,
   const int&, const int&,
   const double *,
   const double *, const double *,
   const double *,
   double *);

void F77_FUNC(consdiff3d, CONSDIFF3D) (
   const int&, const int&,
   const int&, const int&,
   const int&, const int&,
   const double *,
   const double *, const double *,
   const double *,
   const double *,
   double *);

void F77_FUNC(getbdry2d, GETBDRY2D) (const int&,
   const int&, const int&, const int&, const int&,
   const int&, const int&, const int&, const int&,
   const int&,
   const int&,
   const int&,
   const double *, const double&,
   double *,
   const double *, const double *, const int&);

void F77_FUNC(getbdry3d, GETBDRY3D) (const int&,
   const int&, const int&, const int&, const int&,
   const int&, const int&, const int&, const int&,
   const int&, const int&, const int&, const int&,
   const int&,
   const int&,
   const int&,
   const int&,
   const double *, const double&,
   double *,
   const double *, const double *, const int&);

void F77_FUNC(onethirdstate3d, ONETHIRDSTATE3D) (
   const double&, const double *, const int&,
   const int&, const int&, const int&, const int&, const int&, const int&,
   const double *, const double *,
   const double *, const double *, const double *,
   double *);

void F77_FUNC(fluxthird3d, FLUXTHIRD3D) (
   const double&, const double *, const int&,
   const int&, const int&, const int&, const int&, const int&, const int&,
   const double *, const double *,
   const double *,
   double *, double *, double *);

void F77_FUNC(fluxcorrecjt3d, FLUXCORRECJT3D) (
   const double&, const double *, const int&,
   const int&, const int&, const int&, const int&, const int&, const int&,
   const double *, const double *,
   const double *, const double *, const double *,
   double *, double *, double *,
   double *, double *, double *);

void F77_FUNC(detectgrad2d, DETECTGRAD2D) (
   const int&, const int&,
   const int&, const int&,
   const int&, const int&, const int&,
   const int&, const int&, const int&,
   const double *,
   const double&,
   const int&, const int&,
   const double *,
   int *, int *);

void F77_FUNC(detectgrad3d, DETECTGRAD3D) (
   const int&, const int&,
   const int&, const int&,
   const int&, const int&,
   const int&, const int&, const int&,
   const int&, const int&, const int&,
   const int&, const int&, const int&,
   const double *,
   const double&,
   const int&, const int&,
   const double *,
   int *, int *);

void F77_FUNC(detectshock2d, DETECTSHOCK2D) (
   const int&, const int&,
   const int&, const int&,
   const int&, const int&, const int&,
   const int&, const int&, const int&,
   const double *,
   const double&, const double&,
   const int&, const int&,
   const double *,
   int *, int *);

void F77_FUNC(detectshock3d, DETECTSHOCK3D) (
   const int&, const int&,
   const int&, const int&,
   const int&, const int&,
   const int&, const int&, const int&,
   const int&, const int&, const int&,
   const int&, const int&, const int&,
   const double *,
   const double&, const double&,
   const int&, const int&,
   const double *,
   int *, int *);

void F77_FUNC(stufprobc, STUFPROBC) (
   const int&, const int&, const int&,
   const int&, const int&, const int&, const int&,
   const int&, const int&, const int&);

}
