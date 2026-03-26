/
├── processes · struct{compt,eBrem,phot} 
│   ├── compt · real 
│   ├── eBrem · real 
│   └── phot · real 
└── tracks · table{ekin,evtid,parent_trackid,particle,procid,px,py,pz,time,trackid,xloc,yloc,zloc} 
    ├── ekin · array<1>{real} ── {'units': 'MeV'}
    ├── evtid · array<1>{real} 
    ├── parent_trackid · array<1>{real} 
    ├── particle · array<1>{real} 
    ├── procid · array<1>{real} 
    ├── px · array<1>{real} ── {'units': 'MeV'}
    ├── py · array<1>{real} ── {'units': 'MeV'}
    ├── pz · array<1>{real} ── {'units': 'MeV'}
    ├── time · array<1>{real} ── {'units': 'ns'}
    ├── trackid · array<1>{real} 
    ├── xloc · array<1>{real} ── {'units': 'm'}
    ├── yloc · array<1>{real} ── {'units': 'm'}
    └── zloc · array<1>{real} ── {'units': 'm'}
