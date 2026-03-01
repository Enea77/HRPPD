#include <TFile.h>
#include <TString.h>
#include <TGraphErrors.h>
#include <TH1D.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <vector>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cmath>

void PlotScanPDE(TString dir = "July16_PDE/", TString label = "_July16_Int9p95_16chn") {
    double x_start = 43.35;
    double step = 0.5;
    int n_files = 21;

    TString y_str = "Y70p35";

    std::vector<float> x_values;
    std::vector<float> adc_peak_entries;
    std::vector<float> adc_trg_entries;
    std::vector<float> adc_peak_means;
    std::vector<float> qdc_peak_means;
    std::vector<float> effs;
    std::vector<float> effs_err;
    std::vector<float> X_err;

    for (int i = 0; i < n_files; ++i) {
        double x_val = x_start - i * step;

        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2) << x_val;
        std::string x_str = oss.str();
        std::replace(x_str.begin(), x_str.end(), '.', 'p');
        if (x_val < 34) label = "_July17_Int9p95_16chn";

        TString filename = TString::Format("%sadc_histos_scan%s_X%s_%s.root",dir.Data(), label.Data(), x_str.c_str(), y_str.Data());
        TFile* file = TFile::Open(filename);

        if (!file || file->IsZombie()) {
            std::cerr << "Error opening file: " << filename << std::endl;
            continue;
        }

        TH1D* h_adc_peak = (TH1D*)file->Get("tot_adc_peak");
        TH1D* h_adc_trg = (TH1D*)file->Get("tot_adc_trg");
        TH1D* h_qdc_peak = (TH1D*)file->Get("tot_qdc_peak");

        if (!h_adc_peak || !h_adc_trg || !h_qdc_peak) {
            std::cerr << "Missing histogram in file: " << filename << std::endl;
            file->Close();
            delete file;
            continue;
        }

        x_values.push_back(x_val);
        adc_peak_entries.push_back(h_adc_peak->GetEntries());
        adc_trg_entries.push_back(h_adc_trg->GetEntries());
        adc_peak_means.push_back(h_adc_peak->GetMean());
        qdc_peak_means.push_back(h_qdc_peak->GetMean());

        float eff = h_adc_peak->GetEntries() / h_adc_trg->GetEntries();
        float N = h_adc_trg->GetEntries();
        effs.push_back(eff * 100);
        effs_err.push_back(100 * sqrt((1 - eff) * eff / N));
        X_err.push_back(0);

        printf("Point %.2f : Eff= %.3f; ADC= %.3f; QDC= %.3f \n", x_val, eff * 100.0, h_adc_peak->GetMean(), h_qdc_peak->GetMean());

        file->Close();
        delete file;
    }

    // ROOT file graph
    TGraphErrors* g1 = new TGraphErrors(x_values.size(), &x_values[0], &effs[0], &X_err[0], &effs_err[0]);
    g1->SetTitle(Form("Efficiency vs X %s;X [mm];Eff [%%]",label.Data()));
    g1->SetMarkerStyle(20);
    g1->SetMarkerColor(kBlue);
    g1->SetLineColor(kBlue);

    TCanvas* c1 = new TCanvas("c1", "Efficiency Comparison", 800, 600); c1->SetGrid();
    g1->GetXaxis()->SetLimits(33.2, 43.6); // Adjust this to your desired full range

    g1->Draw("APL");
    //c1->SaveAs(Form("Plots/PDEscan%s.png",label.Data()));

    label = "_July17_Int9p95_16chn";
    x_values = {42.35,38.85,34.35,33.85,33.35};
    adc_peak_entries.clear();
    adc_trg_entries.clear();
    adc_peak_means.clear();
    qdc_peak_means.clear();
    effs.clear();
    effs_err.clear();
    X_err.clear();

    for (auto x_val : x_values) {

        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2) << x_val;
        std::string x_str = oss.str();
        std::replace(x_str.begin(), x_str.end(), '.', 'p');

        TString filename = TString::Format("%sadc_histos_scan%s_X%s_%s.root",dir.Data(), label.Data(), x_str.c_str(), y_str.Data());
        TFile* file = TFile::Open(filename);

        if (!file || file->IsZombie()) {
            std::cerr << "Error opening file: " << filename << std::endl;
            continue;
        }

        TH1D* h_adc_peak = (TH1D*)file->Get("tot_adc_peak");
        TH1D* h_adc_trg = (TH1D*)file->Get("tot_adc_trg");
        TH1D* h_qdc_peak = (TH1D*)file->Get("tot_qdc_peak");

        if (!h_adc_peak || !h_adc_trg || !h_qdc_peak) {
            std::cerr << "Missing histogram in file: " << filename << std::endl;
            file->Close();
            delete file;
            continue;
        }

        adc_peak_entries.push_back(h_adc_peak->GetEntries());
        adc_trg_entries.push_back(h_adc_trg->GetEntries());
        adc_peak_means.push_back(h_adc_peak->GetMean());
        qdc_peak_means.push_back(h_qdc_peak->GetMean());

        float eff = h_adc_peak->GetEntries() / h_adc_trg->GetEntries();
        float N = h_adc_trg->GetEntries();
        effs.push_back(eff * 100);
        effs_err.push_back(100 * sqrt((1 - eff) * eff / N));
        X_err.push_back(0);

        printf("Point %.2f : Eff= %.3f; ADC= %.3f; QDC= %.3f \n", x_val, eff * 100.0, h_adc_peak->GetMean(), h_qdc_peak->GetMean());

        file->Close();
        delete file;
    }

    // ROOT file graph
    TGraphErrors* g2 = new TGraphErrors(x_values.size(), &x_values[0], &effs[0], &X_err[0], &effs_err[0]);
    g2->SetTitle(Form("Efficiency vs X %s;X [mm];Eff [%%]",label.Data()));
    g2->SetMarkerStyle(20);
    g2->SetMarkerColor(kRed);
    g2->SetLineColor(kRed);

    c1->cd();
    g2->Draw("PLsame");


    
    /*
    // Manual overlay data
    const int n_overlay = 21;
    float Xs[n_overlay], eff_manual[n_overlay], Xs_err[n_overlay], eff_manual_err[n_overlay];
    float detected[n_overlay] = {
        39, 53, 53, 91, 168, 365, 2296, 7020, 11250, 9417,
        12200, 11833, 11917, 10531, 9689, 7379, 3201, 486,
        153, 70, 51
    };
    float eff_values[n_overlay] = {
        0.032, 0.044, 0.044, 0.075, 0.137, 0.302, 1.902, 6.862, 9.241, 9.298,
        9.856, 9.559, 9.789, 9.711, 9.285, 7.142, 3.068, 0.424, 0.142, 0.068, 0.0498
    };

    for (int i = 0; i < n_overlay; ++i) {
        if (i==10) continue;
        Xs[i] = 47.12 - i * 0.5;
        Xs_err[i] = 0;
        eff_manual[i] = eff_values[i];
        float eff_frac = eff_manual[i] / 100.0;
        float N = detected[i] / eff_frac;
        eff_manual_err[i] = 100.0 * sqrt((1 - eff_frac) * eff_frac / N);
    }

    TGraphErrors* g_overlay = new TGraphErrors(n_overlay-1, Xs, eff_manual, Xs_err, eff_manual_err);
    g_overlay->SetMarkerStyle(24);
    g_overlay->SetMarkerColor(kRed + 1);
    g_overlay->SetLineColor(kRed + 1);

    // Plot
    TCanvas* c1 = new TCanvas("c1", "Efficiency Comparison", 800, 600); c1->SetGrid();
    g1->Draw("APL");
    g_overlay->Draw("P SAME");

    auto legend = new TLegend(0.65, 0.73, 0.88, 0.88);
    legend->AddEntry(g1, "10k events", "lp");
    legend->AddEntry(g_overlay, "100k events", "lp");
    legend->SetLineColor(0);
    legend->Draw();

    

    //c1->SaveAs("Plots/PDE_scan.png");

    //return;

    // --- Zoomed-in canvas ---
    TCanvas* c2 = new TCanvas("c2", "Zoomed Efficiency", 800, 600); c2->SetGrid();
    g1->GetXaxis()->SetLimits(39.9, 43.6);  // X range
    g1->GetYaxis()->SetRangeUser(8.5, 10.5);  // Y range
    g1->Draw("APL");
    g_overlay->Draw("P SAME");
    legend->Draw();


    //c2->SaveAs("Plots/PDE_scan_peak.png");*/

}
