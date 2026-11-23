#include "SawariPrototypeCity.h"

#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/SkyLightComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

ASawariPrototypeCity::ASawariPrototypeCity()
{
    PrimaryActorTick.bCanEverTick = false;
    USceneComponent* SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);

    Ground = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("GroundInstances"));
    Ground->SetupAttachment(SceneRoot);
    Roads = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("RoadInstances"));
    Roads->SetupAttachment(SceneRoot);
    Sidewalks = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("SidewalkInstances"));
    Sidewalks->SetupAttachment(SceneRoot);
    Buildings = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("BuildingInstances"));
    Buildings->SetupAttachment(SceneRoot);
    RoadMarkings = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("RoadMarkingInstances"));
    RoadMarkings->SetupAttachment(SceneRoot);
    PickupMarker = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("PickupMarkerInstances"));
    PickupMarker->SetupAttachment(SceneRoot);
    DropoffMarker = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("DropoffMarkerInstances"));
    DropoffMarker->SetupAttachment(SceneRoot);

    SunLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("KarachiSun"));
    SunLight->SetupAttachment(SceneRoot);
    SunLight->SetRelativeRotation(FRotator(-48.0f, -35.0f, 0.0f));
    SunLight->SetIntensity(100000.0f);
    SunLight->SetAtmosphereSunLight(true);

    SkyLight = CreateDefaultSubobject<USkyLightComponent>(TEXT("AmbientSkyLight"));
    SkyLight->SetupAttachment(SceneRoot);
    SkyLight->SetMobility(EComponentMobility::Movable);
    SkyLight->SetIntensity(1.0f);

    SkyAtmosphere = CreateDefaultSubobject<USkyAtmosphereComponent>(TEXT("SkyAtmosphere"));
    SkyAtmosphere->SetupAttachment(SceneRoot);

    HeightFog = CreateDefaultSubobject<UExponentialHeightFogComponent>(TEXT("KarachiHaze"));
    HeightFog->SetupAttachment(SceneRoot);
    HeightFog->SetFogDensity(0.0015f);
    HeightFog->SetFogHeightFalloff(0.18f);
    HeightFog->SetFogInscatteringColor(FLinearColor(0.60f, 0.70f, 0.82f));
    HeightFog->SetStartDistance(2500.0f);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderFinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialFinder(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    if (CubeFinder.Succeeded()) CubeMesh = CubeFinder.Object;
    if (CylinderFinder.Succeeded()) CylinderMesh = CylinderFinder.Object;
    if (MaterialFinder.Succeeded()) BasicMaterial = MaterialFinder.Object;
}

void ASawariPrototypeCity::ApplyColor(UInstancedStaticMeshComponent* Component, const FLinearColor& Color)
{
    if (!Component || !BasicMaterial) return;
    UMaterialInstanceDynamic* Dynamic = UMaterialInstanceDynamic::Create(BasicMaterial, this);
    if (Dynamic)
    {
        Dynamic->SetVectorParameterValue(TEXT("Color"), Color);
        Dynamic->SetVectorParameterValue(TEXT("BaseColor"), Color);
        Component->SetMaterial(0, Dynamic);
    }
}

void ASawariPrototypeCity::AddBox(UInstancedStaticMeshComponent* Component, const FVector& Location, const FVector& Scale)
{
    if (!Component) return;
    Component->AddInstance(FTransform(FRotator::ZeroRotator, Location, Scale), false);
}

