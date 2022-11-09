#pragma once

#include "types.h"
#include "progress.h"

std::vector<t_polygon> guard_polys(int w, int h);


namespace montager {

    class Positionable
{
public:
	virtual void setPos(const t_point2i&pos)=0;
	virtual t_point2i getPos() const=0;
    virtual ~Positionable() {};
};



class fd_layout
{
public:
    class entry
    {

    public:
        t_point position;
        t_polygon g;
        t_point center;
        t_point force;
        int image_idx;
        bool freeze;

        entry(t_point _pos, t_polygon _g, int _image_idx, Positionable*item, bool _freeze=false);

        t_point force_dir(const entry &o) const;
        t_point force_vector(const entry &o) const;
        double distance1(const entry &o) const;

        t_polygon absolute_poly() const;
        t_point absolute_center() const;
        void reset ();
        void commit ();
        void add_force(const t_point&f);
    };

    std::vector<entry *> elements;
    double max_step;

    fd_layout(int width, int height);
    void add(entry *e)
    {
        elements.push_back(e);
    }
    t_point compute_force(entry*l,entry*r);
    
    void step(double step_size);
    void run(const progress::progress_handler&progress);
};

};