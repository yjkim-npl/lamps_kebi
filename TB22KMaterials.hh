#ifndef TB22KMATERIALS_HH 
#define TB22KMATERIALS_HH

#include "G4Material.hh"
#include "G4NistManager.hh"

class TB22KMaterials {
public:
  virtual ~TB22KMaterials();
  static TB22KMaterials* GetInstance();
  G4Material* GetMaterial(const G4String);

private:
  TB22KMaterials();
  void CreateMaterials();

  static TB22KMaterials* fInstance;
  G4NistManager* fNistMan;
  G4Material* fVac;
  G4Material* fAir;
  G4Material* fAcryl;
  G4Material* fBP;
  G4Material* fPSV;
  G4Material* fAluOxi;
  G4Material* fCH2;
  G4Material* fMylar;
  G4Material* fGraphite;
  G4Material* fHL;
  G4Material* fHS;
  G4Material* p10Gas;
};

#endif
