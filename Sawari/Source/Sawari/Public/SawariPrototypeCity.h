#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SawariPrototypeCity.generated.h"

class UInstancedStaticMeshComponent;
class UStaticMesh;
class UMaterialInterface;
class UDirectionalLightComponent;
class USkyLightComponent;
class USkyAtmosphereComponent;
class UExponentialHeightFogComponent;

/** Procedural first-city blockout and daylight environment for the playable Karachi test. */
UCLASS()
class SAWARI_API ASawariPrototypeCity : public AActor
{
    GENERATED_BODY()
public:
    ASawariPrototypeCity();
    virtual void OnConstruction(const FTransform& Transform) override;

    UFUNCTION(BlueprintCallable, Category = "Sawari|World")
    void SetRideMarkers(bool bShowPickup, bool bShowDropoff);

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sawari|World")
    TObjectPtr<UInstancedStaticMeshComponent> Ground;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sawari|World")
    TObjectPtr<UInstancedStaticMeshComponent> Roads;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sawari|World")
    TObjectPtr<UInstancedStaticMeshComponent> Sidewalks;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sawari|World")
    TObjectPtr<UInstancedStaticMeshComponent> Buildings;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sawari|World")
    TObjectPtr<UInstancedStaticMeshComponent> RoadMarkings;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sawari|World")
    TObjectPtr<UInstancedStaticMeshComponent> PickupMarker;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sawari|World")
    TObjectPtr<UInstancedStaticMeshComponent> DropoffMarker;

private:
    UPROPERTY()
    TObjectPtr<UStaticMesh> CubeMesh;
    UPROPERTY()
    TObjectPtr<UStaticMesh> CylinderMesh;
    UPROPERTY()
    TObjectPtr<UMaterialInterface> BasicMaterial;
    UPROPERTY(VisibleAnywhere, Category = "Sawari|Lighting")
    TObjectPtr<UDirectionalLightComponent> SunLight;
    UPROPERTY(VisibleAnywhere, Category = "Sawari|Lighting")
    TObjectPtr<USkyLightComponent> SkyLight;
    UPROPERTY(VisibleAnywhere, Category = "Sawari|Lighting")
    TObjectPtr<USkyAtmosphereComponent> SkyAtmosphere;
    UPROPERTY(VisibleAnywhere, Category = "Sawari|Lighting")
    TObjectPtr<UExponentialHeightFogComponent> HeightFog;

    void AddBox(UInstancedStaticMeshComponent* Component, const FVector& Location, const FVector& Scale);
    void BuildBlockout();
    void ApplyColor(UInstancedStaticMeshComponent* Component, const FLinearColor& Color);
};
