#include "RootInterface.h"
#include "RecoInterface.h"
#include "DRsimInterface.h"
#include "functions.h"

#include "TROOT.h"
#include "TStyle.h"
#include "TH1.h"
#include "TCanvas.h"
#include "TF1.h"
#include "TPaveStats.h"
#include "TString.h"
#include "TLorentzVector.h"
#include "TGraph.h"
#include "TVector3.h"

#include <iostream>
#include <fstream>
#include <string>

std::vector<std::vector<double>> RotX(double angle);
std::vector<std::vector<double>> RotY(double angle);
std::vector<std::vector<double>> RotZ(double angle);

int main(int argc, char* argv[]) {

  int EventIdx = std::stoi(argv[1]) - 1;
  TString filename;

  if ( EventIdx == 0 ) filename = "./EdgeEffect_NUM_1"; //vyp
  if ( EventIdx == 1 ) filename = "./EdgeEffect_NUM_2"; //vyz
  if ( EventIdx == 2 ) filename = "./EdgeEffect_NUM_3"; //vyp
  if ( EventIdx == 3 ) filename = "./EdgeEffect_NUM_4"; //vyz

  std::ifstream in;
  int i_tmp;
  double ceren, scint;
  std::vector<std::pair<double,double>> fCalibs;
  in.open("calib_0820.csv",std::ios::in);
  while (true) {
    in >> i_tmp >> ceren >> scint;
    if (!in.good()) break;
    fCalibs.push_back(std::make_pair(ceren, scint));
  }
  in.close();

  double Y0[4]    = {0.0000339013224496 , -1.246088501     , 0.               , -4.71346588245365  };
  double Z0[4]    = {-61.6554935603023  , 0.               , -4.71628638809038, -0.0013712590895075};
  double Theta[4] = {1.43774993877991   , 1.4179603267949  , 0.037289272688783, 0.022220009058163  };
  double Phi[4]   = {0.01110            , 0.026179938779915, 0.011101         , 0.026179938779915  };

  RootInterface<DRsimInterface::DRsimEventData>* drInterface = new RootInterface<DRsimInterface::DRsimEventData>(std::string(filename)+".root");
  drInterface->set("DRsim","DRsimEventData");

  std::vector<double> vx_;
  std::vector<double> vy_;
  std::vector<double> vz_;

  std::vector<double> vxp_;
  std::vector<double> vyp_;
  std::vector<double> vzp_;

  std::vector<float> E_Ss,E_Cs;
  float cThres[4] = {42.25, 40.55, 35.85, 36.35}; // should be fixed for each event!!
  int entries = drInterface->entries();

  while (drInterface->numEvt() < entries) {
    if (drInterface->numEvt() % 100 == 0) printf("Analyzing %dth event ...\n", drInterface->numEvt());

    DRsimInterface::DRsimEventData drEvt;
    drInterface->read(drEvt);

    double tmpEc = 0; double tmpEs = 0;
    for (auto tower = drEvt.towers.begin(); tower != drEvt.towers.end(); ++tower) {
      int fEta = 0;
      if (tower->towerTheta.first >= 0 ){
        fEta = tower->towerTheta.first;
      } else {
        fEta = std::abs(tower->towerTheta.first+1);
      }
      for (auto sipm = tower->SiPMs.begin(); sipm != tower->SiPMs.end(); ++sipm) {
        if ( RecoInterface::IsCerenkov(sipm->x,sipm->y) ) {
          for (const auto timepair : sipm->timeStruct) {
            if (timepair.first.first < cThres[EventIdx]) tmpEc += timepair.second / fCalibs.at(fEta).first;
          }
        } else {
          for (const auto timepair : sipm->timeStruct) {
            tmpEs += timepair.second / fCalibs.at(fEta).second;
          }
        }
      }
    }

    E_Ss.push_back(tmpEs);
    E_Cs.push_back(tmpEc);

    for (auto GenP = drEvt.GenPtcs.begin(); GenP != drEvt.GenPtcs.end(); ++GenP) {
      vx_.push_back(GenP->vx);
      vy_.push_back(GenP->vy - Y0[EventIdx]*10);
      vz_.push_back(GenP->vz - Z0[EventIdx]*10);
    }
  } // event loop

  auto RotPhi   = RotZ(-Phi[EventIdx]);
  auto RotTheta = RotY(-Theta[EventIdx]);

  auto TotRot = RotTheta;
  for ( int idx = 0; idx < 3; idx++ ) {
    for ( int jdx = 0; jdx < 3; jdx++ ) {
      TotRot.at(idx).at(jdx) = RotTheta.at(idx).at(0) * RotPhi.at(0).at(jdx) + RotTheta.at(idx).at(1) * RotPhi.at(1).at(jdx) + RotTheta.at(idx).at(2) * RotPhi.at(2).at(jdx);
    }
  }

  for ( int idx = 0; idx < vx_.size(); idx++ ) {
    vxp_.push_back( TotRot.at(0).at(0) * vx_.at(idx) + TotRot.at(0).at(1) * vy_.at(idx) + TotRot.at(0).at(2) * vz_.at(idx) );
    vyp_.push_back( TotRot.at(1).at(0) * vx_.at(idx) + TotRot.at(1).at(1) * vy_.at(idx) + TotRot.at(1).at(2) * vz_.at(idx) );
    vzp_.push_back( TotRot.at(2).at(0) * vx_.at(idx) + TotRot.at(2).at(1) * vy_.at(idx) + TotRot.at(2).at(2) * vz_.at(idx) );
  }

  std::vector<double> Position;
  if (EventIdx == 0 || EventIdx == 2) Position = vyp_;
  if (EventIdx == 1 || EventIdx == 3) Position = vzp_;

  // @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
  // Start Here
  // @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

  // NOTE : E_Cs = Cerenkov ch. eneryhy
  // NOTE : E_Ss = Scintillation ch. eneryhy
  // NOTE : Position = vx' or vy'


  // example
  TH1D* center = new TH1D("1", ";;", 100, 10., 30.); center->SetLineColor(kBlue);
  TH1D* boarder = new TH1D("2", ";;", 100, 10., 30.); boarder->SetLineColor(kRed);

  for ( int i = 0; i < Position.size(); i++ ) {
    if (Position.at(i) < 0.5 && Position.at(i) > -0.5) {
      center->Fill(E_Cs.at(i));
    } else {
      boarder->Fill(E_Cs.at(i));
    }
  }


  // Drawing space
  TCanvas* c = new TCanvas("c","");
  c->cd();
  boarder->Draw("Hist");
  center->Draw("sames&Hist");

  c->SaveAs("test.png");

  // @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
  // End Here
  // @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
}


