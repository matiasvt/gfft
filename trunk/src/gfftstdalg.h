/***************************************************************************
 *   Copyright (C) 2006-2014 by Vladimir Mirnyy                            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 ***************************************************************************/

#ifndef __gfftstdalg_h
#define __gfftstdalg_h

/** \file
    \brief Recursive FFT algorithms on "complex" data types like std::complex
    
*/

#include "gfftstdspec.h"
#include "gfftfactor.h"
#include "gfftswap.h"

#include "metacomplex.h"
#include "metaroot.h"

namespace GFFT {

using namespace MF;


/// In-place scaled FFT algorithm
/**
\tparam K first factor
\tparam M second factor (N=K*M) 
\tparam T value type of the data array
\tparam S sign of the transform: 1 - forward, -1 - backward
\tparam W compile-time root of unity
\tparam doStaticLoop rely on template instantiation loop IterateInTime (only for short loops)

The notation for this template class follows SPIRAL. 
The class performs DFT(k) with the Kronecker product by the mxm identity matrix Im
and twiddle factors (T).
\sa InTime
*/
template<int_t K, int_t M, typename T, int S, class W, bool doStaticLoop>
class DFTk_x_Im_T;

template<int_t K, int_t M, typename T, int S, class W,
template<typename> class Complex>
class DFTk_x_Im_T<K,M,Complex<T>,S,W,false>
{
   //typedef typename TempTypeTrait<T>::Result LocalVType;
   typedef Compute<typename W::Re,2> WR;
   typedef Compute<typename W::Im,2> WI;
   static const int_t N = K*M;
   DFTk_inp<K,M,Complex<T>,S> spec_inp;
public:
   void apply(Complex<T>* data) 
   {
      spec_inp.apply(data);

      Complex<T> w[K-1], wp[K-1];

      // W = (wpr[0], wpi[0])
      wp[0] = Complex<T>(WR::value(), WI::value());
      //LocalVType t = Sin<N,1,LocalVType>::value();
//       wp[0] = Complex<LocalVType>(1 - 2.0*t*t, -S*Sin<N,2,LocalVType>::value());
      
      // W^i = (wpr2, wpi2)
      for (int_t i=0; i<K-2; ++i) 
	wp[i+1] = wp[i]*wp[0];
      
      for (int_t i=0; i<K-1; ++i) 
	w[i] = wp[i];
      
      for (int_t i=1; i<M; i++) {
	spec_inp.apply(data+i, w);

	for (int_t i=0; i<K-1; ++i) 
	  w[i] = w[i]*wp[i];
      }
   }
  
};

template<int_t M, typename T, int S, class W,
template<typename> class Complex>
class DFTk_x_Im_T<3,M,Complex<T>,S,W,false> {
   //typedef typename TempTypeTrait<T>::Result LocalVType;
   typedef Compute<typename W::Re,2> WR;
   typedef Compute<typename W::Im,2> WI;
   static const int_t N = 3*M;
   //static const int_t M2 = M*2;
   DFTk_inp<3,M,Complex<T>,S> spec_inp;
public:
   void apply(Complex<T>* data) 
   {
      spec_inp.apply(data);

      Complex<T> w[2];

      // W = (wpr1, wpi1)
//       LocalVType t = Sin<N,1,LocalVType>::value();
//       const LocalVType wpr1 = 1 - 2.0*t*t;
//       const LocalVType wpi1 = -S*Sin<N,2,LocalVType>::value();
      Complex<T> wp1(WR::value(), WI::value());
      
      // W^2 = (wpr2, wpi2)
      Complex<T> wp2(wp1*wp1);
      
      w[0] = wp1;
      w[1] = wp2;
      for (int_t i=1; i<M; i++) {
	spec_inp.apply(data+i, w);

        w[0] = w[0]*wp1;
        w[1] = w[1]*wp2;
      }
   }
};

template<int_t M, typename T, int S, class W,
template<typename> class Complex>
class DFTk_x_Im_T<2,M,Complex<T>,S,W,false> {
   typedef typename TempTypeTrait<T>::Result LocalVType;
   typedef Compute<typename W::Re,2> WR;
   typedef Compute<typename W::Im,2> WI;
   DFTk_inp<2,M,Complex<T>,S> spec_inp;
public:
   void apply(Complex<T>* data) 
   {
      spec_inp.apply(data);

//    LocalVType  t = Sin<N,1,LocalVType>::value();
//       const LocalVType wpr = 1-2.0*t*t;
//       const LocalVType wpi = -S*Sin<N,2,LocalVType>::value();
      Complex<T> wp(WR::value(), WI::value());

      Complex<T> w(wp);
      for (int_t i=1; i<M; i++) {
	spec_inp.apply(data+i, &w);

        w = w*wp;
      }
   }
};


}  //namespace DFT

#endif /*__gfftstdalg_h*/
