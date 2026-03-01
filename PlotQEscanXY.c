#include <TString.h>
#include <TSystem.h>
#include <TROOT.h>
#include <TFile.h>
#include <TTree.h>
#include <TGraphErrors.h>
#include <vector>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cmath>

TH2D* Graph2DToHist(TGraph2D* g2) {
    int n = g2->GetN();
    double* x = g2->GetX();
    double* y = g2->GetY();
    double* z = g2->GetZ();

    // make histogram with N bins = number of unique x and y values
    // (this assumes your data is on a grid!)
    int nx = TMath::Sqrt(n);
    int ny = nx;

    double xmin = TMath::MinElement(n, x);
    double xmax = TMath::MaxElement(n, x);
    double ymin = TMath::MinElement(n, y);
    double ymax = TMath::MaxElement(n, y);

    TH2D* h2 = new TH2D(Form("%s_h", g2->GetName()), g2->GetTitle(),
                        nx, xmin - 1e-6, xmax + 1e-6,
                        ny, ymin - 1e-6, ymax + 1e-6);


    for (int i = 0; i < n; i++) {
        h2->Fill(x[i], y[i], z[i]);
    }

    return h2;
}

TPad* AddZoomPad(
    TGraphErrors* graph_source,  // graph to zoom
    double x_min, double x_max,  // zoom x range
    double y_min, double y_max,  // zoom y range
    const std::vector<TObject*>& overlays = {} ,
    TString Position = "top-right",
    float marker_size = 0.3, int ndiv = 505   // style options
);


