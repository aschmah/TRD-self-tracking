
#include "Ali_TRD_ST.h"
#include "Ali_TRD_ST_LinkDef.h"



//----------------------------------------------------------------------------------------
class Ali_TRD_ST_Analyze
{
private:
    TChain* input_SE;
    TString TRD_ST_TREE   = "Tree_TRD_ST_Event";
    TString TRD_ST_BRANCH = "Tree_TRD_ST_Event_branch";
    Long64_t file_entries_total;
    Long64_t N_Events;
    Ali_TRD_ST_Tracklets* TRD_ST_Tracklet;
    Ali_TRD_ST_TPC_Track* TRD_ST_TPC_Track;
    Ali_TRD_ST_Event*     TRD_ST_Event;

    TString HistName;
    TH1D* th1d_TRD_layer_radii;

    TEveLine* TEveLine_beam_axis = NULL;
    TEveLine* TPL3D_helix = NULL;
    vector<TEveLine*> vec_TPL3D_helix;
    vector<TEveLine*> vec_TPL3D_helix_inner;
    vector<TEveLine*> vec_TPL3D_helix_hull;
    vector< vector<TEveLine*> > vec_TEveLine_tracklets;
    vector< vector<TEveLine*> > vec_TEveLine_tracklets_match;
    Int_t N_tracklets_layers[6] = {0};
    Double_t scale_length = 10.0;
    Int_t track_color    = kAzure-2;
    Int_t color_layer_match[6] = {kRed,kGreen,kCyan,kYellow,kPink-3,kOrange+8};
    Int_t color_layer[6] = {kGray,kGray,kGray,kGray,kGray,kGray};
    Double_t TRD_layer_radii[6][2] =
    {
        {297.5,306.5},
        {310.0,320.0},
        {323.0,333.0},
        {336.0,345.5},
        {348.0,357.0},
        {361.0,371.0}
    };

    Int_t Not_installed_TRD_detectors[19] = {402,403,404,405,406,407,432,433,434,435,436,437,462,463,464,465,466,467,538};
    TH1D* h_good_bad_TRD_chambers;

    // TRD 3D graphics
    vector< vector<TH1D*> > vec_TH1D_TRD_geometry; // store for all 540 chambers the 8 corner vertices per detector
    vector<TEveBox*> vec_eve_TRD_detector_box;

public:
    Ali_TRD_ST_Analyze();
    ~Ali_TRD_ST_Analyze();
    void Init_tree(TString SEList);
    void Loop_event(Long64_t i_event);
    void Draw_event(Long64_t i_event);
    void Do_TPC_TRD_matching(Long64_t i_event, Double_t xy_matching_window, Double_t z_matching_window);

    ClassDef(Ali_TRD_ST_Analyze, 1)
};
//----------------------------------------------------------------------------------------



