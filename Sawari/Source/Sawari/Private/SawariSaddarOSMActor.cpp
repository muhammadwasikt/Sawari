#include "SawariSaddarOSMActor.h"

#include "Algo/Reverse.h"
#include "Components/BoxComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "HAL/PlatformFileManager.h"
#include "Json.h"
#include "JsonUtilities.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "ProceduralMeshComponent.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "UObject/ConstructorHelpers.h"

namespace SawariOSM
{
    const TCHAR* DataFolder = TEXT("SawariData/Saddar");
    const double PickupLatitude = 24.8558;
    const double PickupLongitude = 67.0280;
    const double DropoffLatitude = 24.8135;
    const double DropoffLongitude = 67.0300;
    const double TaxiLatitude = 24.8680;
    const double TaxiLongitude = 67.0180;

    bool ReadNumber(const TSharedPtr<FJsonObject>& Object, const FString& Key, double& OutValue)
    {
        if (!Object.IsValid()) return false;
        const TSharedPtr<FJsonValue>* Value = Object->Values.Find(Key);
        if (!Value || !Value->IsValid()) return false;
        if ((*Value)->Type == EJson::Number)
        {
            OutValue = (*Value)->AsNumber();
            return true;
        }
        if ((*Value)->Type == EJson::String)
        {
            return LexTryParseString(OutValue, *(*Value)->AsString());
        }
        return false;
    }

    bool ReadLatLon(const TSharedPtr<FJsonValue>& Value, double& OutLon, double& OutLat)
    {
        if (!Value.IsValid() || Value->Type != EJson::Array) return false;
        const TArray<TSharedPtr<FJsonValue>>& Pair = Value->AsArray();
        if (Pair.Num() < 2 || !Pair[0].IsValid() || !Pair[1].IsValid()) return false;
        OutLon = Pair[0]->AsNumber();
        OutLat = Pair[1]->AsNumber();
        return true;
    }

    double Cross2D(const FVector2D& A, const FVector2D& B, const FVector2D& C)
    {
        return (B.X - A.X) * (C.Y - A.Y) - (B.Y - A.Y) * (C.X - A.X);
    }

    bool PointInTriangle(const FVector2D& P, const FVector2D& A, const FVector2D& B, const FVector2D& C)
    {
        const double C1 = Cross2D(A, B, P);
        const double C2 = Cross2D(B, C, P);
        const double C3 = Cross2D(C, A, P);
        const bool bNegative = C1 < -0.01 || C2 < -0.01 || C3 < -0.01;
        const bool bPositive = C1 > 0.01 || C2 > 0.01 || C3 > 0.01;
        return !(bNegative && bPositive);
    }
}

ASawariSaddarOSMActor::ASawariSaddarOSMActor()
{
    PrimaryActorTick.bCanEverTick = false;
    USceneComponent* SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("OSMRoot"));
    SetRootComponent(SceneRoot);

    GroundCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("OSMGroundCollider"));
    GroundCollider->SetupAttachment(SceneRoot);
    GroundCollider->SetCollisionProfileName(TEXT("BlockAll"));
    GroundCollider->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

    RoadMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("OSMRoads"));
    RoadMesh->SetupAttachment(SceneRoot);
    RoadMesh->SetCollisionProfileName(TEXT("BlockAll"));

    BuildingMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("OSMBuildings"));
    BuildingMesh->SetupAttachment(SceneRoot);
    BuildingMesh->SetCollisionProfileName(TEXT("BlockAll"));

    PickupMarker = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("OSMPickupBeacon"));
    PickupMarker->SetupAttachment(SceneRoot);
    DropoffMarker = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("OSMDropoffBeacon"));
    DropoffMarker->SetupAttachment(SceneRoot);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderFinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> BasicFinder(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    if (CylinderFinder.Succeeded()) MarkerMesh = CylinderFinder.Object;
    if (BasicFinder.Succeeded()) BasicMaterial = BasicFinder.Object;

    PickupMarker->SetStaticMesh(MarkerMesh);
    DropoffMarker->SetStaticMesh(MarkerMesh);
    PickupMarker->SetRelativeScale3D(FVector(3.0f, 3.0f, 0.12f));
    DropoffMarker->SetRelativeScale3D(FVector(3.8f, 3.8f, 0.12f));
    PickupMarker->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    DropoffMarker->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    PickupMarker->SetVisibility(false, true);
    DropoffMarker->SetVisibility(false, true);
}

