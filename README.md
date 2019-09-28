# wxECGAnalyzer
This tool is for Electrocardiogram(ECG) signal use to machine learning auto labeling and embedded systems QRS-Complex algorithm validation.

</br>
In ANSI/AAMI EC38,it is required that the detected QRS shall in the 150ms range of the signed point from annotation by human exper.
</br>
</br>

## Use
### Dependence
- Win10
  - wxWidgets 3.1.2
  - VS2017 - MSVD 10.0.17763-SDK
- Mac High Sierra
  - wxWidgets 3.x
  - g++
### Build
- Win10
  - 1. Open wxECGAnalyzer.sln
  - 2. Rebuild
![alt text](https://github.com/GCY/wxECGAnalyzer/blob/master/res/win10.PNG?raw=true)  
- Mac High Sierra
  - 1. Make
![alt text](https://github.com/GCY/wxECGAnalyzer/blob/master/res/high%20sierra.png?raw=true) 
### Operate Manual
  - Clinical trial
    - 1. Setup your ECG device to  clinical trial.
    - 2. Connect VCP to wxECGAnalyzer.
    - 3. Monitor target.
    - 4. Capture and save target morphology of the ECG.
    - 5. Select ECG-Codes to labeling.
[Snapshot.csv](https://github.com/GCY/wxECGAnalyzer/blob/master/res/snapshot.csv): example file.
    ![alt text](https://github.com/GCY/wxECGAnalyzer/blob/master/res/snapshot.png?raw=true) 
  - Validation of Algorithm for Real-time Embedded Systems(Holter)
    - 1. Add *.cpp and .h file, and create new wxRadioBox item to apply.
    - 2. Design algorithm with C/C++.
    - 3. Clinical trial and validation.
    - 4. Port code file to your embedded project.

## Features
- [x] Filter
  - [x] Finite Impulse Response(FIR)
- [x] QRS-Complex Detect Algorithm
  - [x] Adaptive Threshold Algorithm
  - [x] HC_Chen Algorithm
  - [x] Enhanced So & Chen
  - [x] Pan-Tompkins
  - [ ] Deep-ECG(1D-CNN)
- [x] Heart Rate Variability
  - [x] Heart Rate
  - [x] SDNN
  - [x] NN50
  - [x] pNN50
- [x] Labeling
  - [x] Segmentation
  - [ ] Automatic segmentation
  - [x] MIT-BIH ECG-Codes
  - [ ] Automatic labeling(SVM+CNN Feature)
- [ ] MIT-BIH Database Operate
  - [ ] WFDB
- [ ] Quantitative
  - [ ] Sensitivity
  - [ ] Accuracy
  - [ ] Confusion Matrix
- [x] Other Operate  
  - [x] Record RAW data(60s)
  - [x] Save four plot(.png)  
- [x] Fast Furious Transform(FFT) amplitude spectrum
- [x] Connect serial port

## The point of QRS-Complex Detect Algorithm
### Adative Threshold Algorithm
Gradient = RMS * CV(%) * Threshold_Factor
### HC Chen
### So & Chen
### Pan-Tompkins
### Real-Time Complexity 

## Experiment device
- ARM Cortex-M4
  - STM32F407-Discovery
  - AD8232
  
- [cNIBP](https://github.com/GCY/Continuous-Non-Invasive-Blood-Pressure-Research-Platform---ECG-and-PPG-Pulse-Arrival-Time-Based-.git): Use the ECG part.(without Right Leg Drive)

## Video
</br>

### Algorithm test
[![Audi R8](http://img.youtube.com/vi/GpHpex1oun4/0.jpg)](https://youtu.be/GpHpex1oun4)

### Motion artifact - running
[![Audi R8](http://img.youtube.com/vi/KjEhhs2AV2s/0.jpg)](https://youtu.be/KjEhhs2AV2s)
### Disturbance
[![Audi R8](http://img.youtube.com/vi/AtoNvDiFkyU/0.jpg)](https://youtu.be/AtoNvDiFkyU)

</br>

## Reference
- Find the local maxima and minima : http://billauer.co.il/peakdet.html
- Heart rate variability : https://en.wikipedia.org/wiki/Heart_rate_variability
- HC_Chen Algorithm
  - HC Chen, SW Chen, "A Moving Average based Filtering System with its Application to Real-time QRS Detection", Computers in Cardiology, 2003.
  - https://github.com/blakeMilner/real_time_QRS_detection (modify)
- Enhanced So & Chen Algorithm
  - So, H. H., & Chan, K. L. (1997, October). Development of QRS detection method for real-time ambulatory cardiac monitor. In Proceedings of the 19th Annual International Conference of the IEEE Engineering in Medicine and Biology Society.'Magnificent Milestones and Emerging Opportunities in Medical Engineering'(Cat. No. 97CH36136) (Vol. 1, pp. 289-292). IEEE.
  - Lee, R. G., Chou, I. C., Lai, C. C., Liu, M. H., & Chiu, M. J. (2005). A novel QRS detection algorithm applied to the analysis for heart rate variability of patients with sleep apnea. Biomedical Engineering: Applications, Basis and Communications, 17(05), 258-262.
  - Wang, C. Y., Chang, R. C. H., Lin, C. H., & Su, S. H. (2018, May). Fatigue Detection System Using Enhanced So and Chan Method. In 2018 IEEE International Conference on Consumer Electronics-Taiwan (ICCE-TW) (pp. 1-2). IEEE.
- Pan-Tompkins Algorithm
  - Pan, J., & Tompkins, W. J. (1985). A real-time QRS detection algorithm. IEEE transactions on biomedical engineering, (3), 230-236.
  - Afonso, V. X. (1993). Ecg qrs detection. Biomedical digital signal processing, 237-264.
  - https://github.com/rafaelmmoreira/PanTompkinsQRS (modify)

LICENSE
-------

MIT License

Copyright (c) 2019 Tony Guo

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
