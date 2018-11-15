// Fill out your copyright notice in the Description page of Project Settings.

#include "GoKart.h"

#include "Components/InputComponent.h"
#include "Engine/World.h"

// Sets default values
AGoKart::AGoKart()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AGoKart::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AGoKart::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RotateKart(DeltaTime);
	MoveKartForward(DeltaTime);
}

void AGoKart::MoveKartForward(float DeltaTime)
{
	Force += GetAirResistance();
	Force += GetRollingResistance();

	FVector Acceleration = Force / Mass;
	Velocity += Acceleration * DeltaTime;

	FVector Translation = Velocity * DeltaTime * 100;
	
	FHitResult HitResult;
	AddActorWorldOffset(Translation, true, &HitResult);

	if (HitResult.IsValidBlockingHit())
	{
		Velocity = FVector::ZeroVector;
	}
}

void AGoKart::RotateKart(float DeltaTime)
{
	float DeltaLocation = FVector::DotProduct(GetActorForwardVector(), Velocity) * DeltaTime;
	float DeltaRotation = (DeltaLocation / TurningRadius) * SteeringThrow;
	FQuat RotationDelta(GetActorUpVector(), DeltaRotation);

	FHitResult HitResult;
	AddActorWorldRotation(RotationDelta);
	
	Velocity = RotationDelta.RotateVector(Velocity);
}

FVector AGoKart::GetAirResistance()
{
	return -Velocity.GetSafeNormal() * Velocity.SizeSquared() * DragCoefficient;
}

FVector AGoKart::GetRollingResistance()
{
	float RelativeGravity = -GetWorld()->GetGravityZ() / 100;
	float NormalForce = RelativeGravity * Mass;
	return -Velocity.GetSafeNormal() * NormalForce * RollingCoefficient;
}

// Called to bind functionality to input
void AGoKart::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis(FName("MoveForward"), this, &AGoKart::MoveForward);
	PlayerInputComponent->BindAxis(FName("MoveRight"), this, &AGoKart::TurnRight);
}

void AGoKart::MoveForward(float Value)
{
	Force = GetActorForwardVector() * Horsepower * 745.69987158f * Value;
}

void AGoKart::TurnRight(float Value)
{
	SteeringThrow = Value;
}


