/***************************************************************************
 *   Copyright (C) 2006-2013 by Volodymyr Myrnyy                           *
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

#ifndef __gfftspec_h
#define __gfftspec_h

/** \file
    \brief Recursive algorithms and short-radix FFT specifications
*/

#include "metafunc.h"

namespace GFFT {

using namespace MF;

template<typename T, int_t N, int S, int_t K>
struct ComputeTwiddles
{
  static const int_t K2 = K*2;
  
  static void apply(T* c, T* s)
  {
    ComputeTwiddles<T,N,S,K-1>::apply(c, s);
    c[K-1] = Cos<N,K2,T>::value();
    s[K-1] = S*Sin<N,K2,T>::value();
  }
};

template<typename T, int_t N, int S>
struct ComputeTwiddles<T,N,S,1>
{
  static void apply(T* c, T* s)
  {
    c[0] = Cos<N,2,T>::value();
    s[0] = S*Sin<N,2,T>::value();
  }
};

//////////////////////////////////////////////////

template<int_t N, int_t M, typename T, int S>
class DFTk_inplace
{
  //GFFT_STATIC_ASSERT((N%2 == 1))   // N is assumed odd, otherwise compiler would not come here
  
  typedef typename TempTypeTrait<T>::Result LocalVType;
  static const int_t K = (N-1)/2; 
  static const int_t NM = N*M; 
   
  LocalVType m_c[K], m_s[K];
  
public:
  DFTk_inplace() 
  { 
    ComputeTwiddles<LocalVType, N, S, K>::apply(m_c, m_s);
  }
  
  void apply(T* data) 
  { 
    T sr[K], si[K], dr[K], di[K];
    for (int_t i=0; i<K; ++i) {
      const int_t k = (i+1)*M;
      sr[i] = data[k]   + data[NM-k];
      si[i] = data[k+1] + data[NM-k+1];
      dr[i] = data[k]   - data[NM-k];
      di[i] = data[k+1] - data[NM-k+1];
    }
    
    for (int_t i=1; i<K+1; ++i) {
      T re1(0), re2(0), im1(0), im2(0);
      for (int_t j=0; j<K; ++j) {
	const bool sign_change = (i*(j+1) % N) > K;
	const int_t kk = (i+j*i)%N;
	const int_t k = (kk>K) ? N-kk-1 : kk-1;
	const T s1 = m_s[k]*di[j];
	const T s2 = m_s[k]*dr[j];
	re1 += m_c[k]*sr[j];
	im1 += m_c[k]*si[j];
	re2 += sign_change ? -s1 : s1;
	im2 -= sign_change ? -s2 : s2;
      }
      const int_t k = i*M;
      data[k] = data[0] + re1 + re2;
      data[k+1] = data[1] + im1 + im2;
      data[NM-k] = data[0] + re1 - re2;
      data[NM-k+1] = data[1] + im1 - im2;
    }
    
    for (int_t i=0; i<K; ++i) {
      data[0] += sr[i];
      data[1] += si[i];
    }
  }

  template<class LT>
  void apply(T* data, const LT* wr, const LT* wi) 
  { 
    T sr[K], si[K], dr[K], di[K];
    for (int_t i=0; i<K; ++i) {
      const int_t k = (i+1)*M;
      const T tr1 = data[k]*wr[i] - data[k+1]*wi[i];
      const T ti1 = data[k]*wi[i] + data[k+1]*wr[i];
      const T tr2 = data[NM-k]*wr[K-i] - data[NM-k+1]*wi[K-i];
      const T ti2 = data[NM-k]*wi[K-i] + data[NM-k+1]*wr[K-i];
      sr[i] = tr1 + tr2;
      si[i] = ti1 + ti2;
      dr[i] = tr1 - tr2;
      di[i] = ti1 - ti2;
    }
    
    for (int_t i=1; i<K+1; ++i) {
      T re1(0), re2(0), im1(0), im2(0);
      for (int_t j=0; j<K; ++j) {
	const bool sign_change = (i*(j+1) % N) > K;
	const int_t kk = (i+j*i)%N;
	const int_t k = (kk>K) ? N-kk-1 : kk-1;
	const T s1 = m_s[k]*di[j];
	const T s2 = m_s[k]*dr[j];
	re1 += m_c[k]*sr[j];
	im1 += m_c[k]*si[j];
	re2 += sign_change ? -s1 : s1;
	im2 -= sign_change ? -s2 : s2;
      }
      const int_t k = i*M;
      data[k] = data[0] + re1 + re2;
      data[k+1] = data[1] + im1 + im2;
      data[NM-k] = data[0] + re1 - re2;
      data[NM-k+1] = data[1] + im1 - im2;
    }
    
    for (int_t i=0; i<K; ++i) {
      data[0] += sr[i];
      data[1] += si[i];
    }
  }
  