//----------------------------------------------------------------------------------------
Ali_TRD_ST_Analyze::Ali_TRD_ST_Analyze()
{
    // constructor
    TEveManager::Create();

    TPL3D_helix = new TEveLine();
    TEveLine_beam_axis = new TEveLine();
    TEveLine_beam_axis ->SetNextPoint(0.0,0.0,-300.0);
    TEveLine_beam_axis ->SetNextPoint(0.0,0.0,300.0);
    TEveLine_beam_axis ->SetName("beam axis");
    TEveLine_beam_axis ->SetLineStyle(1);
    TEveLine_beam_axis ->SetLineWidth(4);
    TEveLine_beam_axis ->SetMainColor(kBlue);
    gEve->AddElement(TEveLine_beam_axis);

    vec_TEveLine_tracklets.resize(6); // layers
    vec_TEveLine_tracklets_match.resize(6); // layers

    th1d_TRD_layer_radii = new TH1D("th1d_TRD_layer_radii","th1d_TRD_layer_radii",900,250,400.0);



    //--------------------------
    // Open histogram which defines good and bad chambers
    TFile* file_TRD_QA = TFile::Open("./Data/chamber_QC.root");
    h_good_bad_TRD_chambers = (TH1D*)file_TRD_QA ->Get("all_defects_hist");
    //--------------------------



    //--------------------------
    // Load TRD geometry
    TFile* file_TRD_geom = TFile::Open("./Data/TRD_Geom.root");
    vec_TH1D_TRD_geometry.resize(3); // x,y,z
    for(Int_t i_xyz = 0; i_xyz < 3; i_xyz++)
    {
        vec_TH1D_TRD_geometry[i_xyz].resize(8); // 8 vertices
        for(Int_t i_vertex = 0; i_vertex < 8; i_vertex++)
        {
            HistName = "vec_TH1D_TRD_geometry_xyz_";
            HistName += i_xyz;
            HistName += "_V";
            HistName += i_vertex;
            vec_TH1D_TRD_geometry[i_xyz][i_vertex] = (TH1D*)file_TRD_geom->Get(HistName.Data());
        }

    }
    vec_eve_TRD_detector_box.resize(540);
    for(Int_t TRD_detector = 0; TRD_detector < 540; TRD_detector++)
    {
        vec_eve_TRD_detector_box[TRD_detector] = new TEveBox;

        HistName = "TRD_box_";
        HistName += TRD_detector;
        vec_eve_TRD_detector_box[TRD_detector] ->SetName(HistName.Data());
        if(h_good_bad_TRD_chambers ->GetBinContent(TRD_detector)) // chamber is OK flagged by QA
        {
            vec_eve_TRD_detector_box[TRD_detector]->SetMainColor(kCyan);
            vec_eve_TRD_detector_box[TRD_detector]->SetMainTransparency(95); // the higher the value the more transparent
        }
        else // bad chamber
        {
            vec_eve_TRD_detector_box[TRD_detector]->SetMainColor(kRed);
            vec_eve_TRD_detector_box[TRD_detector]->SetMainTransparency(85); // the higher the value the more transparent
        }
        for(Int_t i_vertex = 0; i_vertex < 8; i_vertex++)
        {
            Double_t arr_pos_glb[3] = {vec_TH1D_TRD_geometry[0][i_vertex]->GetBinContent(TRD_detector),vec_TH1D_TRD_geometry[1][i_vertex]->GetBinContent(TRD_detector),vec_TH1D_TRD_geometry[2][i_vertex]->GetBinContent(TRD_detector)};
            vec_eve_TRD_detector_box[TRD_detector]->SetVertex(i_vertex,arr_pos_glb[0],arr_pos_glb[1],arr_pos_glb[2]);
        }

        gEve->AddElement(vec_eve_TRD_detector_box[TRD_detector]);
    }
    //--------------------------



}
//----------------------------------------------------------------------------------------



//----------------------------------------------------------------------------------------
void Ali_TRD_ST_Analyze::Init_tree(TString SEList)
{
    printf("Ali_TRD_ST_Analyze::Init_tree \n");
    TString pinputdir = "./Data/";

    TRD_ST_Tracklet   = new Ali_TRD_ST_Tracklets();
    TRD_ST_TPC_Track  = new Ali_TRD_ST_TPC_Track();
    TRD_ST_Event      = new Ali_TRD_ST_Event();

    // Same event input
    if (!SEList.IsNull())   // if input file is ok
    {
        cout << "Open same event file list " << SEList << endl;
        ifstream in(SEList);  // input stream
        if(in)
        {
            cout << "file list is ok" << endl;
            input_SE  = new TChain( TRD_ST_TREE.Data(), TRD_ST_TREE.Data() );
            char str[255];       // char array for each file name
            Long64_t entries_save = 0;
            while(in)
            {
                in.getline(str,255);  // take the lines of the file list
                if(str[0] != 0)
                {
                    TString addfile;
                    addfile = str;
                    addfile = pinputdir+addfile;
                    input_SE ->AddFile(addfile.Data(),-1, TRD_ST_TREE.Data() );
                    Long64_t file_entries = input_SE->GetEntries();
                    cout << "File added to data chain: " << addfile.Data() << " with " << (file_entries-entries_save) << " entries" << endl;
                    entries_save = file_entries;
                }
            }
            input_SE  ->SetBranchAddress( TRD_ST_BRANCH, &TRD_ST_Event );
        }
        else
        {
            cout << "WARNING: SE file input is problemtic" << endl;
        }
    }

    file_entries_total = input_SE->GetEntries();
    N_Events = file_entries_total;
    cout << "Total number of events in tree: " << file_entries_total << endl;
}
//----------------------------------------------------------------------------------------



