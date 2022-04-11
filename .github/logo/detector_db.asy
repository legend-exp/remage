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

from gedetplots access BEGe, SemiCoax, InvCoax;

BEGe GD79B = BEGe("GD79B", height=29.04, radius=38.42, groove_depth=2, groove_inner_r=7.5, groove_outer_r=10.5);
BEGe GD35C = BEGe("GD35C", height=26.40, radius=37.40, groove_depth=2, groove_inner_r=7.5, groove_outer_r=10.5);
BEGe GD00B = BEGe("GD00B", height=29.46, radius=36.98, groove_depth=2, groove_inner_r=7.5, groove_outer_r=10.5);
BEGe GD61C = BEGe("GD61C", height=26.45, radius=37.28, groove_depth=2, groove_inner_r=7.5, groove_outer_r=10.5);
BEGe GD76C = BEGe("GD76C", height=33.18, radius=37.92, groove_depth=2, groove_inner_r=7.5, groove_outer_r=10.5, is_passivated=true);
BEGe GD32A = BEGe("GD32A", height=24.90, radius=33.13, groove_depth=2, groove_inner_r=7.5, groove_outer_r=10.5, cone_radius=3.13, cone_height=5.50, cone_on_top=false);
BEGe GD61B = BEGe("GD61B", height=30.21, radius=37.98, groove_depth=2, groove_inner_r=7.5, groove_outer_r=10.5, is_passivated=true);
BEGe GD76B = BEGe("GD76B", height=26.29, radius=29.14, groove_depth=2, groove_inner_r=7.5, groove_outer_r=10.5, is_passivated=true);
BEGe GD91D = BEGe("GD91D", height=31.88, radius=35.65, groove_depth=2, groove_inner_r=7.5, groove_outer_r=10.5);
BEGe GD89C = BEGe("GD89C", height=24.75, radius=37.35, groove_depth=2, groove_inner_r=7.5, groove_outer_r=10.5, is_passivated=true);
BEGe GD61A = BEGe("GD61A", height=33.57, radius=36.74, groove_depth=2, groove_inner_r=7.5, groove_outer_r=10.5, cone_radius=4.99, cone_height=16.13, is_passivated=true);
BEGe GD89D = BEGe("GD89D", height=22.89, radius=36.72, groove_depth=2, groove_inner_r=7.5, groove_outer_r=10.5);
BEGe GD91C = BEGe("GD91C", height=29.79, radius=34.95, groove_depth=2, groove_inner_r=7.5, groove_outer_r=10.5, is_passivated=true);
BEGe GD32B = BEGe("GD32B", height=32.16, radius=35.95, groove_depth=2, groove_inner_r=7.5, groove_outer_r=10.5);
BEGe GD32D = BEGe("GD32D", height=32.00, radius=36.10, groove_depth=2, groove_inner_r=7.5, groove_outer_r=10.5);
BEGe GD32C = BEGe("GD32C", height=33.15, radius=35.99, groove_depth=2, groove_inner_r=7.5, groove_outer_r=10.5);
BEGe GD79C = BEGe("GD79C", height=30.22, radius=39.48, groove_depth=2, groove_inner_r=7.5, groove_outer_r=10.5);
BEGe GD02D = BEGe("GD02D", height=27.91, radius=37.30, groove_depth=2, groove_inner_r=7.5, groove_outer_r=10.5, cone_radius=3.05, cone_height=6.83, is_passivated=true);
BEGe GD32B = BEGe("GD32B", height=32.20, radius=35.90, groove_depth=2, groove_inner_r=7.5, groove_outer_r=10.5);
BEGe GD35B = BEGe("GD35B", height=32.10, radius=38.16, groove_depth=2, groove_inner_r=7.5, groove_outer_r=10.5);
BEGe GD02C = BEGe("GD02C", height=32.59, radius=37.44, groove_depth=2, groove_inner_r=7.5, groove_outer_r=10.5);
BEGe GD89B = BEGe("GD89B", height=24.85, radius=38.02, groove_depth=2, groove_inner_r=7.5, groove_outer_r=10.5, is_passivated=true);
BEGe GD00A = BEGe("GD00A", height=26.41, radius=35.16, groove_depth=2, groove_inner_r=7.5, groove_outer_r=10.5, cone_radius=11.91, cone_height=14.35, is_passivated=true);
BEGe GD02B = BEGe("GD02B", height=28.66, radius=35.51, groove_depth=2, groove_inner_r=7.5, groove_outer_r=10.5);
BEGe GD91A = BEGe("GD91A", height=31.18, radius=35.27, groove_depth=2, groove_inner_r=7.5, groove_outer_r=10.5, cone_radius=7.27, cone_height=11.50);
BEGe GD91B = BEGe("GD91B", height=30.26, radius=35.29, groove_depth=2, groove_inner_r=7.5, groove_outer_r=10.5);
BEGe GD00D = BEGe("GD00D", height=32.28, radius=38.20, groove_depth=2, groove_inner_r=7.5, groove_outer_r=10.5, is_passivated=true);
BEGe GD35B = BEGe("GD35B", height=32.00, radius=38.30, groove_depth=2, groove_inner_r=7.5, groove_outer_r=10.5);
BEGe GD32C = BEGe("GD32C", height=33.20, radius=36.00, groove_depth=2, groove_inner_r=7.5, groove_outer_r=10.5);
BEGe GD35A = BEGe("GD35A", height=35.34, radius=36.77, groove_depth=2, groove_inner_r=7.5, groove_outer_r=10.5, cone_radius=7.65, cone_height=12.75);
BEGe GD32D = BEGe("GD32D", height=32.12, radius=36.15, groove_depth=2, groove_inner_r=7.5, groove_outer_r=10.5);
BEGe GD02A = BEGe("GD02A", height=27.55, radius=35.23, groove_depth=2, groove_inner_r=7.5, groove_outer_r=10.5, cone_radius=6.48, cone_height=12.36, is_passivated=true);
BEGe GD89A = BEGe("GD89A", height=28.34, radius=34.31, groove_depth=2, groove_inner_r=7.5, groove_outer_r=10.5, cone_radius=9.06, cone_height=12.00);
BEGe GD00C = BEGe("GD00C", height=33.64, radius=37.76, groove_depth=2, groove_inner_r=7.5, groove_outer_r=10.5, is_passivated=true);
BEGe GD35C = BEGe("GD35C", height=26.32, radius=37.42, groove_depth=2, groove_inner_r=7.5, groove_outer_r=10.5, is_passivated=true);

