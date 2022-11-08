#pragma once

#include "types.h"

class fd_layout
{
public:
    class entry
    {

    public:
        t_point position;
        t_polygon g;
        t_point center;
        int image_idx;

        entry(t_point _pos, t_polygon _g, int _image_idx);
    };

    std::vector<entry *> elements;
    void add(entry *e)
    {
        elements.push_back(e);
    }
};