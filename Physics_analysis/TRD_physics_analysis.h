#define USEEVE

using namespace std;
#include <cmath>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include "TString.h"

#include "TObject.h"

#include "TFitter.h"
#include "TVirtualFitter.h"
#include "TFitResult.h"
#include "Math/Functor.h"

#include<TMath.h>

//for generator
#include "TStyle.h"
#include "TGaxis.h"
#include "TGraph.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TNtuple.h"
#include "TCanvas.h"
#include "TFile.h"

#include "TLorentzVector.h"
#include "TVector3.h"
#include "TChain.h"
#include "TTree.h"
#include "TMath.h"
#include <Math/BinaryOperators.h>
#include "Math/MConfig.h"
#include <iosfwd>
#include "Math/Expression.h"
#include "Math/MatrixRepresentationsStatic.h"
#include "Math/SMatrix.h"
#include "Math/MatrixFunctions.h"
#include "TProfile.h"
#include "TGraphAsymmErrors.h"

#include "TPolyLine.h"

//#if defined(USEEVE)
#include "TEveBox.h"
#include <TEveManager.h>
#include "TEveLine.h"
#include "TEvePointSet.h"
//#endif

#include "../Ali_TRD_Self_Event.h" 
#include "../Ali_TRD_Self_EventLinkDef.h"

ClassImp(Ali_Kalman_Track)
ClassImp(Ali_TPC_Track)
ClassImp(Ali_TRD_Photon)
ClassImp(Ali_TRD_Nuclear_interaction)
ClassImp(Ali_TRD_Self_Event)
ClassImp(Ali_Helix_copy)

class Ali_TRD_physics_analysis
{
private:
    TChain* input_SE;

    TString TRD_Self_TREE   = "ST_Physics/Tree_TRD_Self_Event";
    TString TRD_Self_BRANCH = "Tree_TRD_Self_Event_branch";
    
 	Long64_t file_entries_total;
    Long64_t N_Events;

    //stuff for new classes 
    Ali_Kalman_Track* Kalman_Track_photon;
    Ali_TPC_Track* TPC_Track_photon;    
    Ali_Kalman_Track* Kalman_Track_interact;
    Ali_TPC_Track* TPC_Track_interact;
    Ali_TRD_Self_Event*   TRD_Self_Event;
    Ali_TRD_Photon* TRD_Photon;
    Ali_TRD_Nuclear_interaction* TRD_Nuclear_interaction;

    TH2D* TH2_vertex_photon_XY;
    vector<TH1D*> vec_TH1_vertex_photon_radius;

    TFile* outputfile;
   

    TString HistName;
    TH1D* th1d_TRD_layer_radii;
    
    TString input_dir;
    TString input_dir_lists;

    Double_t EventVertexX = -999.0;
    Double_t EventVertexY = -999.0;
    Double_t EventVertexZ = -999.0;
    Long64_t Global_Event = -999;
    Int_t    Global_RunID = -999;
    TVector3 TV3_EventVertex;

    //things for Photon convertions
    TVector3 TV3_PhotonVertex; //temporary thing
    vector< TVector3 > vec_PhotonVertex; //[i_photon]
    
    vector< vector< Double_t>> vec_photon_kalman_chi2; //[i_photon][i_track]
    vector< vector< Ali_Helix_copy* >> vec_photon_kalman_helices; //[i_photon][i_track]
    vector< vector< Ali_Helix_copy* >> vec_photon_tpc_helices; //[i_photon][i_track]

    vector<TLorentzVector> vec_TLV_photon;
    vector< vector<TLorentzVector> > vec_TLV_photon_daughters;
    vector<TLorentzVector> vec_TLV_photon_mixed_events;

    vector<Double_t> vec_opening_angle_photon;
    vector< vector<Double_t> > vec_nsigma_electron;
    vector< vector<Double_t> > vec_nsigma_electron_mixed_events;

    vector<TH1D*> vec_TH1_mass_pi0;
    vector<TH1D*> vec_TH1_mass_pi0_ME;
    TH1D* TH1_angle_between_photons;
    TH2D* TH2D_angle_photons_vs_inv_mass;
    TH2D* TH2D_phiA_vs_phiB;
    TH1D* th1d_angle_between;

    TH1D* h_angle_AB_raw;
    TH1D* h_angle_AB_raw_cutA;
    TH1D* h_angle_AB_raw_cutB;

    vector<TEveLine*> TEveLine_mother;

    TGraph* tg_pol2_par;

    Double_t par_pT_corr_pos[3];
    Double_t par_pT_corr_neg[3];

    //things for Nuclear interactions
    TVector3 TV3_NIVertex;
    vector< TVector3 > vec_NIVertex;

    vector< vector< Double_t>> vec_ni_kalman_chi2; //[i_interaction][i_track]
    vector< vector< Ali_Helix_copy* >> vec_ni_kalman_helices; //[i_interaction][i_track]
    vector< vector< Ali_Helix_copy* >> vec_ni_tpc_helices; //[i_interaction][i_track]

    //things for Nuclear ingeractions
    
public:
    Ali_TRD_physics_analysis(TString out_dir, TString out_file_name, Int_t graphics);
    //~Ali_TRD_ST_Analyze();

    void Init_tree(TString SEList);
    Int_t Loop_event(Long64_t i_event, Double_t dist_max, Int_t graphics, Int_t ME, Int_t TRD_photon);


    void set_input_lists(TString input_dir_lists_in) {input_dir_lists = input_dir_lists_in;}
    void set_input_dir(TString input_dir_in) {input_dir = input_dir_in;}

    Long64_t get_N_Events() {return N_Events;}
    void Draw(Int_t ME);

    void Calculate_pi0_mass_SE();
    void Calculate_pi0_mass_SE_and_ME();
    
    ClassDef(Ali_TRD_physics_analysis, 1)
};
//----------------------------------------------------------------------------------------

