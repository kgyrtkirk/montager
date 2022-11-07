#include "pmontage.h"

// this is a bit different than Voronoi diagram; but the concept is close-enough
// in this case the generators are not just points - but polygons
void PMontage::assign_voronoi()
{
    if (num_inputs() < 2)
    {
        g_warning("Not enough input layers for this operation; skipping assign_voronoi!");
        return;
    }

    gimp_progress_set_text("assign_voronoi");
    if (images.size() >= 253)
    {
        g_error("more images than supported");
    }
    image aimg;
    aimg.init(width, height);

    dist_queue dq;

    for (int i = 0; i < images.size(); i++)
    {
        dq.add(new dist_queue::entry(images[i].getHull(), i));
    }

    for (int y = 0; y < height; y++)
    {
        gimp_progress_update(((double)y) / height);
        guchar *row = aimg.row(y);

        for (int x0 = 0; x0 < width; x0++)
        {
            int x = (y & 1) ? width - 1 - x0 : x0;

            if (row[x] > 0)
                continue;

            // snake-alike space filling curve; do provide |p-p'|=1 invariant
            point_xy<int> p(x, y);

            auto m = dq.min(p);
            int minPos = m->image_idx;
            double r = dq.min_radius(p);
            aimg.fill_circle(p, r, minPos + 1);
        }
    }

    for (int y = 0; y < height; y++)
    {
        // gimp_progress_update(((double)y) / height);
        guchar *row = aimg.row(y);
        for (int x = 0; x < width; x++)
        {
            point_xy<int> p(x, y);
            images[row[x] - 1].paint(p);
        }
    }
}
