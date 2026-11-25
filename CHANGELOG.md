# Changelog

## v1.2.1 (2026-11-03)

### What's Changed
* feat(confinement): report sampled volume and mass at end of run by @gipert in https://github.com/legend-exp/remage/pull/771
* fix(bxdecay0): convert double beta decay processes to BxDecay0 modes by @gipert in https://github.com/legend-exp/remage/pull/773


**Full Changelog**: https://github.com/legend-exp/remage/compare/v1.2.0...v1.2.1

## v1.2.0 (2026-10-15)

### What's Changed
* also read rmg_color from GDML by @ManuelHu in https://github.com/legend-exp/remage/pull/758
* fix MUSUN reading in multi-process mode by @ManuelHu in https://github.com/legend-exp/remage/pull/760
* add a python implementation for the C++ FNV-1a function by @ManuelHu in https://github.com/legend-exp/remage/pull/761
* fix tests for Geant4 11.4.3 by @ManuelHu in https://github.com/legend-exp/remage/pull/764
* warn for int32 overflow in event ids by @ManuelHu in https://github.com/legend-exp/remage/pull/766


**Full Changelog**: https://github.com/legend-exp/remage/compare/v1.1.0...v1.2.0

## v1.1.0 (2026-09-18)

### What's Changed
* chore: synced file(s) with legend-exp/legend-swdev-standards by @gipert in https://github.com/legend-exp/remage/pull/751
* docs: interactive UI tip, figures from the paper, pixi showcase by @gipert in https://github.com/legend-exp/remage/pull/753
* chore: post-process the validation tests with a python script by @gipert in https://github.com/legend-exp/remage/pull/754
* fix: check also that sys.stderr has the isatty() method by @gipert in https://github.com/legend-exp/remage/pull/756
* Allow storing multiple simulation outputs in a single LH5 file by @gipert in https://github.com/legend-exp/remage/pull/757


**Full Changelog**: https://github.com/legend-exp/remage/compare/v1.0.0...v1.1.0

## v1.0.0 (2026-09-04)

> [!IMPORTANT]
> This release does not introduce any major new features. We believe remage is now stable and feature-complete, with all the features we originally planned for v1 implemented. We therefore think it is time to tag v1!

### What's Changed
* Add custom staging with option to spill to disk by @EricMEsch in https://github.com/legend-exp/remage/pull/723
* innerbrem: conserve energy by @ManuelHu in https://github.com/legend-exp/remage/pull/714

### Build, testing and documentation changes
* ci: remove pull_request trigger for now [skip ci] by @ManuelHu in https://github.com/legend-exp/remage/pull/740
* ci: remove old docker build hook by @ManuelHu in https://github.com/legend-exp/remage/pull/741
* adjust for new docker baseimage by @ManuelHu in https://github.com/legend-exp/remage/pull/742
* python: adjust geombench gdml handling for python 3.14 by @ManuelHu in https://github.com/legend-exp/remage/pull/743
* make clang tidy happy by @ManuelHu in https://github.com/legend-exp/remage/pull/744
* docs: fix validation docs for staging by @ManuelHu in https://github.com/legend-exp/remage/pull/745
* Confinement and clustering plots by @ManuelHu in https://github.com/legend-exp/remage/pull/732
* cmake: write version/config header only to build dir by @ManuelHu in https://github.com/legend-exp/remage/pull/707
* minor build modernization by @ManuelHu in https://github.com/legend-exp/remage/pull/746
* ci: install zlib and expat from conda-forge for macOS by @ManuelHu in https://github.com/legend-exp/remage/pull/748
* docs: align better with the paper draft by @ManuelHu in https://github.com/legend-exp/remage/pull/747
* tests: update observables validation plots by @ManuelHu in https://github.com/legend-exp/remage/pull/749


**Full Changelog**: https://github.com/legend-exp/remage/compare/v0.26.0...v1.0.0

## v0.26.0 (2026-08-25)

