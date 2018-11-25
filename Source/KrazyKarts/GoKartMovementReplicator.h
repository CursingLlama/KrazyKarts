// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GoKartMovementComponent.h"
#include "GoKartMovementReplicator.generated.h"


USTRUCT()
struct FGoKartState
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY() FGoKartMove LastMove;
	UPROPERTY() FVector Velocity;
	UPROPERTY() FTransform Transform;
};

struct FHermiteCubicSpline
{
	FVector StartLocation, StartDerivative, TargetLocation, TargetDerivative;

	FVector InterpolateLocation(float LerpRatio) const
	{
		return FMath::CubicInterp(StartLocation, StartDerivative, TargetLocation, TargetDerivative, LerpRatio);
	}
	FVector InterpolateDerivative(float LerpRatio) const
	{
		return FMath::CubicInterpDerivative(StartLocation, StartDerivative, TargetLocation, TargetDerivative, LerpRatio);
	}
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class KRAZYKARTS_API UGoKartMovementReplicator : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UGoKartMovementReplicator();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(Server, Reliable, WithValidation) void Server_SendMove(FGoKartMove Move);

	FGoKartState GetServerState() { return ServerState; }

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:

	///State
	UPROPERTY(ReplicatedUsing = OnRep_ServerState) FGoKartState ServerState;

	UPROPERTY() UGoKartMovementComponent* MovementComponent;
	UPROPERTY() USceneComponent* MeshOffsetRoot;
	UFUNCTION(BlueprintCallable) void SetMeshOffsetRoot(USceneComponent* Root) { MeshOffsetRoot = Root; }

	TArray<FGoKartMove> UnacknowledgedMoves;

	FTransform ClientStartTransform;
	FVector ClientStartVelocity;
	float ClientTimeSinceUpdate;
	float ClientTimeBetweenUpdates;

	///Functions
	void ClientTick(float DeltaTime);
	
	FHermiteCubicSpline CreateSpline();
	void InterpolateLocation(const FHermiteCubicSpline &Spline, float LerpRatio);
	void InterpolateVelocity(const FHermiteCubicSpline &Spline, float LerpRatio);
	void InterpolateRotation(float LerpRatio);
	
	UFUNCTION() void ClearAcknowledgedMoves();
	UFUNCTION() void UpdateServerState(const FGoKartMove& Move);
	UFUNCTION() void OnRep_ServerState();
	UFUNCTION() void AutonomousProxy_OnRep_ServerState();
	UFUNCTION() void SimulatedProxy_OnRep_ServerState();

	float VelocityToDerivative();
};
