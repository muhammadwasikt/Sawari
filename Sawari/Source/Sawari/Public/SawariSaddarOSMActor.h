#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SawariSaddarOSMActor.generated.h"

class UBoxComponent;
class UProceduralMeshComponent;
class UStaticMeshComponent;
class UStaticMesh;
class UMaterialInterface;
class UPrimitiveComponent;

struct FSawariRoadGraphEdge
{
    int32 TargetNode = INDEX_NONE;
    float DistanceCm = 0.0f;
};

struct FSawariRoadGraphNode
{
    FVector Location = FVector::ZeroVector;
    TArray<FSawariRoadGraphEdge> Outgoing;
};

/**
 * Loads bundled OpenStreetMap GeoJSON for the Saddar-Clifton prototype corridor.
 * The game performs no network requests; Scripts/fetch_saddar_osm.py stages the
 * data into Content/SawariData/Saddar before packaging.
 */
UCLASS()
class SAWARI_API ASawariSaddarOSMActor : public AActor
{
    GENERATED_BODY()

public:
    ASawariSaddarOSMActor();
    virtual void BeginPlay() override;

    UFUNCTION(BlueprintPure, Category = "Sawari|Map")
    bool IsOSMDataLoaded() const { return bOSMDataLoaded; }

    UFUNCTION(BlueprintPure, Category = "Sawari|Map")
    FVector GeoToWorld(double Latitude, double Longitude, double Z = 0.0) const;

    UFUNCTION(BlueprintPure, Category = "Sawari|Map")
    FVector GetTaxiStartLocation() const;

    UFUNCTION(BlueprintPure, Category = "Sawari|Map")
    FVector GetPickupLocation() const;

    UFUNCTION(BlueprintPure, Category = "Sawari|Map")
    FVector GetDropoffLocation() const;

    UFUNCTION(BlueprintCallable, Category = "Sawari|Map")
    void SetRideMarkers(bool bShowPickup, bool bShowDropoff);

    const TArray<TArray<FVector>>& GetTrafficRoutes() const { return TrafficRoutes; }

    /** Finds the shortest connected, direction-aware OSM road route between two world positions. */
    bool FindDrivingRoute(const FVector& StartWorld, const FVector& EndWorld, TArray<FVector>& OutRoute) const;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sawari|Map")
    TObjectPtr<UBoxComponent> GroundCollider;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sawari|Map")
    TObjectPtr<UProceduralMeshComponent> RoadMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sawari|Map")
    TObjectPtr<UProceduralMeshComponent> BuildingMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sawari|Map")
    TObjectPtr<UStaticMeshComponent> PickupMarker;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sawari|Map")
    TObjectPtr<UStaticMeshComponent> DropoffMarker;

private:
    UPROPERTY()
    TObjectPtr<UStaticMesh> MarkerMesh;

    UPROPERTY()
    TObjectPtr<UMaterialInterface> BasicMaterial;

    bool bOSMDataLoaded = false;
    double OriginLatitude = 24.8375;
    double OriginLongitude = 67.0400;
    double LongitudeScale = 101800.0;

    TArray<TArray<FVector>> TrafficRoutes;
    TArray<FSawariRoadGraphNode> NavigationGraph;
    TMap<FIntPoint, int32> NavigationNodeLookup;

    int32 GetOrAddNavigationNode(const FVector& WorldLocation);
    void AddNavigationEdge(int32 FromNode, int32 ToNode);
    bool LoadManifest();
    bool LoadRoads();
    bool LoadBuildings();
    bool ParseGeoJSON(const FString& FileName, TArray<TSharedPtr<class FJsonValue>>& OutFeatures) const;
    void BuildRoadMeshes(const TArray<TSharedPtr<class FJsonValue>>& Features);
    void BuildBuildingMeshes(const TArray<TSharedPtr<class FJsonValue>>& Features);
    void ApplyColor(UPrimitiveComponent* Component, const FLinearColor& Color);
    FVector SnapToNearestRoad(const FVector& WorldPoint) const;

    static float RoadWidthCentimeters(const FString& HighwayType);
    static float EstimateBuildingHeight(const TSharedPtr<class FJsonObject>& Properties);
    static bool TriangulateFootprint(const TArray<FVector2D>& Polygon, TArray<int32>& OutTriangles);
};