SemiCoax ANG1 = SemiCoax("ANG1", height=68,  radius=29.23, groove_depth=4, groove_inner_r=13, groove_outer_r=19.5, borehole_depth=51, borehole_radius=6.75);
SemiCoax ANG2 = SemiCoax("ANG2", height=107, radius=40.00, groove_depth=2, groove_inner_r=15, groove_outer_r=18.0, borehole_depth=94, borehole_radius=7.00, is_passivated=true);
SemiCoax ANG3 = SemiCoax("ANG3", height=93,  radius=39.00, groove_depth=2, groove_inner_r=17, groove_outer_r=20.0, borehole_depth=83, borehole_radius=7.50, is_passivated=true);
SemiCoax ANG4 = SemiCoax("ANG4", height=100, radius=37.50, groove_depth=2, groove_inner_r=15, groove_outer_r=18.0, borehole_depth=89, borehole_radius=7.00, is_passivated=true);
SemiCoax ANG5 = SemiCoax("ANG5", height=105, radius=39.25, groove_depth=2, groove_inner_r=15, groove_outer_r=18.0, borehole_depth=94, borehole_radius=6.25);

SemiCoax RG2 = SemiCoax("RG2", height=84, radius=38.25, groove_depth=2, groove_inner_r=17, groove_outer_r=20, borehole_depth=70, borehole_radius=6.50);
SemiCoax RG1 = SemiCoax("RG1", height=84, radius=38.25, groove_depth=2, groove_inner_r=17, groove_outer_r=20, borehole_depth=73, borehole_radius=6.75);

SemiCoax GTF45  = SemiCoax("GTF45",  height=75,  radius=43.5, groove_depth=2, groove_inner_r=15, groove_outer_r=20, borehole_depth=43.0, borehole_radius=5.75, is_passivated=true);
SemiCoax GTF32  = SemiCoax("GTF32",  height=71,  radius=44.5, groove_depth=2, groove_inner_r=15, groove_outer_r=20, borehole_depth=41.5, borehole_radius=6.00, is_passivated=true);
SemiCoax GTF112 = SemiCoax("GTF112", height=100, radius=42.5, groove_depth=2, groove_inner_r=17, groove_outer_r=20, borehole_depth=63.0, borehole_radius=5.75);

InvCoax IC50A = InvCoax("IC50A", height=80.4, radius=37.35, groove_depth=2, groove_inner_r=9.5, groove_outer_r=13.5, borehole_depth=40.0, borehole_radius=5.25);
InvCoax IC50B = InvCoax("IC50B", height=85.4, radius=36.30, groove_depth=2, groove_inner_r=9.5, groove_outer_r=13.5, borehole_depth=53.9, borehole_radius=5.25);
InvCoax IC48A = InvCoax("IC48A", height=80.4, radius=37.30, groove_depth=2, groove_inner_r=10., groove_outer_r=13.0, borehole_depth=47.4, borehole_radius=5.25);
InvCoax IC48B = InvCoax("IC48B", height=80.5, radius=36.30, groove_depth=2, groove_inner_r=7.5, groove_outer_r=12.5, borehole_depth=56.0, borehole_radius=5.25);
InvCoax IC74A = InvCoax("IC74A", height=82.3, radius=38.30, groove_depth=2, groove_inner_r=9.5, groove_outer_r=13.5, borehole_depth=52.4, borehole_radius=5.25);
