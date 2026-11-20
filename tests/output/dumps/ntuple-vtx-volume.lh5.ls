/
├── detector_origins · table{name,xloc,yloc,zloc} 
│   ├── name · array<1>{string} 
│   ├── xloc · array<1>{real} ── {'units': 'm'}
│   ├── yloc · array<1>{real} ── {'units': 'm'}
│   └── zloc · array<1>{real} ── {'units': 'm'}
├── number_of_simulated_events · real 
├── stp · struct{det1,det2,scint1,scint2} 
│   ├── __by_uid__ · struct{det001,det002,det011,det012} 
│   │   ├── det001 -> /stp/scint1
│   │   ├── det002 -> /stp/scint2
│   │   ├── det011 -> /stp/det1
│   │   └── det012 -> /stp/det2
│   ├── det1 · table{dist_to_surf,edep,evtid,particle,time,xloc,yloc,zloc} 
│   │   ├── dist_to_surf · array<1>{real} ── {'units': 'm'}
│   │   ├── edep · array<1>{real} ── {'units': 'keV'}
│   │   ├── evtid · array<1>{real} 
│   │   ├── particle · array<1>{real} 
│   │   ├── time · array<1>{real} ── {'units': 'ns'}
│   │   ├── xloc · array<1>{real} ── {'units': 'm'}
│   │   ├── yloc · array<1>{real} ── {'units': 'm'}
│   │   └── zloc · array<1>{real} ── {'units': 'm'}
│   ├── det2 · table{dist_to_surf,edep,evtid,particle,time,xloc,yloc,zloc} 
│   │   ├── dist_to_surf · array<1>{real} ── {'units': 'm'}
│   │   ├── edep · array<1>{real} ── {'units': 'keV'}
│   │   ├── evtid · array<1>{real} 
│   │   ├── particle · array<1>{real} 
│   │   ├── time · array<1>{real} ── {'units': 'ns'}
│   │   ├── xloc · array<1>{real} ── {'units': 'm'}
│   │   ├── yloc · array<1>{real} ── {'units': 'm'}
│   │   └── zloc · array<1>{real} ── {'units': 'm'}
│   ├── scint1 · table{edep,evtid,particle,time,xloc,yloc,zloc} 
│   │   ├── edep · array<1>{real} ── {'units': 'keV'}
│   │   ├── evtid · array<1>{real} 
│   │   ├── particle · array<1>{real} 
│   │   ├── time · array<1>{real} ── {'units': 'ns'}
│   │   ├── xloc · array<1>{real} ── {'units': 'm'}
│   │   ├── yloc · array<1>{real} ── {'units': 'm'}
│   │   └── zloc · array<1>{real} ── {'units': 'm'}
│   └── scint2 · table{edep,evtid,particle,time,xloc,yloc,zloc} 
│       ├── edep · array<1>{real} ── {'units': 'keV'}
│       ├── evtid · array<1>{real} 
│       ├── particle · array<1>{real} 
│       ├── time · array<1>{real} ── {'units': 'ns'}
│       ├── xloc · array<1>{real} ── {'units': 'm'}
│       ├── yloc · array<1>{real} ── {'units': 'm'}
│       └── zloc · array<1>{real} ── {'units': 'm'}
├── vtx · table{evtid,n_part,time,volumeid,xloc,yloc,zloc} 
│   ├── evtid · array<1>{real} 
│   ├── n_part · array<1>{real} 
│   ├── time · array<1>{real} ── {'units': 'ns'}
│   ├── volumeid · array<1>{real} 
│   ├── xloc · array<1>{real} ── {'units': 'm'}
│   ├── yloc · array<1>{real} ── {'units': 'm'}
│   └── zloc · array<1>{real} ── {'units': 'm'}
└── vtx_volumes · table{name,volumeid} 
    ├── name · array<1>{string} 
    └── volumeid · array<1>{real} 
