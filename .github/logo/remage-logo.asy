settings.outformat = "png";
settings.prc = false;
settings.render = 8;
settings.maxtile = (250,250);
settings.tex = "xelatex";

// garamond font
usepackage("fontspec");
texpreamble("\setmonofont{Fira Code}");
usepackage("unicode-math", options="mathrm=sym");
texpreamble("\setmathfont{Fira Math}");
defaultpen(fontsize(12pt));

from "paultolcolors" access *;
from "detector_db" access *;
import three;
currentprojection = orthographic((5,0,1));

// predefined stuff
triple vert_sp = -40Z;
triple hor_sp  = 85Y;
triple label_zpos = -80Z;

surface edep = scale3(1.2)*unitsphere;
surface edeplar = scale3(1.3)*unitsphere;
surface alpha_v = scale3(1.2)*unitsphere;
surface beta_v = scale3(1.2)*unitsphere;

pen edepstyle = tolviborange;
pen edeplarstyle = tolvibcyan;
pen hole = linetype(new real[] {4, 2});
pen electron = linetype(new real[] {0, 2});
pen gamma = RGB(0, 68, 136);
pen alpha = tolbripurple;
pen beta = tolbrigreen;
pen optphoton = tolvibblue;

GD91A.draw(angle1=0, angle2=240, pos=O, empty=true, hi_pplus=true);
GD02D.draw(pos=vert_sp, hi_pplus=true);

draw(shift(vert_sp) * surface(yscale(0.5)*xscale(0.25)*("\texttt{remage}"),
     surface(path3((1.01*GD02D.radius,-GD02D.height/2) -- (1.01*GD02D.radius,GD02D.height/2), plane=YZplane), c=O, axis=Z),
     -4.4, 0.1), tolvibred);

triple bb = (0, 0.4*GD91A.radius, -0.2*GD91A.height);
draw(bb{-Y} .. {-Z}(0, 0, -GD91A.height/2), hole);
draw(bb{Y} .. {-Z}(0, GD91A.radius*2/3, -GD91A.height/2), electron);
draw(shift(bb)*edep, edepstyle);
label("\tiny$\beta\beta$", bb, N);

triple start_gamma = 20X-35Y+15Z;
triple edep1 = 15X-10Y+7Z;
triple edep2 = 10X+7Y+10Z;
draw(start_gamma -- edep1 -- edep2 -- -40X+20Y+17Z, gamma);
draw(shift(edep1)*edep, gamma);
draw(shift(edep2)*edep, gamma);
label("\tiny$\gamma$", start_gamma, N);
