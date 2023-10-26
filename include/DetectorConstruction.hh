#ifndef DetectorConstruction_h
#define DetectorConstruction_h 1

#include "G4VUserDetectorConstruction.hh"
#include "globals.hh"

class G4VPhysicalVolume;
class G4Material;
class G4GlobalMagFieldMessenger;

class DetectorConstruction : public G4VUserDetectorConstruction
{
  public:
    DetectorConstruction();
    ~DetectorConstruction();

  public:
    G4VPhysicalVolume* Construct() override;
    void ConstructSDandField() override;

    // get methods
    //
    const G4VPhysicalVolume* GetPhysDetector() const;

  private:
    // methods
    //
    void DefineMaterials();
    G4VPhysicalVolume* DefineVolumes();

  public:
    G4VPhysicalVolume* physWorld;

  private:  
    // data members
    G4VPhysicalVolume* physDetector = nullptr;

    G4bool checkOverlap = true; 
    G4Material* Air;
    G4Material* Vacuum;
    G4Material* Al27;
    G4Material* B10;
    G4Material* Si;
    G4Material* Al_N;
    G4Material* Ti;
};

// inline functions

inline const G4VPhysicalVolume* DetectorConstruction::GetPhysDetector() const 
{
  return physDetector;
}

#endif

