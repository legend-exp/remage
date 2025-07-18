from __future__ import annotations

import numpy as np
from lgdo import lh5

# First check the store always option
track_data_store = lh5.read_as("tracks", "track-store-true.lh5", "ak")
vtx_data_store = lh5.read_as("vtx", "track-store-true.lh5", "ak")

# For this case we expect that all events are stored
track_evtids_store = track_data_store["evtid"]
vtx_evtids_store = vtx_data_store["evtid"]

msg = "TrackOutputScheme did not store all events, even when StoreAlways is set to true"
# vtx should consist of all event ids. So if they are different, something is wrong
assert len(np.unique(track_evtids_store)) == len(np.unique(vtx_evtids_store)), msg

# Now check the discard option
track_data_discard = lh5.read_as("tracks", "track-store-false.lh5", "ak")
stp_data_discard = lh5.read_as("stp/det001", "track-store-false.lh5", "ak")

# For this case we expect that the number of events in track and stp data is the same
# As only events with energy above the threshold in the detector are stored
track_evtids_discard = track_data_discard["evtid"]
stp_evtids_discard = stp_data_discard["evtid"]

msg = "TrackOutputScheme did not properly discard events, even when StoreAlways is set to false"

assert len(np.unique(track_evtids_discard)) == len(np.unique(stp_evtids_discard)), msg
