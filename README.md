# HRPPD Photocathode Aging & Characterization Pipeline

## Overview
This repository contains the hardware control and data analysis pipeline developed for the HRPPD Photocathode Aging & Characterization Study. 

Large Area Picosecond Photodetectors (HRPPDs) require rigorous validation for use in future collider applications. The code in this repository was engineered to precisely characterize these detectors, isolate signals from background noise, and monitor performance degradation over high-photon exposure. 

The project bridges physical hardware automation with high-level data processing and mathematical fitting.

## Repository Structure

This repository is modularized into three core components:

* **[`/Hardware_Control`](Hardware_Control/)**: Contains Python scripts used to engineer an automated optics scanning system. This software directly interfaces with physical hardware to precisely control LED triggers and position stages during data acquisition.
* **[`/QE_Analysis`](QE_Analysis/)**: A C++ and ROOT-based pipeline designed to map Quantum Efficiency (QE) across the detector cathode. It processes raw hardware telemetry (e.g., Keithley instruments) and applies mathematical fits to isolate the true photon signal from background leakage current.
* **[`/PDE_Analysis`](PDE_Analysis/)**: An analysis suite dedicated to quantifying single-pixel Photon Detection Efficiency (PDE). It batch-processes digitized waveforms to compare Analog-to-Digital (ADC) peaks against trigger counts, generating precise spatial performance profiles.

## Technologies & Frameworks
* **Languages:** C++, Python 
* **Data Analysis:** ROOT (CERN) 
* **Hardware Integration:** Zaber position stages, LED triggers, Keithley electrometers, waveform digitizers.

## Navigation
Detailed documentation, mathematical formulas, and visual output examples (such as 2D efficiency mappings and leakage current fits) are provided within the `README.md` files located in each respective sub-directory.