void ASawariSaddarOSMActor::BeginPlay()
{
    Super::BeginPlay();

    if (!LoadManifest())
    {
        UE_LOG(LogTemp, Warning, TEXT("Sawari OSM manifest missing. Run Scripts/fetch_saddar_osm.py to stage the Saddar-Clifton data."));
        return;
    }

    const bool bRoadsOK = LoadRoads();
    const bool bBuildingsOK = LoadBuildings();
    bOSMDataLoaded = bRoadsOK && bBuildingsOK;

    if (bOSMDataLoaded)
    {
        ApplyColor(RoadMesh, FLinearColor(0.095f, 0.105f, 0.12f));
        ApplyColor(BuildingMesh, FLinearColor(0.62f, 0.55f, 0.43f));
        ApplyColor(PickupMarker, FLinearColor(1.0f, 0.67f, 0.05f));
        ApplyColor(DropoffMarker, FLinearColor(0.08f, 0.82f, 0.50f));
        PickupMarker->SetWorldLocation(GetPickupLocation() + FVector(0, 0, 18));
        DropoffMarker->SetWorldLocation(GetDropoffLocation() + FVector(0, 0, 18));
        UE_LOG(LogTemp, Display, TEXT("Sawari real OSM city loaded. Traffic routes available: %d"), TrafficRoutes.Num());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Sawari OSM import incomplete: road or building geometry is empty."));
    }
}

bool ASawariSaddarOSMActor::LoadManifest()
{
    const FString ManifestPath = FPaths::Combine(FPaths::ProjectContentDir(), SawariOSM::DataFolder, TEXT("manifest.json"));
    FString Json;
    if (!FFileHelper::LoadFileToString(Json, *ManifestPath)) return false;

    TSharedPtr<FJsonObject> Object;
    const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Json);
    if (!FJsonSerializer::Deserialize(Reader, Object) || !Object.IsValid()) return false;

    const TArray<TSharedPtr<FJsonValue>>* Bounds = nullptr;
    if (Object->TryGetArrayField(TEXT("bbox_south_west_north_east"), Bounds)
        && Bounds && Bounds->Num() >= 4)
    {
        const double South = (*Bounds)[0]->AsNumber();
        const double West = (*Bounds)[1]->AsNumber();
        const double North = (*Bounds)[2]->AsNumber();
        const double East = (*Bounds)[3]->AsNumber();
        const float HalfWidth = static_cast<float>((East - West) * 111320.0
            * FMath::Cos(FMath::DegreesToRadians((North + South) * 0.5)) * 50.0);
        const float HalfHeight = static_cast<float>((North - South) * 110540.0 * 50.0);
        GroundCollider->SetBoxExtent(FVector(HalfWidth, HalfHeight, 50.0f));
        GroundCollider->SetRelativeLocation(FVector(0.0f, 0.0f, -38.0f));
    }

    const TArray<TSharedPtr<FJsonValue>>* Origin = nullptr;
    if (Object->TryGetArrayField(TEXT("origin_lat_lon"), Origin) && Origin && Origin->Num() >= 2)
    {
        OriginLatitude = (*Origin)[0]->AsNumber();
        OriginLongitude = (*Origin)[1]->AsNumber();
    }
    LongitudeScale = 111320.0 * FMath::Cos(FMath::DegreesToRadians(OriginLatitude));
    return true;
}

bool ASawariSaddarOSMActor::ParseGeoJSON(const FString& FileName, TArray<TSharedPtr<FJsonValue>>& OutFeatures) const
{
    const FString Path = FPaths::Combine(FPaths::ProjectContentDir(), SawariOSM::DataFolder, FileName);
    FString Json;
    if (!FFileHelper::LoadFileToString(Json, *Path)) return false;

    TSharedPtr<FJsonObject> Object;
    const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Json);
    if (!FJsonSerializer::Deserialize(Reader, Object) || !Object.IsValid()) return false;

    const TArray<TSharedPtr<FJsonValue>>* Features = nullptr;
    if (!Object->TryGetArrayField(TEXT("features"), Features) || !Features) return false;
    OutFeatures = *Features;
    return OutFeatures.Num() > 0;
}

FVector ASawariSaddarOSMActor::GeoToWorld(double Latitude, double Longitude, double Z) const
{
    const double X = (Longitude - OriginLongitude) * LongitudeScale * 100.0;
    const double Y = (Latitude - OriginLatitude) * 110540.0 * 100.0;
    return GetActorTransform().TransformPosition(FVector(X, Y, Z));
}

