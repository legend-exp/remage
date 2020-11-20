// The MIT License (MIT)
//
// Copyright (c) 2019 Luigi Pertoldi
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.

import three;

void showaxes(picture pic=currentpicture, triple pos=O) {
    draw(pos -- pos + X, arrow=Arrow3(TeXHead2));
    draw(pos -- pos + Y, arrow=Arrow3(TeXHead2));
    draw(pos -- pos + Z, arrow=Arrow3(TeXHead2));
    label("$x$", pos + X, align=SW);
    label("$y$", pos + Y, align=SE);
    label("$z$", pos + Z, align=N);
}

// define material coloring
material germanium = material(
    diffusepen=gray(0.7),
    emissivepen=gray(0.2),
    shininess=0.5
);

material ge_pplus = material(
    diffusepen=RGB(238,119,51),
    emissivepen=RGB(119, 59, 25),
    shininess=0.5
);

material ge_nplus = material(
    emissivepen=RGB(0,153,136),
    shininess=0.5
);

material ge_oxide = material(
    diffusepen=RGB(24,91,176)+opacity(0.6),
    shininess=0.5
);

real eps_edge_rounding = 0.5;

struct gedet_profile {
    path all;
    path pplus;
    path nplus;
    path groove;
    path passlayer;

    path _namesurf;
}

/*
 * Base struct for detector with common operations
 */

struct gedet {
    gedet_profile profile;

    string name;
    real height;
    real radius;
    real passlayer_thickness = 0.1;

    /*
     * Draw full 3D detector
     */
    void draw(picture pic=currentpicture, triple pos=O,
              bool flip=false, real angle1=0, real angle2=360,
              bool hi_pplus=false, bool empty=false, bool name=false) {

        // sanity checks
        if (angle1 >= angle2 || angle2-angle1 > 360) abort("gedet.draw(): invalid input");

        // shift (and flip?) transformation
        real[][] trans = shift(pos) * rotate(flip ? 180 : 0, X);

        path3 profile3 = path3(this.profile.all, plane=YZplane);

        draw(trans * surface(path3(this.profile.pplus, plane=YZplane),
                             c=O, axis=Z, angle1=angle1, angle2=angle2),
             surfacepen=(hi_pplus ? ge_pplus : germanium));

        draw(trans * surface(path3(this.profile.nplus, plane=YZplane),
                             c=O, axis=Z, angle1=angle1, angle2=angle2),
             surfacepen=germanium);
        draw(trans * surface(path3(this.profile.groove, plane=YZplane),
                             c=O, axis=Z, angle1=angle1, angle2=angle2),
             surfacepen=germanium);

        if (this.profile.passlayer != nullpath) {
            path3 passlayer3 = path3(this.profile.passlayer, plane=YZplane);
            draw(trans * surface(passlayer3, c=O, axis=Z, angle1=angle1, angle2=angle2), surfacepen=ge_oxide);
            if (angle2-angle1 != 360) {
                draw(trans * rotate(angle1, Z) * surface(passlayer3 -- cycle), surfacepen=ge_oxide);
                draw(trans * rotate(angle2, Z) * surface(passlayer3 -- cycle), surfacepen=ge_oxide);
            }
        }
        // draw faces in cut view
        if (angle2-angle1 != 360 && !empty) {
            draw(trans * rotate(angle1, Z) * surface(profile3 -- cycle), surfacepen=germanium);
            draw(trans * rotate(angle2, Z) * surface(profile3 -- cycle), surfacepen=germanium);
            // draw(trans * rotate(angle2, Z) * surface(yscale(1)*xscale(1)*("\texttt{"+this.name+"}"),
            //                                          surface(profile3 -- cycle),
            //                                          0, 0));
        }

        if (name) {
            draw(trans * surface(yscale(0.5)*xscale(0.25)*("\texttt{" + this.name + "}"),
                 surface(path3(this.profile._namesurf, plane=YZplane), c=O, axis=Z),
                 -4.4, 0.1));
        }
    }

    /*
     * Draw just 2D profile
     */
    // pure virtual function to be overloaded later
    void draw_name(picture pic=currentpicture, pair pos=(0,0));
    // actual routine
    void draw(picture pic=currentpicture, pair pos=(0,0),
              bool flip=false, bool name=true) {

        transform trans = shift(pos) * rotate(flip ? 180 : 0);

        // draw groove
        draw(trans * reflect((0,0), (0,1)) * this.profile.groove, linewidth(2)+germanium.p[0]);
        draw(trans * this.profile.groove,                         linewidth(2)+germanium.p[0]);

        // draw passivation layer
        // draw(trans * reflect((0,0), (0,1)) * this.profile.passlayer, linewidth(2)+ge_oxide.p[1]);
        // draw(trans * this.profile.passlayer,                         linewidth(2)+ge_oxide.p[1]);

        // draw pplus
        draw(trans * reflect((0,0), (0,1)) * this.profile.pplus, linewidth(2)+ge_pplus.p[0]);
        draw(trans * this.profile.pplus,                         linewidth(2)+ge_pplus.p[0]);

        // draw nplus
        draw(trans * reflect((0,0), (0,1)) * this.profile.nplus, linewidth(2)+ge_nplus.p[1]);
        draw(trans * this.profile.nplus,                         linewidth(2)+ge_nplus.p[1]);

        // add detector name
        this.draw_name(pos);
    }
}