//----------------------------------------------------------------------------------------
void Ali_TRD_ST_Analyze::Loop_event(Long64_t i_event)
{
    printf("Ali_TRD_ST_Analyze::Loop_event \n");

    if (!input_SE->GetEntry( i_event )) return 0; // take the event -> information is stored in event


    //--------------------------------------------------
    // Event information (more data members available, see Ali_TRD_ST_Event class definition)
    UShort_t NumTracks            = TRD_ST_Event ->getNumTracks(); // number of tracks in this event
    Int_t    NumTracklets         = TRD_ST_Event ->getNumTracklets();
    Double_t EventVertexX         = TRD_ST_Event ->getx();
    Double_t EventVertexY         = TRD_ST_Event ->gety();
    Double_t EventVertexZ         = TRD_ST_Event ->getz();
    Float_t  V0MEq                = TRD_ST_Event ->getcent_class_V0MEq();
    //--------------------------------------------------



    //--------------------------------------------------
    // TPC track loop
    for(Int_t i_track = 0; i_track < NumTracks; i_track++)
    {
        TRD_ST_TPC_Track = TRD_ST_Event ->getTrack(i_track);

        Double_t nsigma_TPC_e   = TRD_ST_TPC_Track ->getnsigma_e_TPC();
        Double_t nsigma_TPC_pi  = TRD_ST_TPC_Track ->getnsigma_pi_TPC();
        Double_t nsigma_TPC_p   = TRD_ST_TPC_Track ->getnsigma_p_TPC();
        Double_t nsigma_TOF_e   = TRD_ST_TPC_Track ->getnsigma_e_TOF();
        Double_t nsigma_TOF_pi  = TRD_ST_TPC_Track ->getnsigma_pi_TOF();
        Double_t TRD_signal     = TRD_ST_TPC_Track ->getTRDSignal();
        Double_t TRDsumADC      = TRD_ST_TPC_Track ->getTRDsumADC();
        Double_t dca            = TRD_ST_TPC_Track ->getdca();  // charge * distance of closest approach to the primary vertex
        TLorentzVector TLV_part = TRD_ST_TPC_Track ->get_TLV_part();
        UShort_t NTPCcls        = TRD_ST_TPC_Track ->getNTPCcls();
        UShort_t NTRDcls        = TRD_ST_TPC_Track ->getNTRDcls();
        UShort_t NITScls        = TRD_ST_TPC_Track ->getNITScls();
        Float_t TPCchi2         = TRD_ST_TPC_Track ->getTPCchi2();
        Float_t TPCdEdx         = TRD_ST_TPC_Track ->getTPCdEdx();
        Float_t TOFsignal       = TRD_ST_TPC_Track ->getTOFsignal(); // in ps (1E-12 s)
        Float_t Track_length    = TRD_ST_TPC_Track ->getTrack_length();

        Float_t momentum        = TLV_part.P();
        Float_t eta_track       = TLV_part.Eta();
        Float_t pT_track        = TLV_part.Pt();
        Float_t theta_track     = TLV_part.Theta();
        Float_t phi_track       = TLV_part.Phi();
    }
    //--------------------------------------------------



    //--------------------------------------------------
    // TRD tracklet loop
    TVector3 TV3_offset;
    TVector3 TV3_dir;
    Int_t    i_det;

    for(Int_t i_tracklet = 0; i_tracklet < NumTracklets; i_tracklet++)
    {
        TRD_ST_Tracklet = TRD_ST_Event    ->getTracklet(i_tracklet);
        TV3_offset      = TRD_ST_Tracklet ->get_TV3_offset();
        TV3_dir         = TRD_ST_Tracklet ->get_TV3_dir();
        i_det           = TRD_ST_Tracklet ->get_TRD_det();

        //create "tracklets"
        Int_t i_sector = (Int_t)(i_det/30);
        Int_t i_stack  = (Int_t)(i_det%30/6);
        Int_t i_layer  = i_det%6;
        //Int_t i_det = layer + 6*stack + 30*sector;

        Double_t radius = TMath::Sqrt( TMath::Power(TV3_offset[0],2) + TMath::Power(TV3_offset[1],2) );

        th1d_TRD_layer_radii ->Fill(radius);
    }
    //--------------------------------------------------


    TCanvas* can_TRD_layer_radii = new TCanvas("can_TRD_layer_radii","can_TRD_layer_radii",10,10,500,500);
    can_TRD_layer_radii ->cd();
    th1d_TRD_layer_radii ->DrawCopy("h");
}
//----------------------------------------------------------------------------------------



