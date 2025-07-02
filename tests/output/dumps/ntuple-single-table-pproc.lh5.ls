/
├── stp · struct{germanium,optical,scintillator} 
│   ├── germanium · table{det_uid,dist_to_surf,edep,evtid,particle,t0,time,xloc,yloc,zloc} 
│   │   ├── det_uid · array<1>{array<1>{real}} 
│   │   │   ├── cumulative_length · array<1>{real} 
│   │   │   └── flattened_data · array<1>{real} 
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
│   ├── optical · table{det_uid,evtid,t0,time,wavelength} 
│   │   ├── det_uid · array<1>{array<1>{real}} 
│   │   │   ├── cumulative_length · array<1>{real} 
│   │   │   └── flattened_data · array<1>{real} 
│   │   ├── evtid · array<1>{real} 
│   │   ├── t0 · array<1>{real} ── {'units': 'ns'}
│   │   ├── time · array<1>{array<1>{real}} 
│   │   │   ├── cumulative_length · array<1>{real} 
│   │   │   └── flattened_data · array<1>{real} ── {'units': 'ns'}
│   │   └── wavelength · array<1>{array<1>{real}} 
│   │       ├── cumulative_length · array<1>{real} 
│   │       └── flattened_data · array<1>{real} ── {'units': 'nm'}
│   └── scintillator · table{det_uid,edep,evtid,particle,t0,time,xloc,yloc,zloc} 
│       ├── det_uid · array<1>{array<1>{real}} 
│       │   ├── cumulative_length · array<1>{real} 
│       │   └── flattened_data · array<1>{real} 
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
├── tcm · table{row_in_table,table_key} ── {'hash_func': 'None', 'tables': "['stp/germanium', 'stp/optical', 'stp/scintillator']"}
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
