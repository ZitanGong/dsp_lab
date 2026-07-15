# DSP Signal Processing Project Laboratory

> **Built with dedication and carefully polished by Zitan Gong during the 2026 Summer Short Semester.**

This repository contains a signal processing and system analysis project developed for the **TI TMS320C6748 DSP** platform. It covers polyphase filter banks, real-time ADC/DAC audio streaming, embedded keyword spotting, LCD interaction, and voice-controlled music playback.

Repository: [ZitanGong/dsp_lab](https://github.com/ZitanGong/dsp_lab)

## Overview

The project uses the C6748 DSP laboratory board and its ADC, DAC, EDMA, PRU, LCD, and touch-screen resources to build a complete pipeline from audio acquisition and digital signal processing to inference, visualization, and device control.

The repository currently includes:

- An eight-band polyphase analysis/synthesis filter-bank experiment;
- ADC/DAC ping-pong buffering with EDMA-based data transfer;
- 20 kHz hardware audio acquisition aligned with a 16 kHz neural-network input;
- Adaptive voice activity detection and endpoint detection;
- Log-Mel extraction using a 30 ms window, 10 ms hop, 512-point FFT, and 40 Mel bands;
- A 12-class BC-ResNet keyword-spotting model;
- Simultaneous real-time music playback and voice-command recognition;
- Synchronous CH3/CH4 music-reference suppression;
- An 800×480 LCD interface for system state and recognition results;
- Model-weight export and pure-C inference optimized for the C6748.

## Experiments

### Project 1: Polyphase Filter Bank

`Code/project1.c` and `Code/project1.h` contain the design and experimental implementation of an eight-band polyphase filter bank, including:

- A 129-tap square-root raised-cosine prototype filter;
- Eight polyphase branches with 17 taps per branch;
- Eight-point DFT/IDFT subband transforms;
- Polyphase analysis and synthesis filters;
- Circular-buffer state management;
- Subband energy, noise-floor, and gain-processing structures;
- An ADC input, subband-processing, and DAC output workflow.

This part preserves the algorithm derivation, fast implementation, and hardware-oriented optimization process. It can be used to study multirate signal processing, filter banks, and subband processing.

### Project 3: Embedded Keyword Spotting and Music Control

`Code/project3.c` and `Code/project3.h` form the core of the current project. They implement an embedded keyword-spotting system that continues listening for voice commands while music is playing.

The processing pipeline is:

```text
Microphone input
      ↓
ADC + EDMA/PRU ping-pong acquisition
      ↓
Music-reference suppression and signal conditioning
      ↓
Adaptive VAD and pre-roll buffering
      ↓
20 kHz → 16 kHz resampling
      ↓
Log-Mel extraction and normalization
      ↓
BC-ResNet forward inference
      ↓
Confidence, class-margin, and unknown rejection
      ↓
LCD output and music-control action
```

The model produces 12 output classes:

```text
silence, unknown, down, go, left, no,
off, on, right, stop, up, yes
```

The primary music-control commands are:

| Command | Action |
| --- | --- |
| `on` | Start or resume playback |
| `off` | Pause playback |
| `up` | Increase volume |
| `down` | Decrease volume |

## Audio Channel Connections

Project 3 currently uses the following channel assignment:

| Signal | Hardware channel | Purpose |
| --- | --- | --- |
| Microphone | ADC CH1 / MIC1 | VAD and keyword recognition |
| Music left | ADC CH3 / LINE2-L | Music input |
| Music right | ADC CH4 / LINE2-R | Music input |
| Music output | DAC CH2 | Speaker or headphone output |

> **Important:** LINE1 shares ADC CH1/CH2 with MIC1/MIC2. Inserting a LINE1 plug may mechanically disconnect the microphone inputs. Music must therefore use the independent **LINE2 input on ADC CH3/CH4**. Using LINE1 may cause the system to treat the left and right music channels as microphone input.

The CH3/CH4 stereo signal is downmixed to mono inside the DSP and also used as a synchronous reference. A least-squares projection suppresses the component of CH1 that is correlated with the electrical music input while retaining as much uncorrelated speech as possible.

## Feature and Model Configuration

| Parameter | Configuration |
| --- | --- |
| ADC/DAC sample rate | 20 kHz |
| Model sample rate | 16 kHz |
| Input duration | 1 second |
| Window length | 480 samples / 30 ms |
| Hop length | 160 samples / 10 ms |
| FFT length | 512 |
| Mel bands | 40 |
| Time frames | 101 |
| Model input | `1 × 40 × 101` |
| Output classes | 12 |

`Code/weights.c` contains floating-point parameters exported from the PC-trained model. `Code/weights.h` provides the declarations required by the DSP inference implementation.

## Repository Structure

```text
dsp_lab/
├─ Code/
│  ├─ project1.c / project1.h       # Polyphase filter-bank experiment
│  ├─ project3.c / project3.h       # Keyword spotting and music control
│  ├─ weights.c / weights.h         # BC-ResNet parameters
│  ├─ Driver/                       # ADC, DAC, LCD, EDMA, PRU, and board drivers
│  └─ User/                         # User-facing peripheral interfaces
├─ Include/                         # StarterWare, DSPLib, IMGLib, VLib, and MathLib headers
├─ Library/                         # TI platform and DSP libraries
├─ TargetConfig/                    # C6748 linker, GEL, and debugger configuration
├─ dataset_pc/                      # Personalized microphone recordings
├─ tools/                           # PC microphone and data-collection utilities
├─ outputs/                         # Presentation files and rendered slide previews
├─ .ccsproject / .cproject          # Code Composer Studio project metadata
└─ Signal-processing laboratory PDF # Course laboratory guide
```

### Main Driver Modules

`Code/Driver` contains the board-level drivers and real-time data paths used by the project:

- `04_adc`: AD7606 initialization, sampling control, and EDMA reception;
- `05_dac`: DAC initialization, output buffering, and playback control;
- `11_lcd`: LCD Raster, DMA, GrLib drawing, and screen refresh;
- `12_touch`: Touch-screen scanning and button-region processing;
- `21_edma`: Shared EDMA resources and interrupt management;
- `24_pru`: PRU0/PRU1 firmware and the pre-build assembler utility;
- Other directories provide LED, key, timer, UART, I²C, SPI, EEPROM, and related peripheral support.

## Development Environment

- Target processor: TI TMS320C6748;
- Device family: C6000;
- Recommended IDE: Code Composer Studio;
- Original CCS project version: 11.0;
- TI C6000 Code Generation Tools: 8.3.x or a compatible release;
- C language mode: C89;
- Target configuration: `TargetConfig/C6748.ccxml`.

## Building and Running

1. Clone the repository:

   ```bash
   git clone https://github.com/ZitanGong/dsp_lab.git
   ```

2. In Code Composer Studio, select:

   ```text
   File → Import → CCS Projects
   ```

3. Select the repository root and import the project.

4. Confirm that a compatible C6000 compiler is installed and verify the emulator settings in `TargetConfig/C6748.ccxml`.

5. Build the project. The pre-build step invokes `Code/Driver/24_pru/build/pasm` to generate the PRU0 and PRU1 firmware headers.

6. Connect the C6748 board, load the generated `.out` file, and run the program.

`Debug/` and `Release/` are local build-output directories and are excluded from version control by default.

## Personalized Speech Collection

`tools/collect_pc_microphone.py` records a PC microphone, performs automatic VAD segmentation, and saves 16 kHz mono 16-bit WAV files. For example:

```bash
python tools/collect_pc_microphone.py --label on --count 100 --output dataset_pc
python tools/collect_pc_microphone.py --label off --count 100 --output dataset_pc
```

Recordings should cover different volumes, microphone distances, speaking rates, sessions, and pronunciation styles to improve generalization to individual speakers and non-standard pronunciation.

## Highlights

- A complete path from neural-network weights to pure-C DSP inference;
- Matched Log-Mel preprocessing between the training and embedded pipelines;
- Continuous real-time audio using EDMA, PRU, and ping-pong buffers;
- Music playback remains active while neural-network inference is running;
- Synchronous reference suppression for electrical music leakage;
- Separate state and data management for VAD, inference, and LCD refresh;
- Runtime diagnostic variables designed for inspection through CCS Watch.

## Notes and Future Work

This project was created for the 2026 Summer Short Semester project laboratory in Signal Processing and System Analysis. It contains course experiments, board drivers, algorithm validation, model deployment, and the results of continuous hardware debugging.

Planned improvements include:

- Better separation of acoustically similar commands such as `on` and `off`;
- Larger personalized, multi-speaker, and music-background datasets;
- Improved music-reference cancellation and VAD under difficult conditions;
- More automated model evaluation, weight export, and deployment.

---

**Author: Zitan Gong (巩子檀)**<br>
**Date: 2026 Summer Short Semester**
