# HPGe detector distance to surface

Geant4 has the possibility to compute the distance of points to the surface of a solid. This is saved in the remage germanium
output scheme for use in detector post-processing.
This section of the validation report validates this calculation by comparing to one from python.

These tests fill fail if the difference is large, and we also produce some plots showing the computed distance 
compared to the profile of the detectors.

In this output scheme it is possible to save the position of the step based on either the pre or post-step point
or the average of the two. All three options are used.


```{figure} ./_img/distances/distance-ge-average.output.png
:width: 800px
For the average of pre and post-step point, used by default.
```
&nbsp;

```{figure} ./_img/distances/distance-ge-poststep.output.png
:width: 800px
For the poststep point.
```
&nbsp;

```{figure} ./_img/distances/distance-ge-prestep.output.png
:width: 800px
For the pre-step point
```

