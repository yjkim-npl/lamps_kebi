#include "TB22KDetectorConstruction.hh"

#include "KBG4RunManager.hh"
#include "KBGeoBoxStack.hh"
#include "KBParameterContainer.hh"

#include "G4AutoDelete.hh"
#include "G4Box.hh"
#include "G4Colour.hh"
#include "G4FieldManager.hh"
#include "G4LogicalVolume.hh"
#include "G4NistManager.hh"
#include "G4Orb.hh"
#include "G4PVPlacement.hh"
#include "G4RunManager.hh"
#include "G4SubtractionSolid.hh"
#include "G4SystemOfUnits.hh"
#include "G4TransportationManager.hh"
#include "G4Trap.hh"
#include "G4Tubs.hh"
#include "G4UserLimits.hh"
#include "G4VisAttributes.hh"
#include "G4VSolid.hh"

#include <iostream>
#include <cmath>
using namespace std;

//Constructor
TB22KDetectorConstruction::TB22KDetectorConstruction() : G4VUserDetectorConstruction()
{
//	fMaterials = TB22KMaterials::GetInstance();
//	DefineMaterials();
	fNist = G4NistManager::Instance();
}

//Destructor
TB22KDetectorConstruction::~TB22KDetectorConstruction()
{
	G4AutoDelete::Register(fNist);
}

void TB22KDetectorConstruction::DefineMaterials()
{
//	fMaterials = TB22KMaterials::GetInstance();
}