  template<class LT>
  void apply(const LT* wr, const LT* wi, T* data) 
  { 
    T sr[K], si[K], dr[K], di[K];
    for (int_t i=0; i<K; ++i) {
      const int_t k = (i+1)*M;
      sr[i] = data[k]   + data[NM-k];
      si[i] = data[k+1] + data[NM-k+1];
      dr[i] = data[k]   - data[NM-k];
      di[i] = data[k+1] - data[NM-k+1];
    }
    
    for (int_t i=1; i<K+1; ++i) {
      T re1(0), re2(0), im1(0), im2(0);
      for (int_t j=0; j<K; ++j) {
	const bool sign_change = (i*(j+1) % N) > K;
	const int_t kk = (i+j*i)%N;
	const int_t k = (kk>K) ? N-kk-1 : kk-1;
	const T s1 = m_s[k]*di[j];
	const T s2 = m_s[k]*dr[j];
	re1 += m_c[k]*sr[j];
	im1 += m_c[k]*si[j];
	re2 += sign_change ? -s1 : s1;
	im2 -= sign_change ? -s2 : s2;
      }
      const int_t k = i*M;
      const T tr1 = data[0] + re1 + re2;
      const T ti1 = data[1] + im1 + im2;
      const T tr2 = data[0] + re1 - re2;
      const T ti2 = data[1] + im1 - im2;
      data[k] = tr1*wr[i-1] - ti1*wi[i-1];
      data[k+1] = tr1*wi[i-1] + ti1*wr[i-1];
      data[NM-k] = tr2*wr[K-i+1] - ti2*wi[K-i+1];
      data[NM-k+1] = tr2*wi[K-i+1] + ti2*wr[K-i+1];
    }
    
    for (int_t i=0; i<K; ++i) {
      data[0] += sr[i];
      data[1] += si[i];
    }
  }
};

template<int_t M, typename T, int S>
class DFTk_inplace<4,M,T,S> 
{
  static const int_t I10 = M;
  static const int_t I11 = M+1;
  static const int_t I20 = M+M;
  static const int_t I21 = I20+1;
  static const int_t I30 = I20+M;
  static const int_t I31 = I30+1;
  
public:
  DFTk_inplace() { }
  
  void apply(T* data) 
  { 
      T tr = data[I20];
      T ti = data[I21];
      data[I20] = data[0]-tr;
      data[I21] = data[1]-ti;
      data[0] += tr;
      data[1] += ti;
      tr = data[I30];
      ti = data[I31];
      data[I30] = S*(data[I11]-ti);
      data[I31] = S*(tr-data[I10]);
      data[I10] += tr;
      data[I11] += ti;

      tr = data[I10];
      ti = data[I11];
      data[I10] = data[0]-tr;
      data[I11] = data[1]-ti;
      data[0] += tr;
      data[1] += ti;
      tr = data[I30];
      ti = data[I31];
      data[I30] = data[I20]-tr;
      data[I31] = data[I21]-ti;
      data[I20] += tr;
      data[I21] += ti;
  }
  template<class LT>
  void apply(T* data, const LT* wr, const LT* wi) 
  { 
  }
  template<class LT>
  void apply(const LT* wr, const LT* wi, T* data) 
  { 
  }
};

template<int_t M, typename T, int S>
class DFTk_inplace<3,M,T,S> 
{
  static const int_t I10 = M;
  static const int_t I11 = M+1;
  static const int_t I20 = M+M;
  static const int_t I21 = I20+1;
  
