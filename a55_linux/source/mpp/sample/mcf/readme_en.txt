1. Sample 0 and sample 1 are demonstration mcf calibration mpi interfaces, and sample 2 and sample 3 are demonstration mcf basic channels.
2. Sample 2 and Sample 3 support IMX347 / os04a10 sensor. When using os04a10, because the sensor is not in slave mode, it cannot guarantee that the time of collecting 2 channels of data is completely synchronized, so the sample effect may not be correct.
3. For the spectroscopic prism module lenses used in sample 2 and sample 3, if the lens has an error, mipi crop can be used to align the two sensor fields of view.
4. When the 2-channel sensor has parallax, you need to use the calibration interface for calibration. For the use of the calibration interface and calibration results, please refer to "Black and White Dual Channel Fusion Development Reference" and "Black and White Dual Channel Fusion Debugging Guide".
5. Sample 3 demonstrates switching between day and night when synchronization is enabled. Sample 4 demonstrates switching between day and night when synchronization is disabled.
6. Sample 5 demonstrates how to start the mcf + VENC service after the mcf online calibration.
7. samples 2 to 5 can use the combination of os08a20 sensor which outputs black and white image and os04a10 sensor which outputs color image to start the mcf service of different resolutions.
