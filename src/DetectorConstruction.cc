#include "DetectorConstruction.hh"

#include "G4Material.hh"
#include "G4NistManager.hh"

#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4PVReplica.hh"
#include "G4GlobalMagFieldMessenger.hh"
#include "G4AutoDelete.hh"

#include "G4GeometryManager.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4SolidStore.hh"

#include "G4VisAttributes.hh"
#include "G4Colour.hh"

#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"

DetectorConstruction::DetectorConstruction()
	: G4VUserDetectorConstruction()
{}

DetectorConstruction::~DetectorConstruction()
{}

G4VPhysicalVolume* DetectorConstruction::Construct()
{
  // Define materials
  DefineMaterials();

  // Define volumes
  return DefineVolumes();
}

void DetectorConstruction::DefineMaterials()
{
	// Materials defined using NIST Manager
	auto nistManager = G4NistManager::Instance();

	// Air
	Air = nistManager->FindOrBuildMaterial("G4_AIR");

	// Vacuum
	Vacuum = nistManager->FindOrBuildMaterial("G4_Galactic");

	// Al27
	Al27 = new G4Material("Al27", 13., 26.9815384*g/mole, 2.699*g/cm3);

	// B10
	B10 = new G4Material("B10", 5., 10.0129369*g/mole, 2.37*g/cm3);

	// Silicon
	Si = nistManager->FindOrBuildMaterial("G4_Si");

	// 
	Ti = nistManager->FindOrBuildMaterial("G4_Ti");

	// Build Al-N material
	G4Element* ElN= nistManager->FindOrBuildElement(7);
	G4Element* ElAl = nistManager->FindOrBuildElement(13);
	Al_N = new G4Material("Al_N", 3.255 *g/cm3, 2);
    Al_N->AddElement(ElAl, 1);
    Al_N->AddElement(ElN, 1);

	// Print materials
	// G4cout << *(G4Material::GetMaterialTable()) << G4endl;
}

G4VPhysicalVolume* DetectorConstruction::DefineVolumes()
{
  //
  // World
  //
	G4Box* solidWorld = new G4Box("solidWorld", 500.*cm, 500.*cm, 500.*cm);
	G4LogicalVolume* logicWorld = new G4LogicalVolume(solidWorld, Vacuum, "logicWorld");
	physWorld = new G4PVPlacement(0, G4ThreeVector(), logicWorld, "physWorld", 
                                0, false, 0, true);
	logicWorld->SetVisAttributes(G4VisAttributes::GetInvisible());

	//
	// Target
	//
	G4double targetXDim = 1.*cm;
	G4double targetYDim = 1.*cm;
	G4double targetZThick =  100.*nm;

	G4double backLayerXDim = targetXDim;
	G4double backLayerYDim = targetYDim;
	G4double backLayerZThick = 150.*nm;

	G4double frameXDim = targetXDim;
	G4double frameYDim = targetYDim;
	G4double frameZThick = targetZThick + backLayerZThick;

	// Target frame construction
	// solid
	G4Box* solidTargetFrame = new G4Box("solidTargetFrame",
	                                    0.5*frameXDim,
				                        0.5*frameYDim,
				                        0.5*frameZThick);
	// logic
	G4LogicalVolume* logicTargetFrame = new G4LogicalVolume(solidTargetFrame,
						                                    Vacuum,
							                                "logicTargetFrame");
	// physics
	G4double rotationAngle = 60.*deg;
	G4RotationMatrix rotm = G4RotationMatrix();
	rotm.rotateY(90*deg - rotationAngle);
	G4ThreeVector targetFramePos = G4ThreeVector(0,0,0);
	G4Transform3D targetFrameTrans = G4Transform3D(rotm, targetFramePos);
	// G4VPhysicalVolume* physTargetFrame =
	new G4PVPlacement(targetFrameTrans, logicTargetFrame, "TargetFrame",
					   logicWorld, false, 0, checkOverlap);

	// Target construction
	// solid
	G4Box* solidTarget =  new G4Box("Target", 
			                        0.5*targetXDim, 
				                    0.5*targetYDim, 
				                    0.5*targetZThick);
	// logic
	G4LogicalVolume* logicTarget =  new G4LogicalVolume(solidTarget, 
						                                Al_N,
							                            "Target");
	// physics
  	G4double targetZPos = (-0.5*frameZThick + 0.5*targetZThick);
    // G4VPhysicalVolume* physTarget = 
	new G4PVPlacement(0, G4ThreeVector(0, 0, targetZPos),
		              logicTarget, "Target", logicTargetFrame,
					  false, 0, checkOverlap);

	G4VisAttributes * targetColor = new G4VisAttributes(G4Colour(0.3 ,0.3 ,0.3));
	targetColor->SetVisibility(true);
	targetColor->SetForceSolid(true);
	logicTarget->SetVisAttributes(targetColor);

	// Backing layer construction
	// solid
	G4Box* solidBackLayer =  new G4Box("solidBackLayer",
		                               0.5*backLayerXDim,
				                       0.5*backLayerYDim,
				                       0.5*backLayerZThick);
	// logic
	G4LogicalVolume* logicBackLayer = new G4LogicalVolume(solidBackLayer,
														  Ti, 
														  "logicBackLayer");
	// physics
	G4double backLayerZPos = 0.5*frameZThick - 0.5*backLayerZThick;
	// G4VPhysicalVolume* physBackLayer = 
	new G4PVPlacement(0, G4ThreeVector(0, 0, backLayerZPos),
						logicBackLayer, "BackLayer",  logicTargetFrame,
						false, 0, checkOverlap);

	G4VisAttributes * backLayerColor = new G4VisAttributes(G4Colour(1, 0., 0.));
	backLayerColor->SetVisibility(true);
	backLayerColor->SetForceSolid(true);
	logicBackLayer->SetVisAttributes(backLayerColor);


	//
	// Detectors
	//
	G4double thetaMat[6] = {40.*deg, 70.*deg, 150.*deg, 233.*deg, 270.*deg, 330.*deg};

	G4double detXDim = 10.*mm;
	G4double detYDim = 10.*mm;
	G4double detZThick = 0.30*mm;

	// Solid
	G4Box* solidDet = new G4Box("solidDet", 0.5*detXDim, 0.5*detYDim, 0.5*detZThick);
	// Logic
	G4LogicalVolume* logicDet = new G4LogicalVolume(solidDet, Si, "logicDet");
	// Phyics
  	for(int copyNo=0; copyNo<6; copyNo++)
  	{
		G4double theta = 0.;
		G4double phi = 0.;
		G4ThreeVector uz;

		G4double detThick = 0.3*mm;
		G4double detTarDis = 6.2*cm; //Khoang cach det-target

		theta = thetaMat[copyNo];
		uz.setX(std::cos(phi)*std::sin(theta));
		uz.setY(std::sin(phi)*std::sin(theta));
		uz.setZ(std::cos(theta));

		G4ThreeVector detPos = (detTarDis + 0.5*detThick)*uz;
		G4RotationMatrix rotm = G4RotationMatrix();
		rotm.rotateY(theta);
		G4Transform3D trans = G4Transform3D(rotm, detPos);
		physDetector = new G4PVPlacement(trans, logicDet, "physDetector", 
										logicWorld, false, copyNo, checkOverlap);
  	}

  //
  // Always return the physical World
  //
  return physWorld;
}

void DetectorConstruction::ConstructSDandField()
{}
