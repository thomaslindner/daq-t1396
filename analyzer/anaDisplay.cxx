#include <stdio.h>
#include <iostream>

#include "TRootanaDisplay.hxx"
#include "TH1D.h"
#include "TV792Data.hxx"

#include "TFancyHistogramCanvas.hxx"
#include "TInterestingEventManager.hxx"


#include "TAnaManager.hxx"
#include "TMulticanvas.h"
#include "TTRB3Data.hxx"

class MyTestLoop: public TRootanaDisplay { 

public:
	
  // An analysis manager.  Define and fill histograms in 
  // analysis manager.
  TAnaManager *anaManager;

  MyTestLoop() {
    SetOutputFilename("example_output");
    DisableRootOutput(false);
    anaManager = new TAnaManager();
    // Number of events to skip before plotting one.
    //SetNumberSkipEvent(10);
    // Choose to use functionality to update after X seconds
    SetOnlineUpdatingBasedSeconds();
    // Uncomment this to enable the 'interesting event' functionality.
    //iem_t::instance()->Enable();
  }

  void AddAllCanvases(){

    // Set up tabbed canvases
        
    if(anaManager->HaveV792Histograms()) 
      AddSingleCanvas(new TFancyHistogramCanvas(anaManager->GetV792Histograms(),"V792"));
    
    if(anaManager->HaveV1190Histograms()) 
      AddSingleCanvas(new TFancyHistogramCanvas(anaManager->GetV1190Histograms(),"V1190"));
    
    if(anaManager->HaveL2249Histograms()) 
      AddSingleCanvas(new TFancyHistogramCanvas(anaManager->GetL2249Histograms(),"L2249"));
    
    if(anaManager->HaveAgilentistograms()) 
      AddSingleCanvas(new TFancyHistogramCanvas(anaManager->GetAgilentistograms(),"AGILENT"));
    
    if(anaManager->HaveV1720Histograms()) 
      AddSingleCanvas(new TFancyHistogramCanvas(anaManager->GetV1720Histograms(),"V1720 Waveforms"));

    if(anaManager->HaveV1720Correlations()) 
      AddSingleCanvas(new TFancyHistogramCanvas(anaManager->GetV1720Correlations(),"V1720 Correlations"));
    
    if(anaManager->HaveV1730DPPistograms()) 
      AddSingleCanvas(new TFancyHistogramCanvas(anaManager->GetV1730DPPistograms(),"V1730 Waveforms"));
    
    if(anaManager->HaveV1730Rawistograms()) 
      AddSingleCanvas(new TFancyHistogramCanvas(anaManager->GetV1730Rawistograms(),"V1730 Waveforms"));
    
    if(anaManager->HaveDT724Histograms()) 
      AddSingleCanvas(new TFancyHistogramCanvas(anaManager->GetDT724Histograms(),"DT724 Waveforms"));
    
    if(anaManager->HaveTRB3Histograms()){
      AddSingleCanvas(new TFancyHistogramCanvas(anaManager->GetTRB3Histograms(),"TRB3"));
      AddSingleCanvas(new TFancyHistogramCanvas(anaManager->GetTRB3DiffHistograms(),"TRB3 Diff"));
    }

    if(anaManager->HaveCamacADCHistograms()){
      AddSingleCanvas(new TFancyHistogramCanvas(anaManager->GetCamacADCHistograms(),"CAMAC ADC"));
      // AddSingleCanvas(new TFancyHistogramCanvas(anaManager->GetCamacADCDifferences(),"CAMAC Comparisons"));
      TMulticanvas *canvas = new TMulticanvas("Gas Cherenkov Comparisons");
      canvas->AddHisto2D(anaManager->adcComparison[0],0);
      canvas->AddHisto2D(anaManager->adcComparison[1],1);
      canvas->AddHisto2D(anaManager->adcComparison[2],2);
      AddSingleCanvas(canvas);

      TMulticanvas *canvas2 = new TMulticanvas("Aerogel Cherenkov vs Lead Comparisons");
      canvas2->AddHisto2D(anaManager->adcComparison[3],0);
      canvas2->AddHisto2D(anaManager->adcComparison[4],1);
      canvas2->AddHisto2D(anaManager->adcComparison[5],2);
      canvas2->AddHisto2D(anaManager->adcComparison[6],3);
      AddSingleCanvas(canvas2);

      TMulticanvas *canvas3 = new TMulticanvas("Aerogel vs Aerogel Comparisons");
      canvas3->AddHisto2D(anaManager->adcComparison[8],0);
      canvas3->AddHisto2D(anaManager->adcComparison[9],1);
      canvas3->AddHisto2D(anaManager->adcComparison[10],2);
      AddSingleCanvas(canvas3);

    }
    
    SetDisplayName("Example Display");
  };

  virtual ~MyTestLoop() {};

  void BeginRun(int transition,int run,int time) {
    std::cout << "User BOR method" << std::endl;
    anaManager->BeginRun(transition, run, time);
  }

  void EndRun(int transition,int run,int time) {
    std::cout << "User EOR method" << std::endl;
    anaManager->EndRun(transition, run, time);
  }

  void ResetHistograms(){}

  void UpdateHistograms(TDataContainer& dataContainer){
    anaManager->ProcessMidasEvent(dataContainer);
  }

  void PlotCanvas(TDataContainer& dataContainer){}


}; 






int main(int argc, char *argv[])
{
  MyTestLoop::CreateSingleton<MyTestLoop>();  
  return MyTestLoop::Get().ExecuteLoop(argc, argv);
}