### Physics fixes / new features
* chore: **relicense from LGPL-3.0 to GPL-3.0-or-later** by @gipert in https://github.com/legend-exp/remage/pull/734
* Implementing gamma cascade filelist (see #698) by @MoritzNeuberger in https://github.com/legend-exp/remage/pull/717
* warn for broken neutron cross section data sets by @ManuelHu in https://github.com/legend-exp/remage/pull/725
* Fix G4.11.4 bug with scintillation from photons by @ManuelHu in https://github.com/legend-exp/remage/pull/728
* print calculated cuts by @ManuelHu in https://github.com/legend-exp/remage/pull/736
* remove some allocations in logging calls by @ManuelHu in https://github.com/legend-exp/remage/pull/729

### Testing and documentation changes
* rtd: disable HTTPS verification for downloading tag file by @ManuelHu in https://github.com/legend-exp/remage/pull/727
* tests: add validation for production cuts by @ManuelHu in https://github.com/legend-exp/remage/pull/721
* docs: fix formatting by @ManuelHu in https://github.com/legend-exp/remage/pull/731
* docs: clarify event vertex and kinematics input by @gipert in https://github.com/legend-exp/remage/pull/730
* ci: update build-and-inspect-python-package action by @ManuelHu in https://github.com/legend-exp/remage/pull/733
* ci: add docker hub build workflow by @ManuelHu in https://github.com/legend-exp/remage/pull/739


**Full Changelog**: https://github.com/legend-exp/remage/compare/v0.25.0...v0.26.0

## v0.25.0 (2026-07-17)

> [!IMPORTANT]
> this release contains an important fix for any users that specify a user-defined seed in multi-process mode. Until now, the same seed was used in all processes!

### What's Changed
* remove reboost post-processing by @ManuelHu in https://github.com/legend-exp/remage/pull/674
* clang-tidy & LH5 compression fixes after calorimeter and post-proc changes by @ManuelHu in https://github.com/legend-exp/remage/pull/687
* calorimeter output scheme: store floats as float32 by @gipert in https://github.com/legend-exp/remage/pull/688
* Surface sampling depth by @tdixon97 in https://github.com/legend-exp/remage/pull/643
* lh5: fix some inconsistencies by @ManuelHu in https://github.com/legend-exp/remage/pull/696
* Misc minor fixes by @ManuelHu in https://github.com/legend-exp/remage/pull/702
* multiple fixes for generators by @ManuelHu in https://github.com/legend-exp/remage/pull/699
* many python fixes by @ManuelHu in https://github.com/legend-exp/remage/pull/701
* python: only propagate aborted processes, not normal errors by @ManuelHu in https://github.com/legend-exp/remage/pull/704
* gps: remove our own GPS lock by @ManuelHu in https://github.com/legend-exp/remage/pull/697
* multiple test fixes by @ManuelHu in https://github.com/legend-exp/remage/pull/703
* packaging fixes by @ManuelHu in https://github.com/legend-exp/remage/pull/706
* simplify gamma cascade reader by @ManuelHu in https://github.com/legend-exp/remage/pull/705
* nCapture: fix kinetic energy and mass of secondaries by @ManuelHu in https://github.com/legend-exp/remage/pull/708
* some core fixes by @ManuelHu in https://github.com/legend-exp/remage/pull/709
* docker: fix build & clone main branch of remage only by @ManuelHu in https://github.com/legend-exp/remage/pull/712
* technical fixes for inner bremsstrahlung by @ManuelHu in https://github.com/legend-exp/remage/pull/711
* output fixes by @ManuelHu in https://github.com/legend-exp/remage/pull/713
* early exit in logging for higher levels by @ManuelHu in https://github.com/legend-exp/remage/pull/715
* lh5: do not attempt to convert scalar vtx entries by @ManuelHu in https://github.com/legend-exp/remage/pull/716
* use different seeds for multi-process mode when user-seed is set by @ManuelHu in https://github.com/legend-exp/remage/pull/720

**Full Changelog**: https://github.com/legend-exp/remage/compare/v0.24.0...v0.25.0

## v0.24.0 (2026-06-24)

### What's Changed
* add Calorimeter detector and output scheme by @gipert in https://github.com/legend-exp/remage/pull/479
* input: add combined vtx/kin table support by @ManuelHu in https://github.com/legend-exp/remage/pull/682
* Unified approach to staging of particles (previously: "feat: optimization of the electron stacking") by @MoritzNeuberger in https://github.com/legend-exp/remage/pull/654
* Revise arm64 Mac installation instructions by @ManuelHu in https://github.com/legend-exp/remage/pull/686


**Full Changelog**: https://github.com/legend-exp/remage/compare/v0.23.2...v0.24.0

## v0.23.2 (2026-06-10)

this release fixes only a docker hub build issue.


**Full Changelog**: https://github.com/legend-exp/remage/compare/v0.23.1...v0.23.2

## v0.23.1 (2026-06-09)

### Minor changes
* python: allow to use the python wrapper with a custom remage-cpp by @ManuelHu in https://github.com/legend-exp/remage/pull/678
* add better C++ extension points by @ManuelHu in https://github.com/legend-exp/remage/pull/679

### Documentation and validation changes
* ci: add skip conditions for validation by @ManuelHu in https://github.com/legend-exp/remage/pull/668
* Add muon physics tests by @MoritzNeuberger in https://github.com/legend-exp/remage/pull/638
* tests: use tolerance in more surface confinement comparisons by @ManuelHu in https://github.com/legend-exp/remage/pull/670
* extend docstrings for public C++ API by @ManuelHu in https://github.com/legend-exp/remage/pull/673
* Improve gespectrum validation page and plots by @gipert in https://github.com/legend-exp/remage/pull/675


**Full Changelog**: https://github.com/legend-exp/remage/compare/v0.23.0...v0.23.1

## v0.23.0 (2026-05-09)

### (minor) breaking changes
* update to lgdo v2 and lh5io by @ManuelHu in https://github.com/legend-exp/remage/pull/657
* drop support for python 3.10 by @ManuelHu in https://github.com/legend-exp/remage/pull/664
* rename number_of_events to number_of_simulated_events by @ManuelHu in https://github.com/legend-exp/remage/pull/667

### What's Changed
* Remove the zero energy skip in ScintillatorDetector::ProcessHits by @EricMEsch in https://github.com/legend-exp/remage/pull/642
* activate the Vertex scheme over the optional output scheme mechanism by @ManuelHu in https://github.com/legend-exp/remage/pull/661
* Update reboost dependency version to 0.11.0 by @ManuelHu in https://github.com/legend-exp/remage/pull/663
* fix gps vertex position assignment by @ManuelHu in https://github.com/legend-exp/remage/pull/665

### Documentation and validation changes
* Add mypy type checks by @ManuelHu in https://github.com/legend-exp/remage/pull/651
* tests: fix confinement-mt tests by @ManuelHu in https://github.com/legend-exp/remage/pull/649
* add hades spectrum test by @ManuelHu in https://github.com/legend-exp/remage/pull/655
* Add neutron validation runs by @EricMEsch in https://github.com/legend-exp/remage/pull/599
* docs: update rtd base image [skip ci] by @ManuelHu in https://github.com/legend-exp/remage/pull/666
* tests: fix vertex tests for odd number of cpus by @ManuelHu in https://github.com/legend-exp/remage/pull/669


**Full Changelog**: https://github.com/legend-exp/remage/compare/v0.22.0...v0.23.0

## v0.22.0 (2026-04-06)

### Important Bug Fix
* use global transformation for distance to surface by @ManuelHu in https://github.com/legend-exp/remage/pull/645

### What's Changed
* Add sphere support ECoMug by @tdixon97 in https://github.com/legend-exp/remage/pull/623
* optics: switch WLS time profile to exponential by @ManuelHu in https://github.com/legend-exp/remage/pull/635
* output: store number of events in LH5 file by @ManuelHu in https://github.com/legend-exp/remage/pull/629

### Docs and Validation Changes
* remove or move examples by @ManuelHu in https://github.com/legend-exp/remage/pull/636
* ci: do not set RMG_BUILD_EXAMPLES by @ManuelHu in https://github.com/legend-exp/remage/pull/637
* tests: update nist and ge validation by @ManuelHu in https://github.com/legend-exp/remage/pull/634


**Full Changelog**: https://github.com/legend-exp/remage/compare/v0.21.0...v0.22.0

## v0.21.0 (2026-02-22)

### What's Changed
* update reboost to v0.10.0 by @tdixon97 in https://github.com/legend-exp/remage/pull/592
* confinement: add option to weigh by mass instead of volume by @ManuelHu in https://github.com/legend-exp/remage/pull/519
* keep units in detector_origins by @ManuelHu in https://github.com/legend-exp/remage/pull/616
* output: add option to store optical photons in track output by @ManuelHu in https://github.com/legend-exp/remage/pull/619
* output: keep vtx.evtid even if vertex writing is disabled by @ManuelHu in https://github.com/legend-exp/remage/pull/626


### Documentation and validation updates
* docs: remove docs landing page warning by @gipert in https://github.com/legend-exp/remage/pull/612
* docs: region defintion in GDML by @ManuelHu in https://github.com/legend-exp/remage/pull/614
* docs: update for output changes (photons in tracks & units in detector_origins) by @gipert in https://github.com/legend-exp/remage/pull/618
* validation infrastructure and updates by @ManuelHu in https://github.com/legend-exp/remage/pull/620
* gamma angular correlation validation by @gipert in https://github.com/legend-exp/remage/pull/435
* validation: optical validation by @ManuelHu in https://github.com/legend-exp/remage/pull/624


**Full Changelog**: https://github.com/legend-exp/remage/compare/v0.20.0...v0.21.0

## v0.20.0 (2026-01-29)

> [!IMPORTANT]
> this release contains a minor breaking change for macros that apply an energy cut on _Scintillator_ detector hits. The handling of the lower bound has been updated to match the handliung for Germanium detectors: https://github.com/legend-exp/remage/pull/603

### What's Changed
* various documentation and tutorial improvements by @ManuelHu in https://github.com/legend-exp/remage/pull/594, https://github.com/legend-exp/remage/pull/567, https://github.com/legend-exp/remage/pull/602, https://github.com/legend-exp/remage/pull/595, https://github.com/legend-exp/remage/pull/604
* improve our CI setup for the validation suite by @ManuelHu
* Navigation performance benchmark by @MoritzNeuberger in https://github.com/legend-exp/remage/pull/530
* align exit behaviour for multi processing mode by @ManuelHu in https://github.com/legend-exp/remage/pull/580
* do not show warning for no polarization of photons in GPS by @ManuelHu in https://github.com/legend-exp/remage/pull/589
* Add combine_tracks validation by @EricMEsch in https://github.com/legend-exp/remage/pull/585
* align scintillator edep cut with germanium by @ManuelHu in https://github.com/legend-exp/remage/pull/603
* RMGAnalysisReader: store temporary files in output directory by @ManuelHu in https://github.com/legend-exp/remage/pull/606
* support pyg4ometry registries in remage_run by @ManuelHu in https://github.com/legend-exp/remage/pull/610
* rework combine_low_energy_tracks by @EricMEsch in https://github.com/legend-exp/remage/pull/607

**Full Changelog**: https://github.com/legend-exp/remage/compare/v0.19.5...v0.20.0

## v0.19.5 (2026-01-14)

<!-- Release notes generated using configuration in .github/release.yml at releases/v0.19 -->

### What's Changed
* ipc: remove timeout and add flag for disabling xml check by @ManuelHu in https://github.com/legend-exp/remage/pull/586


**Full Changelog**: https://github.com/legend-exp/remage/compare/v0.19.4...v0.19.5

## v0.19.4 (2026-01-14)

<!-- Release notes generated using configuration in .github/release.yml at releases/v0.19 -->

### What's Changed
* ipc: add better debugging facilities by @ManuelHu in https://github.com/legend-exp/remage/pull/583


**Full Changelog**: https://github.com/legend-exp/remage/compare/v0.19.3...v0.19.4

## v0.19.3 (2026-01-07)

### What's Changed
* G4 11.4 compatibility and test stable container version by @gipert in https://github.com/legend-exp/remage/pull/572
* ipc: pause all processes while handling blocking ipc from multiprocess mode by @ManuelHu in https://github.com/legend-exp/remage/pull/575


**Full Changelog**: https://github.com/legend-exp/remage/compare/v0.19.2...v0.19.3

## v0.19.2 (2025-12-24)

### What's Changed
* fix: python wrapper log messages by @gipert in https://github.com/legend-exp/remage/pull/570


**Full Changelog**: https://github.com/legend-exp/remage/compare/v0.19.1...v0.19.2

## v0.19.1 (2025-12-23)

### What's Changed
* hotfix: only check gdml in the first process in multi-processing by @ManuelHu in https://github.com/legend-exp/remage/pull/569


**Full Changelog**: https://github.com/legend-exp/remage/compare/v0.19.0...v0.19.1

## v0.19.0 (2025-12-14)

### What's Changed
* ipc: also ignore BrokenPipeError (errno 32) by @ManuelHu in https://github.com/legend-exp/remage/pull/559
* add license headers to python files by @ManuelHu in https://github.com/legend-exp/remage/pull/561
* fix debug_event log level by @ManuelHu in https://github.com/legend-exp/remage/pull/562
* read multiple particles from input file by @ManuelHu in https://github.com/legend-exp/remage/pull/560
* docs: Update extend.md for the OutputManager refactoring by @ManuelHu in https://github.com/legend-exp/remage/pull/564
* docs: use new reference in confinement.md by @ManuelHu in https://github.com/legend-exp/remage/pull/565
* docs: update for new detector registration by @ManuelHu in https://github.com/legend-exp/remage/pull/566


**Full Changelog**: https://github.com/legend-exp/remage/compare/v0.18.0...v0.19.0

## v0.18.0 (2025-12-08)

> [!IMPORTANT]
> this release is _required_ to use geometries created with the recent pygeomtools versions >= 0.0.26

### What's Changed
* remage_run() inputs improvements by @gipert in https://github.com/legend-exp/remage/pull/556
* add parameter to set ntuple name for detector by @ManuelHu in https://github.com/legend-exp/remage/pull/555
* use newer pygeomtools in tests [skip ci] by @ManuelHu in https://github.com/legend-exp/remage/pull/557
* Implement inner bremsstrahlung by @311Francis in https://github.com/legend-exp/remage/pull/475

### New Contributors
* @311Francis made their first contribution in https://github.com/legend-exp/remage/pull/475

**Full Changelog**: https://github.com/legend-exp/remage/compare/v0.17.0...v0.18.0

## v0.17.0 (2025-11-27)

> [!IMPORTANT]
> due to an error, correlated gamma emission never was properly enabled in the versions 0.13–0.16. The automatic activation has been fixed in this version (the activation wih the macro command always worked as expected).

### What's Changed
* Explicitely link against BxDecay0 (not only the Geant4 plugin) by @ManuelHu in https://github.com/legend-exp/remage/pull/542
* tests: print more progress in run_e_sims.py by @ManuelHu in https://github.com/legend-exp/remage/pull/543
* Include process number in log message prefix by @gipert in https://github.com/legend-exp/remage/pull/544
* use newly renamed packages by @ManuelHu in https://github.com/legend-exp/remage/pull/545
* ci: add bxdecay0 for macOS ci by @ManuelHu in https://github.com/legend-exp/remage/pull/547
* IPC and log fixes by @ManuelHu in https://github.com/legend-exp/remage/pull/548
* python: update metadata by @ManuelHu in https://github.com/legend-exp/remage/pull/549
* docs: fix console block for conda installation by @ManuelHu in https://github.com/legend-exp/remage/pull/550
* actually enable correlated gamma emission by @ManuelHu in https://github.com/legend-exp/remage/pull/551


**Full Changelog**: https://github.com/legend-exp/remage/compare/v0.16.0...v0.17.0

## v0.16.0 (2025-11-20)

### Highlights

* New parallel mode: `--procs <N_PROCS>` runs several independent remage instances in parallel, providing a 1:1 scaling of performance with number of CPU cores (at the cost of higher memory consumption). In https://github.com/legend-exp/remage/pull/522
* Our custom WLS process (either 0 or 1 wavelength-shifted photons emitted) is now on by default. In https://github.com/legend-exp/remage/pull/537

More details on our documentation pages.

### Detailed changes
* docs: include docs on optical output by @ManuelHu in https://github.com/legend-exp/remage/pull/513
* [ci skip] Fix log message for SurfaceSampleMaxIntersections by @gipert in https://github.com/legend-exp/remage/pull/516
* docs: add conda-forge to install docs by @ManuelHu in https://github.com/legend-exp/remage/pull/517
* ci: use the conda-provided python for remage building on macOS by @ManuelHu in https://github.com/legend-exp/remage/pull/521
* tests: mark basicfilter as flaky by @ManuelHu in https://github.com/legend-exp/remage/pull/526
* ci: fix validation temporarily by @ManuelHu in https://github.com/legend-exp/remage/pull/527
* reduce RMGConvertLH5 verbosity by @ManuelHu in https://github.com/legend-exp/remage/pull/528
* add option for multi-processing by @ManuelHu in https://github.com/legend-exp/remage/pull/522
* ci: move self-hosted runner workflow parts to a dedicated file by @gipert in https://github.com/legend-exp/remage/pull/533
* remove upper version bounds on LEGEND packages by @gipert in https://github.com/legend-exp/remage/pull/531
* vertex: fix for multiprocessing mode by @ManuelHu in https://github.com/legend-exp/remage/pull/536
* document parallel mode in user manual by @gipert in https://github.com/legend-exp/remage/pull/538
* add defaults to docs and enable WLS process by @ManuelHu in https://github.com/legend-exp/remage/pull/537


**Full Changelog**: https://github.com/legend-exp/remage/compare/v0.15.2...v0.16.0

## v0.15.2 (2025-10-16)

### What's Changed
* check for xml-valid gdml file in python by @ManuelHu in https://github.com/legend-exp/remage/pull/495
* cli: add --ignore-warnings flag by @ManuelHu in https://github.com/legend-exp/remage/pull/498
* Allow for setting the random seed via CLI by @gipert in https://github.com/legend-exp/remage/pull/502
* ci: use python 3.13 on macOS for now by @ManuelHu in https://github.com/legend-exp/remage/pull/507
* Minor update in main docs page by @gipert in https://github.com/legend-exp/remage/pull/508
* docs: clarify support of output file types by @ManuelHu in https://github.com/legend-exp/remage/pull/509
* bump lgdo version by @ManuelHu in https://github.com/legend-exp/remage/pull/503
* Add geantino geometry check by @ManuelHu in https://github.com/legend-exp/remage/pull/506
* cmake: fix testing with `RMG_CONDA_BUILD` by @ManuelHu in https://github.com/legend-exp/remage/pull/510
* tests: introduce a new `flaky` label for confinement tests by @ManuelHu in https://github.com/legend-exp/remage/pull/511
* remove some unneeded test dependencies by @ManuelHu in https://github.com/legend-exp/remage/pull/512
* cmake: fix tests with `RMG_CONDA_BUILD`, part 2 by @ManuelHu in https://github.com/legend-exp/remage/pull/514
* appease pre-commit by @ManuelHu in https://github.com/legend-exp/remage/pull/515


**Full Changelog**: https://github.com/legend-exp/remage/compare/v0.15.1...v0.15.2

## v0.15.1 (2025-09-28)

<!-- Release notes generated using configuration in .github/release.yml at main -->

### What's Changed
* confinement: fix sampling from volumes with daughters by @ManuelHu in https://github.com/legend-exp/remage/pull/494


**Full Changelog**: https://github.com/legend-exp/remage/compare/v0.15.0...v0.15.1

## v0.15.0 (2025-09-24)

<!-- Release notes generated using configuration in .github/release.yml at main -->

### What's Changed
* ci: macOS-latest is now macOS 15 by @ManuelHu in https://github.com/legend-exp/remage/pull/491
* docker: switch to latest base image (Geant4 11.3) by @ManuelHu in https://github.com/legend-exp/remage/pull/492


**Full Changelog**: https://github.com/legend-exp/remage/compare/v0.14.4...v0.15.0

## v0.14.4 (2025-09-17)

<!-- Release notes generated using configuration in .github/release.yml at main -->

### What's Changed
* remove deprecated localtime calls by @ManuelHu in https://github.com/legend-exp/remage/pull/487
* restore time zone capabilities with localtime_r by @ManuelHu in https://github.com/legend-exp/remage/pull/488


**Full Changelog**: https://github.com/legend-exp/remage/compare/v0.14.3...v0.14.4

## v0.14.3 (2025-09-16)

<!-- Release notes generated using configuration in .github/release.yml at main -->

### What's Changed
* cmake: align CLI11 package name by @ManuelHu in https://github.com/legend-exp/remage/pull/486


**Full Changelog**: https://github.com/legend-exp/remage/compare/v0.14.2...v0.14.3

## v0.14.2 (2025-09-16)

<!-- Release notes generated using configuration in .github/release.yml at main -->

**only for conda-forge preparation**

### What's Changed
* cmake: prepare for using system-provided fmt by @ManuelHu in https://github.com/legend-exp/remage/pull/485


**Full Changelog**: https://github.com/legend-exp/remage/compare/v0.14.1...v0.14.2

## v0.14.1 (2025-09-16)

<!-- Release notes generated using configuration in .github/release.yml at main -->

**only for conda-forge preparation**

### What's Changed
* resolve legend-pydataobj version conflict by @ManuelHu in https://github.com/legend-exp/remage/pull/482
* cmake: add option for conda build by @ManuelHu in https://github.com/legend-exp/remage/pull/484


**Full Changelog**: https://github.com/legend-exp/remage/compare/v0.14.0...v0.14.1

## v0.14.0 (2025-09-15)

<!-- Release notes generated using configuration in .github/release.yml at main -->

### What's Changed
* Add unit tests and validation plots for bxdecay by @EricMEsch in https://github.com/legend-exp/remage/pull/457
* Get rid of the `tol_colors` package dependency by @gipert in https://github.com/legend-exp/remage/pull/463
* Enable the self-hosted runner by @gipert in https://github.com/legend-exp/remage/pull/464
* initial changes for C++20 by @ManuelHu in https://github.com/legend-exp/remage/pull/447
* add regex support for copy_nr in RegisterDetector by @ManuelHu in https://github.com/legend-exp/remage/pull/466
* cmake: force minimum c++ version by @ManuelHu in https://github.com/legend-exp/remage/pull/473
* drop python 3.9 support by @ManuelHu in https://github.com/legend-exp/remage/pull/478
* refactor out python package installation from cmake by @ManuelHu in https://github.com/legend-exp/remage/pull/481


**Full Changelog**: https://github.com/legend-exp/remage/compare/v0.13.1...v0.14.0

## v0.13.1 (2025-08-12)

<!-- Release notes generated using configuration in .github/release.yml at main -->

### What's Changed
* Fix typos in analysis.md by @gipert in https://github.com/legend-exp/remage/pull/458


**Full Changelog**: https://github.com/legend-exp/remage/compare/v0.13.0...v0.13.1

## v0.13.0 (2025-08-08)

<!-- Release notes generated using configuration in .github/release.yml at main -->

### What's Changed
* store detector global coordinates, deduplicate auxiliary tables and convert to LH5 structs by @ManuelHu in https://github.com/legend-exp/remage/pull/441
* improve and add more docs by @gipert in https://github.com/legend-exp/remage/pull/451
* Enable gamma angular correlations by default by @gipert in https://github.com/legend-exp/remage/pull/454
* Add regex support for `/RMG/Geometry/RegisterDetector` by @EricMEsch in https://github.com/legend-exp/remage/pull/455


**Full Changelog**: https://github.com/legend-exp/remage/compare/v0.12.2...v0.13.0

## v0.12.2 (2025-07-28)

<!-- Release notes generated using configuration in .github/release.yml at main -->

### What's Changed
* ci: hopefully fix intermittent CI failures by @ManuelHu in https://github.com/legend-exp/remage/pull/449
* python: use same logic as C++ to decide color log support by @ManuelHu in https://github.com/legend-exp/remage/pull/448
* Fix an error occuring when setting the vertexgen late with bxdecay0 by @EricMEsch in https://github.com/legend-exp/remage/pull/446
* Allow the TrackOutput to be discarded. by @EricMEsch in https://github.com/legend-exp/remage/pull/445


**Full Changelog**: https://github.com/legend-exp/remage/compare/v0.12.1...v0.12.2

## v0.12.1 (2025-07-17)

<!-- Release notes generated using configuration in .github/release.yml at main -->

### What's Changed
* Fixes to output docs in user manual by @EricMEsch in https://github.com/legend-exp/remage/pull/440
* lh5: sort column and field names by @ManuelHu in https://github.com/legend-exp/remage/pull/438
* fix link copying in LH5 output when merging flat output by @ManuelHu in https://github.com/legend-exp/remage/pull/443
* cmake: do not reinstall development packages by @ManuelHu in https://github.com/legend-exp/remage/pull/444


**Full Changelog**: https://github.com/legend-exp/remage/compare/v0.12.0...v0.12.1

## v0.12.0 (2025-07-11)

<!-- Release notes generated using configuration in .github/release.yml at main -->

### Notable user-facing and breaking changes
* _remage_ now creates a TCM for simulation output:
    * see [docs on TCM output](https://remage.readthedocs.io/en/stable/manual/output.html#time-coincidence-map)
    * Compute a TCM in the wrapper and store it to disk by @gipert in https://github.com/legend-exp/remage/pull/408
    * Add `det<UID>` symlinks to the output and make the TCM use them by @gipert in https://github.com/legend-exp/remage/pull/426
* The output structure changed. **tables were renamed and moved in the file structure**:
    * see [output docs](https://remage.readthedocs.io/en/stable/manual/output.html#the-vertex-table)
    * Rename vertex ntuple to vtx by @gipert in https://github.com/legend-exp/remage/pull/398
    * consistently handle non-stepping data tables (aux tables) by @gipert in https://github.com/legend-exp/remage/pull/418
* Wrap 2vbb and 0vbb in remage by @EricMEsch in https://github.com/legend-exp/remage/pull/390
  see [docs on BxDecay0 integration](https://remage.readthedocs.io/en/stable/manual/generators.html#double-beta-decay-physics)
* Modify macro handling in RMGManager by @gipert in https://github.com/legend-exp/remage/pull/400
  see [docs on batch mode](https://remage.readthedocs.io/en/stable/manual/running.html#executing-commands-in-batch-mode)
* updates to optical output:
    * Include optical detector output in Python wrapper post-processing by @gipert in https://github.com/legend-exp/remage/pull/425
    * add macro command to set optical storage precision by @ManuelHu in https://github.com/legend-exp/remage/pull/431

### Other changes
* use a FNV-1a hash to compute process indices by @ManuelHu in https://github.com/legend-exp/remage/pull/379
* python: move postproc code to own module by @ManuelHu in https://github.com/legend-exp/remage/pull/381
* Add extend docs by @EricMEsch in https://github.com/legend-exp/remage/pull/383
* clang-tidy: fix most new errors on ubuntu noble by @ManuelHu in https://github.com/legend-exp/remage/pull/391
* Some small fixes for simflow by @tdixon97 in https://github.com/legend-exp/remage/pull/393
* Fix error message in remage finder by @gipert in https://github.com/legend-exp/remage/pull/397
* add process filter to particlefilter by @ManuelHu in https://github.com/legend-exp/remage/pull/380
* Update reboost version to have correct units by @tdixon97 in https://github.com/legend-exp/remage/pull/384
* Add BxDecay0 docs to the user manual by @EricMEsch in https://github.com/legend-exp/remage/pull/396
* Update copyright headers by @gipert in https://github.com/legend-exp/remage/pull/404
* Reshape and merge output files in one go by @gipert in https://github.com/legend-exp/remage/pull/399
* Add docstrings to IPC helper functions by @gipert in https://github.com/legend-exp/remage/pull/412
* Update README.md by @tdixon97 in https://github.com/legend-exp/remage/pull/417
* Add some more docs by @gipert in https://github.com/legend-exp/remage/pull/419
* skip unit tests in Dockerfile by @gipert in https://github.com/legend-exp/remage/pull/430
* Clang tidy fixes by @ManuelHu in https://github.com/legend-exp/remage/pull/432
* ci: split mt and non-mt test runs by @ManuelHu in https://github.com/legend-exp/remage/pull/434
* ci: remove Xfvb setup by @ManuelHu in https://github.com/legend-exp/remage/pull/436


**Full Changelog**: https://github.com/legend-exp/remage/compare/v0.11.0...v0.12.0

## v0.11.0 (2025-05-09)

## Highlights

- Pre-clustering of G4 stepping data from sensitive detectors #331
- Removed physics "realms" functionality #346
- LH5 output post processing: reshaping of LH5 tables into "hit" oriented (jagged) layout and merging of LH5 files #341 #368 #370
- Surface sampling on volumes with daughters is now possible #339
- More documentation and validation suite plots!
- the `gipert/remage-base` Docker image has moved to `legendexp/remage-base` #363

### What's Changed
* change tutorial to use some pygeom-tools functions by @tdixon97 in https://github.com/legend-exp/remage/pull/316
* python: add more infrastructure for post-processing by @ManuelHu in https://github.com/legend-exp/remage/pull/314
* python: also install a wheel file by @ManuelHu in https://github.com/legend-exp/remage/pull/315
* chore: update pre-commit hooks by @pre-commit-ci in https://github.com/legend-exp/remage/pull/321
* ipc: send remage-cpp version to python side by @ManuelHu in https://github.com/legend-exp/remage/pull/323
* Adding observable plots to the validation report by @tdixon97 in https://github.com/legend-exp/remage/pull/312
* python: add kwargs to remage_run by @ManuelHu in https://github.com/legend-exp/remage/pull/325
* Fix to basic tutorial? by @EricMEsch in https://github.com/legend-exp/remage/pull/328
* test: fix with new reboost version by @ManuelHu in https://github.com/legend-exp/remage/pull/327
* gdml: add command to load detectors from GDML by @ManuelHu in https://github.com/legend-exp/remage/pull/320
* claim some copyright for myself by @ManuelHu in https://github.com/legend-exp/remage/pull/329
* docs: docker hub links by @ManuelHu in https://github.com/legend-exp/remage/pull/330
* rename FilterOutputSchemes to FilterSchemes by @EricMEsch in https://github.com/legend-exp/remage/pull/335
* Populate user manual by @gipert in https://github.com/legend-exp/remage/pull/334
* Allow surface sampling on volumes with daughters by @tdixon97 in https://github.com/legend-exp/remage/pull/339
* python: add find_remage module by @ManuelHu in https://github.com/legend-exp/remage/pull/337
* send detector registration over ipc by @ManuelHu in https://github.com/legend-exp/remage/pull/342
* Send table names over IPC by @ManuelHu in https://github.com/legend-exp/remage/pull/343
* Merge threads and reshape by @tdixon97 in https://github.com/legend-exp/remage/pull/341
* populate numba cache at docker container build time by @gipert in https://github.com/legend-exp/remage/pull/345
* standardize macro commands by @gipert in https://github.com/legend-exp/remage/pull/344
* Remove RMG_HAS_GEANT4_11_*_OR_LATER constants by @ManuelHu in https://github.com/legend-exp/remage/pull/347
* preclustering of the remage outputs and refactor of output schemes / detectors by @tdixon97 in https://github.com/legend-exp/remage/pull/331
* update validation suite for surface sampling for volumes with daughters by @tdixon97 in https://github.com/legend-exp/remage/pull/340
* use DYLD_FALLBACK_LIBRARY_PATH for macOS by @ManuelHu in https://github.com/legend-exp/remage/pull/348
* add some basic docs for generators by @tdixon97 in https://github.com/legend-exp/remage/pull/349
* make clang-tidy happy again by @ManuelHu in https://github.com/legend-exp/remage/pull/351
* docs: also build docs for python wrapper by @ManuelHu in https://github.com/legend-exp/remage/pull/354
* ci: also run extra tests on macOS by @ManuelHu in https://github.com/legend-exp/remage/pull/355
* cmake: fix docs build by @ManuelHu in https://github.com/legend-exp/remage/pull/356
* Improve clang-format configuration by @gipert in https://github.com/legend-exp/remage/pull/353
* remove realms functionality by @gipert in https://github.com/legend-exp/remage/pull/346
* docs: revert a accidental wrong change and fix it properly by @ManuelHu in https://github.com/legend-exp/remage/pull/358
* chore: update pre-commit hooks by @pre-commit-ci in https://github.com/legend-exp/remage/pull/361
* output: do not create root file by default by @ManuelHu in https://github.com/legend-exp/remage/pull/359
* gipert/remage-base has moved to legendexp/remage-base by @gipert in https://github.com/legend-exp/remage/pull/363
* Fixes to LH5 post-processing code by @gipert in https://github.com/legend-exp/remage/pull/362
* Add jupyter install guide by @EricMEsch in https://github.com/legend-exp/remage/pull/332
* python: cannot merge/reshape for non-LH5 formats by @ManuelHu in https://github.com/legend-exp/remage/pull/366
* More generator docs! by @gipert in https://github.com/legend-exp/remage/pull/364
* confinement: fix surface sampling on native volumes by @ManuelHu in https://github.com/legend-exp/remage/pull/367
* update the default pars by @tdixon97 in https://github.com/legend-exp/remage/pull/368
* use std::abort() for RMGLog::fatal by @ManuelHu in https://github.com/legend-exp/remage/pull/369
* docs: let prettier enforce line lengths in markdown by @ManuelHu in https://github.com/legend-exp/remage/pull/371
* adjust validation suite to work with new reboost by @tdixon97 in https://github.com/legend-exp/remage/pull/372
* adjustments to discard of events without energy in Ge by @tdixon97 in https://github.com/legend-exp/remage/pull/373
* cmake: correctly depend on version and package extras by @ManuelHu in https://github.com/legend-exp/remage/pull/374
* check for directory access rights before opening output file by @ManuelHu in https://github.com/legend-exp/remage/pull/376
* turn on output reshaping by default by @gipert in https://github.com/legend-exp/remage/pull/370
* Add output "reshaping" documentation by @tdixon97 in https://github.com/legend-exp/remage/pull/377


**Full Changelog**: https://github.com/legend-exp/remage/compare/v0.10.2...v0.11.0

## v0.10.2 (2025-03-25)

Minor fix to the documentation, see release notes for v0.10.1.

**Full Changelog**: https://github.com/legend-exp/remage/compare/v0.10.1...v0.10.2

## v0.10.1 (2025-03-24)

### Physics changes

> [!IMPORTANT]
> this release changes the default position stored for germanium hits from the pre-step point to the average between pre-step and post-step


* Add commands to control which step point is stored for Germanium  by @tdixon97 in https://github.com/legend-exp/remage/pull/279

### What's Changed
* python: add more DEPENDS to install target by @ManuelHu in https://github.com/legend-exp/remage/pull/277
* python: print C++ exit reason similar to bash by @ManuelHu in https://github.com/legend-exp/remage/pull/278
* chore: update pre-commit hooks by @pre-commit-ci in https://github.com/legend-exp/remage/pull/280
* update fmt and fix some compat issues, get fmt from the web with CMake by @gipert in https://github.com/legend-exp/remage/pull/281
* Enable more compiler warnings, turn them into errors 😈 by @gipert in https://github.com/legend-exp/remage/pull/282
* Get CLI11 and magic_enum with CMake's FetchContent by @gipert in https://github.com/legend-exp/remage/pull/283
* cmake: simplify includes by @ManuelHu in https://github.com/legend-exp/remage/pull/285
* cmake: also use find_dependency for fmt by @ManuelHu in https://github.com/legend-exp/remage/pull/286
* Online validation suite reports by @gipert in https://github.com/legend-exp/remage/pull/284
* Support setting G4 aliases to substitute values in macros at runtime by @gipert in https://github.com/legend-exp/remage/pull/291
* error message for invalid substitutions by @ManuelHu in https://github.com/legend-exp/remage/pull/292
* ci: add pipefail shell option to build by @ManuelHu in https://github.com/legend-exp/remage/pull/296
* pre-commit: try to add cmake-lint by @ManuelHu in https://github.com/legend-exp/remage/pull/294
* cmake: move RMGSetuptoolsVersion to modules by @ManuelHu in https://github.com/legend-exp/remage/pull/297
* Add Particle filter stacking action option by @EricMEsch in https://github.com/legend-exp/remage/pull/244
* use static library for libfmt by @ManuelHu in https://github.com/legend-exp/remage/pull/298
* ci: better check for rmg command docs by @ManuelHu in https://github.com/legend-exp/remage/pull/259
* User manual by @gipert in https://github.com/legend-exp/remage/pull/300
* mark UIMAN* warnings as error by @ManuelHu in https://github.com/legend-exp/remage/pull/305
* include all geant4 figs into the validation reports by @tdixon97 in https://github.com/legend-exp/remage/pull/306
* Validation: electron ranges in germanium by @gipert in https://github.com/legend-exp/remage/pull/293
* also handle G4ExceptionSeverity::IgnoreTheIssue correctly by @ManuelHu in https://github.com/legend-exp/remage/pull/310
* Include all vertex confiment plots into the report by @tdixon97 in https://github.com/legend-exp/remage/pull/307
* python: add raise_on_warning parameter to remage_run by @ManuelHu in https://github.com/legend-exp/remage/pull/311


**Full Changelog**: https://github.com/legend-exp/remage/compare/v0.9.1...v0.10.1

## v0.9.1 (2025-02-20)

### What's Changed
* ci: macOS install by @ManuelHu in https://github.com/legend-exp/remage/pull/268
* python: various fixes by @ManuelHu in https://github.com/legend-exp/remage/pull/270
* cmake: split-out version info from RMGConfig.hh by @ManuelHu in https://github.com/legend-exp/remage/pull/271
* python: install venv into subdirectory by @ManuelHu in https://github.com/legend-exp/remage/pull/274


**Full Changelog**: https://github.com/legend-exp/remage/compare/v0.9.0...v0.9.1

## v0.9.0 (2025-02-14)

### What's Changed
* add simple RMGIpc for remage-cpp -> python intercom by @ManuelHu in https://github.com/legend-exp/remage/pull/246
* cmake: get version from git, or git archive by @ManuelHu in https://github.com/legend-exp/remage/pull/207
* ci: compare built version of C++ executable vs python package by @ManuelHu in https://github.com/legend-exp/remage/pull/263
* Fix Python wrapper install by @gipert in https://github.com/legend-exp/remage/pull/266
* Fix `make clean` of Python wrapper targets by @gipert in https://github.com/legend-exp/remage/pull/267


**Full Changelog**: https://github.com/legend-exp/remage/compare/v0.8.0...v0.9.0

## v0.8.0 (2025-02-09)

### What's Changed
* chore: update pre-commit hooks by @pre-commit-ci in https://github.com/legend-exp/remage/pull/251
* docs: output application states in remage-doc-dump by @ManuelHu in https://github.com/legend-exp/remage/pull/252
* ci: also run minimal tests on macOS by @ManuelHu in https://github.com/legend-exp/remage/pull/257
* correct allowed states for ActivateOutputScheme by @ManuelHu in https://github.com/legend-exp/remage/pull/258
* More messenger commands for physics lists by @tdixon97 in https://github.com/legend-exp/remage/pull/254


**Full Changelog**: https://github.com/legend-exp/remage/compare/v0.7.0...v0.8.0

## v0.7.0 (2025-01-29)

### What's Changed
* Infrastructure for Python wrapper by @gipert in https://github.com/legend-exp/remage/pull/210
* setup for running Python scripts in unit tests by @gipert in https://github.com/legend-exp/remage/pull/214
* fix problem with prettier destroying `:::{}:::` in markdown by @gipert in https://github.com/legend-exp/remage/pull/217
* chore: update pre-commit hooks by @pre-commit-ci in https://github.com/legend-exp/remage/pull/221
* Updates to sampling for intersections and subtractions by @tdixon97 in https://github.com/legend-exp/remage/pull/220
* only store G4TouchableHandle if visualization is enabled by @ManuelHu in https://github.com/legend-exp/remage/pull/206
* python: forward signals to remage-cpp by @ManuelHu in https://github.com/legend-exp/remage/pull/225
* chore: update pre-commit hooks by @pre-commit-ci in https://github.com/legend-exp/remage/pull/227
* Add distance to surface tests by @EricMEsch in https://github.com/legend-exp/remage/pull/224
* Generic surface sampler by @tdixon97 in https://github.com/legend-exp/remage/pull/226
* Add vis label to all tests opening a GUI by @EricMEsch in https://github.com/legend-exp/remage/pull/229
* fix ASan warning with RMGVOutputScheme and optional output schemes by @ManuelHu in https://github.com/legend-exp/remage/pull/231
* refactor LH5 reading code into RMGAnalysisReader by @ManuelHu in https://github.com/legend-exp/remage/pull/230
* fix clang-tidy problems in surface sampling by @ManuelHu in https://github.com/legend-exp/remage/pull/232
* chore: update pre-commit hooks by @pre-commit-ci in https://github.com/legend-exp/remage/pull/234
* clang-tidy: fix uninitialized variables and more by @ManuelHu in https://github.com/legend-exp/remage/pull/235
* confinement: fix uninitialized variable by @ManuelHu in https://github.com/legend-exp/remage/pull/233
* add a mutex for G4GeneralParticleSource by @ManuelHu in https://github.com/legend-exp/remage/pull/236
* remove implicit conversion to numpy arrays in ge-distance test by @EricMEsch in https://github.com/legend-exp/remage/pull/237
* gps: only set position when requested by @ManuelHu in https://github.com/legend-exp/remage/pull/240
* clang-tidy: more fixes and enable checks by @ManuelHu in https://github.com/legend-exp/remage/pull/238
* Fixes to tutorials by @tdixon97 in https://github.com/legend-exp/remage/pull/241
* chore: update pre-commit hooks by @pre-commit-ci in https://github.com/legend-exp/remage/pull/245
* fix clang-tidy issues in examples by @ManuelHu in https://github.com/legend-exp/remage/pull/247
* TrackID in Germanium output scheme by @tdixon97 in https://github.com/legend-exp/remage/pull/243
* Analysis manager refactoring by @ManuelHu in https://github.com/legend-exp/remage/pull/239


**Full Changelog**: https://github.com/legend-exp/remage/compare/v0.6.2...v0.7.0

## v0.6.3 (2024-12-31)

Fix problem with prettier destroying `:::{]:::` in the docs.

**Full Changelog**: https://github.com/legend-exp/remage/compare/v0.6.2...v0.6.3

## v0.6.2 (2024-12-29)

### What's Changed
* Setup dummy X server for CI tests by @gipert in https://github.com/legend-exp/remage/pull/201
* include geant4 version in log preamble by @ManuelHu in https://github.com/legend-exp/remage/pull/203
* Update developer's guide by @gipert in https://github.com/legend-exp/remage/pull/202
* Add pyproject.toml and more pre-commit hooks by @gipert in https://github.com/legend-exp/remage/pull/204
* chore: update pre-commit hooks by @pre-commit-ci in https://github.com/legend-exp/remage/pull/208
* Add .git_archival.txt by @gipert in https://github.com/legend-exp/remage/pull/211
* add possibility to print remage version from CLI by @gipert in https://github.com/legend-exp/remage/pull/213
* change IsInside check for UnionAll to act only on the selected solid not the full collection. by @tdixon97 in https://github.com/legend-exp/remage/pull/212

### New Contributors
* @tdixon97 made their first contribution in https://github.com/legend-exp/remage/pull/212

**Full Changelog**: https://github.com/legend-exp/remage/compare/v0.6.1...v0.6.2

## v0.6.1 (2024-12-18)

### What's Changed
* chore: update pre-commit hooks by @pre-commit-ci in https://github.com/legend-exp/remage/pull/199
* include _t<n> suffix in check for existing files  by @ManuelHu in https://github.com/legend-exp/remage/pull/198


**Full Changelog**: https://github.com/legend-exp/remage/compare/v0.6.0...v0.6.1

## v0.6.0 (2024-12-13)

### What's Changed
* Add distance to nearest surface field to GermaniumOutputScheme by @EricMEsch in https://github.com/legend-exp/remage/pull/182


**Full Changelog**: https://github.com/legend-exp/remage/compare/v0.5.1...v0.6.0

## v0.5.1 (2024-12-11)

only contains fixes for the docker build

### What's Changed
* Dockerfile: remove leading v for versions by @ManuelHu in https://github.com/legend-exp/remage/pull/188
* Dockerfile: use "stable" remage-base tag by default (i.e. Geant4 11.2 at the moment) by @ManuelHu in https://github.com/legend-exp/remage/pull/195


**Full Changelog**: https://github.com/legend-exp/remage/compare/v0.5.0...v0.5.1

## v0.5.0 (2024-12-11)

### Breaking physics changes
* physics: increase default time threshold for decays by @ManuelHu in https://github.com/legend-exp/remage/pull/160
* reset decay times by default; add option for global time warning by @ManuelHu in https://github.com/legend-exp/remage/pull/173

### What's Changed
* remove some more unwanted stdout output by @ManuelHu in https://github.com/legend-exp/remage/pull/168
* Add option to reset global time after initial decay by @ManuelHu in https://github.com/legend-exp/remage/pull/136
* Vertex position reader by @ManuelHu in https://github.com/legend-exp/remage/pull/176
* Docs: make the macro command reference more easy to browse by @gipert in https://github.com/legend-exp/remage/pull/184
* output: do not overwrite output files by @ManuelHu in https://github.com/legend-exp/remage/pull/186
* add optional track output scheme by @ManuelHu in https://github.com/legend-exp/remage/pull/181
* fix compiler warnings reported on CI by @ManuelHu in https://github.com/legend-exp/remage/pull/189
* also run CI tests with Geant4 11.3 by @ManuelHu in https://github.com/legend-exp/remage/pull/187
* fix compile on 11.3 by @ManuelHu in https://github.com/legend-exp/remage/pull/192
* Use double to store energy in hits by @ManuelHu in https://github.com/legend-exp/remage/pull/193


**Full Changelog**: https://github.com/legend-exp/remage/compare/v0.4.1...v0.5.0

## v0.4.1 (2024-11-29)

### What's Changed
* Fix Dockerfile versioning strategy by @gipert in https://github.com/legend-exp/remage/pull/171


**Full Changelog**: https://github.com/legend-exp/remage/compare/v0.4.0...v0.4.1

## v0.4.0 (2024-11-29)

### What's Changed
* set verbosity for hadronic processes by @ManuelHu in https://github.com/legend-exp/remage/pull/164
* output: use detector names as ntuple name by @ManuelHu in https://github.com/legend-exp/remage/pull/161
* Adjust examples to the new detector handling by @EricMEsch in https://github.com/legend-exp/remage/pull/165
* CI: also build examples by @ManuelHu in https://github.com/legend-exp/remage/pull/166
* chore: update pre-commit hooks by @pre-commit-ci in https://github.com/legend-exp/remage/pull/167


**Full Changelog**: https://github.com/legend-exp/remage/compare/v0.3.2...v0.4.0

## v0.3.2 (2024-11-12)

### What's Changed
* test more RTD changes by @ManuelHu in https://github.com/legend-exp/remage/pull/159


**Full Changelog**: https://github.com/legend-exp/remage/compare/v0.3.1...v0.3.2

## v0.3.1 (2024-11-12)

### What's Changed
* More additions to the docs by @gipert in https://github.com/legend-exp/remage/pull/155


**Full Changelog**: https://github.com/legend-exp/remage/compare/v0.3.0...v0.3.1

## v0.3.0 (2024-11-07)

### What's Changed
* implement Draw() for optical hits by @ManuelHu in https://github.com/legend-exp/remage/pull/150
* Only allow /RMG/Processes/Stepping/DaughterNucleusMaxLifetime in idle state by @ManuelHu in https://github.com/legend-exp/remage/pull/147
* Fix command docs by @ManuelHu in https://github.com/legend-exp/remage/pull/151
* Update documentation by @gipert in https://github.com/legend-exp/remage/pull/145
* change ntuple directory to `stp` by @ManuelHu in https://github.com/legend-exp/remage/pull/152


**Full Changelog**: https://github.com/legend-exp/remage/compare/v0.2.1...v0.3.0

## v0.2.1 (2024-11-05)

### What's Changed
* chore: update pre-commit hooks by @pre-commit-ci in https://github.com/legend-exp/remage/pull/143
* remage-cli: handle SIGUSR1 signal to preempt the current run by @ManuelHu in https://github.com/legend-exp/remage/pull/141
* Add Dockerfile for automated Docker Hub builds by @gipert in https://github.com/legend-exp/remage/pull/144


**Full Changelog**: https://github.com/legend-exp/remage/compare/v0.2.0...v0.2.1

## v0.2.0 (2024-11-04)

### What's Changed
* Add Grabmayrs gamma-cascade-reader and a custom neutron capture process  by @EricMEsch in https://github.com/legend-exp/remage/pull/123
* remage-to-lh5: do not run r/w pass after dry-run failed, output filenames by @ManuelHu in https://github.com/legend-exp/remage/pull/124
* make clang-tidy happy with RMGnCapture by @ManuelHu in https://github.com/legend-exp/remage/pull/125
* pre-commit: fix clang-tidy hook sometimes destroying files by @ManuelHu in https://github.com/legend-exp/remage/pull/126
* chore: update pre-commit hooks by @pre-commit-ci in https://github.com/legend-exp/remage/pull/127
* add user generator to RMGUserInit by @ManuelHu in https://github.com/legend-exp/remage/pull/128
* chore: update pre-commit hooks by @pre-commit-ci in https://github.com/legend-exp/remage/pull/129
* Make ConstructOptical() virtual in RMGPhysics to allow customization through inheritance by @EricMEsch in https://github.com/legend-exp/remage/pull/130
* Only delay OpticalPhotons if discard macro is set by @EricMEsch in https://github.com/legend-exp/remage/pull/132
* Do not disable persistency if only custom output schemes exist by @ManuelHu in https://github.com/legend-exp/remage/pull/133
* chore: update pre-commit hooks by @pre-commit-ci in https://github.com/legend-exp/remage/pull/134
* misc log fixes and improvements by @ManuelHu in https://github.com/legend-exp/remage/pull/137
* add option for custom WLS sampling by @ManuelHu in https://github.com/legend-exp/remage/pull/131
* remage-to-lh5: do not do a dry-run automatically by @ManuelHu in https://github.com/legend-exp/remage/pull/138
* remage-cli: exit with code 1 if we had errors and warnings by @ManuelHu in https://github.com/legend-exp/remage/pull/139
* Mention pre-built binary on Docker Hub by @gipert in https://github.com/legend-exp/remage/pull/140
* add option to output float32 instead of float64 for some fields by @ManuelHu in https://github.com/legend-exp/remage/pull/142


**Full Changelog**: https://github.com/legend-exp/remage/compare/v0.1.0...v0.2.0

## v0.1.0 (2024-08-27)

### What's Changed
* Implement musun input generator by @MoritzNeuberger in https://github.com/legend-exp/remage/pull/26
* Allow to re-use uids for multiple sensitive detectors by @ManuelHu in https://github.com/legend-exp/remage/pull/90
* Vertex fixes by @ManuelHu in https://github.com/legend-exp/remage/pull/91
* add option to discard event with energy cuts in germanium by @ManuelHu in https://github.com/legend-exp/remage/pull/80
* physics: add option to disable long-lived daughter nuclei in decay chains by @ManuelHu in https://github.com/legend-exp/remage/pull/70
* Add ASan build by @ManuelHu in https://github.com/legend-exp/remage/pull/81
* Add hadronic physics by @MoritzNeuberger in https://github.com/legend-exp/remage/pull/87
* refactor physics option enums by @ManuelHu in https://github.com/legend-exp/remage/pull/92
* confinement: share geometry data between threads by @ManuelHu in https://github.com/legend-exp/remage/pull/94
* Vertex output scheme & flatter ntuple structure by @ManuelHu in https://github.com/legend-exp/remage/pull/95
* chore: update pre-commit hooks by @pre-commit-ci in https://github.com/legend-exp/remage/pull/96
* Misc fixes by @ManuelHu in https://github.com/legend-exp/remage/pull/97
* chore: update pre-commit hooks by @pre-commit-ci in https://github.com/legend-exp/remage/pull/98
* chore: update pre-commit hooks by @pre-commit-ci in https://github.com/legend-exp/remage/pull/99
* tests: skip tests if X11 display not available by @ManuelHu in https://github.com/legend-exp/remage/pull/101
* chore: update pre-commit hooks by @pre-commit-ci in https://github.com/legend-exp/remage/pull/102
* chore: update pre-commit hooks by @pre-commit-ci in https://github.com/legend-exp/remage/pull/103
* add scintillation detector and output scheme by @ManuelHu in https://github.com/legend-exp/remage/pull/105
* Docs commands by @ManuelHu in https://github.com/legend-exp/remage/pull/104
* Add a way for a (library) user to add custom output schemes and user actions by @ManuelHu in https://github.com/legend-exp/remage/pull/100
* output: add units to ntuple column names by @ManuelHu in https://github.com/legend-exp/remage/pull/106
* Fix generated command docs and automate generation by @ManuelHu in https://github.com/legend-exp/remage/pull/108
* output ntuple and collection registration fixes by @ManuelHu in https://github.com/legend-exp/remage/pull/111
* Handle output file opening failure gracefully by @ManuelHu in https://github.com/legend-exp/remage/pull/112
* also apply selected random engine to threads, 2nd try by @ManuelHu in https://github.com/legend-exp/remage/pull/114
* add remage-to-lh5 utility by @ManuelHu in https://github.com/legend-exp/remage/pull/107
* add some more information to docs (DOI, Github links) by @ManuelHu in https://github.com/legend-exp/remage/pull/116
* Fix timestamp in RMGOpticalDetector and Guidance in RMGHardware by @EricMEsch in https://github.com/legend-exp/remage/pull/118
* ci: check if command reference guide is up-to-date by @ManuelHu in https://github.com/legend-exp/remage/pull/119
* Output schemes: tracking action hook & optional output schemes by @ManuelHu in https://github.com/legend-exp/remage/pull/120
* docs: add simple devguide by @ManuelHu in https://github.com/legend-exp/remage/pull/122
* add optional isotope filter output scheme by @ManuelHu in https://github.com/legend-exp/remage/pull/121

### New Contributors
* @EricMEsch made their first contribution in https://github.com/legend-exp/remage/pull/118

**Full Changelog**: https://github.com/legend-exp/remage/compare/v0.1.0a1...v0.1.0

## v0.1.0a1 (2024-05-05) *(pre-release)*

### What's Changed
* First fixes to make the project compile with Clang by @gipert in https://github.com/legend-exp/remage/pull/3
* Integrate bxdecay0 by @gipert in https://github.com/legend-exp/remage/pull/4
* Fix optical detectors & example by @ManuelHu in https://github.com/legend-exp/remage/pull/8
* Set output file name, load birks' constant from GDML by @ManuelHu in https://github.com/legend-exp/remage/pull/16
* Store photon energy in output file by @ManuelHu in https://github.com/legend-exp/remage/pull/19
* Initialize particle source on master thread by @ManuelHu in https://github.com/legend-exp/remage/pull/20
* Enable unit tests in CI by @gipert in https://github.com/legend-exp/remage/pull/22
* Make UserAction user-initializable by @ManuelHu in https://github.com/legend-exp/remage/pull/24
* Fix surface muons by @MoritzNeuberger in https://github.com/legend-exp/remage/pull/25
* General build fixes  by @drbenmorgan in https://github.com/legend-exp/remage/pull/29
* Switch to LGPL3 license by @gipert in https://github.com/legend-exp/remage/pull/27
* chore: update pre-commit hooks by @pre-commit-ci in https://github.com/legend-exp/remage/pull/28
* Make EcoMug and BxDecay0 private dependencies by @drbenmorgan in https://github.com/legend-exp/remage/pull/30
* Starting point/setup for .clang-tidy  by @drbenmorgan in https://github.com/legend-exp/remage/pull/32
* Minimize install footprint/clashes and permit uninstall by @drbenmorgan in https://github.com/legend-exp/remage/pull/33
* Properly configure default `G4Region` for sensitive volume production cuts by @gipert in https://github.com/legend-exp/remage/pull/51
* Remove log file support from `RMGLog` by @gipert in https://github.com/legend-exp/remage/pull/52
* Use better names for generator methods/members by @gipert in https://github.com/legend-exp/remage/pull/53
* Fix CI unit testing by @gipert in https://github.com/legend-exp/remage/pull/54
* Remove support for vertex sampling with a bounding sphere by @gipert in https://github.com/legend-exp/remage/pull/55
* Fix bounding box size by @nikolay327 in https://github.com/legend-exp/remage/pull/57
* Add command /RMG/Geometry/RegisterDetector by @ManuelHu in https://github.com/legend-exp/remage/pull/63
* Setup analysis manager later and fix muti threaded runs by @ManuelHu in https://github.com/legend-exp/remage/pull/65
* cli: add option to set output file, type converter for log level by @ManuelHu in https://github.com/legend-exp/remage/pull/64
* cmake: install headers into subfolder, simplify scripts by @ManuelHu in https://github.com/legend-exp/remage/pull/66
* Misc fixes by @ManuelHu in https://github.com/legend-exp/remage/pull/67
* Refactor messengers by @ManuelHu in https://github.com/legend-exp/remage/pull/69
* clang-tidy: add more modernize-* checks by @ManuelHu in https://github.com/legend-exp/remage/pull/71
* ci: also test on G4 11.2 by @ManuelHu in https://github.com/legend-exp/remage/pull/72
* output time of vertex duration in run statistics by @ManuelHu in https://github.com/legend-exp/remage/pull/73
* Fix CI tests not failing, fix some other minor bugs by @ManuelHu in https://github.com/legend-exp/remage/pull/77
* chore: update pre-commit hooks by @pre-commit-ci in https://github.com/legend-exp/remage/pull/74
* record particle type (PDG ID) in Germanium output scheme by @ManuelHu in https://github.com/legend-exp/remage/pull/78
* Add support for multiple mother volumes to vertex confinement by @ManuelHu in https://github.com/legend-exp/remage/pull/79
* Option to disable overlap check by @ManuelHu in https://github.com/legend-exp/remage/pull/82
* Use custom overlap check instead of the one from G4GDML by @ManuelHu in https://github.com/legend-exp/remage/pull/83
* Add CITATION.cff by @gipert in https://github.com/legend-exp/remage/pull/89

### New Contributors
* @gipert made their first contribution in https://github.com/legend-exp/remage/pull/3
* @ManuelHu made their first contribution in https://github.com/legend-exp/remage/pull/8
* @pre-commit-ci made their first contribution in https://github.com/legend-exp/remage/pull/9
* @MoritzNeuberger made their first contribution in https://github.com/legend-exp/remage/pull/25
* @drbenmorgan made their first contribution in https://github.com/legend-exp/remage/pull/29
* @nikolay327 made their first contribution in https://github.com/legend-exp/remage/pull/57
* @dependabot made their first contribution in https://github.com/legend-exp/remage/pull/60

**Full Changelog**: https://github.com/legend-exp/remage/commits/v0.1.0a1