std::vector<std::vector<double>> RotX(double angle) {

  std::vector<std::vector<double>> RotM;
  std::vector<double> Row1;
  std::vector<double> Row2;
  std::vector<double> Row3;

  Row1.push_back(1); Row1.push_back(0); Row1.push_back(0);
  RotM.push_back(Row1);
  Row2.push_back(0); Row2.push_back(std::cos(angle)); Row2.push_back(-std::sin(angle));
  RotM.push_back(Row2);
  Row3.push_back(0); Row3.push_back(std::sin(angle)); Row3.push_back(std::cos(angle));
  RotM.push_back(Row3);

  return RotM;
}

std::vector<std::vector<double>> RotY(double angle) {

  std::vector<std::vector<double>> RotM;
  std::vector<double> Row1;
  std::vector<double> Row2;
  std::vector<double> Row3;

  Row1.push_back(std::cos(angle)); Row1.push_back(0); Row1.push_back(-std::sin(angle));
  RotM.push_back(Row1);
  Row2.push_back(0); Row2.push_back(1); Row2.push_back(0);
  RotM.push_back(Row2);
  Row3.push_back(std::sin(angle)); Row3.push_back(0); Row3.push_back(std::cos(angle));
  RotM.push_back(Row3);

  return RotM;
}

std::vector<std::vector<double>> RotZ(double angle) {

  std::vector<std::vector<double>> RotM;
  std::vector<double> Row1;
  std::vector<double> Row2;
  std::vector<double> Row3;

  Row1.push_back(std::cos(angle)); Row1.push_back(-std::sin(angle)); Row1.push_back(0);
  RotM.push_back(Row1);
  Row2.push_back(-std::sin(angle)); Row2.push_back(std::cos(angle)); Row2.push_back(0);
  RotM.push_back(Row2);
  Row3.push_back(0); Row3.push_back(0); Row3.push_back(1);
  RotM.push_back(Row3);

  return RotM;
}
