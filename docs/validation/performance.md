# Performance Benchmarks

This section tests the performance of remage for critical functions and options.

## Combine low energy tracks option

This part evaluates the performance of the
`/RMG/Output/.*/Cluster/CombineLowEnergyElectronTracks` option. This option
combines the tracks of low energetic electrons with nearby higher energetic
tracks to reduce file size. This function was found to be problematic in
extensive simulations (like showers) if not optimized.

```{figure} ./_img/performance/combine-tracks-benchmark.output.png
:width: 600px
*Benchmark results. For each energy 1000 gammas were simulated within one event to maximize the number of tracks to merge. The geometry is a 2x2x2m box out of liquid argon.*
```