  T m_coef;
  
public:
  DFTk_inplace() : m_coef(S * Sqrt<3, T>::value() * 0.5) { }
  
  void apply(T* data) 
  { 
      const T sum_r = data[I10] + data[I20];
      const T dif_r = m_coef * (data[I10] - data[I20]);
      const T sum_i = data[I11] + data[I21];
      const T dif_i = m_coef * (data[I11] - data[I21]);
      const T tr = data[0] - 0.5*sum_r;
      const T ti = data[1] - 0.5*sum_i;
      data[0] += sum_r;
      data[1] += sum_i;
      data[I10] = tr + dif_i;
      data[I11] = ti - dif_r;
      data[I20] = tr - dif_i;
      data[I21] = ti + dif_r;
  }
  template<class LT>
  void apply(T* data, const LT* wr, const LT* wi) 
  { 
        const T tr1 = data[I10]*wr[0] - data[I11]*wi[0];
        const T ti1 = data[I10]*wi[0] + data[I11]*wr[0];
        const T tr2 = data[I20]*wr[1] - data[I21]*wi[1];
        const T ti2 = data[I20]*wi[1] + data[I21]*wr[1];

	const T sum_r = tr1 + tr2;
	const T dif_r = m_coef * (tr1 - tr2);
	const T sum_i = ti1 + ti2;
	const T dif_i = m_coef * (ti1 - ti2);
	const T tr = data[0] - 0.5*sum_r;
	const T ti = data[1] - 0.5*sum_i;
	data[0] += sum_r;
	data[1] += sum_i;
	data[I10] = tr + dif_i;
	data[I11] = ti - dif_r;
	data[I20] = tr - dif_i;
	data[I21] = ti + dif_r;
  }
  template<class LT>
  void apply(const LT* wr, const LT* wi, T* data) 
  { 
      const T sum_r = data[I10] + data[I20];
      const T dif_r = m_coef * (data[I10] - data[I20]);
      const T sum_i = data[I11] + data[I21];
      const T dif_i = m_coef * (data[I11] - data[I21]);
      const T tr = data[0] - 0.5*sum_r;
      const T ti = data[1] - 0.5*sum_i;
      const T trpdi = tr + dif_i;
      const T trmdi = tr - dif_i;
      const T tipdr = ti + dif_r;
      const T timdr = ti - dif_r;
      data[0] += sum_r;
      data[1] += sum_i;
      data[I10] = trpdi*wr[0] - timdr*wi[0];
      data[I11] = trpdi*wi[0] + timdr*wr[0];
      data[I20] = trmdi*wr[1] - tipdr*wi[1];
      data[I21] = trmdi*wi[1] + tipdr*wr[1];
  }
};

template<int_t M, typename T, int S>
class DFTk_inplace<2,M,T,S> 
{
public:
  void apply(T* data) 
  { 
      const T tr = data[M];
      const T ti = data[M+1];
      data[M] = data[0]-tr;
      data[M+1] = data[1]-ti;
      data[0] += tr;
      data[1] += ti;
// To test
//       const T tr = data[0] - data[M];
//       const T ti = data[1] - data[M+1];
//       data[0] += data[M];
//       data[1] += data[M+1];
//       data[M] = tr;
//       data[M+1] = ti;
  }
  // For decimation-in-time
  template<class LT>
  void apply(T* data, const LT* wr, const LT* wi) 
  { 
        const T tr = data[M] * (*wr) - data[M+1] * (*wi);
        const T ti = data[M] * (*wi) + data[M+1] * (*wr);
        data[M] = data[0]-tr;
        data[M+1] = data[1]-ti;
        data[0] += tr;
        data[1] += ti;
  }
  // For decimation-in-frequency
  template<class LT>
  void apply(const LT* wr, const LT* wi, T* data) 
  { 
        const T tr = data[0] - data[M];
        const T ti = data[1] - data[M+1];
        data[0] += data[M];
        data[1] += data[M+1];
        data[M]   = tr * (*wr) - ti * (*wi);
        data[M+1] = ti * (*wr) + tr * (*wi);
  }  
};

////////////////////////////////////////////////////////

template<int_t N, int_t SI, int_t DI, typename T, int S>
class DFTk
{
  //GFFT_STATIC_ASSERT(N%2 == 1)   // N is assumed odd, otherwise compiler would not come here
  
