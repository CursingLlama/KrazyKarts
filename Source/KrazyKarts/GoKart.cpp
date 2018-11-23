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

	//We are not the server but in control of the pawn
	if (Role == ROLE_AutonomousProxy)
	{
		FGoKartMove Move = CreateMove(DeltaTime);
		SimulateMove(Move);
		UnacknowledgedMoves.Emplace(Move);
		Server_SendMove(Move);
	}

	//We are the server and in control of the pawn
	if (Role == ROLE_Authority && GetRemoteRole() == ROLE_SimulatedProxy)
	{
		FGoKartMove Move = CreateMove(DeltaTime);
		Server_SendMove(Move);
	}

	if (Role == ROLE_SimulatedProxy)
	{
		SimulateMove(ServerState.LastMove);
	}
		
	DrawDebugString(GetWorld(), FVector(0,0,100), GetRoleText(Role), this, FColor::White, DeltaTime);
}

void AGoKart::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AGoKart, Throttle);
	DOREPLIFETIME(AGoKart, SteeringThrow);
	DOREPLIFETIME(AGoKart, ServerState);
}

void AGoKart::SimulateMove(const FGoKartMove& Move)
{
	RotateKart(Move.DeltaTime, Move.SteeringThrow);
	MoveKartForward(Move.DeltaTime, Move.Throttle);
}

FGoKartMove AGoKart::CreateMove(float DeltaTime)
{
	FGoKartMove Move;
	Move.DeltaTime = DeltaTime;
	Move.Throttle = Throttle;
	Move.SteeringThrow = SteeringThrow;
	Move.TimeStamp = GetWorld()->GetGameState()->GetServerWorldTimeSeconds();
	return Move;
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
	SetActorTransform(ServerState.Transform);
	Velocity = ServerState.Velocity;
	
	ClearAcknowledgedMoves();

	for (const FGoKartMove& Move : UnacknowledgedMoves)
	{
		SimulateMove(Move);
	}
}

void AGoKart::MoveKartForward(float DeltaTime, float Throttle)
{
	FVector Force = GetActorForwardVector() * (Horsepower * 745.69987158f) * Throttle;
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

void AGoKart::RotateKart(float DeltaTime, float SteeringThrow)
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
	Throttle = Value;
}

void AGoKart::TurnRight(float Value)
{
	SteeringThrow = Value;
}

void AGoKart::Server_SendMove_Implementation(FGoKartMove Move)
{
	SimulateMove(Move);

	ServerState.LastMove = Move;
	ServerState.Transform = GetActorTransform();
	ServerState.Velocity = Velocity;
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