/*
 * BEGe geometry
 */

struct BEGe {
    gedet base;
    unravel base;

    real groove_depth;
    real groove_inner_r;
    real groove_outer_r;
    real cone_radius;
    real cone_height;
    bool cone_on_top;

    void operator init(string name="",
                       real keyword height, real keyword radius, real keyword groove_depth,
                       real keyword groove_inner_r, real keyword groove_outer_r,
                       real keyword cone_radius=0, real keyword cone_height=0,
                       bool keyword cone_on_top=true, bool keyword is_passivated=false) {
        this.name = name;
        this.height = height;
        this.radius = radius;
        this.groove_depth = groove_depth;
        this.groove_inner_r = groove_inner_r;
        this.groove_outer_r = groove_outer_r;
        this.cone_radius = cone_radius;
        this.cone_height = cone_height;
        this.cone_on_top = cone_on_top;
        if (!is_passivated) this.passlayer_thickness = 0;

        // apply some edge rounding, to make it look more realistic
        real eps = eps_edge_rounding;

        // define p+ profile
        this.profile.pplus = (0,0) -- (this.groove_inner_r-eps,0){right};

        // define groove profile
        this.profile.groove = (this.groove_inner_r-eps,0){right}
            .. {up}(this.groove_inner_r,eps)
            -- (this.groove_inner_r,this.groove_depth-eps){up}
            .. {right}(this.groove_inner_r+eps,this.groove_depth)
            -- (this.groove_outer_r-eps,this.groove_depth){right}
            .. {down}(this.groove_outer_r,this.groove_depth-eps)
            -- (this.groove_outer_r,eps){down} .. {right}(this.groove_outer_r+eps,0);

        // define nplus profile, check for tapering
        if (this.cone_height != 0 && this.cone_radius != 0) {
            real theta = atan(this.cone_height/this.cone_radius);
            if (this.cone_on_top == true) {
                this.profile.nplus = (this.groove_outer_r+eps,0)
                    -- (this.radius-eps,0){right} .. {up}(this.radius,eps)
                    -- (this.radius,this.height-this.cone_height-eps){up}
                    .. (this.radius-eps*cos(theta),this.height-this.cone_height+eps*sin(theta))
                    -- (this.radius-this.cone_radius+eps*cos(theta),this.height-eps*sin(theta))
                    .. {left}(this.radius-this.cone_radius-eps,this.height) -- (0,this.height);
            }
            else {
                this.profile.nplus = (this.groove_outer_r+eps,0)
                    -- (this.radius-this.cone_radius-eps,0){right}
                    .. (this.radius-this.cone_radius+eps*cos(theta),eps*sin(theta))
                    -- (this.radius-eps*cos(theta),this.cone_height-eps*sin(theta))
                    .. {up}(this.radius,this.cone_height+eps)
                    -- (this.radius,this.height-eps){up} .. {left}(this.radius-eps,this.height)
                    -- (0,this.height);
            }
        }
        else {
            this.profile.nplus = (this.groove_outer_r+eps,0)
                -- (this.radius-eps,0){right} .. {up}(this.radius,eps)
                -- (this.radius,this.height-eps){up} .. {left}(this.radius-eps,this.height)
                -- (0,this.height);
        }
        // center profile (detector) in origin
        this.profile.nplus = shift(0,-this.height/2) * this.profile.nplus;
        this.profile.pplus = shift(0,-this.height/2) * this.profile.pplus;
        this.profile.groove = shift(0,-this.height/2) * this.profile.groove;

        // define total profile
        this.profile.all = this.profile.pplus -- this.profile.groove -- this.profile.nplus;

        // passivation layer
        if (this.passlayer_thickness > 0) {
            this.profile.passlayer = (this.groove_inner_r-eps,0){right}
                .. {up}(this.groove_inner_r,eps)
                -- (this.groove_inner_r,this.groove_depth-eps){up}
                .. {right}(this.groove_inner_r+eps,this.groove_depth)
                -- (this.groove_outer_r-eps,this.groove_depth){right}
                .. {down}(this.groove_outer_r,this.groove_depth-eps)
                -- (this.groove_outer_r,eps){down} .. {right}(this.groove_outer_r+eps,0); // and back...
            real delta = this.passlayer_thickness;
            this.profile.passlayer = this.profile.passlayer{left}
                .. {up}(this.groove_outer_r-delta,eps)
                -- (this.groove_outer_r-delta,this.groove_depth-eps){up}
                .. {left}(this.groove_outer_r-eps,this.groove_depth-delta)
                -- (this.groove_inner_r+eps,this.groove_depth-delta){left}
                .. {down}(this.groove_inner_r+delta,this.groove_depth-eps)
                -- (this.groove_inner_r+delta,eps){down} .. {left}(this.groove_inner_r-eps,0);

            // center profile (passivation layer) in origin
            this.profile.passlayer = shift(0,-this.height/2) * this.profile.passlayer;
        }
        this.profile._namesurf = (1.01*this.radius,-this.height/2) -- (1.01*this.radius,this.height/2);
    }

