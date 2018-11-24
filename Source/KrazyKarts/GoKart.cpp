// Fill out your copyright notice in the Description page of Project Settings.

#include "GoKart.h"

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

	MovementComponent = CreateDefaultSubobject<UGoKartMovementComponent>(FName("Movement Component"));
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

	if (!MovementComponent) { return; }

	if (Role == ROLE_AutonomousProxy)
	{
		FGoKartMove Move = MovementComponent->CreateMove(DeltaTime);
		MovementComponent->SimulateMove(Move);
		UnacknowledgedMoves.Emplace(Move);
		Server_SendMove(Move);
	}

	//We are the server and in control of the pawn
	if (Role == ROLE_Authority && GetRemoteRole() == ROLE_SimulatedProxy)
	{
		FGoKartMove Move = MovementComponent->CreateMove(DeltaTime);
		Server_SendMove(Move);
	}

	if (Role == ROLE_SimulatedProxy)
	{
		MovementComponent->SimulateMove(ServerState.LastMove);
	}
		
	DrawDebugString(GetWorld(), FVector(0,0,100), GetRoleText(Role), this, FColor::White, DeltaTime);
}

void AGoKart::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AGoKart, ServerState);
}

void AGoKart::ClearAcknowledgedMoves()
{
	TArray<FGoKartMove> NewMoves;
		
	for (const FGoKartMove& Move: UnacknowledgedMoves)
	{
		if (Move.TimeStamp > ServerState.LastMove.TimeStamp)
		{
			NewMoves.Emplace(Move);
		}
	}

	UnacknowledgedMoves = NewMoves;
}

void AGoKart::OnRep_ServerState()
{
	if (!MovementComponent) { return; }

	SetActorTransform(ServerState.Transform);
	MovementComponent->SetVelocity(ServerState.Velocity);
	
	ClearAcknowledgedMoves();

	for (const FGoKartMove& Move : UnacknowledgedMoves)
	{
		MovementComponent->SimulateMove(Move);
	}
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

void AGoKart::Server_SendMove_Implementation(FGoKartMove Move)
{
	if (!MovementComponent) { return; }
	MovementComponent->SimulateMove(Move);

	ServerState.LastMove = Move;
	ServerState.Transform = GetActorTransform();
	ServerState.Velocity = MovementComponent->GetVelocity();
}

bool AGoKart::Server_SendMove_Validate(FGoKartMove Move)
{
	if (Move.Throttle > 1 || Move.Throttle < -1) return false;
	if (Move.SteeringThrow > 1 || Move.SteeringThrow < -1) return false;
	return true;
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
