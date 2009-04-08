/***************************************************************************
 *   Copyright (C) 2008 by Volodymyr Myrnyy                                *
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

/** \file
    \brief Sample program to represent GFFT usage
*/

#include <iostream>

#include "gfftconf.h"

#include <omp.h>

using namespace std;

typedef DFT::FLOAT ValueType;

const unsigned Min = 1;
const unsigned Max = 7;

int main(int argc, char *argv[])
{

//    omp_set_num_threads(4);

    unsigned int i,p=6;
    unsigned int n= 1<<p;

// There are three ways to create object to perform FFT of the length 2^p
// 1) Singleton holds the object factory for GFFT
    DFT::GFFT_Singleton<Min,Max,ValueType,DFT::COMPLEX,DFT::INTIME,DFT::FORWARD>* gfft;
    DFT::AbstractFFT<ValueType::ValueType>* fftobj = gfft->Instance().CreateObject(p);

    DFT::GFFT_Singleton<Min,Max,ValueType,DFT::COMPLEX,DFT::INTIME,DFT::BACKWARD>* igfft;
    DFT::AbstractFFT<ValueType::ValueType>* ifftobj = igfft->Instance().CreateObject(p);

// 2) Create the object factory without singleton
//    Loki::Factory<DFT::AbstractFFT<ValueType>,unsigned int> gfft;
//    FactoryInit<DFT::GFFTList<Min,Max,ValueType,DFT::COMPLEX,DFT::INFREQ,DFT::FORWARD>::Result>::apply(gfft);
//    DFT::AbstractFFT<ValueType>* fftobj = gfft.CreateObject(p);
//
//    Loki::Factory<DFT::AbstractFFT<ValueType>,unsigned int> igfft;
//    FactoryInit<DFT::GFFTList<Min,Max,ValueType,DFT::COMPLEX,DFT::INFREQ,DFT::BACKWARD>::Result>::apply(igfft);
//    DFT::AbstractFFT<ValueType>* ifftobj = igfft.CreateObject(p);

// 3) create FFT object of specific length, if the length is known at compile-time
//     typedef DFT::GFFT<2,ValueType,DFT::REAL,DFT::INTIME,DFT::FORWARD> MyGFFT;
//     MyGFFT* fftobj = new MyGFFT;

// create sample data
    ValueType::ValueType* data = new ValueType::ValueType [2*n];
    for (i=0; i < n; ++i) {
       data[2*i] = 2*i;
       data[2*i+1] = 2*i+1; //2*i+1;
    }

// print out sample data
    cout<<"Input data:"<<endl;
    for (i=0; i < n; ++i)
      cout<<"("<<data[2*i]<<","<<data[2*i+1]<<")"<<endl;

// apply FFT in-place
    fftobj->fft(data);

// print out transformed data
    cout<<"Result of transform:"<<endl;
    for (i=0; i < n; ++i)
      cout<<"("<<data[2*i]<<","<<data[2*i+1]<<")"<<endl;

    ifftobj->fft(data);

// print out transformed data
    cout<<"Result of backward transform:"<<endl;
    for (i=0; i < n; ++i)
      cout<<"("<<data[2*i]<<","<<data[2*i+1]<<")"<<endl;

}