FVector ASawariSaddarOSMActor::SnapToNearestRoad(const FVector& WorldPoint) const
{
    FVector BestPoint = WorldPoint;
    double BestDistanceSquared = TNumericLimits<double>::Max();

    for (const TArray<FVector>& Route : TrafficRoutes)
    {
        for (int32 Index = 0; Index < Route.Num() - 1; ++Index)
        {
            const FVector A(Route[Index].X, Route[Index].Y, 0.0);
            const FVector B(Route[Index + 1].X, Route[Index + 1].Y, 0.0);
            const FVector Segment = B - A;
            const double SegmentLengthSquared = Segment.SizeSquared2D();
            if (SegmentLengthSquared <= 1.0) continue;

            const double Alpha = FMath::Clamp(
                FVector::DotProduct(FVector(WorldPoint.X, WorldPoint.Y, 0.0) - A, Segment)
                    / SegmentLengthSquared,
                0.0, 1.0);
            const FVector Candidate = A + Segment * Alpha;
            const double DistanceSquared = FVector::DistSquared2D(Candidate, WorldPoint);
            if (DistanceSquared < BestDistanceSquared)
            {
                BestDistanceSquared = DistanceSquared;
                BestPoint = Candidate;
            }
        }
    }

    BestPoint.Z = WorldPoint.Z;
    return BestPoint;
}

FVector ASawariSaddarOSMActor::GetTaxiStartLocation() const
{
    const FVector Candidate = GeoToWorld(SawariOSM::TaxiLatitude, SawariOSM::TaxiLongitude, 190.0);
    return SnapToNearestRoad(Candidate);
}

FVector ASawariSaddarOSMActor::GetPickupLocation() const
{
    const FVector Candidate = GeoToWorld(SawariOSM::PickupLatitude, SawariOSM::PickupLongitude, 0.0);
    return SnapToNearestRoad(Candidate);
}

FVector ASawariSaddarOSMActor::GetDropoffLocation() const
{
    const FVector Candidate = GeoToWorld(SawariOSM::DropoffLatitude, SawariOSM::DropoffLongitude, 0.0);
    return SnapToNearestRoad(Candidate);
}

void ASawariSaddarOSMActor::SetRideMarkers(bool bShowPickup, bool bShowDropoff)
{
    PickupMarker->SetVisibility(bShowPickup, true);
    DropoffMarker->SetVisibility(bShowDropoff, true);
}

void ASawariSaddarOSMActor::ApplyColor(UPrimitiveComponent* Component, const FLinearColor& Color)
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

float ASawariSaddarOSMActor::RoadWidthCentimeters(const FString& HighwayType)
{
    if (HighwayType == TEXT("motorway") || HighwayType == TEXT("trunk")) return 1500.0f;
    if (HighwayType == TEXT("primary")) return 1200.0f;
    if (HighwayType == TEXT("secondary")) return 950.0f;
    if (HighwayType == TEXT("tertiary")) return 780.0f;
    if (HighwayType == TEXT("residential")) return 650.0f;
    if (HighwayType == TEXT("unclassified")) return 560.0f;
    if (HighwayType == TEXT("service")) return 420.0f;
    if (HighwayType == TEXT("living_street")) return 500.0f;
    return 0.0f;
}

float ASawariSaddarOSMActor::EstimateBuildingHeight(const TSharedPtr<FJsonObject>& Properties)
{
    double Value = 0.0;
    if (SawariOSM::ReadNumber(Properties, TEXT("height"), Value))
    {
        return FMath::Clamp(static_cast<float>(Value * 100.0), 300.0f, 10000.0f);
    }
    if (SawariOSM::ReadNumber(Properties, TEXT("building:levels"), Value)
        || SawariOSM::ReadNumber(Properties, TEXT("levels"), Value))
    {
        return FMath::Clamp(static_cast<float>(Value * 320.0), 300.0f, 10000.0f);
    }

    FString BuildingType;
    if (Properties.IsValid() && Properties->TryGetStringField(TEXT("building"), BuildingType))
    {
        if (BuildingType == TEXT("commercial") || BuildingType == TEXT("office")
            || BuildingType == TEXT("retail") || BuildingType == TEXT("industrial")) return 1500.0f;
        if (BuildingType == TEXT("apartments") || BuildingType == TEXT("residential")) return 900.0f;
    }
    return 700.0f;
}