//=======================================================
G4VPhysicalVolume* TB22KDetectorConstruction::Construct()
{
	auto fRun = (KBG4RunManager*)G4RunManager::GetRunManager();
	auto fPar = fRun->GetParameterContainer();

	auto fVisFrame = new G4VisAttributes();
	fVisFrame->SetColor(G4Color::Gray());
	fVisFrame->SetForceWireframe(true);

	G4ThreeVector fZero(0, 0, 0);
	G4double fSTPTemp = 273.15 * kelvin;
	G4double fLabTemp = fSTPTemp + 20 * kelvin;

	//For detectors located in specific theta angle (SC tilted, AT-TPC)
	G4double comTheta0 = fPar->GetParDouble("comTheta0");
	G4double comTheta1 = fPar->GetParDouble("comTheta1");
	G4RotationMatrix rotTheta0 = G4RotationMatrix(); rotTheta0.rotateY(comTheta0 * deg);
	G4RotationMatrix rotTheta1 = G4RotationMatrix(); rotTheta1.rotateY(comTheta1 * deg);
	G4RotationMatrix rot0 = G4RotationMatrix();

	//Argon (gas)
	G4double ArGasD = 1.7836 * mg/cm3 * fSTPTemp/fLabTemp;
	G4Material* ArGas = new G4Material("ArGas", 18, 39.948*g/mole, ArGasD, kStateGas, fLabTemp);

	//CH4 (Methane)
	G4double CH4GasD = 0.717e-3 * g/cm3 * fSTPTemp/fLabTemp; //Methane density
	G4Material* CH4Gas = new G4Material("CH4Gas", CH4GasD, 2, kStateGas, fLabTemp);
	G4Element* elH = new G4Element("Hydrogen", "H", 1., 1.00794 * g/mole);
	G4Element* elC = new G4Element("Carbon", "C", 6., 12.011 * g/mole);
	G4Element* elO = new G4Element("Oxygen", "O", 8., 16.000 * g/mole);
	CH4Gas->AddElement(elC, 1);
	CH4Gas->AddElement(elH, 4);

	//P10
	G4double P10GasD = 0.9*ArGasD + 0.1*CH4GasD;
	G4Material* P10Gas = new G4Material("P10Gas", P10GasD, 2, kStateGas, fLabTemp);
	P10Gas->AddMaterial(ArGas, 0.9 * ArGasD/P10GasD);
	P10Gas->AddMaterial(CH4Gas, 0.1 * CH4GasD/P10GasD);

	// Al frame
	G4Material* matAl = fNist -> FindOrBuildMaterial("G4_Al");

	// translation 
	G4double trans = fPar -> GetParDouble("Translation");

	//World volume
	//--------------------------------------------------------------------

	G4Material* worldMat;
	if      (fPar->GetParInt("worldOpt") == 0) worldMat = fNist->FindOrBuildMaterial("G4_Galactic");
	else if (fPar->GetParInt("worldOpt") == 1) worldMat = fNist->FindOrBuildMaterial("G4_AIR");
//	if      (fPar->GetParInt("worldOpt") == 0) worldMat = FindMaterial("G4_Galactic");
//	else if (fPar->GetParInt("worldOpt") == 1) worldMat = FindMaterial("G4_AIR");
	else { cout <<"\nWorld material???\n"; assert(false); return nullptr;}

	G4int    worldID = fPar->GetParInt("worldID");
	G4double worldDX = fPar->GetParDouble("worlddX");
	G4double worldDY = fPar->GetParDouble("worlddY");
	G4double worldDZ = fPar->GetParDouble("worlddZ");
	G4double worldDR = fPar->GetParDouble("worlddR");

	G4VSolid* WorldSol;
	if		(fPar -> GetParInt("worldShape") == 0 )
		WorldSol = new G4Box("WorldSolid", worldDX, worldDY, worldDZ);
	else if	(fPar -> GetParInt("worldShape") == 1)
		WorldSol = new G4Orb("WorldSolid", worldDR);
	else WorldSol = new G4Box("WorldSolid", worldDX, worldDY, worldDZ);

	G4LogicalVolume*   WorldLog = new G4LogicalVolume(WorldSol, worldMat, "World");
	G4VPhysicalVolume* WorldPhy = new G4PVPlacement(0, fZero, WorldLog, "WorldPhys", 0, false, worldID, true);

	auto fVisWorld = new G4VisAttributes();
	fVisWorld->SetColor(G4Color::White());
	fVisWorld->SetForceWireframe(true);
	WorldLog->SetVisAttributes(fVisWorld);

	
	// trash - ? ckim
	//-------------------------------------------------------------------
	G4Box* solid_T = new G4Box("solid_T",1.,1.,1.);
	G4LogicalVolume* logic_T = new G4LogicalVolume(solid_T,fNist->FindOrBuildMaterial("G4_Galactic"),"logic_T");
	G4VPhysicalVolume* physic_T = new G4PVPlacement(
			0, G4ThreeVector(worldDX-1.,worldDY-1.,0), logic_T, "physic_T", WorldLog, false, 1, true);
	fRun -> SetSensitiveDetector(physic_T);
	auto fVistrash = new G4VisAttributes();
	fVistrash -> SetVisibility(false);
	logic_T -> SetVisAttributes(fVistrash);
	

	//Acrylic Shield
	//-------------------------------------------------------------------
	if (fPar -> GetParBool("AcrylShieldIn"))
	{
//		G4Material* ArcShieldmat = FindMaterial("Acrylic");
		G4Material* ArcShieldmat = new G4Material("Acrylic", 1.19*g/cm3, 3, kStateSolid);
		ArcShieldmat -> AddElement(elC,5);
		ArcShieldmat -> AddElement(elH,8);
		ArcShieldmat -> AddElement(elO,2);
		G4int	 ArcShieldID 	= fPar->GetParInt("ArcShieldID");
		G4double ArcShieldDimX	= fPar->GetParDouble("ArcShieldDimX");
		G4double ArcShieldDimY	= fPar->GetParDouble("ArcShieldDimY");
		G4double ArcShieldDimZ	= fPar->GetParDouble("ArcShieldDimZ");
		G4double ArcShieldHoleR1 = fPar->GetParDouble("ArcShieldHoleR1");
		G4double ArcShieldHoleR2 = fPar->GetParDouble("ArcShieldHoleR2");
		G4double ArcShieldHoleX = fPar->GetParDouble("ArcShieldHoleX");
		G4double ArcShieldHoleY = fPar->GetParDouble("ArcShieldHoleY");
		G4double ArcShieldOffZ1 = fPar->GetParDouble("ArcShieldOffZ1");
		G4double ArcShieldOffZ2 = fPar->GetParDouble("ArcShieldOffZ2");

		G4Box* baseShield = new G4Box("baseShield",ArcShieldDimX/2., ArcShieldDimY/2., ArcShieldDimZ/2.);
//		G4Box* subBox	  = new G4Box("subBox", ArcShieldHoleX/2., ArcShieldHoleY/2., 1.03*ArcShieldDimZ/2.);
		G4Tubs* subHole1   = new G4Tubs("Hole1", 0, ArcShieldHoleR1, 1.03*ArcShieldDimZ/2., 0, 2*M_PI);
		G4Tubs* subHole2   = new G4Tubs("Hole2", 0, ArcShieldHoleR2, 1.03*ArcShieldDimZ/2., 0, 2*M_PI);
		G4SubtractionSolid* solidArcShield1 = new G4SubtractionSolid(
				"solidArcShield1", baseShield, subHole1, 0, G4ThreeVector(0,0,0));
		G4SubtractionSolid* solidArcShield2 = new G4SubtractionSolid(
				"solidArcShield2", baseShield, subHole2, 0, G4ThreeVector(0,0,0));
		G4LogicalVolume* logicArcShield1 = new G4LogicalVolume(solidArcShield1, ArcShieldmat, "logicArcShield1");
		G4LogicalVolume* logicArcShield2 = new G4LogicalVolume(solidArcShield2, ArcShieldmat, "logicArcShield2");
		G4VisAttributes* attArcShield = new G4VisAttributes(G4Colour(G4Colour::Gray()));
		attArcShield -> SetForceWireframe(true);
		logicArcShield1 -> SetVisAttributes(attArcShield);
		logicArcShield2 -> SetVisAttributes(attArcShield);

		new G4PVPlacement(0, G4ThreeVector(0,0,ArcShieldOffZ1+ArcShieldDimZ/2.+trans), logicArcShield1, "ArcShield1", WorldLog, false, 0, true);
		new G4PVPlacement(0, G4ThreeVector(0,0,ArcShieldOffZ2+ArcShieldDimZ/2.+trans), logicArcShield2, "ArcShield2", WorldLog, false, 0, true);
	}

	//Acrylic Collimator
	//-------------------------------------------------------------------
	if (fPar->GetParBool("CollimatorIn"))
	{
//		G4Material* Collmat = FindMaterial("Acrylic");
		G4Material* Collmat = new G4Material("Acrylic",1.19*g/cm3,3,kStateSolid, 293.15*kelvin);
		Collmat -> AddElement(elC,5);
		Collmat -> AddElement(elH,8);
		Collmat -> AddElement(elO,2);

		G4int	 CollID	   = fPar->GetParInt("CollID");
		G4double CollDimX  = fPar->GetParDouble("CollDimX");	// one brick [] 
		G4double CollDimY  = fPar->GetParDouble("CollDimY");
		G4double CollDimZ  = fPar->GetParDouble("CollDimZ");
		G4double CollslitX = fPar->GetParDouble("CollslitX");
		G4double CollslitY = fPar->GetParDouble("CollslitY");
		G4double CollPosZ  = fPar->GetParDouble("CollPosZ");

		//Volumes
		G4Box* solidBoxColl = new G4Box("solidBoxColl", CollDimX/2., CollDimY/2., CollDimZ/2.);
		//G4Box* solidSubColl = new G4Box("solidSubColl",CollslitX/2., CollDimY/2., 1.03*CollDimZ/2.);
		//G4SubtractionSolid* solidColl = new G4SubtractionSolid(
		//		"solidColl", solidBoxColl, solidSubColl, 0, G4ThreeVector(0,0,0));
		//G4LogicalVolume* logicColl = new G4LogicalVolume(solidColl, Collmat, "logicCollimator");
		G4Box* solidSubCollX = new G4Box("solidSubCollX", CollslitX/2., CollDimY/2. - 0.1, CollDimZ/2.);
		G4Box* solidSubCollY = new G4Box("solidSubCollY", CollDimX/2. - 0.1, CollslitY/2., CollDimZ/2.);
		G4SubtractionSolid* solidCollX = new G4SubtractionSolid("solidCollX", solidBoxColl, solidSubCollX, 0, fZero);
		G4SubtractionSolid* solidCollY = new G4SubtractionSolid("solidCollY", solidBoxColl, solidSubCollY, 0, fZero);
		G4LogicalVolume* logicCollX = new G4LogicalVolume(solidCollX, Collmat, "logicCollimatorX");
		G4LogicalVolume* logicCollY = new G4LogicalVolume(solidCollY, Collmat, "logicCollimatorY");

		//vis attributes
		G4VisAttributes* attColl = new G4VisAttributes(G4Colour(G4Colour::Gray()));
		attColl->SetVisibility(true);
		attColl->SetForceWireframe(true);
		logicCollX->SetVisAttributes(attColl);
		logicCollY->SetVisAttributes(attColl);

		//Rotation
		G4RotationMatrix* Rot = new G4RotationMatrix;
		Rot->rotateZ(90*deg);

		//Position
		G4ThreeVector posCollX(0, 0, CollPosZ + CollDimZ/2. + trans);
		G4ThreeVector posCollY(0, 0, CollPosZ + CollDimZ/2. + CollDimZ + trans);
		new G4PVPlacement(0, posCollX, logicCollY, "CollimatorX", WorldLog, false, CollID, true);
		new G4PVPlacement(0, posCollY, logicCollX, "CollimatorY", WorldLog, false, CollID, true);
		/*
		// front block set (up, down)
		new G4PVPlacement(Rot, G4ThreeVector(0,0,CollPosZ+CollDimZ/2.), logicColl,"Collimator_1", WorldLog, false, fPar->GetParInt("CollID"), true);
		// back block set (left, right)
		new G4PVPlacement(0, G4ThreeVector(0,0,CollPosZ+3*CollDimZ/2.), logicColl,"Collimator_2", WorldLog, false, fPar->GetParInt("CollID"), true);
		*/
	}

	//Boron Shield
	//--------------------------------------------------------------------
	if(fPar -> GetParBool("ShieldIn"))
	{
		G4Material* elB = fNist->FindOrBuildMaterial("G4_B");
		G4Material* matCH2 = fNist->FindOrBuildMaterial("G4_POLYETHYLENE");
		G4double density = 0.3*elB->GetDensity() + 0.7*matCH2->GetDensity();
		G4Material* Shieldmat = new G4Material("BoratedPolyethylene",density,2);
		Shieldmat -> AddMaterial(elB,0.3);
		Shieldmat -> AddMaterial(matCH2,0.7);

//		G4Material* Shieldmat = FindMaterial("BoratedPolyethylene");
		G4double ShieldDimX  = fPar -> GetParDouble("ShieldDimX");
		G4double ShieldDimY  = fPar -> GetParDouble("ShieldDimY");
		G4double ShieldDimZ  = fPar -> GetParDouble("ShieldDimZ");
		G4double ShieldHoleX = fPar -> GetParDouble("ShieldHoleX");
		G4double ShieldHoleY = fPar -> GetParDouble("ShieldHoleY");
		G4double ShieldPos0Z  = fPar -> GetParDouble("ShieldPos0Z");
		G4double ShieldPos1Z  = fPar -> GetParDouble("ShieldPos1Z");

		G4Box* solidBoxShield = new G4Box("solidBoxShield", ShieldDimX/2.,ShieldDimY/2.,ShieldDimZ/2.);
		G4Box* solidSubShield1 = new G4Box("solidSubShield",10*ShieldHoleX/2.,10*ShieldHoleY/2.,ShieldDimZ/2.*1.03);
		G4Box* solidSubShield2 = new G4Box("solidSubShield",ShieldHoleX/2.,ShieldHoleY/2.,ShieldDimZ/2.*1.03);
		G4SubtractionSolid* solidShield1 = new G4SubtractionSolid("solidShield",solidBoxShield,solidSubShield1,0,G4ThreeVector(0,0,0));
		G4SubtractionSolid* solidShield2 = new G4SubtractionSolid("solidShield",solidBoxShield,solidSubShield2,0,G4ThreeVector(0,0,0));
		G4LogicalVolume* logicShield1 = new G4LogicalVolume(solidShield1, Shieldmat, "logicShield");
		G4LogicalVolume* logicShield2 = new G4LogicalVolume(solidShield2, Shieldmat, "logicShield");
		//vis attributes
		G4VisAttributes* attShield = new G4VisAttributes(G4Colour(G4Colour::Brown()));
		attShield -> SetVisibility(true);
		attShield -> SetForceWireframe(true);
		logicShield1 -> SetVisAttributes(attShield);
		logicShield2 -> SetVisAttributes(attShield);

		new G4PVPlacement(0,G4ThreeVector(0,0,ShieldPos0Z+ShieldDimZ/2. + trans),logicShield1,"Shield1",WorldLog,false,fPar->GetParInt("ShieldID"),true);
		new G4PVPlacement(0,G4ThreeVector(0,0,ShieldPos1Z+ShieldDimZ/2. + trans),logicShield2,"Shield2",WorldLog,false,fPar->GetParInt("ShieldID"),true);
	}
	//SC
	//--------------------------------------------------------------------

	if (fPar->GetParBool("SCIn"))
	{
		G4Material* scMat = fNist->FindOrBuildMaterial("G4_PLASTIC_SC_VINYLTOLUENE");
//		G4Material* scMat = FindMaterial("G4_PLASTIC_SC_VINYLTOLUENE");

		G4int    scID    = fPar->GetParInt("scID");
		G4double scDimX  = fPar->GetParDouble("scDimX");
		G4double scDimY  = fPar->GetParDouble("scDimY");
		G4double scDimZ  = fPar->GetParDouble("scDimZ");
		G4double scPosZ0 = fPar->GetParDouble("scPosZ0");
		G4double scPosZ1 = fPar->GetParDouble("scPosZ1");

		G4Box*           SCSol = new G4Box("SCSolid", scDimX/2, scDimY/2, scDimZ/2);
		G4LogicalVolume* SCLog = new G4LogicalVolume(SCSol, scMat, "SCLogic");

		G4ThreeVector SCPos0(0, 0, scPosZ0 + scDimZ/2. + trans);
		G4ThreeVector SCPos1(0, 0, scPosZ1 + scDimZ/2. + trans);
		G4VPhysicalVolume* SCPhys0 = new G4PVPlacement(0, SCPos0, SCLog, "SC0", WorldLog, false, scID+0, true);
		G4VPhysicalVolume* SCPhys1 = new G4PVPlacement(0, SCPos1, SCLog, "SC1", WorldLog, false, scID+1, true);
		fRun->SetSensitiveDetector(SCPhys0);
		fRun->SetSensitiveDetector(SCPhys1);

		auto fVisSC = new G4VisAttributes();
		fVisSC->SetColor(G4Color::Cyan());
		fVisSC->SetForceWireframe(true);
		SCLog->SetVisAttributes(fVisSC);
		if(fPar -> GetParBool("SC1fIn"))
		{
			G4double SC1f1x = fPar -> GetParDouble("SC1f1x");
			G4double SC1f1y = fPar -> GetParDouble("SC1f1y");
			G4double SC1f1z = fPar -> GetParDouble("SC1f1z");

			G4double SC1f2x = fPar -> GetParDouble("SC1f2x");
			G4double SC1f2y = fPar -> GetParDouble("SC1f2y");
			G4double SC1f2z = fPar -> GetParDouble("SC1f2z");

			G4Box* solidF1 = new G4Box("solidF1",SC1f1x/2,SC1f1y/2,SC1f1z/2);
			G4LogicalVolume* logicF1 = new G4LogicalVolume(solidF1,matAl,"logicF1");
			G4Box* solidF2 = new G4Box("solidF2",SC1f2x/2,SC1f2y/2,SC1f2z/2);
			G4LogicalVolume* logicF2 = new G4LogicalVolume(solidF2,matAl,"logicF2");

			G4VisAttributes* attFrame = new G4VisAttributes(G4Colour(G4Colour::Gray()));
			attFrame -> SetForceWireframe(true);
			logicF1 -> SetVisAttributes(attFrame);
			logicF2 -> SetVisAttributes(attFrame);

		    auto pvpf1u = new G4PVPlacement(0, G4ThreeVector(0,-(SC1f2y+SC1f1y)/2,scPosZ0 + trans), logicF1, "SC1f1u", WorldLog, false, scID+2, true);
            auto pvpf1d = new G4PVPlacement(0, G4ThreeVector(0,(SC1f2y+SC1f1y)/2,scPosZ0 + trans), logicF1, "SC1f1d", WorldLog, false, scID+3, true);
            auto pvpf1l = new G4PVPlacement(0, G4ThreeVector(-(SC1f1x-SC1f2x)/2,0,scPosZ0 + trans), logicF2, "SC1f1l", WorldLog, false, scID+4, true);
            auto pvpf1r = new G4PVPlacement(0, G4ThreeVector((SC1f1x-SC1f2x)/2,0,scPosZ0 + trans), logicF2, "SC1f1r", WorldLog, false, scID+5, true);

		    auto pvpf2u = new G4PVPlacement(0, G4ThreeVector(0,-(SC1f2y+SC1f1y)/2,scPosZ1 + trans), logicF1, "SC2f1u", WorldLog, false, scID+12, true);
            auto pvpf2d = new G4PVPlacement(0, G4ThreeVector(0,(SC1f2y+SC1f1y)/2,scPosZ1 + trans), logicF1, "SC2f1d", WorldLog, false, scID+13, true);
            auto pvpf2l = new G4PVPlacement(0, G4ThreeVector(-(SC1f1x-SC1f2x)/2,0,scPosZ1 + trans), logicF2, "SC2f1l", WorldLog, false, scID+14, true);
            auto pvpf2r = new G4PVPlacement(0, G4ThreeVector((SC1f1x-SC1f2x)/2,0,scPosZ1 + trans), logicF2, "SC2f1r", WorldLog, false, scID+15, true);
		}
	}//SC

	//BDC
	//--------------------------------------------------------------------

	if (fPar->GetParBool("BDCIn"))
	{
		//Mylar and Al oxide
		G4Material* Mylar   = fNist->FindOrBuildMaterial("G4_MYLAR");
//		G4Material* Mylar 	= FindMaterial("G4_MYLAR");
		G4Material* AlOxide = fNist->FindOrBuildMaterial("G4_ALUMINUM_OXIDE");
//		G4Material* AlOxide = FindMaterial("G4_ALUMINUM_OXIDE");

		//+++++++++++++++++++++++++++++++++++++++

		G4int    bdcID    = fPar->GetParInt("bdcID");
		G4double bdcDimX  = fPar->GetParDouble("bdcDimX");
		G4double bdcDimY  = fPar->GetParDouble("bdcDimY");
		G4double bdcDimZ  = fPar->GetParDouble("bdcDimZ");
		G4double bdcPosZ0 = fPar->GetParDouble("bdcPosZ0");
		G4double bdcPosZ1 = fPar->GetParDouble("bdcPosZ1");

		G4double bdcMylarT = 50 * um;
		G4double bdcAlmT = 2.5 * um; //Aluminized mylar, one layer
		const int nAlm = 5; //Total # of Alm layers

		//Enclosure volume (all)
		G4Box*           BDCSol = new G4Box("BDCSolid", (bdcDimX+20)/2, (bdcDimY+20)/2, (bdcDimZ+bdcMylarT)/2);
		G4LogicalVolume* BDCLog = new G4LogicalVolume(BDCSol, fNist->FindOrBuildMaterial("G4_AIR"), "BDCLogic");
		BDCLog->SetVisAttributes(G4VisAttributes::GetInvisible());

		//p10
		G4Box*           BDCSolP10 = new G4Box("BDCSolidP10", bdcDimX/2, bdcDimY/2, bdcDimZ/2);
		G4LogicalVolume* BDCLogP10 = new G4LogicalVolume(BDCSolP10, P10Gas, "BDCLogicP10");
		new G4PVPlacement(0, fZero, BDCLogP10, "BDCP10", BDCLog, false, bdcID + 10, false);

		auto fVisBDC = new G4VisAttributes();
		fVisBDC->SetColor(G4Color::Yellow());
		fVisBDC->SetForceWireframe(true);
		BDCLogP10->SetVisAttributes(fVisBDC);

		if (fPar->GetParBool("BDCfIn"))
		{
			G4double bdc1fx = fPar -> GetParDouble("bdc1fx");
			G4double bdc1fy = fPar -> GetParDouble("bdc1fy");
			G4double bdc1fz = fPar -> GetParDouble("bdc1fz");
			G4double bdc2fx = fPar -> GetParDouble("bdc2fx");
			G4double bdc2fy = fPar -> GetParDouble("bdc2fy");
			G4double bdc2fz = fPar -> GetParDouble("bdc2fz");

			G4Box*           BDCSolMylar = new G4Box("BDCSolidMylar", (bdcDimX+20)/2, (bdcDimY+20)/2,1.3*bdc1fz/2);
			G4VSolid* solid_bdc1f = new G4Box("solid_BDC1f",bdc1fx/2.,bdc1fy/2.,bdc1fz/2.);
			G4VSolid* solid_bdc2f = new G4Box("solid_BDC2f",bdc2fx/2.,bdc2fy/2.,bdc2fz/2.);
			solid_bdc1f = new G4SubtractionSolid("subsolid_BDC1f",solid_bdc1f, BDCSolMylar);
			solid_bdc2f = new G4SubtractionSolid("subsolid_BDC2f",solid_bdc2f, BDCSolMylar);
			G4LogicalVolume* logic_bdc1f = new G4LogicalVolume(solid_bdc1f, matAl, "logic_BDC1f");
			G4LogicalVolume* logic_bdc2f = new G4LogicalVolume(solid_bdc2f, matAl, "logic_BDC2f");

			G4VisAttributes* attFrame = new G4VisAttributes(G4Colour(G4Colour::Gray()));
			attFrame -> SetForceWireframe(true);
			logic_bdc1f -> SetVisAttributes(attFrame);
			logic_bdc2f -> SetVisAttributes(attFrame);
			new G4PVPlacement(0,G4ThreeVector(0,0,bdcPosZ0+bdcDimZ/2. + trans),logic_bdc1f,"BDCF1",WorldLog,false,bdcID+11,false);
			new G4PVPlacement(0,G4ThreeVector(0,0,bdcPosZ1+bdcDimZ/2. + trans),logic_bdc2f,"BDCF2",WorldLog,false,bdcID+12,false);
		}
		if( fPar -> GetParBool("DetFrameOn"))
		{

			//Mylar sheets (upstream/downstream)
			G4Box*           BDCSolMylar = new G4Box("BDCSolidMylar", (bdcDimX+20)/2, (bdcDimY+20)/2, bdcMylarT/2);
			G4LogicalVolume* BDCLogMylar = new G4LogicalVolume(BDCSolMylar, Mylar, "BDCLogicMylar");
			G4ThreeVector MylarPosU(0, 0, -(bdcDimZ + bdcMylarT)/2 );
			G4ThreeVector MylarPosD(0, 0, +(bdcDimZ + bdcMylarT)/2 );
			new G4PVPlacement(0, MylarPosU, BDCLogMylar, "BDCMylarU", BDCLog, false, bdcID + 20, false);
			new G4PVPlacement(0, MylarPosD, BDCLogMylar, "BDCMylarD", BDCLog, false, bdcID + 21, false);
			BDCLogMylar->SetVisAttributes(fVisFrame);

			//Aluminized mylar (assume entire thickness is Al oxide for now)
			G4Box*           BDCSolAlm = new G4Box("BDCSolidAlm", bdcDimX/2, bdcDimY/2, bdcAlmT/2);
			G4LogicalVolume* BDCLogAlm = new G4LogicalVolume(BDCSolAlm, AlOxide, "BDCLogicAlm");
			for (int i=0; i<nAlm; i++)
			{
				G4ThreeVector AlmPos(0, 0, -bdcDimZ/2 + (bdcDimZ/(float)nAlm) * (i+1) );
				new G4PVPlacement(0, AlmPos, BDCLogAlm, Form("BDCAlm%i", i), BDCLog, false, bdcID + 30+i, false);
			}
			BDCLogAlm->SetVisAttributes(fVisFrame);
		}

		G4ThreeVector BDCPos0(0, 0, bdcPosZ0 + bdcDimZ/2. + trans);
		G4ThreeVector BDCPos1(0, 0, bdcPosZ1 + bdcDimZ/2. + trans);
		auto BDCPhys0 = new G4PVPlacement(0, BDCPos0, BDCLog, "BDC0", WorldLog, false, bdcID+0, true);
		auto BDCPhys1 = new G4PVPlacement(0, BDCPos1, BDCLog, "BDC1", WorldLog, false, bdcID+1, true);
		fRun->SetSensitiveDetector(BDCPhys0);
		fRun->SetSensitiveDetector(BDCPhys1);
	}//BDC

	//Target (C2H4)
	//--------------------------------------------------------------------

	if (fPar->GetParBool("TargetIn"))
	{
		G4Material* targetMat = nullptr;;

		if      (fPar->GetParInt("targetMat") == 0)
		{
//			targetMat = FindMaterial("G4_POLYETHYLENE");;
			targetMat = fNist->FindOrBuildMaterial("G4_POLYETHYLENE");;
		}
		else if (fPar->GetParInt("targetMat") == 1)
		{
			//Liquid hydrogen (H2)
			G4Material* HLiquid = new G4Material("HLiquid", 1, 1.008*g/mole, 70.85*mg/cm3);
//			targetMat = FindMaterial("HLiquid");
		}
		else if (fPar->GetParInt("targetMat") == 2)
		{
			//Solid hydrogen: Nuclear Physics A 805 (2008)
			G4Material* HSolid = new G4Material("HSolid", 1, 1.008*g/mole, 44.0*mg/cm3);
//			targetMat = FindMaterial("HSolid");
		}
		else if (fPar->GetParInt("targetMat") == 3) // graphite will be added soon
		{
//			targetMat = FindMaterial("Graphite");
//			targetMat = new G4Material("Graphite",12,12.011*g/mole,1.88*g/cm3,kStateSolid);
			targetMat = new G4Material("Graphite",1.88*g/cm3,1,kStateSolid);
			targetMat -> AddElement(elC,100*perCent);
		}
		else { cout <<"\nTarget material???\n"; assert(false); }

		G4int    targetID   = fPar->GetParInt("targetID");
		G4double targetDimX = fPar->GetParDouble("targetDimX");
		G4double targetDimY = fPar->GetParDouble("targetDimY");
		G4double targetDimZ = fPar->GetParDouble("targetDimZ");
		G4double targetPosZ = fPar->GetParDouble("targetPosZ");
		G4ThreeVector targetPos(0, 0, targetPosZ + targetDimZ/2 + trans);

		G4Box*           TargetSol = new G4Box("TargetSolid", targetDimX/2, targetDimY/2, targetDimZ/2);
		G4LogicalVolume* TargetLog = new G4LogicalVolume(TargetSol, targetMat, "TargetLogic");
		G4VPhysicalVolume* TargetPhy = new G4PVPlacement(0, targetPos, TargetLog, "Target", WorldLog, false, targetID, true);

		auto fVisTarget = new G4VisAttributes();
		fVisTarget->SetColor(G4Color::White());
		fVisTarget->SetForceWireframe(true);
		TargetLog->SetVisAttributes(fVisTarget);

		fRun -> SetSensitiveDetector(TargetPhy);
	}//Target

	//SC tilted
	//--------------------------------------------------------------------

	if (fPar->GetParBool("SCTiltIn"))
	{
//		G4Material* scMat = FindMaterial("G4_PLASTIC_SC_VINYLTOLUENE");
		G4Material* scMat = fNist->FindOrBuildMaterial("G4_PLASTIC_SC_VINYLTOLUENE");

		G4int    sctID    = fPar->GetParInt("sctID");
		G4double sctDimX  = fPar->GetParDouble("sctDimX");
		G4double sctDimY  = fPar->GetParDouble("sctDimY");
		G4double sctDimZ  = fPar->GetParDouble("sctDimZ");
		G4double sctPosZ0 = fPar->GetParDouble("sctPosZ0");
		G4double sctPosZ1 = fPar->GetParDouble("sctPosZ1");

		G4Box*           SCTSol = new G4Box("SCTiltSolid", sctDimX/2, sctDimY/2, sctDimZ/2);
		G4LogicalVolume* SCTLog = new G4LogicalVolume(SCTSol, scMat, "SCTiltLogic");

		const double SCTOfsY0 = sin(comTheta0 * deg) * (sctPosZ0 + sctDimZ/2.);
		const double SCTOfsZ0 = cos(comTheta0 * deg) * (sctPosZ0 + sctDimZ/2.);
//		G4Transform3D SCTTr0 = G4Transform3D(rotTheta0, G4ThreeVector(0, SCTOfsY0, SCTOfsZ0));
		G4Transform3D SCTTr0 = G4Transform3D(rot0, G4ThreeVector(0, SCTOfsY0, SCTOfsZ0 + trans));
		G4VPhysicalVolume* SCTPhys0 = new G4PVPlacement(SCTTr0, SCTLog, "SCT0", WorldLog, false, sctID+0, true);
		fRun->SetSensitiveDetector(SCTPhys0);

		const double SCTOfsY1 = sin(comTheta1 * deg) * (sctPosZ1 + sctDimZ/2.);
		const double SCTOfsZ1 = cos(comTheta1 * deg) * (sctPosZ1 + sctDimZ/2.);
//		G4Transform3D SCTTr1 = G4Transform3D(rotTheta1, G4ThreeVector(SCTOfsX1, 0, SCTOfsZ1));
		G4Transform3D SCTTr1 = G4Transform3D(rot0, G4ThreeVector(0, SCTOfsY1, SCTOfsZ1 + trans));
		G4VPhysicalVolume* SCTPhys1 = new G4PVPlacement(SCTTr1, SCTLog, "SCT1", WorldLog, false, sctID+1, true);
		fRun->SetSensitiveDetector(SCTPhys1);

		auto fVisSCT = new G4VisAttributes();
		fVisSCT->SetColor(G4Color::Cyan());
		fVisSCT->SetForceWireframe(true);
		SCTLog->SetVisAttributes(fVisSCT);

		if (fPar -> GetParBool("SCTiltfIn"))
		{
			G4double sctfx = fPar -> GetParDouble("sctfx");
			G4double sctfy = fPar -> GetParDouble("sctfy");
			G4double sctfz = fPar -> GetParDouble("sctfz");

			G4VSolid* solid_sctf = new G4Box("solid_sctf",sctfx/2.,sctfy/2.,sctfz/2.);
			G4VSolid* solid_sctf_hole = new G4Box("solid_sctf_hole",sctDimX/2.,sctDimY/2.,1.3*sctfz/2.);
			solid_sctf = new G4SubtractionSolid("solid_sub_sctf",solid_sctf,solid_sctf_hole);
			G4LogicalVolume* logic_sctf = new G4LogicalVolume(solid_sctf,matAl,"logic_sctf");

			G4VisAttributes* attFrame = new G4VisAttributes(G4Colour(G4Colour::Gray()));
			attFrame -> SetForceWireframe(true);
			logic_sctf -> SetVisAttributes(attFrame);

//			G4VPhysicalVolume* physic_sctf = 
			new G4PVPlacement(SCTTr0,logic_sctf,"SCTF1",WorldLog,false,sctID+10,true);
		}
		

		if (fPar->GetParInt("comSetup") == 0) 
		{
			//Naming scheme: SC (beamline) -> SCT (tilted) -> SCA (tiled + AT-TPC altermnative)
			G4double scaDimX = 60.;
			G4double scaDimY = 60.;
			G4double scaDimZ =  3.;
			G4double scaPosZ0 = sctPosZ1 +  50.;
			G4double scaPosZ1 = sctPosZ1 + 100.;

			G4Box*           SCASol = new G4Box("SCAltSolid", scaDimX/2, scaDimY/2, scaDimZ/2);
			G4LogicalVolume* SCALog = new G4LogicalVolume(SCASol, scMat, "SCAltLogic");
			SCALog->SetVisAttributes(fVisSCT);

			const double SCAOfsX0 = sin(comTheta1 * deg) * (scaPosZ0 + scaDimZ/2.);
			const double SCAOfsZ0 = cos(comTheta1 * deg) * (scaPosZ0 + scaDimZ/2.);
			G4Transform3D SCATr0 = G4Transform3D(rotTheta1, G4ThreeVector(SCAOfsX0, 0, SCAOfsZ0));
			G4VPhysicalVolume* SCAPhys0 = new G4PVPlacement(SCATr0,SCALog,"SCA0", WorldLog, false, sctID+10, true);
			fRun->SetSensitiveDetector(SCAPhys0);

			const double SCAOfsX1 = sin(comTheta1 * deg) * (scaPosZ1 + scaDimZ/2.);
			const double SCAOfsZ1 = cos(comTheta1 * deg) * (scaPosZ1 + scaDimZ/2.);
			G4Transform3D SCATr1 = G4Transform3D(rotTheta1, G4ThreeVector(SCAOfsX1, 0, SCAOfsZ1));
//			G4VPhysicalVolume* SCAPhys1 = new G4PVPlacement(SCATr1,SCALog,"SCA1", WorldLog, false, sctID+11, true);
//			fRun->SetSensitiveDetector(SCAPhys1);
		}

	}//SC tilt

	//AT-TPC
	//--------------------------------------------------------------------

	if (fPar->GetParBool("ATTPCIn"))
	{
		G4int    attpcID    = fPar->GetParInt("attpcID");
		G4double attpcDimX;  
		G4double attpcDimY; 
		G4double attpcDimZ;  
		if(fPar -> GetParBool("attpcFC"))
		{
			attpcDimX = fPar -> GetParDouble("attpcFCX");
			attpcDimY = fPar -> GetParDouble("attpcFCY");
			attpcDimZ = fPar -> GetParDouble("attpcFCZ");
		}else{
			attpcDimX = fPar -> GetParDouble("attpcDimX");
			attpcDimY = fPar -> GetParDouble("attpcDimY");
			attpcDimZ = fPar -> GetParDouble("attpcDimZ");
		}
		G4double attpcPosZ0 = fPar->GetParDouble("attpcPosZ0");
		G4double attpcPosZ1 = fPar->GetParDouble("attpcPosZ1");

		G4Box*           ATPSol = new G4Box("ATTPCSolid", attpcDimX/2, attpcDimY/2, attpcDimZ/2);
		G4LogicalVolume* ATPLog = new G4LogicalVolume(ATPSol, P10Gas, "ATTPCLogic");

		const double ATPOfsX0 = sin(comTheta0 * deg) * (attpcPosZ0 + attpcDimZ/2.);
		const double ATPOfsZ0 = cos(comTheta0 * deg) * (attpcPosZ0 + attpcDimZ/2.);
		G4Transform3D ATPTr0 = G4Transform3D(rotTheta0, G4ThreeVector(ATPOfsX0, 0, ATPOfsZ0));
		G4VPhysicalVolume* ATPPhys0 = new G4PVPlacement(ATPTr0, ATPLog, "ATTPC0", WorldLog, false, attpcID+0, true);
		fRun->SetSensitiveDetector(ATPPhys0);

		const double dX = (double)(attpcDimX/2);
		const double dZ0 = (double)(attpcPosZ0+attpcDimZ/2);
		const double dTheta0 = std::atan2(dZ0, dX);
		cout <<Form("ATTPC0 acceptance: [%7.3f, %7.3f]\n", comTheta0-dTheta0, comTheta0+dTheta0);

		if (fPar->GetParInt("comSetup") == 1)
		{
			const double ATPOfsX1 = sin(comTheta1 * deg) * (attpcPosZ1 + attpcDimZ/2.);
			const double ATPOfsZ1 = cos(comTheta1 * deg) * (attpcPosZ1 + attpcDimZ/2.);
			G4Transform3D ATPTr1 = G4Transform3D(rotTheta1, G4ThreeVector(ATPOfsX1, 0, ATPOfsZ1));
//			G4VPhysicalVolume* ATPPhys1 = new G4PVPlacement(ATPTr1,ATPLog,"ATTPC1",WorldLog,false,attpcID+1,true);
//			fRun->SetSensitiveDetector(ATPPhys1);

			const double dZ1 = (double)(attpcPosZ1+attpcDimZ/2);
			const double dTheta1 = std::atan2(dZ1, dX);
			cout <<Form("ATTPC1 acceptance: [%7.3f, %7.3f]\n", comTheta1-dTheta1, comTheta1+dTheta1);
		}
		if (fPar -> GetParBool("ATTPCfIn"))
		{
			const double attpcfx = fPar -> GetParDouble("attpcfx");
			const double attpcfy = fPar -> GetParDouble("attpcfy");
			const double attpcfz = fPar -> GetParDouble("attpcfz");

			G4VSolid* solid_attpcf_hole = new G4Box("solid_attpcf_hole",attpcDimX/2.,attpcDimY/2.,1.3*attpcfz/2.);
			G4VSolid* solid_attpcf = new G4Box("solid_attpcf",attpcfx/2.,attpcfy/2.,attpcfz/2.);
			solid_attpcf = new G4SubtractionSolid("solid_sub_attpcf",solid_attpcf,solid_attpcf_hole);

			G4LogicalVolume* logic_attpcf = new G4LogicalVolume(solid_attpcf,matAl,"logic_attpcf");

			auto fVisFrame = new G4VisAttributes();
			fVisFrame -> SetForceWireframe(true);
			logic_attpcf -> SetVisAttributes(fVisFrame);
			new G4PVPlacement(ATPTr0,logic_attpcf,"ATTPCf",WorldLog,false,attpcID+20,true);
		}

		auto fVisATP = new G4VisAttributes();
		fVisATP->SetColor(G4Color::Yellow());
		fVisATP->SetForceWireframe(true);
		ATPLog->SetVisAttributes(fVisATP);
	}//ATTPC

	//TOF
	//--------------------------------------------------------------------

	if (fPar->GetParBool("BTOFIn") || fPar->GetParBool("FTOFIn"))
	{
//		G4Material* tofMat = FindMaterial("G4_PLASTIC_SC_VINYLTOLUENE");
		G4Material* tofMat = fNist->FindOrBuildMaterial("G4_PLASTIC_SC_VINYLTOLUENE");

		G4double tofSlatL_b = fPar->GetParDouble("tofSlatL_b");
		G4double tofSlatL_f = fPar->GetParDouble("tofSlatL_f");
		G4double tofSlatW   = fPar->GetParDouble("tofSlatW");
		G4double tofSlatT   = fPar->GetParDouble("tofSlatT");

		//A module
		//++++++++++++++++++++++++++++++++++++++++++++++++++++++

		G4RotationMatrix* rotZ90 = new G4RotationMatrix(); rotZ90->rotateZ(90 * deg);
		G4RotationMatrix* rotZ180 = new G4RotationMatrix(); rotZ180->rotateZ(180 * deg);

		//Enclosure volume (box + fishtails)
		G4Box* TOFSolM = new G4Box("TOFSolM", tofSlatW/2, (tofSlatL_b+2*tofSlatL_f)/2, tofSlatT/2);
		G4LogicalVolume* TOFLogM = new G4LogicalVolume(TOFSolM, WorldLog->GetMaterial(), "TOFLogM");
		TOFLogM->SetVisAttributes(G4VisAttributes::GetInvisible());

		//Box at center
		G4Box*           TOFSolM_b = new G4Box("TOFSolM_b", tofSlatW/2, tofSlatL_b/2, tofSlatT/2);
		G4LogicalVolume* TOFLogM_b = new G4LogicalVolume(TOFSolM_b, tofMat, "TOFLogM_b");

		//Fish tails
		G4Trap* TOFSolM_f = new G4Trap("TOFSolM_f");
		TOFSolM_f->SetAllParameters(
				tofSlatT/2, 0, 0,
				tofSlatL_f/2, tofSlatW/2, tofSlatW/4, 0,
				tofSlatL_f/2, tofSlatW/2, tofSlatW/4, 0);
		G4LogicalVolume* TOFLogM_f = new G4LogicalVolume(TOFSolM_f, tofMat, "TOFLogM_f");

		//Visualization
		auto fVisTOF = new G4VisAttributes();
		fVisTOF->SetColor(G4Color::Brown());
		fVisTOF->SetForceWireframe(true);
		TOFLogM_b->SetVisAttributes(fVisTOF);
		TOFLogM_f->SetVisAttributes(fVisTOF);

		//BTOF
		//++++++++++++++++++++++++++++++++++++++++++++++++++++++

		//FTOF
		//++++++++++++++++++++++++++++++++++++++++++++++++++++++

		if (fPar->GetParBool("FTOFIn"))
		{
			G4int    ftofID    = fPar->GetParInt("ftofID");
			G4int    ftofSlatN = fPar->GetParInt("ftofSlatN");
			G4double ftofPosY  = fPar->GetParDouble("ftofPosY");
			G4double ftofPosZ  = fPar->GetParDouble("ftofPosZ");

			new G4PVPlacement(0, fZero, TOFLogM_b, "TOFM_b", TOFLogM, false, ftofID, true);
			new G4PVPlacement(0, G4ThreeVector(0, (tofSlatL_b + tofSlatL_f)/2, 0),
					TOFLogM_f, "TOFM_f0", TOFLogM, false, ftofID, true);
			new G4PVPlacement(rotZ180, G4ThreeVector(0, -(tofSlatL_b + tofSlatL_f)/2, 0),
					TOFLogM_f, "TOFM_f1", TOFLogM, false, ftofID, true);

			for (int a=0; a<ftofSlatN; a++) //# of slat pairs, by radial distance from beam axis
			for (int b=0; b<2; b++) //Pair
			{
				const int mID = a*10 + b;
				const float posY = tofSlatW*a + tofSlatW/2 + ftofPosY;
				auto FTOFPhy = new G4PVPlacement(rotZ90, G4ThreeVector(0, b==0?posY:(-posY), ftofPosZ + tofSlatT/2),
						TOFLogM, Form("FTOF_%i", mID), WorldLog, false, ftofID+mID, true);
				fRun->SetSensitiveDetector(FTOFPhy);
			}//a, b
		}//FTOF
	}//B/FTOF
	
	//--------------------------------------------------------------------
	if (fPar -> GetParBool("Si-CsIIn"))
	{
		G4Material* matSi = fNist -> FindOrBuildMaterial("G4_Si");
		G4Material* matCsI = fNist -> FindOrBuildMaterial("G4_CESIUM_IODIDE");

		G4int SiID = fPar -> GetParInt("SiID");
		G4int CsIID = fPar -> GetParInt("CsIID");
		G4double dX = fPar -> GetParDouble("Si-CsIdX");
		G4double dY = fPar -> GetParDouble("Si-CsIdY");
		G4double SidZ = fPar -> GetParDouble("SidZ");
		G4double CsIdZ = fPar -> GetParDouble("CsIdZ");
//		G4double posOffZSi = fPar -> GetParDouble("posOffZSi");			// offset = target(0,0,0)
//		G4double posOffZCsI = fPar -> GetParDouble("posOffZCsI");
		G4double posRSi = fPar -> GetParDouble("posRSi");
//		G4double posRCsI = fPar -> GetParDouble("posRCsI");

		G4Box*           solidSi  = new G4Box("solidSi", dX/2, dY/2, SidZ/2);
		G4Box*           solidCsI = new G4Box("solidCsI", dX/2, dY/2, CsIdZ/2);
		G4LogicalVolume* logicSi  = new G4LogicalVolume(solidSi, matSi, "logicSi");
		G4LogicalVolume* logicCsI = new G4LogicalVolume(solidCsI, matCsI, "logicCsI");

		const double posX0 = sin(comTheta0 * deg) * (posRSi + SidZ/2.);
		const double posZ0 = cos(comTheta0 * deg) * (posRSi + SidZ/2.);
		const double posX1 = posX0 + sin(comTheta0 * deg) * (SidZ + CsIdZ/2.);
		const double posZ1 = posZ0 + cos(comTheta0 * deg) * (SidZ + CsIdZ/2.);
		G4Transform3D SiTr = G4Transform3D(rotTheta0, G4ThreeVector(posX0, 0, posZ0));
		G4Transform3D CsITr = G4Transform3D(rotTheta0, G4ThreeVector(posX1, 0, posZ1));
		G4VPhysicalVolume* SiPhys = new G4PVPlacement(SiTr, logicSi, "physicSi", WorldLog, false, SiID+0, true);
		G4VPhysicalVolume* CsIPhys = new G4PVPlacement(CsITr, logicCsI, "physicCsI", WorldLog, false, SiID+1, true);
		fRun->SetSensitiveDetector(SiPhys);
		fRun->SetSensitiveDetector(CsIPhys);

		if (fPar -> GetParBool("Si-CsIfIn"))
		{
			const double Fx = fPar -> GetParDouble("Fx");
			const double Fy = fPar -> GetParDouble("Fy");
			const double Fz = fPar -> GetParDouble("Fz");

			G4VSolid* solid_sicsif_hole = new G4Box("solid_sicsif_hole",dX/2.,dY/2.,1.3*Fz/2.);
			G4VSolid* solid_sicsif = new G4Box("solid_sicsif",Fx/2.,Fy/2.,Fz/2.);
			solid_sicsif = new G4SubtractionSolid("solid_sub_sicsif",solid_sicsif,solid_sicsif_hole);

			G4LogicalVolume* logic_sicsif = new G4LogicalVolume(solid_sicsif,matAl,"logic_sicsif");

			auto fVisFrame = new G4VisAttributes();
			fVisFrame -> SetForceWireframe(true);
			logic_sicsif -> SetVisAttributes(fVisFrame);

			new G4PVPlacement(SiTr,logic_sicsif,"physic_sicsif",WorldLog,false,SiID+2,true);
		}

		auto fVisSi = new G4VisAttributes();
		fVisSi->SetColor(G4Color::Cyan());
		fVisSi->SetForceWireframe(true);
		logicSi->SetVisAttributes(fVisSi);
		logicCsI->SetVisAttributes(fVisSi);

//		if (fPar->GetParInt("comSetup") == 0) 
//		{
//			//Naming scheme: SC (beamline) -> SCT (tilted) -> SCA (tiled + AT-TPC altermnative)
//			G4double scaDimX = 60.;
//			G4double scaDimY = 60.;
//			G4double scaDimZ =  3.;
//			G4double scaPosZ0 = sctPosZ1 +  50.;
//			G4double scaPosZ1 = sctPosZ1 + 100.;
//
//			G4Box*           SCASol = new G4Box("SCAltSolid", scaDimX/2, scaDimY/2, scaDimZ/2);
//			G4LogicalVolume* SCALog = new G4LogicalVolume(SCASol, scMat, "SCAltLogic");
//			SCALog->SetVisAttributes(fVisSCT);
//
//			const double SCAOfsX0 = sin(comTheta1 * deg) * (scaPosZ0 + scaDimZ/2.);
//			const double SCAOfsZ0 = cos(comTheta1 * deg) * (scaPosZ0 + scaDimZ/2.);
//			G4Transform3D SCATr0 = G4Transform3D(rotTheta1, G4ThreeVector(SCAOfsX0, 0, SCAOfsZ0));
//			G4VPhysicalVolume* SCAPhys0 = new G4PVPlacement(SCATr0,SCALog,"SCA0", WorldLog, false, sctID+10, true);
//			fRun->SetSensitiveDetector(SCAPhys0);
//
//			const double SCAOfsX1 = sin(comTheta1 * deg) * (scaPosZ1 + scaDimZ/2.);
//			const double SCAOfsZ1 = cos(comTheta1 * deg) * (scaPosZ1 + scaDimZ/2.);
//			G4Transform3D SCATr1 = G4Transform3D(rotTheta1, G4ThreeVector(SCAOfsX1, 0, SCAOfsZ1));
////			G4VPhysicalVolume* SCAPhys1 = new G4PVPlacement(SCATr1,SCALog,"SCA1", WorldLog, false, sctID+11, true);
////			fRun->SetSensitiveDetector(SCAPhys1);
//		}
	}

	//ND
	//--------------------------------------------------------------------

	if (fPar->GetParBool("NDIn"))
	{
//		G4Material* ndMat = FindMaterial("G4_PLASTIC_SC_VINYLTOLUENE");
		G4Material* ndMat = fNist->FindOrBuildMaterial("G4_PLASTIC_SC_VINYLTOLUENE");

		G4int    ndID    = fPar->GetParInt("ndID");
		G4int    ndSlatN = fPar->GetParInt("ndSlatN"); //# of pairs: i.e., 1 means two slats
		G4double ndSlatL = fPar->GetParDouble("ndSlatL"); //Slat length (x)
		G4double ndSlatW = fPar->GetParDouble("ndSlatW"); //Slat width (y)
		G4double ndSlatT = fPar->GetParDouble("ndSlatT"); //Slat thickness (z)
		G4double ndPosY  = fPar->GetParDouble("ndPosY");
		G4double ndPosZ  = fPar->GetParDouble("ndPosZ");

		G4Box*           NDSol = new G4Box("NDSol", ndSlatL/2, ndSlatW/2, ndSlatT/2);
		G4LogicalVolume* NDLog = new G4LogicalVolume(NDSol, ndMat, "NDLog");

		auto fVisND = new G4VisAttributes();
		fVisND->SetColor(G4Color::Magenta());
		fVisND->SetForceWireframe(true);
		NDLog->SetVisAttributes(fVisND);
		NDLog->SetVisAttributes(fVisND);

		for (int a=0; a<ndSlatN; a++) //# of slat pairs,  by radial distance from beam axis
		for (int b=0; b<2; b++) //Pair
		{
			const short mID  = a*10 + b + 1;
			const float posY = ndSlatW*a + ndSlatW/2 + ndPosY;
			G4ThreeVector NDPos(0, b==0?posY:(-posY), ndSlatT/2 + ndPosZ);
			auto NDPhy = new G4PVPlacement(0, NDPos, NDLog, Form("ND_%i", mID), WorldLog, false, ndID+mID, true);
			fRun->SetSensitiveDetector(NDPhy);
		}//a, b
	}//ND

	return WorldPhy;
}//Construct
