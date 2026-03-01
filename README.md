# HRPPD Photocathode Characterization Pipeline

## Overview
This repository contains a C++ and ROOT-based analysis pipeline developed for the HRPPD Photocathode Aging & Characterization Study. The codebase is designed to process raw hardware outputs, isolate signal from background noise, and accurately map Quantum Efficiency (QE) and Photon Detection Efficiency (PDE) across Large Area Picosecond Photodetectors. 

This specific pipeline parses hardware telemetry (Keithley output files) to monitor degradation and validate detector longevity under high-photon exposure.

## Repository Structure
* `/QE_Analysis`: Scripts for extracting and calculating Quantum Efficiency from hardware measurements.
* `/PDE_Analysis`: (Coming soon) Scripts for quantifying single-pixel photon detection efficiency.
* `/data`: (Directory for sample raw `.txt` or `.csv` outputs from the Keithley instruments).

## QE Analysis Pipeline

The Quantum Efficiency pipeline is broken down into modular parsing and fitting scripts. 

### 1. Spatial QE Mapping (`ReadQEscanXY` & `PlotQEscanXY`)
Used for generating 2D distributions of photocurrents across the detector surface.

* **`ReadQEscanXY.cpp`**: Parses raw output files from Keithley instruments. It sorts the telemetry into MCP current, MCP leak current, and PD (Photodiode) current, outputting a `.root` file with 2D distributions stored as `TGraphs`.
* **`PlotQEscanXY.c`**: Ingests the `TGraphs`. It performs automated fitting for the leakage current at every measurement interval and calculates the localized Quantum Efficiency using the following calibration:
  
  `QE = ((I_MCP - I_leak) / I_PD) * (CC / SR) * QE_calibrated`

  It outputs the final fits, residuals, and calculated QE maps into a finalized `.root` file for visualization.

### 2. Average QE Measurements (`ReadQEaverage` & `PlotQEaverage`)
Used for bulk characterization over specific intervals.

* **`ReadQEaverage.cpp`**: Similar to the scan parser, this script reads and sorts Keithley outputs (MCP current, MCP leak, PD current) for averaged intervals.
* **`PlotQEaverage.c`**: Fits the leakage current, calculates the average QE, and plots the associated fits and residuals to validate measurement accuracy.

## Prerequisites
* **C++** * **ROOT (CERN)**: Required for data structuring (`TGraph`) and mathematical fitting algorithms.

## Usage
To execute the QE spatial scan analysis:
```bash
# 1. Parse the raw Keithley data
root -l -q 'ReadQEscanXY.cpp("path/to/QE/Measurements")'

# 2. Fit the data and calculate QE
root -l -q 'PlotQEscanXY.c("output_TGraphs_file.root")'
```
