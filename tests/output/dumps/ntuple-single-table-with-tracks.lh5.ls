/
├── detector_origins · table{name,xloc,yloc,zloc} 
│   ├── name · array<1>{string} 
│   ├── xloc · array<1>{real} ── {'units': 'm'}
│   ├── yloc · array<1>{real} ── {'units': 'm'}
│   └── zloc · array<1>{real} ── {'units': 'm'}
├── processes · table{name,procid} 
│   ├── name · array<1>{string} 
│   └── procid · array<1>{real} 
├── stp · struct{general,germanium,optical,scintillator} 
│   ├── __by_uid__ · struct{det001,det011,det012,det101} 
│   │   ├── det001 -> /stp/scintillator
│   │   ├── det011 -> /stp/germanium
│   │   ├── det012 -> /stp/general
│   │   └── det101 -> /stp/optical
│   ├── general · table{det_uid,dist_to_surf,edep,evtid,particle,time,xloc,yloc,zloc} 
│   │   ├── det_uid · array<1>{real} 
│   │   ├── dist_to_surf · array<1>{real} ── {'units': 'm'}
│   │   ├── edep · array<1>{real} ── {'units': 'keV'}
│   │   ├── evtid · array<1>{real} 
│   │   ├── particle · array<1>{real} 
│   │   ├── time · array<1>{real} ── {'units': 'ns'}
│   │   ├── xloc · array<1>{real} ── {'units': 'm'}
│   │   ├── yloc · array<1>{real} ── {'units': 'm'}
│   │   └── zloc · array<1>{real} ── {'units': 'm'}
│   ├── germanium · table{det_uid,dist_to_surf,edep,evtid,parent_trackid,particle,time,trackid,xloc,yloc,zloc} 
│   │   ├── det_uid · array<1>{real} 
│   │   ├── dist_to_surf · array<1>{real} ── {'units': 'm'}
│   │   ├── edep · array<1>{real} ── {'units': 'keV'}
│   │   ├── evtid · array<1>{real} 
│   │   ├── parent_trackid · array<1>{real} 
│   │   ├── particle · array<1>{real} 
│   │   ├── time · array<1>{real} ── {'units': 'ns'}
│   │   ├── trackid · array<1>{real} 
│   │   ├── xloc · array<1>{real} ── {'units': 'm'}
│   │   ├── yloc · array<1>{real} ── {'units': 'm'}
│   │   └── zloc · array<1>{real} ── {'units': 'm'}
│   ├── optical · table{det_uid,evtid,time,wavelength} 
│   │   ├── det_uid · array<1>{real} 
│   │   ├── evtid · array<1>{real} 
│   │   ├── time · array<1>{real} ── {'units': 'ns'}
│   │   └── wavelength · array<1>{real} ── {'units': 'nm'}
│   └── scintillator · table{det_uid,edep,evtid,particle,time,xloc,yloc,zloc} 
│       ├── det_uid · array<1>{real} 
│       ├── edep · array<1>{real} ── {'units': 'keV'}
│       ├── evtid · array<1>{real} 
│       ├── particle · array<1>{real} 
│       ├── time · array<1>{real} ── {'units': 'ns'}
│       ├── xloc · array<1>{real} ── {'units': 'm'}
│       ├── yloc · array<1>{real} ── {'units': 'm'}
│       └── zloc · array<1>{real} ── {'units': 'm'}
├── tracks · table{ekin,evtid,parent_trackid,particle,procid,px,py,pz,time,trackid,xloc,yloc,zloc} 
│   ├── ekin · array<1>{real} ── {'units': 'MeV'}
│   ├── evtid · array<1>{real} 
│   ├── parent_trackid · array<1>{real} 
│   ├── particle · array<1>{real} 
│   ├── procid · array<1>{real} 
│   ├── px · array<1>{real} ── {'units': 'MeV'}
│   ├── py · array<1>{real} ── {'units': 'MeV'}
│   ├── pz · array<1>{real} ── {'units': 'MeV'}
│   ├── time · array<1>{real} ── {'units': 'ns'}
│   ├── trackid · array<1>{real} 
│   ├── xloc · array<1>{real} ── {'units': 'm'}
│   ├── yloc · array<1>{real} ── {'units': 'm'}
│   └── zloc · array<1>{real} ── {'units': 'm'}
└── vtx · table{evtid,n_part,time,xloc,yloc,zloc} 
    ├── evtid · array<1>{real} 
    ├── n_part · array<1>{real} 
    ├── time · array<1>{real} ── {'units': 'ns'}
    ├── xloc · array<1>{real} ── {'units': 'm'}
    ├── yloc · array<1>{real} ── {'units': 'm'}
    └── zloc · array<1>{real} ── {'units': 'm'}
