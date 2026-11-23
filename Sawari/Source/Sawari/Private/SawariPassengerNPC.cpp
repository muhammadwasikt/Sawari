#include "SawariPassengerNPC.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

ASawariPassengerNPC::ASawariPassengerNPC()
{
    PrimaryActorTick.bCanEverTick = false;
    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("PassengerRoot"));
    SetRootComponent(SceneRoot);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> Basic(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    if (!Cube.Succeeded() || !Sphere.Succeeded()) return;

    auto Part = [this, &Basic](const TCHAR* Name, UStaticMesh* Mesh, const FVector& Location, const FVector& Scale, const FLinearColor& Color)
    {
        UStaticMeshComponent* Component = CreateDefaultSubobject<UStaticMeshComponent>(FName(Name));
        Component->SetupAttachment(SceneRoot);
        Component->SetStaticMesh(Mesh);
        Component->SetRelativeLocation(Location);
        Component->SetRelativeScale3D(Scale);
        Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        if (Basic.Succeeded())
        {
            UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(Basic.Object, this);
            if (Mat)
            {
                Mat->SetVectorParameterValue(TEXT("Color"), Color);
                Mat->SetVectorParameterValue(TEXT("BaseColor"), Color);
                Component->SetMaterial(0, Mat);
            }
        }
        return Component;
    };

    const FLinearColor Skin(0.62f, 0.39f, 0.25f);
    const FLinearColor Shirt(0.10f, 0.27f, 0.56f);
    const FLinearColor Trousers(0.08f, 0.095f, 0.13f);
    const FLinearColor HairColor(0.045f, 0.035f, 0.025f);
    Torso = Part(TEXT("PassengerTorso"), Cube.Object, FVector(0, 0, 111), FVector(0.32f, 0.24f, 0.70f), Shirt);
    Head = Part(TEXT("PassengerHead"), Sphere.Object, FVector(0, 0, 158), FVector(0.18f, 0.18f, 0.20f), Skin);
    Hair = Part(TEXT("PassengerHair"), Sphere.Object, FVector(-2, 0, 166), FVector(0.185f, 0.185f, 0.10f), HairColor);
    LeftArm = Part(TEXT("PassengerLeftArm"), Cube.Object, FVector(0, -30, 111), FVector(0.13f, 0.12f, 0.68f), Shirt);
    RightArm = Part(TEXT("PassengerRightArm"), Cube.Object, FVector(0, 30, 111), FVector(0.13f, 0.12f, 0.68f), Shirt);
    LeftLeg = Part(TEXT("PassengerLeftLeg"), Cube.Object, FVector(0, -14, 38), FVector(0.15f, 0.15f, 0.76f), Trousers);
    RightLeg = Part(TEXT("PassengerRightLeg"), Cube.Object, FVector(0, 14, 38), FVector(0.15f, 0.15f, 0.76f), Trousers);
}
