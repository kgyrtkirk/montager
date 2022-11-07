#include "pimage.h"

void PImage::compute_hull()
{
    multi_point<point_xy> points;
    gint w = drawable->width;
    gint h = drawable->height;

    guchar *img = this->img.get();
    for (int y = 0; y < h; y++)
    {
        int g = -1;
        for (int x = 0; x < w; x++)
        {
            if (img[y * w + x] >= 255)
            {
                if (g == -1)
                    append(points, make<point_xy>(x, y));
                g = x;
            }
        }
        if (g >= 0)
            append(points, make<point_xy>(g, y));
    }

    convex_hull(points, local_hull);

    // make global hull
    translate_transformer<int, 2, 2> translate(pos.x(), pos.y());
    transform(local_hull, hull, translate);
}
