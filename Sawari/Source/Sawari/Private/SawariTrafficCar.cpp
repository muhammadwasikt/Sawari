#include "SawariTrafficCar.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

ASawariTrafficCar::ASawariTrafficCar()
{
    PrimaryActorTick.bCanEverTick = true;
    SetActorTickInterval(0.0f);

    CollisionBody = CreateDefaultSubobject<UBoxComponent>(TEXT("TrafficCollision"));
    SetRootComponent(CollisionBody);
    CollisionBody->SetBoxExtent(FVector(118.0f, 54.0f, 48.0f));
    CollisionBody->SetCollisionProfileName(TEXT("BlockAll"));
    CollisionBody->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    CollisionBody->SetSimulatePhysics(false);

    BuildVisuals();
}

void ASawariTrafficCar::BuildVisuals()
{
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> Basic(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    if (!Cube.Succeeded() || !Cylinder.Succeeded()) return;

    const FLinearColor BodyColor = FLinearColor::MakeRandomColor();
    UMaterialInstanceDynamic* BodyMaterial = Basic.Succeeded() ? UMaterialInstanceDynamic::Create(Basic.Object, this) : nullptr;
    if (BodyMaterial)
    {
        BodyMaterial->SetVectorParameterValue(TEXT("Color"), BodyColor);
        BodyMaterial->SetVectorParameterValue(TEXT("BaseColor"), BodyColor);
    }
    UMaterialInstanceDynamic* GlassMaterial = Basic.Succeeded() ? UMaterialInstanceDynamic::Create(Basic.Object, this) : nullptr;
    if (GlassMaterial)
    {
        GlassMaterial->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.035f, 0.10f, 0.13f));
        GlassMaterial->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.035f, 0.10f, 0.13f));
    }

    BodyVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TrafficBodyVisual"));
    BodyVisual->SetupAttachment(CollisionBody);
    BodyVisual->SetStaticMesh(Cube.Object);
    BodyVisual->SetRelativeLocation(FVector(0, 0, 0));
    BodyVisual->SetRelativeScale3D(FVector(2.15f, 1.05f, 0.62f));
    BodyVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    if (BodyMaterial) BodyVisual->SetMaterial(0, BodyMaterial);

    RoofVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TrafficRoofVisual"));
    RoofVisual->SetupAttachment(CollisionBody);
    RoofVisual->SetStaticMesh(Cube.Object);
    RoofVisual->SetRelativeLocation(FVector(-4.0f, 0.0f, 54.0f));
    RoofVisual->SetRelativeScale3D(FVector(1.02f, 0.84f, 0.46f));
    RoofVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    if (GlassMaterial) RoofVisual->SetMaterial(0, GlassMaterial);

    for (int32 Index = 0; Index < 4; ++Index)
    {
        UStaticMeshComponent* Wheel = CreateDefaultSubobject<UStaticMeshComponent>(
            *FString::Printf(TEXT("TrafficWheel_%d"), Index));
        Wheel->SetupAttachment(CollisionBody);
        Wheel->SetStaticMesh(Cylinder.Object);
        const float X = Index < 2 ? 66.0f : -66.0f;
        const float Y = (Index % 2 == 0) ? 55.0f : -55.0f;
        Wheel->SetRelativeLocation(FVector(X, Y, -14.0f));
        Wheel->SetRelativeRotation(FRotator(0.0f, 0.0f, 90.0f));
        Wheel->SetRelativeScale3D(FVector(0.62f, 0.62f, 0.18f));
        Wheel->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Wheels.Add(Wheel);
    }
}

void ASawariTrafficCar::InitializeRoute(const TArray<FVector>& InRoute, int32 StartIndex, float SpeedKmh)
{
    RoutePoints = InRoute;
    if (RoutePoints.Num() < 2)
    {
        SetActorHiddenInGame(true);
        SetActorEnableCollision(false);
        return;
    }

    CurrentPointIndex = FMath::Clamp(StartIndex, 0, RoutePoints.Num() - 1);
    SpeedCentimetersPerSecond = FMath::Clamp(SpeedKmh, 18.0f, 58.0f) * (100000.0f / 3600.0f);
    FVector Start = RoutePoints[CurrentPointIndex];
    Start.Z = 60.0f;
    SetActorLocation(Start);
    if (CurrentPointIndex + 1 < RoutePoints.Num())
    {
        const FVector Delta = RoutePoints[CurrentPointIndex + 1] - Start;
        SetActorRotation(FRotator(0.0f, Delta.Rotation().Yaw, 0.0f));
    }
}

void ASawariTrafficCar::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (RoutePoints.Num() < 2 || DeltaSeconds <= 0.0f) return;

    const int32 NextIndex = CurrentPointIndex + 1;
    if (NextIndex >= RoutePoints.Num())
    {
        // Road ways are open polylines, not two-way loops. Restart at the
        // route's beginning instead of driving the wrong direction in-lane.
        CurrentPointIndex = 0;
        FVector Restart = RoutePoints[0];
        Restart.Z = GetActorLocation().Z;
        SetActorLocation(Restart, false, nullptr, ETeleportType::TeleportPhysics);
        if (RoutePoints.Num() > 1)
        {
            const FVector Heading = RoutePoints[1] - RoutePoints[0];
            SetActorRotation(FRotator(0.0f, Heading.Rotation().Yaw, 0.0f));
        }
        return;
    }
    if (!RoutePoints.IsValidIndex(NextIndex)) return;

    const FVector Current = GetActorLocation();
    FVector Target = RoutePoints[NextIndex];
    Target.Z = Current.Z;

    const FVector Delta = Target - Current;
    if (Delta.Size2D() <= 240.0f)
    {
        CurrentPointIndex = NextIndex;
        BlockedTime = FMath::Max(0.0f, BlockedTime - DeltaSeconds);
        return;
    }

    const FVector Direction2D = Delta.GetSafeNormal2D();
    const FRotator TargetRotation(0.0f, Direction2D.Rotation().Yaw, 0.0f);
    const FVector NextLocation = FMath::VInterpConstantTo(Current, Target, DeltaSeconds, SpeedCentimetersPerSecond);

    FHitResult Hit;
    SetActorRotation(FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaSeconds, 3.0f));
    const bool bMoved = SetActorLocation(NextLocation, true, &Hit, ETeleportType::None);

    if (!bMoved && Hit.bBlockingHit)
    {
        BlockedTime += DeltaSeconds;
        SpeedCentimetersPerSecond = FMath::Max(0.0f, SpeedCentimetersPerSecond - 500.0f * DeltaSeconds);
        if (BlockedTime > 2.0f)
        {
            // Resume gently after a temporary blockage; route state is preserved.
            BlockedTime = 0.0f;
            SpeedCentimetersPerSecond = FMath::Max(650.0f, SpeedCentimetersPerSecond);
        }
    }
    else
    {
        SpeedCentimetersPerSecond = FMath::FInterpTo(SpeedCentimetersPerSecond,
            FMath::Clamp(SpeedCentimetersPerSecond, 800.0f, 1600.0f), DeltaSeconds, 1.5f);
        BlockedTime = 0.0f;
    }
}
