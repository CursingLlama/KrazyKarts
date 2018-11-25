// Fill out your copyright notice in the Description page of Project Settings.

#include "GoKart.h"
#include "GoKartMovementComponent.h"
#include "GoKartMovementReplicator.h"

#include "Components/InputComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "UnrealNetwork.h"
#include "GameFramework/GameStateBase.h"

// Sets default values
AGoKart::AGoKart()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	bReplicateMovement = false;

	MovementComponent = CreateDefaultSubobject<UGoKartMovementComponent>(FName("Movement Component"));
	MovementReplicator = CreateDefaultSubobject<UGoKartMovementReplicator>(FName("Movement Replicator"));
}

// Called when the game starts or when spawned
void AGoKart::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		NetUpdateFrequency = 1;
	}
}

// Called every frame
void AGoKart::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);	
		
	DrawDebugString(GetWorld(), FVector(0,0,100), GetRoleText(Role), this, FColor::White, DeltaTime);
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
	if (!MovementComponent) { return; }
	MovementComponent->SetThrottle(Value);
}

void AGoKart::TurnRight(float Value)
{
	if (!MovementComponent) { return; }
	MovementComponent->SetSteeringThrow(Value);
}

FString AGoKart::GetRoleText(ENetRole Role)
{
	switch (Role)
	{
	case ROLE_None:
		return "None";
	case ROLE_Authority:
		return "Authority";
	case ROLE_AutonomousProxy:
		return "AutonomousProxy";
	case ROLE_SimulatedProxy:
		return "SimulatedProxy";
	default:
		return "Error";;
	}
}
