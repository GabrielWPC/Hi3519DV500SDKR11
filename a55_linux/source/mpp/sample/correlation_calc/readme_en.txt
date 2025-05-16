This sample is used to evaluate the correlation between sensor pixels.

Precautions:
1. You are advised to use the color chart scenario and select the third gray block as the statistical area. Note that the size of the ROI cannot exceed 80 x 80. Otherwise, the size of the ROI may exceed the memory limit when there are too many frames.
2. This sample is the time domain statistics of pixel values. Therefore, the number of frames must be greater than 5000.
3. During the statistics, ensure that the light does not change and the scene is stationary.
4. The illumination may change periodically during statistics collection. Therefore, you are advised to set the frame interval to 15. A larger interval indicates a longer time required for statistics collection when the number of frames remains unchanged.
5. The larger the average value of the correlation coefficient in the statistical result, the stronger the correlation between pixels. If the average value is greater than 0.1, the correlation between pixels is obvious. In this case, if the hnr and high ISO are used, obvious grid problems occur. You are advised to confirm with the sensor manufacturer whether DPC or other denoising operations are enabled. These algorithms must be disabled when used.
