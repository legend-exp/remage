/
├── stp · struct{det1,det2,optdet1,optdet2,scint1,scint2} 
│   ├── __links__ · struct{uid0,uid1,uid2,uid3,uid4,uid5} 
│   │   ├── uid0 -> /stp/det1
│   │   ├── uid1 -> /stp/det2
│   │   ├── uid2 -> /stp/optdet1
│   │   ├── uid3 -> /stp/optdet2
│   │   ├── uid4 -> /stp/scint1
│   │   └── uid5 -> /stp/scint2
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
│   ├── scint1 · table{edep,evtid,particle,t0,time,xloc,yloc,zloc} 
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
│   └── scint2 · table{edep,evtid,particle,t0,time,xloc,yloc,zloc} 
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
│       ├── xloc · array<1>{array<1>{real}} 
│       │   ├── cumulative_length · array<1>{real} 
│       │   └── flattened_data · array<1>{real} ── {'units': 'm'}
│       ├── yloc · array<1>{array<1>{real}} 
│       │   ├── cumulative_length · array<1>{real} 
│       │   └── flattened_data · array<1>{real} ── {'units': 'm'}
│       └── zloc · array<1>{array<1>{real}} 
│           ├── cumulative_length · array<1>{real} 
│           └── flattened_data · array<1>{real} ── {'units': 'm'}
├── tcm · table{row_in_table,table_key} ── {'hash_func': '\\d+', 'tables': "['stp/__links__/uid0', 'stp/__links__/uid1', 'stp/__links__/uid2', 'stp/__links__/uid3', 'stp/__links__/uid4', 'stp/__links__/uid5']"}
│   ├── row_in_table · array<1>{array<1>{real}} 
│   │   ├── cumulative_length · array<1>{real} 
│   │   └── flattened_data · array<1>{real} 
│   └── table_key · array<1>{array<1>{real}} 
│       ├── cumulative_length · array<1>{real} 
│       └── flattened_data · array<1>{real} 
└── vtx · table{evtid,n_part,time,xloc,yloc,zloc} 
    ├── evtid · array<1>{real} 
    ├── n_part · array<1>{real} 
    ├── time · array<1>{real} ── {'units': 'ns'}
    ├── xloc · array<1>{real} ── {'units': 'm'}
    ├── yloc · array<1>{real} ── {'units': 'm'}
    └── zloc · array<1>{real} ── {'units': 'm'}