    // define how to write detector name (reimplement virtual function)
    void draw_name(picture pic=currentpicture, pair pos=(0,0)) {
        label(Label("\large\texttt{" + this.name + "}"), pos);
    }
    base.draw_name = draw_name;
}

/*
 * Semi-coaxial geometry
 */

struct SemiCoax {
    BEGe bege; // use BEGe as base class
    unravel bege;

    // .. but add borehole (part of p+)
    real borehole_depth;
    real borehole_radius;

    void operator init(string name="",
                       real keyword height, real keyword radius, real keyword groove_depth,
                       real keyword groove_inner_r, real keyword groove_outer_r,
                       real keyword borehole_radius, real keyword borehole_depth,
                       real keyword cone_radius=0, real keyword cone_height=0,
                       bool keyword cone_on_top=true, bool keyword is_passivated=false) {
        bege.operator init(name, height=height, radius=radius, groove_depth=groove_depth,
                           groove_inner_r=groove_inner_r, groove_outer_r=groove_outer_r,
                           cone_radius=cone_radius, cone_height=cone_height, cone_on_top=cone_on_top,
                           is_passivated=is_passivated);
        this.borehole_radius = borehole_radius;
        this.borehole_depth = borehole_depth;

        // apply some edge rounding, to make it look more realistic
        real eps = eps_edge_rounding;

        // define p+ profile
        this.profile.pplus = (0,this.borehole_depth)
                 -- (this.borehole_radius-eps,this.borehole_depth){right}
                 .. {down}(this.borehole_radius,this.borehole_depth-eps)
                 -- (this.borehole_radius,0+eps){down} .. {right}(this.borehole_radius+eps,0)
                 -- (this.groove_inner_r-eps,0);

        // define groove profile
        this.profile.groove = (this.groove_inner_r-eps,0){right} .. {up}(this.groove_inner_r,eps)
                 -- (this.groove_inner_r,this.groove_depth-eps){up}
                 .. {right}(this.groove_inner_r+eps,this.groove_depth)
                 -- (this.groove_outer_r-eps,this.groove_depth){right}
                 .. {down}(this.groove_outer_r,this.groove_depth-eps)
                 -- (this.groove_outer_r,eps){down} .. {right}(this.groove_outer_r+eps,0);

        // define n+ profile, check if tapering
        if (this.cone_height != 0 && this.cone_radius != 0) {
            real theta = atan(this.cone_height/this.cone_radius);
            if (this.cone_on_top == true) {
                this.profile.nplus = (this.groove_outer_r+eps,0)
                    -- (this.radius-eps,0){right} .. {up}(this.radius,eps)
                    -- (this.radius,this.height-this.cone_height-eps){up}
                    .. (this.radius-eps*cos(theta),this.height-this.cone_height+eps*sin(theta))
                    -- (this.radius-this.cone_radius+eps*cos(theta),this.height-eps*sin(theta))
                    .. {left}(this.radius-this.cone_radius-eps,this.height) -- (0,this.height);
            }
            else {
                this.profile.nplus = (this.groove_outer_r+eps,0)
                    -- (this.radius-this.cone_radius-eps,0){right}
                    .. (this.radius-this.cone_radius+eps*cos(theta),eps*sin(theta))
                    -- (this.radius-eps*cos(theta),this.cone_height-eps*sin(theta))
                    .. {up}(this.radius,this.cone_height+eps)
                    -- (this.radius,this.height-eps){up} .. {left}(this.radius-eps,this.height)
                    -- (0,this.height);
            }
        }
        else {
            this.profile.nplus = (this.groove_outer_r+eps,0)
                -- (this.radius-eps,0){right} .. {up}(this.radius,eps)
                -- (this.radius,this.height-eps){up} .. {left}(this.radius-eps,this.height)
                -- (0,this.height);
        }
        // center profile (detector) in origin
        this.profile.nplus = shift(0,-this.height/2) * this.profile.nplus;
        this.profile.pplus = shift(0,-this.height/2) * this.profile.pplus;
        this.profile.groove = shift(0,-this.height/2) * this.profile.groove;

        // define total profile
        this.profile.all = this.profile.pplus -- this.profile.groove -- this.profile.nplus;
    }

