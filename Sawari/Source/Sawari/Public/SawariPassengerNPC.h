#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SawariPassengerNPC.generated.h"

class USceneComponent;
class UStaticMeshComponent;

/** Visible low-poly human passenger for the Phase 0 pickup checkpoint. */
UCLASS()
class SAWARI_API ASawariPassengerNPC : public AActor
{
    GENERATED_BODY()
public:
    ASawariPassengerNPC();
private:
    UPROPERTY()
    TObjectPtr<USceneComponent> SceneRoot;
    UPROPERTY()
    TObjectPtr<UStaticMeshComponent> Torso;
    UPROPERTY()
    TObjectPtr<UStaticMeshComponent> Head;
    UPROPERTY()
    TObjectPtr<UStaticMeshComponent> Hair;
    UPROPERTY()
    TObjectPtr<UStaticMeshComponent> LeftArm;
    UPROPERTY()
    TObjectPtr<UStaticMeshComponent> RightArm;
    UPROPERTY()
    TObjectPtr<UStaticMeshComponent> LeftLeg;
    UPROPERTY()
    TObjectPtr<UStaticMeshComponent> RightLeg;
};
