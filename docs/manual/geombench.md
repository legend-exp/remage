(manual-geombench)=

# Geometry benchmarking

The `remage-geombench` command-line tool helps identify performance bottlenecks
in detector geometries during the design phase. It systematically samples points
on three grids of starting positions and simulates geantinos through the volume.
It returns the median simulation time per geantino. Based on these overview
plots and statistics are generated.

:::{tip}

Use this tool during geometry development to identify components that may slow
down simulations. Complex nested structures, boolean operations, and highly
detailed volumes can significantly increase navigation time.

:::

## Overview

The geometry benchmark works by:

1. Loading a GDML geometry file
2. Creating three 2D grids in XY, XZ, and YZ direction according to grid
   specifications
3. Sampling geantinos and estimating the median simulation time per grid point
4. Generating statistics and visualizations showing where navigation is slow

This information helps you optimize geometries before running full physics
simulations, potentially saving significant computation time.

## Basic usage

The simplest usage requires only a GDML file:

```console
$ remage-geombench detector.gdml
```

This will:

- Benchmark the entire geometry
- Use default settings (10M events, 1 mm grid spacing, 25% buffer)
- Save results to `detector.lh5` in the current directory
- Print summary statistics to the console and generate overview plots in the
  current directory

## Command-line options

### Geometry selection

**`geometry`** (required) : Path to the GDML geometry file to benchmark.

**`--logical-volume NAME`** (optional) : Extract and benchmark only a specific
logical volume including daughters from the geometry. Useful for isolating
performance issues in complex assemblies.

Example:

```console
$ remage-geombench l1000.gdml --logical-volume V0101
```

### Grid configuration

**`--grid-increment SPACING`** (default: `1`) : Uniform spacing between grid
points in millimeters for all dimensions.

Example (coarse grid for quick tests):

```console
$ remage-geombench l1000.gdml --grid-increment 5
```

**`--grid-increments DICT`** (optional) : Specify different spacing per
dimension using a Python dictionary literal. Overrides `--grid-increment` if
provided.

Example:

```console
$ remage-geombench l1000.gdml --grid-increments "{'x': 1.0, 'y': 2.0, 'z': 0.5}"
```

### Buffer region

**`--buffer-fraction FRACTION`** (default: `0.25`) : Fractional buffer space
around the geometry. A value of 0.25 adds 12.5% extra space on each side,
creating a world volume large enough to contain the geometry with margin.

Example (tighter bounds):

```console
$ remage-geombench l1000.gdml --buffer-fraction 0.1
```

### Simulation control

**`--num-events N`** (default: `10000000`) : Number of navigation events to
simulate. Higher values give more stable statistics but take longer.

Example (quick test):

```console
$ remage-geombench l1000.gdml --num-events 1000000
```

**`--output-dir PATH`** (default: `./`) : Directory to store output files.

Example:

```console
$ remage-geombench l1000.gdml --output-dir ./benchmark_results/
```

**`--dry-run`** : Generate and display the macro file without running the
simulation. Useful for verifying configuration before long runs.

Example:

```console
$ remage-geombench l1000.gdml --dry-run
```

## Interpreting results

The tool generates an LH5 output file containing spatial navigation performance
data and returns a yaml summary file with key statistics. Example summary
output:

```
hotspots: # List of voxels with largest navigation times calculated via multiplicative combination of profiles
- - -7.0
  - -20.0
  - -37.352000000000004
  [...]
simulation_time_per_event:
  max: 3.8e-05
  mean: 7.426031999999999e-06
  min: 6.0e-06
  std: 4.6970280747485426e-06
```

In addition, the visualization of the data can help identify hotspots.

:::{warning}

Navigation times are relative and depend on the hardware. Focus on identifying
spatial patterns and relative differences between geometric components rather
than absolute timing values.

In addition, these statistics also depend on the ratio of empty space to actual
material. If one is not interested in potential slowdowns far away from the
object, one should choose a small enough buffer to reduce the impact of the
empty space.

:::

## Workflow examples

### Full detector benchmark

Benchmark an entire detector assembly with default settings:

```console
$ remage-geombench l1000.gdml --output-dir benchmarks/
```

### Component isolation

Test a specific problematic component:

```console
$ remage-geombench l1000.gdml --logical-volume V0101 \
    --grid-increment 0.5 --output-dir component_tests/
```

This generates a `part_{logical-volume}.lh5` and
`part_{logical-volume}_[...].pdf` files in the output directory.

### Anisotropic sampling

For elongated geometries, use different grid spacing per dimension:

```console
$ remage-geombench l1000.gdml \
    --grid-increments "{'x': 0.5, 'y': 0.5, 'z': 2.0}"
```

## Performance optimization tips

Based on benchmark results, consider these optimization strategies:

1. **Simplify boolean operations**: Multiple nested unions/subtractions are
   expensive. Consider alternative representations.

2. **Reduce tessellated solid complexity**: Decrease facet count where possible
   without losing essential features.

3. **Limit nesting depth**: Deeply nested volumes increase navigation time.

## Technical details

The benchmark uses the `GeomBench` generator, which systematically steps through
three 2D grids and measures median simulation time per grid point. The geometry
is automatically wrapped in a world volume sized to contain it with the
specified buffer fraction.

When extracting a specific logical volume, the tool:

1. Identifies the volume in the GDML registry
2. Copies all dependent resources (materials, solids, daughter volumes)
3. Creates a minimal world volume containing only the extracted component
4. Applies the buffer and generates a temporary GDML file for benchmarking

This allows testing individual components without the overhead of the full
geometry.

## See also

- {ref}`manual-geometry` – General geometry setup
- {ref}`manual-running` – Running simulations
- {ref}`manual-output` – Output data formats
