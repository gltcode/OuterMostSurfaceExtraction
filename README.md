# OuterMostSurfaceExtraction
This project presents a algorithm to segment the outer-most surface of head in CT/MRI 3D medical image.
The algorithm is built based on itk, and is encapsulated as a class of itk filter. You can add the files (itkOuterMostSurfaceExtractionFilter.h and itkOuterMostSurfaceExtractionFilter.hxx) to your project, and call the function in itk style easily.

# OverView of The Algorithm
step0: Preprocesse the image to normalize the distribution of the value of voxels.
        For CT, the voxels whose value are higher than -150 are directly set to -150, and the voxels whose value are less than -450 are directly set to -450. Because the voxels whose value is not in the range of -150 to -450 are impossible to be the skin which is the outer-most surface of the head.
        For MRI， the distribution is shift and scaled to [0,2560] for normalization.
        
step 1: Use otsu algorithm to segment the background in each cross section of the 3d medical image.

step 2: Analyze the connection of segmentation resul in each cross plane of the 3d medical image, and select the region with max pixels which is the background air.

step3: Analyze the connection of segmentation result in each sagittal plane of the 3d medical image, and select the region with max pixels which is the background air. 

After step3, the outer-most surface of the head is suceffuly segmented.