bool ASawariSaddarOSMActor::LoadRoads()
{
    TArray<TSharedPtr<FJsonValue>> Features;
    if (!ParseGeoJSON(TEXT("roads.geojson"), Features)) return false;
    BuildRoadMeshes(Features);
    return RoadMesh->GetNumSections() > 0;
}

bool ASawariSaddarOSMActor::LoadBuildings()
{
    TArray<TSharedPtr<FJsonValue>> Features;
    if (!ParseGeoJSON(TEXT("buildings.geojson"), Features)) return false;
    BuildBuildingMeshes(Features);
    return BuildingMesh->GetNumSections() > 0;
}

void ASawariSaddarOSMActor::BuildRoadMeshes(const TArray<TSharedPtr<FJsonValue>>& Features)
{
    TArray<FVector> Vertices;
    TArray<int32> Triangles;
    TArray<FVector2D> UVs;
    TArray<FLinearColor> Colors;

    for (const TSharedPtr<FJsonValue>& FeatureValue : Features)
    {
        if (!FeatureValue.IsValid() || FeatureValue->Type != EJson::Object) continue;
        const TSharedPtr<FJsonObject> Feature = FeatureValue->AsObject();
        const TSharedPtr<FJsonObject>* GeometryPtr = nullptr;
        const TSharedPtr<FJsonObject>* PropertiesPtr = nullptr;
        if (!Feature->TryGetObjectField(TEXT("geometry"), GeometryPtr) || !GeometryPtr->IsValid()) continue;
        Feature->TryGetObjectField(TEXT("properties"), PropertiesPtr);
        const TSharedPtr<FJsonObject> Geometry = *GeometryPtr;

        FString GeometryType;
        if (!Geometry->TryGetStringField(TEXT("type"), GeometryType) || GeometryType != TEXT("LineString")) continue;
        const TArray<TSharedPtr<FJsonValue>>* Coordinates = nullptr;
        if (!Geometry->TryGetArrayField(TEXT("coordinates"), Coordinates) || !Coordinates || Coordinates->Num() < 2) continue;

        FString HighwayType;
        const float Width = PropertiesPtr && PropertiesPtr->IsValid()
            ? (PropertiesPtr->Get()->TryGetStringField(TEXT("highway"), HighwayType) ? RoadWidthCentimeters(HighwayType) : 0.0f)
            : 0.0f;
        if (Width <= 0.0f) continue;

        TArray<FVector> Points;
        Points.Reserve(Coordinates->Num());
        for (const TSharedPtr<FJsonValue>& Coordinate : *Coordinates)
        {
            double Lon = 0.0, Lat = 0.0;
            if (!SawariOSM::ReadLatLon(Coordinate, Lon, Lat)) continue;
            Points.Add(FVector(
                static_cast<float>((Lon - OriginLongitude) * LongitudeScale * 100.0),
                static_cast<float>((Lat - OriginLatitude) * 110540.0 * 100.0),
                12.0f));
        }
        if (Points.Num() < 2) continue;

        // Save full drivable OSM way polylines for the AI traffic system.
        FString OneWayTag;
        const bool bOneWay = PropertiesPtr && PropertiesPtr->IsValid()
            && PropertiesPtr->Get()->TryGetStringField(TEXT("oneway"), OneWayTag)
            && (OneWayTag == TEXT("yes") || OneWayTag == TEXT("true")
                || OneWayTag == TEXT("1") || OneWayTag == TEXT("-1"));

        if (!bOneWay && Points.Num() >= 2 && (HighwayType == TEXT("primary") || HighwayType == TEXT("secondary")
            || HighwayType == TEXT("tertiary") || HighwayType == TEXT("residential")
            || HighwayType == TEXT("unclassified") || HighwayType == TEXT("trunk")
            || HighwayType == TEXT("service")))
        {
            TArray<FVector> Route;
            Route.Reserve(Points.Num());
            for (const FVector& Point : Points) Route.Add(GetActorTransform().TransformPosition(Point));
            if (Route.Num() >= 2 && FVector::Dist2D(Route[0], Route.Last()) > 5000.0f)
            {
                TrafficRoutes.Add(MoveTemp(Route));
            }
        }

        for (int32 Index = 0; Index < Points.Num() - 1; ++Index)
        {
            const FVector A = Points[Index];
            const FVector B = Points[Index + 1];
            FVector2D Direction(B.X - A.X, B.Y - A.Y);
            if (Direction.SizeSquared() < 1.0) continue;
            Direction.Normalize();
            const FVector2D Side(-Direction.Y, Direction.X);
            const FVector Offset(Side.X * Width * 0.5f, Side.Y * Width * 0.5f, 0.0f);

            const int32 Base = Vertices.Num();
            Vertices.Add(A + Offset);
            Vertices.Add(B + Offset);
            Vertices.Add(B - Offset);
            Vertices.Add(A - Offset);
            // Winding is counter-clockwise from above so the asphalt surface
            // normal faces upward regardless of road segment direction.
            Triangles.Add(Base); Triangles.Add(Base + 2); Triangles.Add(Base + 1);
            Triangles.Add(Base); Triangles.Add(Base + 3); Triangles.Add(Base + 2);
            UVs.Add(FVector2D(0, 0)); UVs.Add(FVector2D(1, 0));
            UVs.Add(FVector2D(1, 1)); UVs.Add(FVector2D(0, 1));
            Colors.Add(FLinearColor::White); Colors.Add(FLinearColor::White);
            Colors.Add(FLinearColor::White); Colors.Add(FLinearColor::White);
        }
    }

    if (Vertices.Num() > 0)
    {
        RoadMesh->CreateMeshSection_LinearColor(0, Vertices, Triangles, TArray<FVector>(), UVs,
            Colors, TArray<FProcMeshTangent>(), true, true);
        RoadMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    }
}

