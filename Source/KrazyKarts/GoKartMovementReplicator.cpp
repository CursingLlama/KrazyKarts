// Fill out your copyright notice in the Description page of Project Settings.

#include "GoKartMovementReplicator.h"

#include "UnrealNetwork.h"
#include "GameFramework/Actor.h"


// Sets default values for this component's properties
UGoKartMovementReplicator::UGoKartMovementReplicator()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	bReplicates = true;
}

// Called when the game starts
void UGoKartMovementReplicator::BeginPlay()
{
	Super::BeginPlay();

	MovementComponent = GetOwner()->FindComponentByClass<UGoKartMovementComponent>();	
}

// Called every frame
void UGoKartMovementReplicator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (GetOwnerRole() == ROLE_SimulatedProxy)
	{
		ClientTick(DeltaTime);
	}

	if (!MovementComponent) { return; }
	FGoKartMove LastMove = MovementComponent->GetLastMove();

	if (GetOwnerRole() == ROLE_AutonomousProxy)
	{
		UnacknowledgedMoves.Emplace(LastMove);
		Server_SendMove(LastMove);
	}

	//We are the server and in control of the pawn
	if (GetOwner()->GetRemoteRole() == ROLE_SimulatedProxy)
	{
		UpdateServerState(LastMove);
	}	
}

void UGoKartMovementReplicator::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UGoKartMovementReplicator, ServerState);
}

void UGoKartMovementReplicator::UpdateServerState(const FGoKartMove& Move)
{
	ServerState.LastMove = Move;
	ServerState.Transform = GetOwner()->GetActorTransform();
	ServerState.Velocity = MovementComponent->GetVelocity();
}

void UGoKartMovementReplicator::ClientTick(float DeltaTime)
{
	ClientTimeSinceUpdate += DeltaTime;
	if (ClientTimeBetweenUpdates < KINDA_SMALL_NUMBER) { return; }

	float LerpRatio = ClientTimeSinceUpdate / ClientTimeBetweenUpdates;

	FVector TargetLocation = ServerState.Transform.GetLocation();
	FVector StartLocation = ClientStartTransform.GetLocation();
	FVector NewLocation = FMath::LerpStable(StartLocation, TargetLocation, LerpRatio);
	GetOwner()->SetActorLocation(NewLocation);

	FQuat TargetRotation = ServerState.Transform.GetRotation();
	FQuat StartingRotation = ClientStartTransform.GetRotation();
	FQuat NewRotation = FQuat::Slerp(StartingRotation, TargetRotation, LerpRatio);
	GetOwner()->SetActorRotation(NewRotation);
}

void UGoKartMovementReplicator::OnRep_ServerState()
{
	switch (GetOwnerRole())
	{
	case ROLE_AutonomousProxy:
		AutonomousProxy_OnRep_ServerState();
		break;
	case ROLE_SimulatedProxy:
		SimulatedProxy_OnRep_ServerState();
		break;
	default:
		break;
	}	
}

void UGoKartMovementReplicator::AutonomousProxy_OnRep_ServerState()
{
	if (!MovementComponent) { return; }

	GetOwner()->SetActorTransform(ServerState.Transform);
	MovementComponent->SetVelocity(ServerState.Velocity);

	ClearAcknowledgedMoves();

	for (const FGoKartMove& Move : UnacknowledgedMoves)
	{
		MovementComponent->SimulateMove(Move);
	}
}

void UGoKartMovementReplicator::SimulatedProxy_OnRep_ServerState()
{
	ClientTimeBetweenUpdates = ClientTimeSinceUpdate;
	ClientTimeSinceUpdate = 0;
	ClientStartTransform = GetOwner()->GetActorTransform();
}

void UGoKartMovementReplicator::ClearAcknowledgedMoves()
{
	TArray<FGoKartMove> NewMoves;

	for (const FGoKartMove& Move : UnacknowledgedMoves)
	{
		if (Move.TimeStamp > ServerState.LastMove.TimeStamp)
		{
			NewMoves.Emplace(Move);
		}
	}

	UnacknowledgedMoves = NewMoves;
}

void UGoKartMovementReplicator::Server_SendMove_Implementation(FGoKartMove Move)
{
	if (!MovementComponent) { return; }
	MovementComponent->SimulateMove(Move);

	UpdateServerState(Move);
}

bool UGoKartMovementReplicator::Server_SendMove_Validate(FGoKartMove Move)
{
	if (Move.Throttle > 1 || Move.Throttle < -1) { return false; }
	if (Move.SteeringThrow > 1 || Move.SteeringThrow < -1) { return false; }
	return true;
}