  typedef typename TempTypeTrait<T>::Result LocalVType;
  static const int_t K = (N-1)/2; 
  static const int_t NSI = N*SI; 
  static const int_t NDI = N*DI; 
   
  LocalVType m_c[K], m_s[K];
  
public:
  DFTk() 
  { 
    ComputeTwiddles<LocalVType, N, S, K>::apply(m_c, m_s);
  }
  
  void apply(const T* src, T* dst) 
  { 
    T sr[K], si[K], dr[K], di[K];
    for (int_t i=0; i<K; ++i) {
      const int_t k = (i+1)*SI;
      sr[i] = src[k]   + src[NSI-k];
      si[i] = src[k+1] + src[NSI-k+1];
      dr[i] = src[k]   - src[NSI-k];
      di[i] = src[k+1] - src[NSI-k+1];
    }
    
    for (int_t i=1; i<K+1; ++i) {
      T re1(0), re2(0), im1(0), im2(0);
      for (int_t j=0; j<K; ++j) {
	const bool sign_change = (i*(j+1) % N) > K;
	const int_t kk = (i+j*i)%N;
	const int_t k = (kk>K) ? N-kk-1 : kk-1;
	const T s1 = m_s[k]*di[j];
	const T s2 = m_s[k]*dr[j];
	re1 += m_c[k]*sr[j];
	im1 += m_c[k]*si[j];
	re2 += sign_change ? -s1 : s1;
	im2 -= sign_change ? -s2 : s2;
      }
      const int_t k = i*DI;
      dst[k] = src[0] + re1 + re2;
      dst[k+1] = src[1] + im1 + im2;
      dst[NDI-k] = src[0] + re1 - re2;
      dst[NDI-k+1] = src[1] + im1 - im2;
    }

    dst[0] = src[0];
    dst[1] = src[1];
    for (int_t i=0; i<K; ++i) {
      dst[0] += sr[i];
      dst[1] += si[i];
    }
  }

  template<class LT>
  void apply(const LT* wr, const LT* wi, const T* src, T* dst) 
  { 
  }
};

template<int_t SI, int_t DI, typename T, int S>
class DFTk<3,SI,DI,T,S> 
{
  static const int_t SI2 = SI+SI;
  static const int_t DI2 = DI+DI;
  T m_coef;
  
public:
  DFTk() : m_coef(S * Sqrt<3, T>::value() * 0.5) { } // sqrt(3)/2 = sin(2pi/3)
  