bool ASawariSaddarOSMActor::TriangulateFootprint(const TArray<FVector2D>& Polygon, TArray<int32>& OutTriangles)
{
    OutTriangles.Reset();
    if (Polygon.Num() < 3) return false;

    TArray<int32> Remaining;
    Remaining.Reserve(Polygon.Num());
    for (int32 Index = 0; Index < Polygon.Num(); ++Index) Remaining.Add(Index);

    double Area = 0.0;
    for (int32 Index = 0; Index < Polygon.Num(); ++Index)
    {
        const FVector2D& A = Polygon[Index];
        const FVector2D& B = Polygon[(Index + 1) % Polygon.Num()];
        Area += A.X * B.Y - B.X * A.Y;
    }
    if (Area < 0.0) Algo::Reverse(Remaining);

    int32 Guard = Polygon.Num() * Polygon.Num();
    while (Remaining.Num() > 3 && Guard-- > 0)
    {
        bool bFoundEar = false;
        for (int32 Index = 0; Index < Remaining.Num(); ++Index)
        {
            const int32 Previous = Remaining[(Index - 1 + Remaining.Num()) % Remaining.Num()];
            const int32 Current = Remaining[Index];
            const int32 Next = Remaining[(Index + 1) % Remaining.Num()];
            const FVector2D& A = Polygon[Previous];
            const FVector2D& B = Polygon[Current];
            const FVector2D& C = Polygon[Next];

            if (SawariOSM::Cross2D(A, B, C) <= 0.01) continue;

            bool bContainsPoint = false;
            for (int32 Candidate : Remaining)
            {
                if (Candidate == Previous || Candidate == Current || Candidate == Next) continue;
                if (SawariOSM::PointInTriangle(Polygon[Candidate], A, B, C))
                {
                    bContainsPoint = true;
                    break;
                }
            }
            if (bContainsPoint) continue;

            OutTriangles.Add(Previous); OutTriangles.Add(Current); OutTriangles.Add(Next);
            Remaining.RemoveAt(Index);
            bFoundEar = true;
            break;
        }
        if (!bFoundEar) return false;
    }

    if (Remaining.Num() == 3)
    {
        OutTriangles.Add(Remaining[0]); OutTriangles.Add(Remaining[1]); OutTriangles.Add(Remaining[2]);
        return true;
    }
    return false;
}

