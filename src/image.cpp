#include "image.h"

void image::fill_circle(const point_xy &p, double radius, int value)
{
    int r = radius;
    if (r > 60000)
    {
        g_error("radius is pretty big - is everything alright?");
        r = 60000;
    }
    if (r > width)
    {
        r = width;
    }
    if (r < 1)
    {
        paint(p, value);
        return;
    }

    int c_x = p.x();
    int c_y = p.y();
    int r2 = r * r;

    for (int y0 = -r; y0 <= r; y0++)
    {
        for (int x0 = -r; x0 <= r; x0++)
        {
            if (x0 * x0 + y0 * y0 > r2)
            {
                continue;
            }
            int x = x0 + c_x;
            int y = y0 + c_y;
            if (0 <= x && x < width &&
                0 <= y && y < height)
            {
                safe_paint(x, y, value);
            }
        }
    }
}
