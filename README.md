# Sound_Processor
Adaptive Noise Reduction using RLS algorithm, NLMS algorithm for Echo Cancelation, VAD of G729 to separate noise from voice
Now, we can use only NLMS and RLS algorithms.
The main.c function is contained in Sound_Processor_V1.1
You can check RLS algorithm using your own raw files
MATLAB folder contains MATALB files where I created my 
all algorithms for voice processing and noise reduction.

21.08.2024 Added Eigen Library as a submodule to my project, 
I hope my math function will be faster and more convenient.
Also, here i have Google Tests as a submodule it is cool
In general, i could use vcpkg or make, but not for this moment
vcpkg or cmake will be in the next iteration after upgrading 
Sound_Processor_V1.1
