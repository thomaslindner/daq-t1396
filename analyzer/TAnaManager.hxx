#ifndef TAnaManager_h
#define TAnaManager_h

// Use this list here to decide which type of equipment to use.

//#define USE_V792
//#define USE_V1190
//#define USE_L2249
//#define USE_AGILENT
//#define USE_V1720
//#define USE_V1720_CORRELATIONS
//#define USE_V1730DPP
//#define USE_V1730RAW
//#define USE_DT724
//#define USE_TRB3
#define USE_CAMACADC

#include "TV792Histogram.h"
#include "TV1190Histogram.h"
#include "TL2249Histogram.h"
#include "TAgilentHistogram.h"
#include "TV1720Waveform.h"
#include "TV1720Correlations.h"
#include "TV1730DppWaveform.h"
#include "TV1730RawWaveform.h"
#include "TDT724Waveform.h"
#include "TTRB3Histogram.hxx"
#include "TCamacADCHistogram.h"
#include "TCamacADCDifferences.h"
#include "TH2D.h"
/// This is an example of how to organize a set of different histograms
/// so that we can access the same information in a display or a batch
/// analyzer.
/// Change the set of ifdef's above to define which equipment to use.
class TAnaManager  {
public:
  TAnaManager();
  virtual ~TAnaManager(){};

  /// Processes the midas event, fills histograms, etc.
  int ProcessMidasEvent(TDataContainer& dataContainer);

  void BeginRun(int transition,int run,int time) {};
  void EndRun(int transition,int run,int time) {};


	/// Methods for determining if we have a particular set of histograms.
	bool HaveV792Histograms();
	bool HaveV1190Histograms();
	bool HaveL2249Histograms();
	bool HaveAgilentistograms();
	bool HaveV1720Histograms();
  	bool HaveV1720Correlations();
	bool HaveV1730DPPistograms();
	bool HaveV1730Rawistograms();
	bool HaveDT724Histograms();
  	bool HaveTRB3Histograms();
  	bool HaveCamacADCHistograms();

	/// Methods for getting particular set of histograms.
	TV792Histograms* GetV792Histograms();
	TV1190Histograms* GetV1190Histograms();
	TL2249Histograms* GetL2249Histograms();
	TAgilentHistograms* GetAgilentistograms();
	TV1720Waveform* GetV1720Histograms();
  	TV1720Correlations* GetV1720Correlations();
	TV1730DppWaveform* GetV1730DPPistograms();
	TV1730RawWaveform* GetV1730Rawistograms();
	TDT724Waveform* GetDT724Histograms();
        TTRB3Histograms* GetTRB3Histograms();
        TTRB3DiffHistograms* GetTRB3DiffHistograms();
        TCamacADCHistograms* GetCamacADCHistograms();
        TCamacADCDifferences* GetCamacADCDifferences();

  TH2D *adcComparison[16];

private:

	TV792Histograms *fV792Histogram;
	TV1190Histograms *fV1190Histogram;
	TL2249Histograms *fL2249Histogram;
	TAgilentHistograms *fAgilentHistogram;
	TV1720Waveform *fV1720Waveform;
  	TV1720Correlations *fV1720Correlations;
	TV1730DppWaveform *fV1730DppWaveform;
	TV1730RawWaveform *fV1730RawWaveform;
	TDT724Waveform *fDT724Waveform;
  	TTRB3Histograms *fTRB3Histograms;
    	TTRB3DiffHistograms *fTRB3DiffHistograms;
    	TCamacADCHistograms *fCamacADCHistograms;
    	TCamacADCDifferences *fCamacADCDifferences;


  // Make some cross-channel histograms
  TH2F *fV1720PHCompare;
  TH2F *fV1720TimeCompare;
};



#endif


