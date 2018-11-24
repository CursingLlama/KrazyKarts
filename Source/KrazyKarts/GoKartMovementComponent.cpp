// Fill out your copyright notice in the Description page of Project Settings.

#include "GoKartMovementComponent.h"

#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/GameState.h"
#include "UnrealNetwork.h"


// Sets default values for this component's properties
UGoKartMovementComponent::UGoKartMovementComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	bReplicates = true;
}

// Called when the game starts
void UGoKartMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

// Called every frame
void UGoKartMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UGoKartMovementComponent::SimulateMove(const FGoKartMove& Move)
{
	RotateKart(Move.DeltaTime, Move.SteeringThrow);
	MoveKartForward(Move.DeltaTime, Move.Throttle);
}

FGoKartMove UGoKartMovementComponent::CreateMove(float DeltaTime)
{
	FGoKartMove Move;
	Move.DeltaTime = DeltaTime;
	Move.Throttle = Throttle;
	Move.SteeringThrow = SteeringThrow;
	Move.TimeStamp = GetWorld()->GetGameState()->GetServerWorldTimeSeconds();
	return Move;
}

void UGoKartMovementComponent::MoveKartForward(float DeltaTime, float Throttle)
{
	FVector Force = GetOwner()->GetActorForwardVector() * (Horsepower * 745.69987158f) * Throttle;
	Force += GetAirResistance();
	Force += GetRollingResistance();

	FVector Acceleration = Force / Mass;
	Velocity += Acceleration * DeltaTime;

	FVector Translation = Velocity * DeltaTime * 100;

	FHitResult HitResult;
	GetOwner()->AddActorWorldOffset(Translation, true, &HitResult);

	if (HitResult.IsValidBlockingHit())
	{
		Velocity = FVector::ZeroVector;
	}
}

void UGoKartMovementComponent::RotateKart(float DeltaTime, float SteeringThrow)
{
	float DeltaLocation = FVector::DotProduct(GetOwner()->GetActorForwardVector(), Velocity) * DeltaTime;
	float DeltaRotation = (DeltaLocation / TurningRadius) * SteeringThrow;
	FQuat RotationDelta(GetOwner()->GetActorUpVector(), DeltaRotation);

	FHitResult HitResult;
	GetOwner()->AddActorWorldRotation(RotationDelta);

	Velocity = RotationDelta.RotateVector(Velocity);
}

FVector UGoKartMovementComponent::GetAirResistance()
{
	return -Velocity.GetSafeNormal() * Velocity.SizeSquared() * DragCoefficient;
}

FVector UGoKartMovementComponent::GetRollingResistance()
{
	float RelativeGravity = -GetWorld()->GetGravityZ() / 100;
	float NormalForce = RelativeGravity * Mass;
	return -Velocity.GetSafeNormal() * NormalForce * RollingCoefficient;
}