void PlotQEscanXY(float leak_timing = 10, float data_timing = 30, float C_factor =   48.459) { //timing is in min   QE_PD=0.565  C_factor before July24 = 43.275, After July 24 = 48.459

    TString filepath = "M3/AGE_D1B/QE";
    TString filename = filepath;
    filename.ReplaceAll("/", "_");
    //TFile* f = TFile::Open(Form("Leakscan_leaktime%.0f_datatime%.0f_%s.root",leak_timing,data_timing), "UPDATE"); //MCP_lOFF_50V_20250707_182
    TFile* f = TFile::Open(Form("%s.root",filepath.Data()), "READ"); //MCP_lOFF_50V_20250707_182

    TGraphErrors* graphMCP = (TGraphErrors*) f->Get("graphMCP");
    TGraphErrors* graphPD = (TGraphErrors*) f->Get("graphPD");
    TGraph2D* graph2D_MCP = (TGraph2D*) f->Get("graph2D_MCP");
    TGraphErrors* graph_leak = (TGraphErrors*) f->Get("graph_leak");
    TGraph* graph_start_points = (TGraph*) f->Get("graph_start_points");

    //ScaleGraphErrorsY(graphMCP,pow(10,9));
    //ScaleGraphErrorsY(graph_leak,pow(10,9));

    graph_leak->SetTitle("Leak;Time [min]; |I_{MCP}| [nA]");
    graph_leak->SetMarkerStyle(21);
    graph_leak->SetMarkerSize(0.5);
    //graphMCP->SetMinimum(200);

    TGraphErrors* graphCorrected = new TGraphErrors(); int iCorrected=0;
    graphCorrected->SetTitle(Form("MCP - Fit (%.0f min leak, %.0f min light);Time [min];|Current| [nA]",leak_timing,data_timing));
    graphCorrected->SetMarkerStyle(21);
    graphCorrected->SetLineColor(kBlue);
    graphCorrected->SetMarkerColor(kBlue);

    TGraphErrors* graphOffset = new TGraphErrors(); int iOffset=0;
    graphOffset->SetTitle(Form("Leak Measured - Fit (%.0f min leak, %.0f min light);Time [min];|Current| [nA]",leak_timing,data_timing));
    graphOffset->SetMarkerStyle(21);
    graphOffset->SetLineColor(7);
    graphOffset->SetMarkerColor(7);

    TGraph2D* graph2D_MCP_Corrected = new TGraph2D();

    // Output QE graphs
    TGraphErrors* graph_QE = new TGraphErrors();
    graph_QE->SetMarkerStyle(21);
    graph_QE->SetTitle(Form("%s;Time [min];QE [%%]",filepath.Data()));
    TGraph2D* graph2D_QE = new TGraph2D();
    graph2D_QE->SetTitle(Form("%s;X [mm];Y [mm]",filepath.Data()));


        // ---- ZOOM REGION ----
    double x_min = 100;// 600;  
    double x_max = 200; //800;
    double y_min = 200.8; //204; //198.8; 
    double y_max = 201.15; //208;   //199.8;
    graph_leak->Fit("pol0","","",x_min,x_max);
    double avg_window = graph_leak->GetFunction("pol0")->GetParameter(0);
    y_min = avg_window*0.9995;
    y_max = avg_window*1.0005;

    gStyle->SetOptStat(0);
    TCanvas* c = new TCanvas("c","c",1000,800);
    c->SetLeftMargin(0.15);
    //graphMCP->Draw("AP");
    //graph_leak->SetMarkerColor(kRed);
    graph_leak->SetMarkerStyle(8);
    graph_leak->SetMarkerSize(0.5);
    graph_leak->Draw("AP");
    TF1* fit_leak;
    std::vector<TObject*> overlays = {};
    graph_leak->Fit("pol2","Q","",0,50);
    fit_leak = graph_leak->GetFunction("pol2");

    float lowedge, upedge;
    int phase_length = 6 * (leak_timing + data_timing); // total length of one leak-data cycle in 10sec points

    int nPoints = graphMCP->GetN();
    cout<<nPoints<<endl;
    int iLeak=0;
    int iStart=0;
    //Loop over graph points, use t-told to find where the data is and then fit graph_leak +- 10 min from the data
    for (int i = 0; i < nPoints; i++) {
        double t=0, MCP=0, t_leak=0, MCP_leak=0, t_start, index_start;
        graphMCP->GetPoint(i, t, MCP); 
        graph_leak->GetPoint(iLeak, t_leak, MCP_leak); 
        graph_start_points->GetPoint(iStart, t_start, index_start);

        if (i == index_start){

            lowedge = t-10;
            upedge = t + 40; 
            //printf("i=%.1i t=%.2f lowed=%.2f uped=%.2f \n",i,t,lowedge,upedge);
            //if (upedge*6 > nPoints) break;
            graph_leak->Fit("pol2","Q","",lowedge,upedge);
            fit_leak = graph_leak->GetFunction("pol2");
            TF1* new_fit = (TF1*) fit_leak->Clone(Form("fit%.1i",i));
            new_fit->SetLineColor(kRed);
            new_fit->Draw("same");
            if ((lowedge>x_min && lowedge<x_max) || (upedge>x_min && upedge<x_max)){
                overlays.push_back(new_fit);
                y_min = TMath::Min(y_min, new_fit->Eval(x_min));
                y_min = TMath::Min(y_min, new_fit->Eval(x_max));
                y_max = TMath::Max(y_max, new_fit->Eval(x_max));
                y_max = TMath::Max(y_max, new_fit->Eval(x_min));
            }

            iStart++;
        }
        
        double leak_fit = fit_leak->Eval(t);
        double MCP_corr = MCP - leak_fit;
        double MCP_err = graphMCP->GetErrorY(i); // preserve original error     
        
        if (MCP != MCP_leak) {
            float x = graph2D_MCP->GetX()[iCorrected];
            float y = graph2D_MCP->GetY()[iCorrected];
            double MCP_corr_err = sqrt(MCP_err*MCP_err+0.01);
            graph2D_MCP_Corrected->SetPoint(iCorrected,x,y,MCP_corr);
            graphCorrected->SetPoint(iCorrected, t, MCP_corr);
            graphCorrected->SetPointError(iCorrected, 0, MCP_corr_err); 
            
            double I_PD;
            graphPD->GetPoint(iCorrected, t, I_PD);
            double PD_err = graphPD->GetErrorY(iCorrected);
            float QE = 0;
            if (I_PD > 1e-9) QE = MCP_corr * C_factor / I_PD;
            float QE_err = QE*sqrt(pow(PD_err/I_PD,2)+pow(MCP_corr_err/MCP_corr,2));

            graph_QE->SetPoint(iCorrected, t, QE);
            graph_QE->SetPointError(iCorrected, 0, QE_err);

            graph2D_QE->SetPoint(iCorrected, x, y, QE);
            iCorrected++;

            //printf("i=%.1i t=%.2f MCP=%.2f MCP_corr=%.2f PD=%.2f QE=%.2f \n",i,t,MCP,MCP_corr,I_PD,QE);

        }    
        else{
            graphOffset->SetPoint(iOffset, t, MCP_corr);
            graphOffset->SetPointError(iOffset, 0, MCP_err); 
            iOffset++;
            iLeak++;

            printf("i=%.1i t=%.2f MCP=%.2f MCP_leak=%.2f \n",i,t,MCP,MCP_leak);

        } 

    }

    TLegend* legend = new TLegend(0.18,0.15,0.5,0.28);
    legend->AddEntry(graph_leak, "Leak", "P");
    legend->AddEntry(fit_leak, "Fit (pol2)", "L");
    legend->SetLineColor(0);
    legend->Draw("same");

    // Draw rectangle to highlight zoom region
    TBox* box = new TBox(x_min, y_min, x_max, y_max);
    box->SetLineWidth(1);
    box->SetFillStyle(0);
    //box->Draw("same");

    

    // Add inset zoom pad
    //TPad* inset = AddZoomPad(graph_leak, x_min, x_max, y_min, y_max,overlays,"top-right"); //0.4,0.55

    c->cd();
    double x_max_axis = graphMCP->GetXaxis()->GetXmax();
    double y_max_axis = graphMCP->GetYaxis()->GetXmax();
    double y_min_axis = graphMCP->GetYaxis()->GetXmin();

    // Draw arrow pointing to top-left corner
    TArrow* arrow = new TArrow(x_max, y_max, x_max_axis * 0.75, y_min_axis + (y_max_axis-y_min_axis) * 0.25, 0.02, "|>");
    arrow->SetLineWidth(1);
    //arrow->Draw();

    c->SaveAs(Form("Plots/Leak Fit leaktime%.0f datatime%.0f %s.png",leak_timing,data_timing,filename.Data()));

    TCanvas* c2 = new TCanvas("c2","c2",1000,500); c2->Divide(2,1);    
    c2->cd(1); c2->cd(1)->SetGrid(); c2->cd(1)->SetLeftMargin(0.15);
    graphCorrected->SetTitle("MCP Signal - Fit; Time [min]; #DeltaI [nA]");
    graphCorrected->Draw("AP");
    c2->Update();

    c2->cd(2); c2->cd(2)->SetGrid(); c2->cd(2)->SetLeftMargin(0.15);
    graphOffset->SetTitle("Leak Measured - Fit; Time [min]; #DeltaI [nA]");
    graphOffset->Draw("AP");

    TH2D* Hist2D_QE = Graph2DToHist(graph2D_QE);

    //return;

    TFile* f_out = TFile::Open(Form("%s_corrected.root",filepath.Data()), "RECREATE");
    
    graphMCP->Write("graphMCP");
    graph_leak->Write("graph_leak");
    graph2D_MCP->Write("graph2D_MCP");
    graph2D_MCP_Corrected->Write("graph2D_MCP_Corrected");
    graphCorrected->Write("graphCorrected");
    graphOffset->Write("graphOffset");
    graph_QE->Write("Graph_QE");
    graph2D_QE->Write("Graph2D_QE");
    Hist2D_QE->Write("Hist2D_QE");

    c->Write();
    c2->Write();

    c2->SaveAs(Form("Plots/Leak Fit Subtraction leaktime%.0f datatime%.0f %s.png",leak_timing,data_timing,filename.Data()));

    TCanvas* c_QE = new TCanvas("c_QE", "QE Measurement", 1200, 600);
    c_QE->Divide(2, 1);

    c_QE->cd(1);
    gPad->SetLeftMargin(0.15);
    graph_QE->SetTitle(Form("QE %s;Time (min);QE (%%)", filepath.Data()));
    graph_QE->Draw("AP");

    c_QE->cd(2);
    gPad->SetRightMargin(0.15);
    Hist2D_QE->Draw("colz");

    c_QE->SaveAs(Form("Plots/QE Measurement %s.png",filename.Data()));
    c_QE->Write();

}

