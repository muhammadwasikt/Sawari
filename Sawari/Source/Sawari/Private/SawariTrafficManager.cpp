#include "SawariTrafficManager.h"

#include "SawariSaddarOSMActor.h"
#include "SawariTrafficCar.h"
#include "Algo/Reverse.h"
#include "Engine/World.h"

ASawariTrafficManager::ASawariTrafficManager()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ASawariTrafficManager::InitializeTraffic(ASawariSaddarOSMActor* City)
{
    if (!City || !City->IsOSMDataLoaded() || !GetWorld()) return;

    const TArray<TArray<FVector>>& Routes = City->GetTrafficRoutes();
    int32 Spawned = 0;

    for (int32 RouteIndex = 0; RouteIndex < Routes.Num() && Spawned < MaximumTrafficCars; ++RouteIndex)
    {
        const TArray<FVector>& SourceRoute = Routes[RouteIndex];
        if (SourceRoute.Num() < 2) continue;

        float RouteLength = 0.0f;
        for (int32 PointIndex = 0; PointIndex < SourceRoute.Num() - 1; ++PointIndex)
        {
            RouteLength += FVector::Dist2D(SourceRoute[PointIndex], SourceRoute[PointIndex + 1]);
        }
        if (RouteLength < 9000.0f) continue;

        const int32 CarsOnRoute = RouteLength > 45000.0f ? 2 : 1;
        for (int32 CarOnRoute = 0; CarOnRoute < CarsOnRoute && Spawned < MaximumTrafficCars; ++CarOnRoute)
        {
            TArray<FVector> LaneRoute;
            LaneRoute.Reserve(SourceRoute.Num());

            TArray<FVector> DirectedRoute = SourceRoute;
            // Pakistan is left-hand traffic. A second vehicle uses the reverse
            // direction and the left lane relative to its own direction.
            if (CarOnRoute == 1)
            {
                Algo::Reverse(DirectedRoute);
            }

            const float LaneOffset = 330.0f;
            for (int32 PointIndex = 0; PointIndex < DirectedRoute.Num(); ++PointIndex)
            {
                const FVector& Current = DirectedRoute[PointIndex];
                const FVector& Previous = DirectedRoute[FMath::Max(0, PointIndex - 1)];
                const FVector& Next = DirectedRoute[FMath::Min(PointIndex + 1, DirectedRoute.Num() - 1)];
                const FVector Direction = (Next - Previous).GetSafeNormal2D();
                const FVector Side(-Direction.Y, Direction.X, 0.0f);
                LaneRoute.Add(Current + Side * LaneOffset);
            }

            const int32 StartIndex = CarOnRoute == 0 ? 0 : LaneRoute.Num() / 2;
            FActorSpawnParameters Params;
            Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
            const FVector SpawnLocation = LaneRoute[StartIndex] + FVector(0, 0, 60.0f);
            ASawariTrafficCar* Car = GetWorld()->SpawnActor<ASawariTrafficCar>(
                ASawariTrafficCar::StaticClass(), SpawnLocation, FRotator::ZeroRotator, Params);
            if (!Car) continue;

            const float SpeedKmh = 26.0f + static_cast<float>((RouteIndex * 11 + CarOnRoute * 7) % 28);
            Car->InitializeRoute(LaneRoute, StartIndex, SpeedKmh);
            SpawnedCars.Add(Car);
            ++Spawned;
        }
    }

    UE_LOG(LogTemp, Display, TEXT("Sawari AI traffic spawned %d moving cars across %d OSM road routes."),
        SpawnedCars.Num(), Routes.Num());
}
