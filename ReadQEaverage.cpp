#include <TString.h>
#include <TSystem.h>
#include <TROOT.h>
#include <TFile.h>
#include <TGraphErrors.h>
#include <TGraph2D.h>
#include <TSystemDirectory.h>
#include <TSystemFile.h>
#include <TList.h>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <cmath>
#include <algorithm>

// Helper: convert HH:MM:SS to minutes
float TimeStringToMinutes(const TString& timeStr) {
    int h, m, s, y, mth, day;
    sscanf(timeStr.Data(), "%d-%d-%d %d:%d:%d", &y, &mth, &day, &h, &m, &s);
    return day * 24.0f*60.0f + h * 60.0f + m + s / 60.0f;
}

struct DataPoint {
    float time;
    double mean;
    double error;
};

void ReadQEaverage(TString dirPath = "M3/AGE_D1B/QEavg") { //M2/REF_A1T/QEavg
    
    float tmin = 999999;

    std::vector<TString> fileNames;
    TSystemDirectory dir("dir", dirPath);
    TList* files = dir.GetListOfFiles();
    if (!files) {
        std::cerr << "Could not open directory " << dirPath << std::endl;
        return;
    }

    TIter next(files);
    TSystemFile* file;
    while ((file = (TSystemFile*)next())) {
        TString fname = file->GetName();
        if (!file->IsDirectory() && fname.EndsWith(".dat")) {
            fileNames.push_back(fname);
        }
    }

    // Containers for data
    std::vector<DataPoint> dataMCP;
    std::vector<DataPoint> dataPD;
    std::vector<DataPoint> dataLeak;

    for (const TString& filename : fileNames) {
        TString fullPath = dirPath + "/" + filename;
        std::ifstream infile(fullPath.Data());
        if (!infile.is_open()) {
            std::cerr << "Could not open file: " << fullPath << std::endl;
            continue;
        }

        for (int i = 0; i < 5000; i++) {
            std::vector<float> values;
            std::vector<TString> times;
            std::string line;

            for (int j = 0; j < 10; j++) {
                if (!std::getline(infile, line)) break;
                std::istringstream iss(line);
                float value;
                std::string date,time, tz;

                if (!(iss >> value >> date >> time >> tz)) {
                    std::cerr << "Error parsing line in " << filename << ": " << line << std::endl;
                    continue;
                }
                value = fabs(value) * 1e9;
                values.push_back(value);
                times.push_back(Form("%s %s",date.c_str(),time.c_str()));
            }

            if (values.empty()) break;

            double sum = 0;
            for (auto v : values) sum += v;
            double mean = sum / values.size();

            double sq_sum = 0;
            for (auto v : values) sq_sum += (v - mean) * (v - mean);
            double stdev = (values.size() > 1) ? std::sqrt(sq_sum / (values.size() - 1)) : 0;
            double statError = stdev / std::sqrt(values.size());

            float t0 = TimeStringToMinutes(times[0]);
            tmin = TMath::Min(tmin, t0);

            DataPoint dp{t0, mean, statError};
            
            if (filename.Contains("MCP")) dataMCP.push_back(dp);
            else if (filename.Contains("PD")) dataPD.push_back(dp);
            if (filename.Contains("MCP_leak")) dataLeak.push_back(dp);

            //std::cout << "i " << i << ": mean=" << mean << ", stderr=" << statError << ", time(min)=" << t0 << std::endl;
        }
    }

    for (auto& dp : dataMCP) dp.time -= tmin;
    for (auto& dp : dataPD)  dp.time -= tmin;
    for (auto& dp : dataLeak) dp.time -= tmin;

    // Sort the data by time
    std::sort(dataMCP.begin(), dataMCP.end(), [](const DataPoint& a, const DataPoint& b) {
        return a.time < b.time;
    });

    std::sort(dataPD.begin(), dataPD.end(), [](const DataPoint& a, const DataPoint& b) {
        return a.time < b.time;
    });

    std::sort(dataLeak.begin(), dataLeak.end(), [](const DataPoint& a, const DataPoint& b) {
        return a.time < b.time;
    });

    // Fill graphs
    TGraphErrors* graphMCP = new TGraphErrors();
    TGraphErrors* graphPD = new TGraphErrors();
    TGraphErrors* graph_leak = new TGraphErrors();
    TGraph* graph_start_points = new TGraph(); int iStat_Points=0;
    

    for (size_t i = 0; i < dataMCP.size(); ++i) {
        graphMCP->SetPoint(i, dataMCP[i].time, dataMCP[i].mean);
        graphMCP->SetPointError(i, 0, dataMCP[i].error);
        if (i>0){
            if (dataMCP[i-1].mean < 240 && dataMCP[i].mean > 240){
                graph_start_points->SetPoint(iStat_Points, dataMCP[i].time, i);
                iStat_Points++;
            }
        }
    }

    for (size_t i = 0; i < dataPD.size(); ++i) {
        graphPD->SetPoint(i, dataPD[i].time, dataPD[i].mean);
        graphPD->SetPointError(i, 0, dataPD[i].error);
    }

    for (size_t i = 0; i < dataLeak.size(); ++i) {
        graph_leak->SetPoint(i, dataLeak[i].time, dataLeak[i].mean);
        graph_leak->SetPointError(i, 0, dataLeak[i].error);
    }

    // Plot and save
    graphMCP->SetTitle(Form("Current scan %s %s;Time (minutes);Current", "MCP", dirPath.Data())); graphMCP->SetMarkerStyle(20);
    graphPD->SetTitle(Form("Current scan %s %s;Time (minutes);Current", "PD", dirPath.Data())); graphPD->SetMarkerStyle(20);
    graph_leak->SetTitle(Form("Current scan %s %s;Time (minutes);Current", "MCP Leak", dirPath.Data())); graph_leak->SetMarkerStyle(20);
    graph_start_points->SetTitle(Form("Starting time and index of each measurement after leak;time [min];Index of GraphMCP")); graph_start_points->SetMarkerStyle(20);
    graph_start_points->Draw("AP");
// Create canvas with two pads
TCanvas* c1 = new TCanvas("c1", "QE Scan Results", 1800, 600);
c1->Divide(3, 1); // two pads side by side

// Draw graph1D on left pad
c1->cd(1);
graphMCP->Draw("AP");
c1->cd(2); graphPD->Draw("AP");
c1->cd(3); graph_leak->Draw("AP");
c1->SaveAs(Form("Plots/MCP measured %s.png",dirPath.Data()));

// Save everything to file
TFile* outFile = new TFile(Form("%s.root", dirPath.Data()), "RECREATE");
graphMCP->Write("graphMCP");
graph_leak->Write("graph_leak");
graphPD->Write("graphPD");
c1->Write("Canvas");
graph_start_points->Write("graph_start_points");
outFile->Close();


    printf("MCP points: 1D=%d /n", graphMCP->GetN());
    printf("PD points: 1D=%d /n", graphPD->GetN());
    printf("Leak points: 1D=%d /n", graph_leak->GetN());
    printf("Time zero-shift applied: tmin = %f minutes/n", tmin);
    printf("Starting points: %d /n", graph_start_points->GetN());
}
