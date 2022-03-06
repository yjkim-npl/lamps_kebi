#ifndef TB22KDETECTORCONSTRUCTION_HH
#define TB22KDETECTORCONSTRUCTION_HH

#include "TB22KMaterials.hh"
#include "G4VUserDetectorConstruction.hh"
#include "globals.hh"

class G4VPhysicalVolume;
class G4LogicalVolume;
class G4NistManager;
class G4VisAttributes;

class TB22KDetectorConstruction : public G4VUserDetectorConstruction
{
	public:

		TB22KDetectorConstruction();
		virtual ~TB22KDetectorConstruction();

		virtual G4VPhysicalVolume* Construct();

	private:
		void DefineMaterials();
//		TB22KMaterials* fMaterials;
//		G4Material* FindMaterial(G4String matName) {return fMaterials -> GetMaterial(matName);}
		G4NistManager* fNist;
};
#endif
