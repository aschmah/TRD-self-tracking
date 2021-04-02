#ifndef ALI_MAKE_TRACKLETS_FROM_DIGITS_H
#define ALI_MAKE_TRACKLETS_FROM_DIGITS_H

class AliTRDdigitsManager;


//#if ROOT_VERSION_CODE >= ROOT_VERSION(6,00,0)
//#include "AliAnalysisTaskSE.h"
//#endif

#include "AliAnalysisTaskSE.h"

#include "TMath.h"
#include "TObject.h"
#include "TClonesArray.h"
#include "TLorentzVector.h"
#include "TDecompSVD.h"
#include "TMatrixD.h"
#include "TGrid.h"
#include "Ali_AS_Event.h"
#include "Ali_AS_EventLinkDef.h"
#include "Ali_TRD_ST.h"
#include "Ali_TRD_ST_LinkDef.h"


ClassImp(Ali_AS_TRD_digit)
ClassImp(Ali_AS_Track)
ClassImp(Ali_AS_Tracklet)
ClassImp(Ali_AS_offline_Tracklet)
ClassImp(Ali_AS_Event)
ClassImp(Ali_TRD_ST_Tracklets)
ClassImp(Ali_MC_particle)
ClassImp(Ali_TRD_ST_TPC_Track)
ClassImp(Ali_TRD_ST_Event)


using namespace std;

static vector< vector< vector< vector<Double_t> > > > vec_connected_clusters; //i_det i_trkl i_point i_xyz



class Ali_make_tracklets_from_digits : public AliAnalysisTaskSE
{
public:
    Ali_make_tracklets_from_digits()
	: AliAnalysisTaskSE(),
        //fDigitsInputFileName("TRD.FltDigits.root"),
        fDigitsInputFileName("TRD.Digits.root"),
        fDigitsInputFile(0),
	fDigitsOutputFileName(""), fDigitsOutputFile(0),
        fMCEvent(0),fDigMan(0),fGeo(0),AS_Event(0),AS_Track(0),AS_Tracklet(0),AS_offline_Tracklet(0),AS_Digit(0),Tree_AS_Event(0),
        TRD_ST_Tracklet(0),TRD_ST_MC_particle(0),TRD_ST_TPC_Track(0),TRD_ST_Event(0),Tree_TRD_ST_Event(0),fEventNoInFile(-2), N_good_events(0), N_total_events(0), fDigitsLoadedFlag(kFALSE),N_time_bins(24),
        aliHelix(),fListOfHistos(),fTree(),fPIDResponse(),EsdTrackCuts(),TV3_SVD_tracklet_offset(),TV3_SVD_tracklet_dir(),
        vec_self_tracklet_fit_points(),vec_ADC_val(),vec_TV3_TRD_center_offset(),vec_TV3_TRD_center(),TV3_trkl_offset(),TV3_trkl_dir()
    {
	cout << "" << endl;
	cout << "***************************************************************************************" << endl;
	cout << "In Ali_make_tracklets_from_digits.h constructor" << endl;
	cout << "fDigitsInputFileName: " << fDigitsInputFileName << endl;
	cout << "***************************************************************************************" << endl;
	//AS_Event       = new Ali_AS_Event();
	//AS_Track       = new Ali_AS_Track();
	cout << "" << endl;
    }
	Ali_make_tracklets_from_digits(const char *name);
	//virtual ~Ali_make_tracklets_from_digits() {}

	virtual void   UserCreateOutputObjects();
	virtual Bool_t UserNotify();
	virtual void   UserExec(Option_t *option);
	virtual void   Terminate(Option_t *);
        void FillHelix(AliESDtrack* track_in, Double_t magF_in);
        void FindDCAHelixPoint(TVector3 space_vec, AliHelix helixA, Float_t path_initA, Float_t path_initB, Float_t &pathA, Float_t &dcaAB);
        TVector3 calculate_point_on_Straight_dca_to_Point_2D(TVector3 &base, TVector3 &dir, TVector3 &point);
        Double_t Calc_SVD_tracklet(Int_t i_det, Int_t i_trkl);
        void Make_clusters_and_get_tracklets_fit(Double_t Delta_x, Double_t Delta_z, Double_t factor_missing);

	void SetDigitsInputFilename(TString x)
	{
	    fDigitsInputFileName=x;
	    cout << "" << endl;
	    cout << "***************************************************************************************" << endl;
	    cout << "fDigitsInputFileName: " << fDigitsInputFileName << endl;
	    cout << "***************************************************************************************" << endl;
	}
	void SetDigitsOutputFilename(TString x) {fDigitsOutputFileName=x;}

  void EnableLocalMode() { fLocalMode = kTRUE; }

	AliHelix aliHelix;

    protected:

	Bool_t NextEvent(Bool_t preload=kFALSE);
	Bool_t ReadDigits();

	TList           *fListOfHistos;       //! list of output histograms
	TTree           *fTree;               //! output tree
	AliPIDResponse  *fPIDResponse;        //! PID handling
	AliESDtrackCuts *EsdTrackCuts;        //!

    private:

        Bool_t fLocalMode;  // flag for running mode: default kFALSE (GRID mode)
	TFile* OpenDigitsFile(TString inputfile, TString digfile, TString opt);


	TString fDigitsInputFileName;         //! Name of digits file for reading
	TFile*  fDigitsInputFile;             //! Digits file for reading
	TString fDigitsOutputFileName;        //! Name of digits file for writing
	TFile*  fDigitsOutputFile;            //! Digits file for writing

        AliMCEvent*             fMCEvent;       //! corresponding MC event
	AliTRDdigitsManager* fDigMan; //! digits manager
	AliTRDgeometry* fGeo; //! TRD geometry
	Ali_AS_Event* AS_Event;
        Ali_AS_Track* AS_Track;
        Ali_AS_Tracklet* AS_Tracklet;
        Ali_AS_offline_Tracklet* AS_offline_Tracklet;
        Ali_AS_TRD_digit* AS_Digit;
        TTree       *Tree_AS_Event;

        Ali_MC_particle*      TRD_ST_MC_particle;
        Ali_TRD_ST_Tracklets* TRD_ST_Tracklet;
        Ali_TRD_ST_TPC_Track* TRD_ST_TPC_Track;
        Ali_TRD_ST_Event*     TRD_ST_Event;
        TTree* Tree_TRD_ST_Event;

	Int_t fEventNoInFile;
        Int_t N_good_events;
        Int_t N_total_events;
        Int_t fDigitsLoadedFlag;
        Int_t N_time_bins;


        TVector3 TV3_SVD_tracklet_offset;
        TVector3 TV3_SVD_tracklet_dir;
        vector< vector< vector< vector<Double_t> > > > vec_self_tracklet_fit_points;
        vector< vector< vector<Double_t> > > vec_ADC_val; //[i_det][i_trkl][i_timebin]
        vector<TVector3> vec_TV3_TRD_center_offset; // 540 chambers
        vector< vector<TVector3> >     vec_TV3_TRD_center; // 540 chambers, 3 axes
        Float_t  helix_par[9];
        Double_t ADC_val[30];
        TVector3 TV3_trkl_offset;
        TVector3 TV3_trkl_dir;

        Float_t digit_pos[3];

	Ali_make_tracklets_from_digits(const Ali_make_tracklets_from_digits&); // not implemented
	Ali_make_tracklets_from_digits& operator=(const Ali_make_tracklets_from_digits&); // not implemented

	ClassDef(Ali_make_tracklets_from_digits, 1);
};




#endif
