/
├── detector_origins · struct{det1,det2} 
│   ├── det1 · struct{xloc,yloc,zloc} 
│   │   ├── xloc · real ── {'units': 'm'}
│   │   ├── yloc · real ── {'units': 'm'}
│   │   └── zloc · real ── {'units': 'm'}
│   └── det2 · struct{xloc,yloc,zloc} 
│       ├── xloc · real ── {'units': 'm'}
│       ├── yloc · real ── {'units': 'm'}
│       └── zloc · real ── {'units': 'm'}
├── number_of_events · real 
├── particles · table{ekin,evtid,particle,px,py,pz,vertexid} 
│   ├── ekin · array<1>{real} ── {'units': 'MeV'}
│   ├── evtid · array<1>{real} 
│   ├── particle · array<1>{real} 
│   ├── px · array<1>{real} ── {'units': 'MeV'}
│   ├── py · array<1>{real} ── {'units': 'MeV'}
│   ├── pz · array<1>{real} ── {'units': 'MeV'}
│   └── vertexid · array<1>{real} 
├── processes · struct{compt,eBrem,phot} 
│   ├── compt · real 
│   ├── eBrem · real 
│   └── phot · real 
├── stp · struct{det1,det2,optdet1,optdet2,scint1,scint2} 
│   ├── __by_uid__ · struct{det001,det002,det011,det012,det101,det102} 
│   │   ├── det001 -> /stp/scint1
│   │   ├── det002 -> /stp/scint2
│   │   ├── det011 -> /stp/det1
│   │   ├── det012 -> /stp/det2
│   │   ├── det101 -> /stp/optdet1
│   │   └── det102 -> /stp/optdet2
│   ├── det1 · table{dist_to_surf,edep,evtid,particle,t0,time,xloc,yloc,zloc} 
│   │   ├── dist_to_surf · array<1>{array<1>{real}} 
│   │   │   ├── cumulative_length · array<1>{real} 
│   │   │   └── flattened_data · array<1>{real} ── {'units': 'm'}
│   │   ├── edep · array<1>{array<1>{real}} 
│   │   │   ├── cumulative_length · array<1>{real} 
│   │   │   └── flattened_data · array<1>{real} ── {'units': 'keV'}
│   │   ├── evtid · array<1>{real} 
│   │   ├── particle · array<1>{array<1>{real}} 
│   │   │   ├── cumulative_length · array<1>{real} 
│   │   │   └── flattened_data · array<1>{real} 
│   │   ├── t0 · array<1>{real} ── {'units': 'ns'}
│   │   ├── time · array<1>{array<1>{real}} 
│   │   │   ├── cumulative_length · array<1>{real} 
│   │   │   └── flattened_data · array<1>{real} ── {'units': 'ns'}
│   │   ├── xloc · array<1>{array<1>{real}} 
│   │   │   ├── cumulative_length · array<1>{real} 
│   │   │   └── flattened_data · array<1>{real} ── {'units': 'm'}
│   │   ├── yloc · array<1>{array<1>{real}} 
│   │   │   ├── cumulative_length · array<1>{real} 
│   │   │   └── flattened_data · array<1>{real} ── {'units': 'm'}
│   │   └── zloc · array<1>{array<1>{real}} 
│   │       ├── cumulative_length · array<1>{real} 
│   │       └── flattened_data · array<1>{real} ── {'units': 'm'}
│   ├── det2 · table{dist_to_surf,edep,evtid,particle,t0,time,xloc,yloc,zloc} 
│   │   ├── dist_to_surf · array<1>{array<1>{real}} 
│   │   │   ├── cumulative_length · array<1>{real} 
│   │   │   └── flattened_data · array<1>{real} ── {'units': 'm'}
│   │   ├── edep · array<1>{array<1>{real}} 
│   │   │   ├── cumulative_length · array<1>{real} 
│   │   │   └── flattened_data · array<1>{real} ── {'units': 'keV'}
│   │   ├── evtid · array<1>{real} 
│   │   ├── particle · array<1>{array<1>{real}} 
│   │   │   ├── cumulative_length · array<1>{real} 
│   │   │   └── flattened_data · array<1>{real} 
│   │   ├── t0 · array<1>{real} ── {'units': 'ns'}
│   │   ├── time · array<1>{array<1>{real}} 
│   │   │   ├── cumulative_length · array<1>{real} 
│   │   │   └── flattened_data · array<1>{real} ── {'units': 'ns'}
│   │   ├── xloc · array<1>{array<1>{real}} 
│   │   │   ├── cumulative_length · array<1>{real} 
│   │   │   └── flattened_data · array<1>{real} ── {'units': 'm'}
│   │   ├── yloc · array<1>{array<1>{real}} 
│   │   │   ├── cumulative_length · array<1>{real} 
│   │   │   └── flattened_data · array<1>{real} ── {'units': 'm'}
│   │   └── zloc · array<1>{array<1>{real}} 
│   │       ├── cumulative_length · array<1>{real} 
│   │       └── flattened_data · array<1>{real} ── {'units': 'm'}
│   ├── optdet1 · table{evtid,t0,time,wavelength} 
│   │   ├── evtid · array<1>{real} 
│   │   ├── t0 · array<1>{real} ── {'units': 'ns'}
│   │   ├── time · array<1>{array<1>{real}} 
│   │   │   ├── cumulative_length · array<1>{real} 
│   │   │   └── flattened_data · array<1>{real} ── {'units': 'ns'}
│   │   └── wavelength · array<1>{array<1>{real}} 
│   │       ├── cumulative_length · array<1>{real} 
│   │       └── flattened_data · array<1>{real} ── {'units': 'nm'}
│   ├── optdet2 · table{evtid,t0,time,wavelength} 
│   │   ├── evtid · array<1>{real} 
│   │   ├── t0 · array<1>{real} ── {'units': 'ns'}
│   │   ├── time · array<1>{array<1>{real}} 
│   │   │   ├── cumulative_length · array<1>{real} 
│   │   │   └── flattened_data · array<1>{real} ── {'units': 'ns'}
│   │   └── wavelength · array<1>{array<1>{real}} 
│   │       ├── cumulative_length · array<1>{real} 
│   │       └── flattened_data · array<1>{real} ── {'units': 'nm'}
│   ├── scint1 · table{edep,evtid,particle,t0,time,v_post,v_pre,xloc,yloc,zloc} 
│   │   ├── edep · array<1>{array<1>{real}} 
│   │   │   ├── cumulative_length · array<1>{real} 
│   │   │   └── flattened_data · array<1>{real} ── {'units': 'keV'}
│   │   ├── evtid · array<1>{real} 
│   │   ├── particle · array<1>{array<1>{real}} 
│   │   │   ├── cumulative_length · array<1>{real} 
│   │   │   └── flattened_data · array<1>{real} 
│   │   ├── t0 · array<1>{real} ── {'units': 'ns'}
│   │   ├── time · array<1>{array<1>{real}} 
│   │   │   ├── cumulative_length · array<1>{real} 
│   │   │   └── flattened_data · array<1>{real} ── {'units': 'ns'}
│   │   ├── v_post · array<1>{array<1>{real}} 
│   │   │   ├── cumulative_length · array<1>{real} 
│   │   │   └── flattened_data · array<1>{real} ── {'units': 'm/ns'}
│   │   ├── v_pre · array<1>{array<1>{real}} 
│   │   │   ├── cumulative_length · array<1>{real} 
│   │   │   └── flattened_data · array<1>{real} ── {'units': 'm/ns'}
│   │   ├── xloc · array<1>{array<1>{real}} 
│   │   │   ├── cumulative_length · array<1>{real} 
│   │   │   └── flattened_data · array<1>{real} ── {'units': 'm'}
│   │   ├── yloc · array<1>{array<1>{real}} 
│   │   │   ├── cumulative_length · array<1>{real} 
│   │   │   └── flattened_data · array<1>{real} ── {'units': 'm'}
│   │   └── zloc · array<1>{array<1>{real}} 
│   │       ├── cumulative_length · array<1>{real} 
│   │       └── flattened_data · array<1>{real} ── {'units': 'm'}
│   └── scint2 · table{edep,evtid,particle,t0,time,v_post,v_pre,xloc,yloc,zloc} 
│       ├── edep · array<1>{array<1>{real}} 
│       │   ├── cumulative_length · array<1>{real} 
│       │   └── flattened_data · array<1>{real} ── {'units': 'keV'}
│       ├── evtid · array<1>{real} 
│       ├── particle · array<1>{array<1>{real}} 
│       │   ├── cumulative_length · array<1>{real} 
│       │   └── flattened_data · array<1>{real} 
│       ├── t0 · array<1>{real} ── {'units': 'ns'}
│       ├── time · array<1>{array<1>{real}} 
│       │   ├── cumulative_length · array<1>{real} 
│       │   └── flattened_data · array<1>{real} ── {'units': 'ns'}
│       ├── v_post · array<1>{array<1>{real}} 
│       │   ├── cumulative_length · array<1>{real} 
│       │   └── flattened_data · array<1>{real} ── {'units': 'm/ns'}
│       ├── v_pre · array<1>{array<1>{real}} 
│       │   ├── cumulative_length · array<1>{real} 
│       │   └── flattened_data · array<1>{real} ── {'units': 'm/ns'}
│       ├── xloc · array<1>{array<1>{real}} 
│       │   ├── cumulative_length · array<1>{real} 
│       │   └── flattened_data · array<1>{real} ── {'units': 'm'}
│       ├── yloc · array<1>{array<1>{real}} 
│       │   ├── cumulative_length · array<1>{real} 
│       │   └── flattened_data · array<1>{real} ── {'units': 'm'}
│       └── zloc · array<1>{array<1>{real}} 
│           ├── cumulative_length · array<1>{real} 
│           └── flattened_data · array<1>{real} ── {'units': 'm'}
├── tcm · table{row_in_table,table_key} ── {'hash_func': '(?<=stp/__by_uid__/det)\\d+', 'tables': "['stp/__by_uid__/det001', 'stp/__by_uid__/det002', 'stp/__by_uid__/det011', 'stp/__by_uid__/det012', 'stp/__by_uid__/det101', 'stp/__by_uid__/det102']"}
│   ├── row_in_table · array<1>{array<1>{real}} 
│   │   ├── cumulative_length · array<1>{real} 
│   │   └── flattened_data · array<1>{real} 
│   └── table_key · array<1>{array<1>{real}} 
│       ├── cumulative_length · array<1>{real} 
│       └── flattened_data · array<1>{real} 
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