void ASawariPrototypeCity::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    if (!CubeMesh || !CylinderMesh) return;

    const TArray<UInstancedStaticMeshComponent*> AllComponents = {
        Ground, Roads, Sidewalks, Buildings, RoadMarkings, PickupMarker, DropoffMarker
    };
    for (UInstancedStaticMeshComponent* Component : AllComponents)
    {
        if (Component)
        {
            Component->ClearInstances();
            Component->SetStaticMesh(CubeMesh);
            Component->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            Component->SetCollisionResponseToAllChannels(ECR_Block);
        }
    }

    Ground->SetStaticMesh(CubeMesh);
    Roads->SetStaticMesh(CubeMesh);
    Sidewalks->SetStaticMesh(CubeMesh);
    Buildings->SetStaticMesh(CubeMesh);
    RoadMarkings->SetStaticMesh(CubeMesh);
    PickupMarker->SetStaticMesh(CylinderMesh);
    DropoffMarker->SetStaticMesh(CylinderMesh);

    ApplyColor(Ground, FLinearColor(0.20f, 0.27f, 0.19f));
    ApplyColor(Roads, FLinearColor(0.095f, 0.105f, 0.12f));
    ApplyColor(Sidewalks, FLinearColor(0.48f, 0.45f, 0.38f));
    ApplyColor(Buildings, FLinearColor(0.62f, 0.55f, 0.43f));
    ApplyColor(RoadMarkings, FLinearColor(0.92f, 0.84f, 0.58f));
    ApplyColor(PickupMarker, FLinearColor(1.0f, 0.67f, 0.05f));
    ApplyColor(DropoffMarker, FLinearColor(0.08f, 0.82f, 0.50f));

    Ground->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    Roads->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    Buildings->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    Sidewalks->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    RoadMarkings->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    PickupMarker->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    DropoffMarker->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    BuildBlockout();
}

void ASawariPrototypeCity::BuildBlockout()
{
    // 1 Unreal unit = 1 cm. 11 km of boulevard leaves room for the 1.2 km
    // pickup approach plus the 5 km passenger trip and a destination buffer.
    AddBox(Ground, FVector(0, 0, -90), FVector(11000, 620, 1.0f));
    AddBox(Roads, FVector(0, 0, -28), FVector(11000, 24, 0.28f));
    AddBox(Roads, FVector(0, 0, -25), FVector(22, 620, 0.30f));

    for (float X : {-310000.0f, -170000.0f, 70000.0f, 230000.0f, 380000.0f})
    {
        AddBox(Roads, FVector(X, 0, -27), FVector(12, 120, 0.26f));
    }

    AddBox(Sidewalks, FVector(0, 1550, -15), FVector(11000, 4.5f, 0.32f));
    AddBox(Sidewalks, FVector(0, -1550, -15), FVector(11000, 4.5f, 0.32f));

    for (int32 Index = 0; Index < 184; ++Index)
    {
        const float X = -546000.0f + Index * 6000.0f;
        AddBox(RoadMarkings, FVector(X, 0, -12), FVector(28.0f, 0.14f, 0.025f));
    }
    for (float X : {-170000.0f, 70000.0f, 230000.0f})
    {
        for (int32 Stripe = -4; Stripe <= 4; ++Stripe)
        {
            AddBox(RoadMarkings, FVector(X + Stripe * 420.0f, 0, -11), FVector(1.1f, 12.0f, 0.03f));
        }
    }

    for (int32 Block = 0; Block < 9; ++Block)
    {
        const float X = -320000.0f + Block * 85000.0f;
        const float HeightA = 1800.0f + (Block % 4) * 1100.0f;
        const float HeightB = 1500.0f + ((Block + 2) % 4) * 900.0f;
        AddBox(Buildings, FVector(X, 6200, HeightA / 2.0f - 15.0f), FVector(24 + (Block % 3) * 5, 28, HeightA / 100.0f));
        AddBox(Buildings, FVector(X + 2500, 10500, HeightB / 2.0f - 15.0f), FVector(18, 22, HeightB / 100.0f));
        AddBox(Buildings, FVector(X, -6200, HeightB / 2.0f - 15.0f), FVector(22, 30, HeightB / 100.0f));
        AddBox(Buildings, FVector(X - 2500, -10800, HeightA / 2.0f - 15.0f), FVector(16, 20, HeightA / 100.0f));
    }

    PickupMarker->AddInstance(FTransform(FRotator::ZeroRotator, FVector(-60000, 0, 8), FVector(3.5f, 3.5f, 0.12f)), false);
    DropoffMarker->AddInstance(FTransform(FRotator::ZeroRotator, FVector(440000, 0, 8), FVector(4.5f, 4.5f, 0.12f)), false);
    SetRideMarkers(false, false);
}

void ASawariPrototypeCity::SetRideMarkers(bool bShowPickup, bool bShowDropoff)
{
    if (PickupMarker) PickupMarker->SetVisibility(bShowPickup, true);
    if (DropoffMarker) DropoffMarker->SetVisibility(bShowDropoff, true);
}