//----------------------------------------------------------------------------------------
void Ali_TRD_ST_Analyze::Draw_event(Long64_t i_event)
{
    printf("Ali_TRD_ST_Analyze::Draw_event \n");

    if (!input_SE->GetEntry( i_event )) return 0; // take the event -> information is stored in event


    //--------------------------------------------------
    // Event information (more data members available, see Ali_TRD_ST_Event class definition)
    UShort_t NumTracks            = TRD_ST_Event ->getNumTracks(); // number of tracks in this event
    Int_t    NumTracklets         = TRD_ST_Event ->getNumTracklets();
    Double_t EventVertexX         = TRD_ST_Event ->getx();
    Double_t EventVertexY         = TRD_ST_Event ->gety();
    Double_t EventVertexZ         = TRD_ST_Event ->getz();
    //--------------------------------------------------



    //--------------------------------------------------
    // TPC track loop
    Double_t track_pos[3];
    Double_t radius_helix;
    for(Int_t i_track = 0; i_track < NumTracks; i_track++)
    {
        TRD_ST_TPC_Track = TRD_ST_Event ->getTrack(i_track);

        Double_t nsigma_TPC_e   = TRD_ST_TPC_Track ->getnsigma_e_TPC();
        Double_t nsigma_TPC_pi  = TRD_ST_TPC_Track ->getnsigma_pi_TPC();
        Double_t nsigma_TPC_p   = TRD_ST_TPC_Track ->getnsigma_p_TPC();
        Double_t nsigma_TOF_e   = TRD_ST_TPC_Track ->getnsigma_e_TOF();
        Double_t nsigma_TOF_pi  = TRD_ST_TPC_Track ->getnsigma_pi_TOF();
        Double_t TRD_signal     = TRD_ST_TPC_Track ->getTRDSignal();
        Double_t TRDsumADC      = TRD_ST_TPC_Track ->getTRDsumADC();
        Double_t dca            = TRD_ST_TPC_Track ->getdca();  // charge * distance of closest approach to the primary vertex
        TLorentzVector TLV_part = TRD_ST_TPC_Track ->get_TLV_part();
        UShort_t NTPCcls        = TRD_ST_TPC_Track ->getNTPCcls();
        UShort_t NTRDcls        = TRD_ST_TPC_Track ->getNTRDcls();
        UShort_t NITScls        = TRD_ST_TPC_Track ->getNITScls();
        Float_t TPCchi2         = TRD_ST_TPC_Track ->getTPCchi2();
        Float_t TPCdEdx         = TRD_ST_TPC_Track ->getTPCdEdx();
        Float_t TOFsignal       = TRD_ST_TPC_Track ->getTOFsignal(); // in ps (1E-12 s)
        Float_t Track_length    = TRD_ST_TPC_Track ->getTrack_length();

        Float_t momentum        = TLV_part.P();
        Float_t eta_track       = TLV_part.Eta();
        Float_t pT_track        = TLV_part.Pt();
        Float_t theta_track     = TLV_part.Theta();
        Float_t phi_track       = TLV_part.Phi();

        if(momentum < 0.3) continue;

        vec_TPL3D_helix.resize(i_track+1);
        vec_TPL3D_helix_hull.resize(i_track+1);
        vec_TPL3D_helix_inner.resize(i_track+1);
        vec_TPL3D_helix[i_track] = new TEveLine();
        vec_TPL3D_helix_hull[i_track] = new TEveLine();
        vec_TPL3D_helix_inner[i_track] = new TEveLine();

        for(Double_t track_path = 0.0; track_path < 1000; track_path += 1.0)
        {
            TRD_ST_TPC_Track ->Evaluate(track_path,track_pos);
            radius_helix = TMath::Sqrt( TMath::Power(track_pos[0],2) + TMath::Power(track_pos[1],2) );
            if(radius_helix > 300.0) break;
            if(fabs(track_pos[2]) > 320.0) break;
            if(radius_helix > 80.0)
            {
                vec_TPL3D_helix[i_track]        ->SetNextPoint(track_pos[0],track_pos[1],track_pos[2]);
                vec_TPL3D_helix_hull[i_track]   ->SetNextPoint(track_pos[0],track_pos[1],track_pos[2]);
            }
            if(radius_helix < 80.0)
            {
                vec_TPL3D_helix_inner[i_track] ->SetNextPoint(track_pos[0],track_pos[1],track_pos[2]);
            }


            //if(i_track == 0) printf("track_path: %4.3f, pos: {%4.2f, %4.2f, %4.2f} \n",track_path,track_pos[0],track_pos[1],track_pos[2]);
        }

        HistName = "track ";
        HistName += i_track;
        vec_TPL3D_helix[i_track]    ->SetName(HistName.Data());
        vec_TPL3D_helix[i_track]    ->SetLineStyle(1);
        vec_TPL3D_helix[i_track]    ->SetLineWidth(3);
        vec_TPL3D_helix[i_track]    ->SetMainColor(track_color);
        vec_TPL3D_helix[i_track]    ->SetMainAlpha(1.0);

        HistName = "track (h) ";
        HistName += i_track;
        vec_TPL3D_helix_hull[i_track]    ->SetName(HistName.Data());
        vec_TPL3D_helix_hull[i_track]    ->SetLineStyle(1);
        vec_TPL3D_helix_hull[i_track]    ->SetLineWidth(8);
        vec_TPL3D_helix_hull[i_track]    ->SetMainColor(kWhite);
        vec_TPL3D_helix_hull[i_track]    ->SetMainAlpha(0.3);

        HistName = "track (h) ";
        HistName += i_track;
        vec_TPL3D_helix_inner[i_track]    ->SetName(HistName.Data());
        vec_TPL3D_helix_inner[i_track]    ->SetLineStyle(1);
        vec_TPL3D_helix_inner[i_track]    ->SetLineWidth(2);
        vec_TPL3D_helix_inner[i_track]    ->SetMainColor(kGray);
        vec_TPL3D_helix_inner[i_track]    ->SetMainAlpha(0.8);
        //if(i_track == 3)
        {
            gEve->AddElement(vec_TPL3D_helix[i_track]);
            gEve->AddElement(vec_TPL3D_helix_hull[i_track]);
            gEve->AddElement(vec_TPL3D_helix_inner[i_track]);
        }
    }

    //--------------------------------------------------



    //--------------------------------------------------
    // TRD tracklet loop
    TVector3 TV3_offset;
    TVector3 TV3_dir;
    Int_t    i_det;

    for(Int_t i_tracklet = 0; i_tracklet < NumTracklets; i_tracklet++)
    {
        TRD_ST_Tracklet = TRD_ST_Event    ->getTracklet(i_tracklet);
        TV3_offset      = TRD_ST_Tracklet ->get_TV3_offset();
        TV3_dir         = TRD_ST_Tracklet ->get_TV3_dir();
        i_det           = TRD_ST_Tracklet ->get_TRD_det();

        if(TV3_offset.Mag() > 1000.0) continue;

        //create "tracklets"
        Int_t i_sector = (Int_t)(i_det/30);
        Int_t i_stack  = (Int_t)(i_det%30/6);
        Int_t i_layer  = i_det%6;
        //Int_t i_det = layer + 6*stack + 30*sector;

        vec_TEveLine_tracklets[i_layer].resize(N_tracklets_layers[i_layer]+1);
        vec_TEveLine_tracklets[i_layer][N_tracklets_layers[i_layer]] = new TEveLine();

        vec_TEveLine_tracklets[i_layer][N_tracklets_layers[i_layer]] ->SetNextPoint(TV3_offset[0],TV3_offset[1],TV3_offset[2]);
        vec_TEveLine_tracklets[i_layer][N_tracklets_layers[i_layer]] ->SetNextPoint(TV3_offset[0] + scale_length*TV3_dir[0],TV3_offset[1] + scale_length*TV3_dir[1],TV3_offset[2] + scale_length*TV3_dir[2]);

        Double_t radius = TMath::Sqrt( TMath::Power(TV3_offset[0],2) + TMath::Power(TV3_offset[1],2) );
        //printf("i_tracklet: %d, radius: %4.3f, pos A: {%4.2f, %4.2f, %4.2f}, pos B: {%4.2f, %4.2f, %4.2f} \n",i_tracklet,radius,TV3_offset[0],TV3_offset[1],TV3_offset[2],TV3_offset[0] + scale_length*TV3_dir[0],TV3_offset[1] + scale_length*TV3_dir[1],TV3_offset[2] + scale_length*TV3_dir[2]);

        HistName = "tracklet ";
        HistName += i_tracklet;
        vec_TEveLine_tracklets[i_layer][N_tracklets_layers[i_layer]]    ->SetName(HistName.Data());
        vec_TEveLine_tracklets[i_layer][N_tracklets_layers[i_layer]]    ->SetLineStyle(1);
        vec_TEveLine_tracklets[i_layer][N_tracklets_layers[i_layer]]    ->SetLineWidth(3);
        vec_TEveLine_tracklets[i_layer][N_tracklets_layers[i_layer]]    ->SetMainColor(color_layer[i_layer]);
        //if(i_tracklet == 63 || i_tracklet == 67 || i_tracklet == 72 || i_tracklet == 75 || i_tracklet == 83 || i_tracklet == 88)
        {
            gEve->AddElement(vec_TEveLine_tracklets[i_layer][N_tracklets_layers[i_layer]]);
        }

        N_tracklets_layers[i_layer]++;
    }

    gEve->Redraw3D(kTRUE);
    //--------------------------------------------------
}
//----------------------------------------------------------------------------------------



