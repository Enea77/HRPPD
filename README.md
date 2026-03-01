# HRPPD Photocathode Aging & Characterization Pipeline

## Overview
[cite_start]This repository contains the hardware control and data analysis pipeline developed for the HRPPD Photocathode Aging & Characterization Study[cite: 19]. 

[cite_start]Large Area Picosecond Photodetectors (HRPPDs) require rigorous validation for use in future collider applications[cite: 22]. [cite_start]The code in this repository was engineered to precisely characterize these detectors, isolate signals from background noise, and monitor performance degradation over high-photon exposure[cite: 21, 22]. 

The project bridges physical hardware automation with high-level data processing and mathematical fitting.

## Repository Structure

This repository is modularized into three core components:

* [cite_start]**[`/Hardware_Control`](Hardware_Control/)**: Contains Python scripts used to engineer an automated optics scanning system[cite: 20]. [cite_start]This software directly interfaces with physical hardware to precisely control LED triggers and position stages during data acquisition[cite: 20].
* [cite_start]**[`/QE_Analysis`](QE_Analysis/)**: A C++ and ROOT-based pipeline designed to map Quantum Efficiency (QE) across the detector cathode[cite: 21]. [cite_start]It processes raw hardware telemetry (e.g., Keithley instruments) and applies mathematical fits to isolate the true photon signal from background leakage current[cite: 21].
* [cite_start]**[`/PDE_Analysis`](PDE_Analysis/)**: An analysis suite dedicated to quantifying single-pixel Photon Detection Efficiency (PDE)[cite: 22]. It batch-processes digitized waveforms to compare Analog-to-Digital (ADC) peaks against trigger counts, generating precise spatial performance profiles.

## Technologies & Frameworks
* [cite_start]**Languages:** C++, Python [cite: 12]
* [cite_start]**Data Analysis:** ROOT (CERN) [cite: 12]
* [cite_start]**Hardware Integration:** Zaber position stages, LED triggers, Keithley electrometers, waveform digitizers[cite: 20].

## Navigation
Detailed documentation, mathematical formulas, and visual output examples (such as 2D efficiency mappings and leakage current fits) are provided within the `README.md` files located in each respective sub-directory.
