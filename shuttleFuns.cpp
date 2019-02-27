//
//  shuttleFuns.cpp
//  hmm
//
//  Created by Adam Willats on 2/27/19.
//  Copyright © 2019 Adam Willats. All rights reserved.
//

#include "shuttleFuns.hpp"

//extract to a different header
std::vector<double> array2vec(double ary[], int len)
{
    std::vector<double> v(len,0);
    for (int i=0; i<len;i++)
    {
        v[i] =ary[i];
    }
    return v;
};

void vec2array(std::vector<double> vIn,double *ary)
{
    //return &vIn[0];
    std::copy(vIn.begin(),vIn.end(),ary);
};


/*
double * returnDub()
{
    double ary[2] = {4.0,3.2};
    return ary; //this is bad
}
*/
void modDub(double * pDub)
{
    *pDub= *pDub + 1;
    //return modVal;
}

void modDubVec(double * pDub,int lenDub)
{
    for (int i = 0; i<lenDub; i++)
    {
        *(pDub+i)= *(pDub+i) + 1;
    }
    //return modVal;
}
