/
├── particles · table{evtid,vertexid,particle,px,py,pz,ekin} 
│   ├── ekin · array<1>{real} ── {'units': 'MeV'}
│   ├── evtid · array<1>{real} 
│   ├── particle · array<1>{real} 
│   ├── px · array<1>{real} ── {'units': 'MeV'}
│   ├── py · array<1>{real} ── {'units': 'MeV'}
│   ├── pz · array<1>{real} ── {'units': 'MeV'}
│   └── vertexid · array<1>{real} 
├── processes · table{procid,name} 
│   ├── name · array<1>{string} 
│   └── procid · array<1>{real} 
├── stp · struct{det1,det2,optdet1,optdet2,scint1,scint2} 
│   ├── __by_uid__ · struct{det001,det002,det011,det012,det101,det102} 
│   │   ├── det001 -> /stp/scint1
│   │   ├── det002 -> /stp/scint2
│   │   ├── det011 -> /stp/det1
│   │   ├── det012 -> /stp/det2
│   │   ├── det101 -> /stp/optdet1
│   │   └── det102 -> /stp/optdet2
│   ├── det1 · table{evtid,particle,edep,time,xloc,yloc,zloc,dist_to_surf} 
│   │   ├── dist_to_surf · array<1>{real} ── {'units': 'm'}
│   │   ├── edep · array<1>{real} ── {'units': 'keV'}
│   │   ├── evtid · array<1>{real} 
│   │   ├── particle · array<1>{real} 
│   │   ├── time · array<1>{real} ── {'units': 'ns'}
│   │   ├── xloc · array<1>{real} ── {'units': 'm'}
│   │   ├── yloc · array<1>{real} ── {'units': 'm'}
│   │   └── zloc · array<1>{real} ── {'units': 'm'}
│   ├── det2 · table{evtid,particle,edep,time,xloc,yloc,zloc,dist_to_surf} 
│   │   ├── dist_to_surf · array<1>{real} ── {'units': 'm'}
│   │   ├── edep · array<1>{real} ── {'units': 'keV'}
│   │   ├── evtid · array<1>{real} 
│   │   ├── particle · array<1>{real} 
│   │   ├── time · array<1>{real} ── {'units': 'ns'}
│   │   ├── xloc · array<1>{real} ── {'units': 'm'}
│   │   ├── yloc · array<1>{real} ── {'units': 'm'}
│   │   └── zloc · array<1>{real} ── {'units': 'm'}
│   ├── optdet1 · table{evtid,wavelength,time} 
│   │   ├── evtid · array<1>{real} 
│   │   ├── time · array<1>{real} ── {'units': 'ns'}
│   │   └── wavelength · array<1>{real} ── {'units': 'nm'}
│   ├── optdet2 · table{evtid,wavelength,time} 
│   │   ├── evtid · array<1>{real} 
│   │   ├── time · array<1>{real} ── {'units': 'ns'}
│   │   └── wavelength · array<1>{real} ── {'units': 'nm'}
│   ├── scint1 · table{evtid,particle,edep,time,xloc,yloc,zloc,v_pre,v_post} 
│   │   ├── edep · array<1>{real} ── {'units': 'keV'}
│   │   ├── evtid · array<1>{real} 
│   │   ├── particle · array<1>{real} 
│   │   ├── time · array<1>{real} ── {'units': 'ns'}
│   │   ├── v_post · array<1>{real} ── {'units': 'm/ns'}
│   │   ├── v_pre · array<1>{real} ── {'units': 'm/ns'}
│   │   ├── xloc · array<1>{real} ── {'units': 'm'}
│   │   ├── yloc · array<1>{real} ── {'units': 'm'}
│   │   └── zloc · array<1>{real} ── {'units': 'm'}
│   └── scint2 · table{evtid,particle,edep,time,xloc,yloc,zloc,v_pre,v_post} 
│       ├── edep · array<1>{real} ── {'units': 'keV'}
│       ├── evtid · array<1>{real} 
│       ├── particle · array<1>{real} 
│       ├── time · array<1>{real} ── {'units': 'ns'}
│       ├── v_post · array<1>{real} ── {'units': 'm/ns'}
│       ├── v_pre · array<1>{real} ── {'units': 'm/ns'}
│       ├── xloc · array<1>{real} ── {'units': 'm'}
│       ├── yloc · array<1>{real} ── {'units': 'm'}
│       └── zloc · array<1>{real} ── {'units': 'm'}
├── tracks · table{evtid,trackid,parent_trackid,procid,particle,time,xloc,yloc,zloc,px,py,pz,ekin} 
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
└── vtx · table{evtid,time,xloc,yloc,zloc,n_part} 
    ├── evtid · array<1>{real} 
    ├── n_part · array<1>{real} 
    ├── time · array<1>{real} ── {'units': 'ns'}
    ├── xloc · array<1>{real} ── {'units': 'm'}
    ├── yloc · array<1>{real} ── {'units': 'm'}
    └── zloc · array<1>{real} ── {'units': 'm'}
