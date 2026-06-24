# Th-228 HPGe energy spectrum

This validation test compares a _remage_ simulation against experimental data
collected at the HADES[^HEROICA][^BEGeChar] underground laboratory, which hosts
a HPGe detector characterization test stand used by the
[LEGEND](https://legend-exp.org) experiment. A BEGe detector is exposed to a
Th-228 calibration source and the recorded energy spectrum is compared to the
_remage_ prediction, providing an end-to-end check of the simulation chain from
particle transport to spectrum generation.

## Setup

**Detector.** The simulated detector (B00000B) is a BEGe with a height of 29 mm
and a radius of 37 mm. The geometry is simplified: it omits the p+ contact,
groove, and taper.

**Source.** A Th-228 point source is placed on the detector symmetry axis, 41 mm
above the detector face. The source activity at the time of measurement
(2021-01-18) was ~6.3 kBq.

**Experimental data.** The data were recorded in a 17-hour run (including DAQ
dead-time). Quality cuts targeting pile-up and unphysical events retain ~84% of
events. The measured spectrum is normalised to the expected number of source
decays during the live-time, accounting for the QC survival fraction.

## Simulation and post-processing

Th-228 decays are simulated with the GPS ion generator confined to the source
volume. In the post-processing step:

- a piecewise-linear activeness model is applied using the distance of each
  energy deposition to the n+ contact surface, with FCCD = 1.3 mm and a
  dead-layer fraction (DLF) of 0.5;
- a Gaussian energy resolution smearing is applied with FWHM = 2.5 √(E / 2039
  keV) keV;
- variable-width binning (wider around gamma-line peaks) is used so that the
  comparison is not sensitive to the detailed peak shape.

The simulated spectrum is normalised by the ratio of expected source decays to
simulated primary events. The active volume model and energy resolution are not
the focus of this comparison and are only applied to obtain a realistic-looking
spectrum.

## Results

```{figure} ./_img/gespectrum/hades-spectrum-full.output.png
:width: 800px
*Energy spectrum of a Th-228 source recorded in a germanium detector (log scale).*
```

```{figure} ./_img/gespectrum/hades-spectrum-linear.output.png
:width: 800px
*Energy spectrum of a Th-228 source recorded in a germanium detector (linear scale).*
```

Simulation and analysis scripts are available in
[`tests/gespectrum`](https://github.com/legend-exp/remage/tree/main/tests/gespectrum).

## References

[^HEROICA]:
    E. Andreotti et al., HEROICA: an Underground Facility for the Fast Screening
    of Germanium Detectors, JINST 8 (2013) P06012,
    [10.1088/1748-0221/8/06/P06012](https://doi.org/10.1088/1748-0221/8/06/P06012).

[^BEGeChar]:
    M. Agostini et al. (GERDA Collaboration), Characterization of 30 ⁷⁶Ge
    enriched Broad Energy Ge detectors for GERDA Phase II, Eur. Phys. J. C 79,
    978 (2019),
    [10.1140/epjc/s10052-019-7353-8](https://doi.org/10.1140/epjc/s10052-019-7353-8).
