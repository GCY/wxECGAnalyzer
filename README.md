# wxECGAnalyzer
This tool is for Electrocardiogram(ECG) signal use to machine learning auto labeling and embedded systems QRS-Complex algorithm validation.

</br>
In ANSI/AAMI EC38,it is required that the detected QRS shall in the 150ms range of the signed point.
</br>
</br>

## Use

## Features
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
- [ ] Quantitative
  - [ ] Sensitivity
  - [ ] Accuracy
  - [ ] Confusion Matrix
- [x] Other Operate  
  - [x] Record RAW data(60s)
  - [x] Save four plot(.png)  
- [x] Fast Furious Transform(FFT) amplitude spectrum
- [x] Connect serial port

## Youtube
</br>

### Algorithm test
[![Audi R8](http://img.youtube.com/vi/GpHpex1oun4/0.jpg)](https://youtu.be/GpHpex1oun4)

### Motion artifact
[![Audi R8](http://img.youtube.com/vi/KjEhhs2AV2s/0.jpg)](https://youtu.be/KjEhhs2AV2s)
### Disturbance
[![Audi R8](http://img.youtube.com/vi/AtoNvDiFkyU/0.jpg)](https://youtu.be/AtoNvDiFkyU)

</br>

## Reference
- HC_Chen Algorithm
  - HC Chen, SW Chen, "A Moving Average based Filtering System with its Application to Real-time QRS Detection", Computers in Cardiology, 2003.
  - https://github.com/blakeMilner/real_time_QRS_detection
- Enhanced So & Chen Algorithm
  - So, H. H., & Chan, K. L. (1997, October). Development of QRS detection method for real-time ambulatory cardiac monitor. In Proceedings of the 19th Annual International Conference of the IEEE Engineering in Medicine and Biology Society.'Magnificent Milestones and Emerging Opportunities in Medical Engineering'(Cat. No. 97CH36136) (Vol. 1, pp. 289-292). IEEE.
  - Lee, R. G., Chou, I. C., Lai, C. C., Liu, M. H., & Chiu, M. J. (2005). A novel QRS detection algorithm applied to the analysis for heart rate variability of patients with sleep apnea. Biomedical Engineering: Applications, Basis and Communications, 17(05), 258-262.
  - Wang, C. Y., Chang, R. C. H., Lin, C. H., & Su, S. H. (2018, May). Fatigue Detection System Using Enhanced So and Chan Method. In 2018 IEEE International Conference on Consumer Electronics-Taiwan (ICCE-TW) (pp. 1-2). IEEE.
- Pan-Tompkins Algorithm
  - Pan, J., & Tompkins, W. J. (1985). A real-time QRS detection algorithm. IEEE transactions on biomedical engineering, (3), 230-236.
  - Afonso, V. X. (1993). Ecg qrs detection. Biomedical digital signal processing, 237-264.
  - https://github.com/rafaelmmoreira/PanTompkinsQRS
