# wxECGAnalyzer
This tool is for Electrocardiogram(ECG) signal use to machine learning auto labeling and embedded systems QRS-Complex algorithm validation.

</br>
In ANSI/AAMI EC38,it is required that the detected QRS shall in the 150ms range of the signed point.
</br>
</br>

## Function
- [x] Filter
  - [x] Finite Impulse Response(FIR)
- [x] QRS-Complex Detect Algorithm
  - [x] Adaptive Threshold Algorithm
  - [x] HC_Chen Algorithm
  - [x] Enhanced So & Chen
  - [x] Pan-Tompkins
  - [ ] Deep-ECG
- [x] Heart Rate Variability
  - [x] Heart Rate
  - [x] SDNN
  - [x] NN50
  - [x] pNN50
- [x] Labeling
  - [x] Segmentation
  - [ ] Automatic segmentation
  - [x] MIT-BIH ECG-Codes
  - [ ] Automatic labeling
- [ ] MIT-BIH Database Operate
  - [ ] WFDB
- [x] Fast Furious Transform(FFT) amplitude spectrum

## Youtube
</br>

[![Audi R8](http://img.youtube.com/vi/GpHpex1oun4/0.jpg)](https://youtu.be/GpHpex1oun4)

</br>

## Reference
- HC_Chen Algorithm
  - HC Chen, SW Chen, "A Moving Average based Filtering System with its Application to Real-time QRS Detection", Computers in Cardiology, 2003.
  - https://github.com/blakeMilner/real_time_QRS_detection
- Enhanced So & Chen
- Pan-Tompkins
  - Pan, J., & Tompkins, W. J. (1985). A real-time QRS detection algorithm. IEEE transactions on biomedical engineering, (3), 230-236.
  - https://github.com/rafaelmmoreira/PanTompkinsQRS
