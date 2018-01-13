#include "TAnaManager.hxx"
#include "TV1720RawData.h"

#include "TCamacADCData.hxx"

TAnaManager::TAnaManager(){

	fV792Histogram = 0;
#ifdef USE_V792
	fV792Histogram = new TV792Histograms();
	fV792Histogram->DisableAutoUpdate();  // disable auto-update.  Update histo in AnaManager.
#endif

	fV1190Histogram = 0;
#ifdef USE_V1190
	fV1190Histogram = new TV1190Histograms();
	fV1190Histogram->DisableAutoUpdate();  // disable auto-update.  Update histo in AnaManager.
#endif

	fL2249Histogram = 0;
#ifdef USE_L2249
  fL2249Histogram = new TL2249Histograms();
	fL2249Histogram->DisableAutoUpdate();  // disable auto-update.  Update histo in AnaManager.
#endif

	fAgilentHistogram = 0;
#ifdef USE_AGILENT
	fAgilentHistogram = new TAgilentHistograms();
	fAgilentHistogram->DisableAutoUpdate();  // disable auto-update.  Update histo in AnaManager.
#endif

	fV1720Waveform = 0;
#ifdef USE_V1720

	fV1720Waveform = new TV1720Waveform();
	fV1720Waveform->DisableAutoUpdate();  // disable auto-update.  Update histo in AnaManager.

        fV1720PHCompare = new TH2F("V1720PHCompare","Pulse Height: Channel 1 vs 0",100,300,700,100,300,700);
        fV1720PHCompare->SetXTitle("Channel 0");
        fV1720PHCompare->SetYTitle("Channel 1");
        fV1720TimeCompare = new TH2F("V1720TimeCompare","Time: Channel 1 vs 0",100,0,2000,100,0,2000);
        fV1720TimeCompare->SetXTitle("Channel 0");
        fV1720TimeCompare->SetYTitle("Channel 1");
#endif

	fV1720Correlations = 0;
#ifdef USE_V1720
	fV1720Correlations = new TV1720Correlations();
	fV1720Correlations->DisableAutoUpdate();  // disable auto-update.  Update histo in AnaManager.
#endif

	fV1730DppWaveform = 0;
#ifdef USE_V1730DPP
  fV1730DppWaveform = new TV1730DppWaveform();
	fV1730DppWaveform->DisableAutoUpdate();  // disable auto-update.  Update histo in AnaManager.
#endif

	fV1730RawWaveform = 0;
#ifdef USE_V1730RAW
	fV1730RawWaveform = new TV1730RawWaveform();
	fV1730RawWaveform->DisableAutoUpdate();  // disable auto-update.  Update histo in AnaManager.
#endif

	fDT724Waveform = 0;
#ifdef USE_DT724
	fDT724Waveform = new TDT724Waveform();
	fDT724Waveform->DisableAutoUpdate();  // disable auto-update.  Update histo in AnaManager.
#endif

        fTRB3Histograms = 0;
        fTRB3DiffHistograms = 0;
#ifdef USE_TRB3
	fTRB3Histograms = new TTRB3Histograms();
	fTRB3Histograms->DisableAutoUpdate();  // disable auto-update.  Update histo in AnaManager.
        fTRB3DiffHistograms = new TTRB3DiffHistograms();
        fTRB3DiffHistograms->DisableAutoUpdate();  // disable auto-update.  Update histo in AnaManager.
#endif

#ifdef USE_CAMACADC
   fCamacADCHistograms = new TCamacADCHistograms();
   fCamacADCHistograms->DisableAutoUpdate();  // disable auto-update.  Update histo in AnaManager.
   //	fCamacADCDifferences = new TCamacADCDifferences();
   //fCamacADCDifferences->DisableAutoUpdate();  // disable auto-update.  Update histo in AnaManager.
   
   for(int i = 0; i < 16; i++){ // loop over channels    
     
     char name[100];
     char title[100];
     sprintf(name,"CamacADC_2D_%i_%i",0,i);
     
     // Delete old histograms, if we already have them
     //	  TH1D *old = (TH1D*)gDirectory->Get(name);
     //if (old){
     //  delete old;
     // }
     
     // Create new histograms
	  
     sprintf(title,"CAMAC ADC for channel=%i vs Lead Glass",i);
     if(i == 0) sprintf(title,"Gas Cherenkov1 vs Lead Glass",i);
     if(i == 1) sprintf(title,"Gas Cherenkov2 vs Lead Glass",i);
     if(i == 2) sprintf(title,"Gas Cherenkov3 vs Lead Glass",i);
     if(i == 3) sprintf(title,"TRIUMF index=1.0257 1 vs Lead Glass",i);
     if(i == 4) sprintf(title,"TRIUMF index=1.0257 2 vs Lead Glass",i);
     if(i == 5) sprintf(title,"Chiba index=1.0046 vs Lead Glass",i);
     if(i == 6) sprintf(title,"Chiba index=1.0126 vs Lead Glass",i);
     if(i == 7) sprintf(title,"Lead Glass",i);
     if(i == 8) sprintf(title,"TRIUMF PMT 2 vs TRIUMF PMT 1",i);
     if(i == 9) sprintf(title,"Chiba 1.0046 vs TRIUMF PMT 1 ",i);
     if(i == 10) sprintf(title,"Chiba 1.0126 vs TRIUMF PMT 1 ",i);
      
     adcComparison[i] = new TH2D(name,title,800,0,800,400,0,400);
     adcComparison[i]->SetYTitle("ADC value");
     adcComparison[i]->SetXTitle("Lead Glass ADC Value");
     
   }
   

#endif

};



