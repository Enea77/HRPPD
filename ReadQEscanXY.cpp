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

// Structs to hold data
struct DataPoint {
    float time;
    double mean;
    double error;
    double x, y;  // for graph2D
};

struct DataPointLeak {
    float time;
    double mean;
    double error;
};

void ReadQEscanXY(TString dirPath = "M3/AGE_D1B/QE") {
    
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
    std::vector<DataPointLeak> dataLeak;

    for (const TString& filename : fileNames) {
        TString fullPath = dirPath + "/" + filename;
        std::ifstream infile(fullPath.Data());
        if (!infile.is_open()) {
            std::cerr << "Could not open file: " << fullPath << std::endl;
            continue;
        }

        bool hasXY = false;
        double x = 0, y = 0;
        Ssiz_t xpos = filename.Index("X");
        Ssiz_t ypos = filename.Index("Y");

        if (xpos != kNPOS && ypos != kNPOS && ypos > xpos) {
            TString xStr = filename(xpos + 1, ypos - xpos - 2);
            Ssiz_t nextUnderscore = filename.Index("_", ypos + 1);
            TString yStr = filename(ypos + 1, nextUnderscore - ypos - 1);

            xStr.ReplaceAll("p", ".");
            yStr.ReplaceAll("p", ".");
            x = xStr.Atof();
            y = yStr.Atof();
            hasXY = true;
        }

        if (hasXY) {
            std::vector<float> values;
            std::vector<TString> times;
            std::string line;

            while (std::getline(infile, line)) {
                std::istringstream iss(line);
                float value;
                std::string time, date, tz;
                if (!(iss >> value >> date >> time >> tz)) {
                    std::cerr << "Error parsing line in " << filename << ": " << line << std::endl;
                    continue;
                }

                value = fabs(value) * 1e9;
                values.push_back(value);
                times.push_back(Form("%s %s",date.c_str(),time.c_str()));
            }
            infile.close();

            if (values.empty()) {
                std::cout << "No data in file: " << filename << std::endl;
                continue;
            }

            double sum = 0;
            for (float v : values) sum += v;
            double mean = sum / values.size();

            double sq_sum = 0;
            for (float v : values) sq_sum += (v - mean) * (v - mean);
            double stdev = (values.size() > 1) ? std::sqrt(sq_sum / (values.size() - 1)) : 0;
            double statError = stdev / std::sqrt(values.size());

            float t0 = TimeStringToMinutes(times[0]);
            //cout<<times[0]<<endl; cout<<t0<<endl; return;
            tmin = TMath::Min(tmin, t0);

            DataPoint dp{t0, mean, statError, x, y};
            if (filename.Contains("MCP")) dataMCP.push_back(dp);
            else if (filename.Contains("PD")) dataPD.push_back(dp);
            //if (t0 > 1000) std::cout << "file " << filename << ": mean=" << mean << ", stderr=" << statError << ", time(min)=" << t0 << std::endl;

        }

        else {
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

                dataLeak.push_back({t0, mean, statError});
                DataPoint dp{t0, mean, statError, 0, 0};
                dataMCP.push_back(dp);

                //std::cout << "i " << i << ": mean=" << mean << ", stderr=" << statError << ", time(min)=" << t0 << std::endl;
            }
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

    std::sort(dataLeak.begin(), dataLeak.end(), [](const DataPointLeak& a, const DataPointLeak& b) {
        return a.time < b.time;
    });

    // Fill graphs
    TGraphErrors* graphMCP = new TGraphErrors();
    TGraphErrors* graphPD = new TGraphErrors();
    TGraph2D* graph2D_MCP = new TGraph2D(); int isignal=0;
    TGraph2D* graph2D_PD = new TGraph2D();
    TGraphErrors* graph_leak = new TGraphErrors();
    TGraph* graph_start_points = new TGraph(); int iStat_Points=0;
    

    for (size_t i = 0; i < dataMCP.size(); ++i) {
        graphMCP->SetPoint(i, dataMCP[i].time, dataMCP[i].mean);
        graphMCP->SetPointError(i, 0, dataMCP[i].error);
        if (dataMCP[i].x > 0) {
            graph2D_MCP->SetPoint(isignal, dataMCP[i].x, dataMCP[i].y, dataMCP[i].mean);
            isignal++;
        }
        if (i>0){
            if (dataMCP[i-1].x == 0 && dataMCP[i].x > 0){
                graph_start_points->SetPoint(iStat_Points, dataMCP[i].time, i);
                iStat_Points++;
            }
        }
    }

    for (size_t i = 0; i < dataPD.size(); ++i) {
        graphPD->SetPoint(i, dataPD[i].time, dataPD[i].mean);
        graphPD->SetPointError(i, 0, dataPD[i].error);
        graph2D_PD->SetPoint(i, dataPD[i].x, dataPD[i].y, dataPD[i].mean);
    }

    for (size_t i = 0; i < dataLeak.size(); ++i) {
        graph_leak->SetPoint(i, dataLeak[i].time, dataLeak[i].mean);
        graph_leak->SetPointError(i, 0, dataLeak[i].error);
    }

    // Plot and save
    graphMCP->SetTitle(Form("Current scan %s %s;Time (minutes);Current", "MCP", dirPath.Data())); graphMCP->SetMarkerStyle(20);
    graphPD->SetTitle(Form("Current scan %s %s;Time (minutes);Current", "PD", dirPath.Data())); graphPD->SetMarkerStyle(20);
    graph_leak->SetTitle(Form("Current scan %s %s;Time (minutes);Current", "MCP Leak", dirPath.Data())); graph_leak->SetMarkerStyle(20);
    graph2D_MCP->SetTitle(Form("Current scan %s %s;x [mm];y [mm];Current", "MCP", dirPath.Data())); graph2D_MCP->SetMarkerStyle(20);
    graph2D_PD->SetTitle(Form("Current scan %s %s;x [mm];y [mm];Current", "PD", dirPath.Data())); graph2D_PD->SetMarkerStyle(20);
    graph_start_points->SetTitle(Form("Starting time and index of each measurement after leak;time [min];Index of GraphMCP")); graph_start_points->SetMarkerStyle(20);
    graph_start_points->Draw("AP");
// Create canvas with two pads
TCanvas* c1 = new TCanvas("c1", "QE Scan Results", 1200, 600);
c1->Divide(2, 1); // two pads side by side

// Draw graph1D on left pad
c1->cd(1);
graphMCP->Draw("AP");

// Draw graph2D on right pad
c1->cd(2);
graph2D_MCP->Draw("colz"); // "P0" preferred for 2D graphs like TGraph2D

c1->SaveAs(Form("Plots/MCP measured %s.png",dirPath.Data()));

// Create canvas with two pads
TCanvas* c2 = new TCanvas("c2", "QE Scan Results", 1200, 600);
c2->Divide(2, 1); // two pads side by side

// Draw graph1D on left pad
c2->cd(1);
graphPD->Draw("AP");

// Draw graph2D on right pad
c2->cd(2);
graph2D_PD->Draw("colz"); // "P0" preferred for 2D graphs like TGraph2D
c2->SaveAs(Form("Plots/PD measured %s.png",dirPath.Data()));

// Save everything to file
TFile* outFile = new TFile(Form("%s.root", dirPath.Data()), "RECREATE");
graphMCP->Write("graphMCP");
graph2D_MCP->Write("graph2D_MCP");
graph_leak->Write("graph_leak");
graphPD->Write("graphPD");
graph2D_PD->Write("graph2D_PD");
c1->Write("MCPCanvas");
c2->Write("PDCanvas");
graph_start_points->Write("graph_start_points");
outFile->Close();


    printf("MCP points: 1D=%d 2D=%d \n", graphMCP->GetN(), graph2D_MCP->GetN());
    printf("PD points: 1D=%d 2D=%d \n", graphPD->GetN(), graph2D_PD->GetN());
    printf("Leak points: 1D=%d \n", graph_leak->GetN());
    printf("Time zero-shift applied: tmin = %f minutes\n", tmin);
    printf("Starting points: %d \n", graph_start_points->GetN());
}