  void apply(const T* src, T* dst) 
  { 
    // 4 mult, 12 add
      const T sr = src[SI] + src[SI2];
      const T dr = m_coef * (src[SI] - src[SI2]);
      const T si = src[SI+1] + src[SI2+1];
      const T di = m_coef * (src[SI+1] - src[SI2+1]);
      const T tr = src[0] - 0.5*sr;
      const T ti = src[1] - 0.5*si;
      dst[0]     = src[0] + sr;
      dst[1]     = src[1] + si;
      dst[DI]    = tr + di;
      dst[DI+1]  = ti - dr;
      dst[DI2]   = tr - di;
      dst[DI2+1] = ti + dr;     
  }
  template<class LT>
  void apply(const LT* wr, const LT* wi, const T* src, T* dst) 
  { 
    // 4 mult, 12 add
      const T sr = src[SI] + src[SI2];
      const T dr = m_coef * (src[SI] - src[SI2]);
      const T si = src[SI+1] + src[SI2+1];
      const T di = m_coef * (src[SI+1] - src[SI2+1]);
      const T tr = src[0] - 0.5*sr;
      const T ti = src[1] - 0.5*si;
      const T trpdi = tr + di;
      const T trmdi = tr - di;
      const T tipdr = ti + dr;
      const T timdr = ti - dr;
      dst[0]     = src[0] + sr;
      dst[1]     = src[1] + si;
      dst[DI]    = trpdi*wr[0] - timdr*wi[0];
      dst[DI+1]  = trpdi*wi[0] + timdr*wr[0];
      dst[DI2]   = trmdi*wr[1] - tipdr*wi[1];
      dst[DI2+1] = trmdi*wi[1] + tipdr*wr[1];     
  }
};

template<int_t SI, int_t DI, typename T, int S>
class DFTk<2,SI,DI,T,S> 
{
public:
  void apply(const T* src, T* dst) 
  { 
    // the temporaries tr, ti are necessary, because may happen src == dst
        const T tr = src[0] - src[SI];
        const T ti = src[1] - src[SI+1];
        dst[0] = src[0] + src[SI];
        dst[1] = src[1] + src[SI+1];
        dst[DI]   = tr;
        dst[DI+1] = ti;
  }
  template<class LT>
  void apply(const LT* wr, const LT* wi, const T* src, T* dst) 
  { 
        const T tr = src[0] - src[SI];
        const T ti = src[1] - src[SI+1];
        dst[0] = src[0] + src[SI];
        dst[1] = src[1] + src[SI+1];
        dst[DI]   = tr * (*wr) - ti * (*wi);
        dst[DI+1] = ti * (*wr) + tr * (*wi);
  }
};

/// Specialization for complex-valued radix 2 FFT in-place
/// \tparam T is value type
/// \param data is the array of length 4, containing two complex numbers (real,imag,real,imag).
template<typename T>
inline void _spec2(T* data) 
{
      const T tr = data[2];
      const T ti = data[3];
      data[2] = data[0]-tr;
      data[3] = data[1]-ti;
      data[0] += tr;
      data[1] += ti;
}

template<typename T>
inline void _spec2(const T* src, T* dst) 
{ 
    const T *a(src + 2), *a2(src + 1), *a3(src + 3);
    *(dst) = (*(src) + *(a));
    *((dst + 1)) = (*(a2) + *(a3));
    *((dst + 2)) = (*(src) - *(a));
    *((dst + 3)) = (*(a2) - *(a3));
// test against above version (from spiral)
//     const T v1(src[1]), v2(src[2]), v3(src[3]);
//     dst[0] = (*src + v2);
//     dst[1] = (v1 + v3);
//     dst[2] = (*src - v2);
//     dst[3] = (v1 - v3);
}


template<typename T>
inline void _spec4_fwd(const T* src, T* dst) 
{
  const double  *a, *a2, *a3, *a4, *a5, *a6, *a7;
    double t, t2, t3, t4, t5, t6, t7, t8;
    a = (src + 4);
    t = (*(src) + *(a));
    a2 = (src + 1);
    a3 = (src + 5);
    t2 = (*(a2) + *(a3));
    t3 = (*(src) - *(a));
    t4 = (*(a2) - *(a3));
    a4 = (src + 2);
    a5 = (src + 6);
    t5 = (*(a4) + *(a5));
    a6 = (src + 3);
    a7 = (src + 7);
    t6 = (*(a6) + *(a7));
    t7 = (*(a4) - *(a5));
    t8 = (*(a6) - *(a7));
    *(dst) = (t + t5);
    *((dst + 1)) = (t2 + t6);
    *((dst + 4)) = (t - t5);
    *((dst + 5)) = (t2 - t6);
    *((dst + 2)) = (t3 + t8);
    *((dst + 3)) = (t4 - t7);
    *((dst + 6)) = (t3 - t8);
    *((dst + 7)) = (t4 + t7);
}



}  //namespace DFT

#endif /*__gfftspec_h*/