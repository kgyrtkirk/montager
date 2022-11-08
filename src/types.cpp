#include "types.h"

t_point operator+(const t_point &lh, const t_point &rh)
{
    auto x = lh.x() + rh.x();
    auto y = lh.y() + rh.y();
    return t_point(x, y);
}
t_point operator-(const t_point &lh, const t_point &rh)
{
    auto x = lh.x() - rh.x();
    auto y = lh.y() - rh.y();
    return t_point(x, y);
}
t_point operator*(const t_point &lh, const double &rh)
{
    auto x = lh.x() * rh;
    auto y = lh.y() * rh;
    return t_point(x, y);
}
