#include "cartesian.h"
#include <math.h>
CartesianPoint::CartesianPoint(const Trackpoint &p)
{
   x=cos(rad(p.latitude))*cos(rad(p.longitude));
   y=cos(rad(p.latitude))*sin(rad(p.longitude));
   z=sin(rad(p.latitude));
}

CartesianPoint CartesianPoint::cross(const CartesianPoint &p1, const CartesianPoint &p2)
{
    return CartesianPoint(p1.y*p2.z-p1.z*p2.y ,
                          -p1.x*p2.z+p1.z*p2.x ,
                          p1.x*p2.y-p1.y*p2.x);
}