void ASawariSaddarOSMActor::BuildBuildingMeshes(const TArray<TSharedPtr<FJsonValue>>& Features)
{
    TArray<FVector> Vertices;
    TArray<int32> Triangles;
    TArray<FVector2D> UVs;
    TArray<FLinearColor> Colors;

    for (const TSharedPtr<FJsonValue>& FeatureValue : Features)
    {
        if (!FeatureValue.IsValid() || FeatureValue->Type != EJson::Object) continue;
        const TSharedPtr<FJsonObject> Feature = FeatureValue->AsObject();
        const TSharedPtr<FJsonObject>* GeometryPtr = nullptr;
        const TSharedPtr<FJsonObject>* PropertiesPtr = nullptr;
        if (!Feature->TryGetObjectField(TEXT("geometry"), GeometryPtr) || !GeometryPtr->IsValid()) continue;
        Feature->TryGetObjectField(TEXT("properties"), PropertiesPtr);

        const TSharedPtr<FJsonObject> Geometry = *GeometryPtr;
        FString GeometryType;
        if (!Geometry->TryGetStringField(TEXT("type"), GeometryType) || GeometryType != TEXT("Polygon")) continue;

        const TArray<TSharedPtr<FJsonValue>>* Rings = nullptr;
        if (!Geometry->TryGetArrayField(TEXT("coordinates"), Rings) || !Rings || Rings->Num() == 0) continue;
        const TArray<TSharedPtr<FJsonValue>>& OuterRing = (*Rings)[0]->AsArray();
        if (OuterRing.Num() < 4) continue;

        TArray<FVector> Footprint3D;
        for (const TSharedPtr<FJsonValue>& Coordinate : OuterRing)
        {
            double Lon = 0.0, Lat = 0.0;
            if (!SawariOSM::ReadLatLon(Coordinate, Lon, Lat)) continue;
            Footprint3D.Add(FVector(
                static_cast<float>((Lon - OriginLongitude) * LongitudeScale * 100.0),
                static_cast<float>((Lat - OriginLatitude) * 110540.0 * 100.0),
                0.0f));
        }
        if (Footprint3D.Num() > 1 && FVector::DistSquared2D(Footprint3D[0], Footprint3D.Last()) < 1.0)
        {
            Footprint3D.Pop();
        }
        if (Footprint3D.Num() < 3 || Footprint3D.Num() > 2000) continue;

        double SignedArea = 0.0;
        for (int32 PointIndex = 0; PointIndex < Footprint3D.Num(); ++PointIndex)
        {
            const FVector& A = Footprint3D[PointIndex];
            const FVector& B = Footprint3D[(PointIndex + 1) % Footprint3D.Num()];
            SignedArea += A.X * B.Y - B.X * A.Y;
        }
        if (SignedArea < 0.0)
        {
            Algo::Reverse(Footprint3D);
        }

        TArray<FVector2D> Footprint2D;
        for (const FVector& Point : Footprint3D) Footprint2D.Add(FVector2D(Point.X, Point.Y));
        TArray<int32> RoofTriangles;
        if (!TriangulateFootprint(Footprint2D, RoofTriangles)) continue;

        const TSharedPtr<FJsonObject> Properties = (PropertiesPtr && PropertiesPtr->IsValid()) ? *PropertiesPtr : nullptr;
        const float Height = EstimateBuildingHeight(Properties);
        const int32 RoofBase = Vertices.Num();
        for (const FVector& Point : Footprint3D)
        {
            Vertices.Add(FVector(Point.X, Point.Y, Height));
            UVs.Add(FVector2D(Point.X / 1000.0f, Point.Y / 1000.0f));
            Colors.Add(FLinearColor::White);
        }
        for (int32 TriIndex : RoofTriangles) Triangles.Add(RoofBase + TriIndex);

        for (int32 Index = 0; Index < Footprint3D.Num(); ++Index)
        {
            const FVector& A = Footprint3D[Index];
            const FVector& B = Footprint3D[(Index + 1) % Footprint3D.Num()];
            const int32 Base = Vertices.Num();
            Vertices.Add(FVector(A.X, A.Y, 0.0f));
            Vertices.Add(FVector(B.X, B.Y, 0.0f));
            Vertices.Add(FVector(B.X, B.Y, Height));
            Vertices.Add(FVector(A.X, A.Y, Height));
            const FVector2D SideUV(FVector::Dist2D(A, B) / 400.0f, Height / 400.0f);
            UVs.Add(FVector2D(0,0)); UVs.Add(FVector2D(SideUV.X,0));
            UVs.Add(SideUV); UVs.Add(FVector2D(0,SideUV.Y));
            Colors.Add(FLinearColor::White); Colors.Add(FLinearColor::White);
            Colors.Add(FLinearColor::White); Colors.Add(FLinearColor::White);
            Triangles.Add(Base); Triangles.Add(Base + 1); Triangles.Add(Base + 2);
            Triangles.Add(Base); Triangles.Add(Base + 2); Triangles.Add(Base + 3);
        }
    }

    if (Vertices.Num() > 0)
    {
        BuildingMesh->CreateMeshSection_LinearColor(0, Vertices, Triangles, TArray<FVector>(), UVs,
            Colors, TArray<FProcMeshTangent>(), true, true);
        BuildingMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    }
}
