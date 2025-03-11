# Germanium spectra

This test is modeled after a real-world example of a characterization test stand
used for LEGEND (HADES). It simulates a BEGe detector with a very simple
geometry next to a Th-228 source and copares the resulting spectrum with
experimental data. The test uses a simplified geometry of the chosen BEGe
detector.

The data is scaled by the expected source activity at the point of measurement,
and to match the simulated decay count. The efficiency of the applied quality
cuts is accounted for.

In the simulation post-processing, an piecewise activeness model and a variable
energy resolution is applied. The binning of the spectrum is widened around the
gamma line peaks, so that we do not compare the detailed peak shape.

A good agreement for higher energies is found:

```{figure} ./_img/gespectrum/hades-spectrum.output.png
:width: 800px
*Energy spectrum of a Th-228 source recorded in a germanium detector.*
```

In the lower-energy spectrum (<200 keV), some minor deviations can be observed:

```{figure} ./_img/gespectrum/hades-spectrum-full.output.png
:width: 800px
*Energy spectrum of a Th-228 source recorded in a germanium detector (full range).*
```

---

```{figure} ./_img/gespectrum/hades-spectrum-linear.output.png
:width: 800px
*Energy spectrum of a Th-228 source recorded in a germanium detector (linear scale).*
```