TPad* AddZoomPad(
    TGraphErrors* graph_source,
    double x_min, double x_max,
    double y_min, double y_max,
    const std::vector<TObject*>& overlays = {},
    TString Position = "bottom-right",
    float marker_size = 0.3, int ndiv = 505) 
{
    float pad_x1, pad_x2, pad_y1, pad_y2;
    if (Position == "bottom-right"){
        pad_x1 = 0.55;
        pad_y1 = 0.15;
        pad_x2 = 0.9;
        pad_y2 = 0.4;
    }

    if (Position == "top-right"){
        pad_x1 = 0.55;
        pad_y1 = 0.55;
        pad_x2 = 0.9;
        pad_y2 = 0.9;
    }

    // Create the inset pad
    TPad* pad_inset = new TPad("pad_inset", "Zoom", pad_x1, pad_y1, pad_x2, pad_y2);
    pad_inset->SetFillStyle(0);
    pad_inset->SetFrameLineWidth(1);
    pad_inset->SetGrid();
    pad_inset->SetLeftMargin(0.2);
    pad_inset->Draw();
    pad_inset->cd();

    // Clone the source graph
    TGraphErrors* graph_zoom = (TGraphErrors*) graph_source->Clone("graph_zoom");
    graph_zoom->GetXaxis()->SetRangeUser(x_min, x_max);
    graph_zoom->GetYaxis()->SetRangeUser(y_min, y_max);
    graph_zoom->SetMarkerSize(marker_size);
    graph_zoom->SetTitle(";;");  // no axis titles
    graph_zoom->Draw("AP");

    // Style axes
    graph_zoom->GetXaxis()->SetLabelSize(0.1);
    graph_zoom->GetYaxis()->SetLabelSize(0.1);
    graph_zoom->GetXaxis()->SetNdivisions(ndiv);
    graph_zoom->GetYaxis()->SetNdivisions(ndiv);

    for (TObject* obj : overlays) {
        if (obj->InheritsFrom(TGraph::Class())) {
        obj->Draw("P same");  // force points-only for TGraph / TGraphErrors
        } else {
            obj->Draw("same");    // default for TF1, TLine, etc.
        }
    }

    return pad_inset;
}