    // define how to write the name (reimplement wirtual function)
    void draw_name(picture pic=currentpicture, pair pos=(0,0)) {
        label(rotate(90)*Label("\large\texttt{" + this.name + "}"),
              pos - (this.radius, this.height/2) + (3,3), align=NE);
    }
    base.draw_name = draw_name;
}

/*
 * Inverted-coaxial geometry
 */

struct InvCoax {
    BEGe bege; // use BEGe as base class
    unravel bege;

    real borehole_depth;
    real borehole_radius;

    void operator init(string name="",
                       real keyword height, real keyword radius, real keyword groove_depth,
                       real keyword groove_inner_r, real keyword groove_outer_r,
                       real keyword borehole_radius, real keyword borehole_depth,
                       real keyword cone_radius=0, real keyword cone_height=0,
                       bool keyword cone_on_top=true, bool keyword is_passivated=false) {
        bege.operator init(name, height=height, radius=radius, groove_depth=groove_depth,
                           groove_inner_r=groove_inner_r, groove_outer_r=groove_outer_r,
                           cone_radius=cone_radius, cone_height=cone_height, cone_on_top=cone_on_top,
                           is_passivated=is_passivated);
        this.borehole_radius = borehole_radius;
        this.borehole_depth = borehole_depth;

        // apply some edge rounding, to make it look more realistic
        real eps = eps_edge_rounding;

        // define p+ profile
        this.profile.pplus = (0,0) -- (this.groove_inner_r-eps,0);

        // define groove
        this.profile.groove = (this.groove_inner_r-eps,0){right}
            .. {up}(this.groove_inner_r,eps)
            -- (this.groove_inner_r,this.groove_depth-eps){up}
            .. {right}(this.groove_inner_r+eps,this.groove_depth)
            -- (this.groove_outer_r-eps,this.groove_depth){right}
            .. {down}(this.groove_outer_r,this.groove_depth-eps)
            -- (this.groove_outer_r,eps){down} .. {right}(this.groove_outer_r+eps,0);

        // define n+ profile
        if (this.cone_height != 0 && this.cone_radius != 0) {
            real theta = atan(this.cone_height/this.cone_radius);
            if (this.cone_on_top == true) {
                this.profile.nplus = (this.groove_outer_r+eps,0)
                    -- (this.radius-eps,0){right} .. {up}(this.radius,eps)
                    -- (this.radius,this.height-this.cone_height-eps){up}
                    .. (this.radius-eps*cos(theta),this.height-this.cone_height+eps*sin(theta))
                    -- (this.radius-this.cone_radius+eps*cos(theta),this.height-eps*sin(theta))
                    .. {left}(this.radius-this.cone_radius-eps,this.height);
            }
            else {
                this.profile.nplus = (this.groove_outer_r+eps,0)
                    -- (this.radius-this.cone_radius-eps,0){right}
                    .. (this.radius-this.cone_radius+eps*cos(theta),eps*sin(theta))
                    -- (this.radius-eps*cos(theta),this.cone_height-eps*sin(theta))
                    .. {up}(this.radius,this.cone_height+eps)
                    -- (this.radius,this.height-eps){up} .. {left}(this.radius-eps,this.height);
            }
        }
        else {
            this.profile.nplus = (this.groove_outer_r+eps,0)
                -- (this.radius-eps,0){right} .. {up}(this.radius,eps)
                -- (this.radius,this.height-eps){up} .. {left}(this.radius-eps,this.height);
        }
        // add borehole on top
        this.profile.nplus = this.profile.nplus -- (this.borehole_radius+eps,this.height){left}
              .. {down}(this.borehole_radius,this.height-eps)
              -- (this.borehole_radius,this.height-this.borehole_depth+eps){down}
              .. {left}(this.borehole_radius-eps,this.height-this.borehole_depth)
              -- (0,this.height-this.borehole_depth);

        // center profile (detector) in origin
        this.profile.nplus = shift(0,-this.height/2) * this.profile.nplus;
        this.profile.pplus = shift(0,-this.height/2) * this.profile.pplus;
        this.profile.groove = shift(0,-this.height/2) * this.profile.groove;

        // define total profile
        this.profile.all = this.profile.pplus -- this.profile.groove -- this.profile.nplus;
    }

    // define how to write the name (reimplement wirtual function)
    void draw_name(picture pic=currentpicture, pair pos=(0,0)) {
        label(rotate(90)*Label("\large\texttt{" + this.name + "}"),
              pos - (this.radius, this.height/2) + (3,3), align=NE);
    }
    base.draw_name = draw_name;
}