//----------------------------------------------------------------------------------------
void Ali_TRD_ST_Analyze::Do_TPC_TRD_matching(Long64_t i_event, Double_t xy_matching_window, Double_t z_matching_window)
{
    printf("Ali_TRD_ST_Analyze::Do_TPC_TRD_matching \n");

    if (!input_SE->GetEntry( i_event )) return 0; // take the event -> information is stored in event


    //--------------------------------------------------
    // Event information (more data members available, see Ali_TRD_ST_Event class definition)
    UShort_t NumTracks            = TRD_ST_Event ->getNumTracks(); // number of tracks in this event
    Int_t    NumTracklets         = TRD_ST_Event ->getNumTracklets();
    Double_t EventVertexX         = TRD_ST_Event ->getx();
    Double_t EventVertexY         = TRD_ST_Event ->gety();
    Double_t EventVertexZ         = TRD_ST_Event ->getz();
    Float_t  V0MEq                = TRD_ST_Event ->getcent_class_V0MEq();
    //--------------------------------------------------



    //--------------------------------------------------
    // Store all TRD tracklet in an array to keep it in memory
    // TRD tracklet loop
    vector< vector<TVector3> > vec_TV3_dir_tracklets;
    vector< vector<TVector3> > vec_TV3_offset_tracklets;
    vec_TV3_dir_tracklets.resize(540);
    vec_TV3_offset_tracklets.resize(540);

    TVector3 TV3_offset;
    TVector3 TV3_dir;
    Int_t    i_det;
    Double_t ADC_val[24] = {-1.0};

    for(Int_t i_tracklet = 0; i_tracklet < NumTracklets; i_tracklet++)
    {
        TRD_ST_Tracklet = TRD_ST_Event    ->getTracklet(i_tracklet);
        TV3_offset      = TRD_ST_Tracklet ->get_TV3_offset();
        TV3_dir         = TRD_ST_Tracklet ->get_TV3_dir();
        i_det           = TRD_ST_Tracklet ->get_TRD_det();

        //create "tracklets"
        Int_t i_sector = (Int_t)(i_det/30);
        Int_t i_stack  = (Int_t)(i_det%30/6);
        Int_t i_layer  = i_det%6;
        //Int_t i_det = layer + 6*stack + 30*sector;

        vec_TV3_dir_tracklets[i_det].push_back(TV3_dir);
        vec_TV3_offset_tracklets[i_det].push_back(TV3_offset);

        if (i_tracklet%50 == 0) 
        {
            printf("i_trkl: %d, ADC_val: \n",i_tracklet);
            for (Int_t i_tbn = 0; i_tbn < 24; i_tbn++)
            {
                ADC_val[i_tbn]         = TRD_ST_Tracklet ->get_ADC_val(i_tbn);
                printf("i_timebin: %d; ADC_val: %4.3f \n \n",i_tbn,ADC_val[i_tbn]);
            }
        }


    }
    //--------------------------------------------------



    //--------------------------------------------------
    // TPC track loop
    Double_t track_pos[3];
    Double_t radius_helix;
    for(Int_t i_track = 0; i_track < NumTracks; i_track++)
    //for(Int_t i_track = 3; i_track < 4; i_track++)
    {
        TRD_ST_TPC_Track = TRD_ST_Event ->getTrack(i_track);

        Double_t nsigma_TPC_e   = TRD_ST_TPC_Track ->getnsigma_e_TPC();
        Double_t nsigma_TPC_pi  = TRD_ST_TPC_Track ->getnsigma_pi_TPC();
        Double_t nsigma_TPC_p   = TRD_ST_TPC_Track ->getnsigma_p_TPC();
        Double_t nsigma_TOF_e   = TRD_ST_TPC_Track ->getnsigma_e_TOF();
        Double_t nsigma_TOF_pi  = TRD_ST_TPC_Track ->getnsigma_pi_TOF();
        Double_t TRD_signal     = TRD_ST_TPC_Track ->getTRDSignal();
        Double_t TRDsumADC      = TRD_ST_TPC_Track ->getTRDsumADC();
        Double_t dca            = TRD_ST_TPC_Track ->getdca();  // charge * distance of closest approach to the primary vertex
        TLorentzVector TLV_part = TRD_ST_TPC_Track ->get_TLV_part();
        UShort_t NTPCcls        = TRD_ST_TPC_Track ->getNTPCcls();
        UShort_t NTRDcls        = TRD_ST_TPC_Track ->getNTRDcls();
        UShort_t NITScls        = TRD_ST_TPC_Track ->getNITScls();
        Float_t TPCchi2         = TRD_ST_TPC_Track ->getTPCchi2();
        Float_t TPCdEdx         = TRD_ST_TPC_Track ->getTPCdEdx();
        Float_t TOFsignal       = TRD_ST_TPC_Track ->getTOFsignal(); // in ps (1E-12 s)
        Float_t Track_length    = TRD_ST_TPC_Track ->getTrack_length();

        Float_t momentum        = TLV_part.P();
        Float_t eta_track       = TLV_part.Eta();
        Float_t pT_track        = TLV_part.Pt();
        Float_t theta_track     = TLV_part.Theta();
        Float_t phi_track       = TLV_part.Phi();

        if(momentum < 0.3) continue;

        vector<TVector3> vec_TV3_helix_points_at_TRD_layers;
        vec_TV3_helix_points_at_TRD_layers.resize(6);

        for(Int_t i_layer = 0; i_layer < 6; i_layer++)
        {
            Double_t radius_layer_center = 0.5*(TRD_layer_radii[i_layer][0] + TRD_layer_radii[i_layer][1]);

            // Find the helix path which touches the first TRD layer
            Double_t track_path_add      = 10.0;
            Double_t track_path_layer0   = 0.0;
            Double_t radius_helix_layer0 = 0.0;
            for(Double_t track_path = TRD_layer_radii[i_layer][0]; track_path < 1000; track_path += track_path_add)
            {
                TRD_ST_TPC_Track ->Evaluate(track_path,track_pos);
                radius_helix = TMath::Sqrt( TMath::Power(track_pos[0],2) + TMath::Power(track_pos[1],2) );
                if(radius_helix < radius_layer_center)
                {
                    track_path_layer0   = track_path;
                    radius_helix_layer0 = radius_helix;
                    printf("radius_helix_layer0: %4.3f, track_path_add: %4.3f \n",radius_helix_layer0,track_path_add);
                }
                else
                {
                    track_path -= track_path_add;
                    track_path_add *= 0.5;
                }
                if(track_path_add < 1.0)
                {
                    vec_TV3_helix_points_at_TRD_layers[i_layer].SetXYZ(track_pos[0],track_pos[1],track_pos[2]);
                    break;
                }
            }
            printf("   --> i_layer: %d, track_path_layer0: %4.3f, radius_helix_layer0: %4.3f \n",i_layer,track_path_layer0,radius_helix_layer0);


            TVector3 TV3_diff_vec;
            Double_t dist_min      = 10.0;
            Double_t dist          = 0.0;
            Int_t    det_best      = -1;
            Int_t    tracklet_best = -1;
            for(Int_t i_det = 0; i_det < 540; i_det++)
            {
                Int_t i_layer_from_det  = i_det%6;
                if(i_layer_from_det != i_layer) continue;

                for(Int_t i_tracklet = 0; i_tracklet < (Int_t)vec_TV3_offset_tracklets[i_det].size(); i_tracklet++)
                {
                    TV3_diff_vec = vec_TV3_offset_tracklets[i_det][i_tracklet] - vec_TV3_helix_points_at_TRD_layers[i_layer];
                    dist = TV3_diff_vec.Mag();
                    if(dist < dist_min)
                    {
                        dist_min      = dist;
                        det_best      = i_det;
                        tracklet_best = i_tracklet;
                    }
                }
            }

            if(det_best < 0 || tracklet_best < 0) continue;

            Int_t size_tracklet = (Int_t)vec_TEveLine_tracklets_match[i_layer].size();

            //printf("i_layer: %d, size_tracklet: %d \n",i_layer,size_tracklet);

            vec_TEveLine_tracklets_match[i_layer].resize(size_tracklet+1);
            vec_TEveLine_tracklets_match[i_layer][size_tracklet] = new TEveLine();
            vec_TEveLine_tracklets_match[i_layer][size_tracklet] ->SetNextPoint(vec_TV3_offset_tracklets[det_best][tracklet_best][0],vec_TV3_offset_tracklets[det_best][tracklet_best][1],vec_TV3_offset_tracklets[det_best][tracklet_best][2]);
            vec_TEveLine_tracklets_match[i_layer][size_tracklet] ->SetNextPoint(vec_TV3_offset_tracklets[det_best][tracklet_best][0] + scale_length*vec_TV3_dir_tracklets[det_best][tracklet_best][0],vec_TV3_offset_tracklets[det_best][tracklet_best][1] + scale_length*vec_TV3_dir_tracklets[det_best][tracklet_best][1],vec_TV3_offset_tracklets[det_best][tracklet_best][2] + scale_length*vec_TV3_dir_tracklets[det_best][tracklet_best][2]);

            HistName = "tracklet (m) ";
            HistName += size_tracklet;
            vec_TEveLine_tracklets_match[i_layer][size_tracklet]    ->SetName(HistName.Data());
            vec_TEveLine_tracklets_match[i_layer][size_tracklet]    ->SetLineStyle(1);
            vec_TEveLine_tracklets_match[i_layer][size_tracklet]    ->SetLineWidth(6);
            vec_TEveLine_tracklets_match[i_layer][size_tracklet]    ->SetMainColor(color_layer_match[i_layer]);
            //if(i_tracklet == 63 || i_tracklet == 67 || i_tracklet == 72 || i_tracklet == 75 || i_tracklet == 83 || i_tracklet == 88)
            {
                gEve->AddElement(vec_TEveLine_tracklets_match[i_layer][size_tracklet]);
            }
        }

    }
    //--------------------------------------------------

    gEve->Redraw3D(kTRUE);
}
//----------------------------------------------------------------------------------------