int TAnaManager::ProcessMidasEvent(TDataContainer& dataContainer){


  if(fV792Histogram) fV792Histogram->UpdateHistograms(dataContainer); 
  if(fV1190Histogram)  fV1190Histogram->UpdateHistograms(dataContainer); 
  if(fL2249Histogram)  fL2249Histogram->UpdateHistograms(dataContainer); 
  if(fAgilentHistogram)  fAgilentHistogram->UpdateHistograms(dataContainer); 
  if(fV1720Waveform)  fV1720Waveform->UpdateHistograms(dataContainer);
  if(fV1720Correlations)  fV1720Correlations->UpdateHistograms(dataContainer); 
  if(fV1730DppWaveform)  fV1730DppWaveform->UpdateHistograms(dataContainer); 
  if(fV1730RawWaveform)  fV1730RawWaveform->UpdateHistograms(dataContainer);        
  if(fDT724Waveform)  fDT724Waveform->UpdateHistograms(dataContainer);
  if(fTRB3Histograms)  fTRB3Histograms->UpdateHistograms(dataContainer);
  if(fTRB3DiffHistograms)  fTRB3DiffHistograms->UpdateHistograms(dataContainer); 
  if(fCamacADCHistograms)  fCamacADCHistograms->UpdateHistograms(dataContainer); 
  
  TCamacData *data = dataContainer.GetEventData<TCamacData>("ADC8");
  if(!data) return 1;
  
  /// Get the Vector of ADC Measurements.
  std::vector<CamacADCEvent> measurements = data->GetMeasurements();
  for(unsigned int i = 0; i < measurements.size(); i++){ // loop over measurements
    
    // For each measurement, fill the ADC values for each channel.
    for(unsigned int ch = 0; ch < 6; ch++){
      adcComparison[ch]->Fill(measurements[i].GetADC(7),measurements[i].GetADC(ch));
    }

    adcComparison[8]->Fill(measurements[i].GetADC(3),measurements[i].GetADC(4));
    adcComparison[9]->Fill(measurements[i].GetADC(3),measurements[i].GetADC(5));
    adcComparison[10]->Fill(measurements[i].GetADC(3),measurements[i].GetADC(6));
  }
  
  
  
  return 1;
}



bool TAnaManager::HaveV792Histograms(){
	if(fV792Histogram) return true; 
	return false;
}
bool TAnaManager::HaveV1190Histograms(){
	if(fV1190Histogram) return true; 
	return false;
};
bool TAnaManager::HaveL2249Histograms(){
	if(fL2249Histogram) return true; 
	return false;
};
bool TAnaManager::HaveAgilentistograms(){
	if(fAgilentHistogram) return true; 
	return false;
};
bool TAnaManager::HaveV1720Histograms(){
	if(fV1720Waveform) return true; 
	return false;
};
bool TAnaManager::HaveV1720Correlations(){
	if(fV1720Correlations) return true; 
	return false;
};
bool TAnaManager::HaveV1730DPPistograms(){
	if(fV1730DppWaveform) return true; 
	return false;
};
bool TAnaManager::HaveV1730Rawistograms(){
	if(fV1730RawWaveform) return true; 
	return false;
};
bool TAnaManager::HaveDT724Histograms(){
	if(fDT724Waveform) return true; 
	return false;
};
bool TAnaManager::HaveTRB3Histograms(){
	if(fTRB3Histograms) return true; 
	return false;
};
bool TAnaManager::HaveCamacADCHistograms(){
	if(fCamacADCHistograms) return true; 
	return false;
};

TV792Histograms* TAnaManager::GetV792Histograms() {return fV792Histogram;}
TV1190Histograms* TAnaManager::GetV1190Histograms(){return fV1190Histogram;}
TL2249Histograms* TAnaManager::GetL2249Histograms(){return fL2249Histogram;}
TAgilentHistograms* TAnaManager::GetAgilentistograms(){return fAgilentHistogram;}
TV1720Waveform* TAnaManager::GetV1720Histograms(){return fV1720Waveform;}
TV1720Correlations* TAnaManager::GetV1720Correlations(){return fV1720Correlations;}
TV1730DppWaveform* TAnaManager::GetV1730DPPistograms(){return fV1730DppWaveform;}
TV1730RawWaveform* TAnaManager::GetV1730Rawistograms(){return fV1730RawWaveform;}
TDT724Waveform* TAnaManager::GetDT724Histograms(){return fDT724Waveform;}
TTRB3Histograms* TAnaManager::GetTRB3Histograms(){return fTRB3Histograms;}
TTRB3DiffHistograms* TAnaManager::GetTRB3DiffHistograms(){return fTRB3DiffHistograms;}
TCamacADCHistograms* TAnaManager::GetCamacADCHistograms(){return fCamacADCHistograms;}
TCamacADCDifferences* TAnaManager::GetCamacADCDifferences(){return fCamacADCDifferences;}

