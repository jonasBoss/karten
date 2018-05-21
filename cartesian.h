#ifndef CARTESIAN_H
#define CARTESIAN_H
#include "track.h"
class CartesianPoint{
public:
    CartesianPoint(){x=y=z=0;}
    CartesianPoint(double ax,double ay,double az){x=ax;y=ay;z=az;}
    CartesianPoint(double * p){x=p[0];y=p[1];z=p[2];}
    CartesianPoint(const Trackpoint & p);
    double length(){return sqrt(x*x+y*y+z*z);}
    double x,y,z;
    double operator[](short i){
        if(i<0 || i>2) return 0;
        double p[3]={x,y,z};
        return p[i];
        //switch (i) { case 0 : return x; case 1 : return y; case 2 : return z; default:return 0;}
    }
    static CartesianPoint cross(const CartesianPoint & p1,const CartesianPoint & p2);
    static double rad(double phi){return phi/180.0*M_PI;}
    static double degree(double x){return x/M_PI*180.0;}
};

#endif // CARTESIAN_H
