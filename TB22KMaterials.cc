#include "TB22KMaterials.hh"
#include "G4SystemOfUnits.hh"

#include <stdio.h>
#include <algorithm>
#include <cmath>

TB22KMaterials* TB22KMaterials::fInstance = 0;

TB22KMaterials::TB22KMaterials() {
  fNistMan = G4NistManager::Instance();
  CreateMaterials();
}

TB22KMaterials::~TB22KMaterials() {}

TB22KMaterials* TB22KMaterials::GetInstance() {
  if (fInstance==0) fInstance = new TB22KMaterials();

  return fInstance;
}

G4Material* TB22KMaterials::GetMaterial(const G4String matName) {
  G4Material* mat = fNistMan->FindOrBuildMaterial(matName);

  if (!mat) mat = G4Material::GetMaterial(matName);
  if (!mat) {
    std::ostringstream o;
    o << "Material " << matName << " not found!";
    G4Exception("TB22KMaterials::GetMaterial","",FatalException,o.str().c_str());
  }

  return mat;
}


void TB22KMaterials::CreateMaterials() {
  G4double fSTPTemp = 273.15 * kelvin;
  G4double fLabTemp = fSTPTemp + 20 * kelvin;

  fNistMan->FindOrBuildMaterial("G4_Galactic");
  fNistMan->FindOrBuildMaterial("G4_AIR");
  fNistMan->FindOrBuildMaterial("G4_PLASTIC_SC_VINYLTOLUENE");
  fNistMan->FindOrBuildMaterial("G4_MYLAR");
  fNistMan->FindOrBuildMaterial("G4_ALUMINUM_OXIDE");
  fNistMan->FindOrBuildMaterial("G4_POLYETHYLENE");
  fNistMan->FindOrBuildMaterial("G4_B");

  //define elements
  G4String symbol;
  G4double a, z, density;
  G4int ncomponents, natoms;
  G4Element* H  = new G4Element("Hydrogen",symbol="H" , z=1., a=1.01*g/mole);
  G4Element* C  = new G4Element("Carbon"  ,symbol="C" , z=6., a=12.01*g/mole);
  G4Element* N  = new G4Element("Nitrogen",symbol="N" , z=7., a=14.01*g/mole);
  G4Element* O  = new G4Element("Oxygen"  ,symbol="O" , z=8., a=16.00*g/mole);
  G4Element* F  = new G4Element("Fluorine",symbol="F" , z=9., a=18.9984*g/mole);

  fVac = G4Material::GetMaterial("G4_Galactic");
  fAir = G4Material::GetMaterial("G4_AIR");
  fPSV = G4Material::GetMaterial("G4_PLASTIC_SC_VINYLTOLUENE");
  fCH2 = G4Material::GetMaterial("G4_POLYETHYLENE");
  fAluOxi = G4Material::GetMaterial("G4_ALUMINUM_OXIDE");
  fMylar = G4Material::GetMaterial("G4_MYLAR");
  G4Material* B = G4Material::GetMaterial("G4_B");

  // p10Gas
  G4double ArGasD = 1.7836 * mg/cm3 * fSTPTemp/fLabTemp;
  G4Material* ArGas = new G4Material("ArGas",18,39.948 * g/mole, ArGasD, kStateGas, fLabTemp);

  G4double CH4GasD = 0.717e-3 * g/cm3 * fSTPTemp/fLabTemp;
  G4Material* CH4Gas = new G4Material("CH4Gas",CH4GasD, 2, kStateGas, fLabTemp);
  CH4Gas -> AddElement(C,1);
  CH4Gas -> AddElement(H,4);

  G4double p10GasD = 0.9*ArGasD + 0.1* CH4GasD;
  p10Gas = new G4Material("p10Gas",p10GasD,2,kStateGas, fLabTemp);
  p10Gas -> AddMaterial(ArGas, 0.9* ArGasD/p10GasD);
  p10Gas -> AddMaterial(CH4Gas, 0.1* CH4GasD/p10GasD);

  // target material
  fHL = new G4Material("HLiquid",1,1.008*g/mole, 70.85*mg/cm3);
  fHS = new G4Material("HSolid",1,1.008*g/mole, 44.0*mg/cm3);
  fGraphite = new G4Material("Graphite",12,12.011*g/mole, 2.26*g/cm3, kStateSolid);
  
  // Acryl
  fAcryl = new G4Material("Acrylic",1.19*g/cm3, 3);
  fAcryl -> AddElement(C,5);
  fAcryl -> AddElement(H,8);
  fAcryl -> AddElement(O,2);

  // BP
  G4double BPD = 0.3*B->GetDensity() + 0.7*fCH2->GetDensity();
  fBP = new G4Material("BoratedPolyethylene",BPD,2);
  fBP -> AddMaterial(B,0.3);
  fBP -> AddMaterial(fCH2,0.7);

}
