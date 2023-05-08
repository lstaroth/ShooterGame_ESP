#pragma once
#include <iostream>

struct FNameEntryHeader
{
	uint16_t bIsWide : 1;
	uint16_t LowercaseProbeHash : 5;
	uint16_t Len : 10;
};

struct FNameEntry
{
	FNameEntryHeader Header;
	union
	{
		char	AnsiName[1];
		wchar_t	WideName[1];
	};
};

struct FName
{
	uint32_t ComparisonIndex;
	uint32_t Number;
};

struct UObjectBase
{
	int* virtualPointer;
	uint32_t ObjectFlags;
	uint32_t InternalIndex;
	UObjectBase* ClassPrivate;
	FName NamePrivate;
	UObjectBase* OuterPrivate;
};

struct FUObjectItem
{
	UObjectBase* Object;
	int32_t Flags;
	int32_t ClusterRootIndex;
	int32_t SerialNumber;
};

struct FChunkedFixedUObjectArray
{
	FUObjectItem** Objects;
	FUObjectItem* PreAllocatedObjects;
	int32_t MaxElements;
	int32_t NumElements;
	int32_t MaxChunks;
	int32_t NumChunks;
};

struct FUObjectArray
{
	int32_t ObjFirstGCIndex;
	int32_t ObjLastNonGCIndex;
	int32_t MaxObjectsNotConsideredByGC;
	bool OpenForDisregardForGC;
	FChunkedFixedUObjectArray ObjObjects;
	//...
};

struct FNameEntryAllocator
{
	uint64_t Lock;
	int32_t CurrentBlock;
	int32_t CurrentByteCursor;
	uint8_t* Blocks[8192];
};

struct FNamePool
{
	FNameEntryAllocator Entries;
	//...
};

struct FVector
{
	float x;
	float y;
	float z;

	FVector(float* v);

	FVector(float f_x = 0, float f_y = 0, float f_z = 0);

	float distance(FVector v);

	float Dot(const FVector& v) const;

	FVector operator-(FVector v) const;
};

struct USceneComponent 
{
	UObjectBase baseobj;
	BYTE unnamed[0xf4];
	FVector RelativeLocation;
	//...
};

struct AActor
{
	UObjectBase baseobj;
	BYTE unnamed[0x108];
	USceneComponent* RootComponent;
	//...
};

struct AShooterCharacter 
{
	BYTE unnamed[0x6a4];
	float Health;
};

template<typename OBJTYPE>
struct TArray
{
	OBJTYPE* Data;
	uint32_t ArrayNum;
	uint32_t ArrayMax;
};

struct ULevel 
{
	UObjectBase baseobj;
	int* virtualPointer;
	BYTE URL[0x68];
	TArray<AActor*> Actors;
	//...
};

struct FRotator
{
	float Pitch;
	float Yaw;
	float Roll;
};

struct FMinimalViewInfo 
{
	FVector Location;
	FRotator Rotation;
	float FOV;
	//...
};

struct FCameraCacheEntry 
{
	BYTE unnamed[0x10];
	FMinimalViewInfo POV;
};

struct APlayerCameraManager 
{
	BYTE unnamed[0x1aa0];
	FCameraCacheEntry CameraCachePrivate;
	//...
};

struct APlayerController
{
	BYTE unnamed[0x2b8];
	APlayerCameraManager* PlayerCameraManager;
	//...
};

struct ULocalPlayer 
{
	UObjectBase baseobj;
	int* virtualPointer;
	APlayerController* PlayerController;
	//...
};

struct UGameInstance 
{
	UObjectBase baseobj;
	int* virtualPointer;
	int* WorldContext;
	TArray<ULocalPlayer*> LocalPlayers;
	//...
};

struct UWorld
{
	UObjectBase baseobj;
	int* virtualPointer;
	ULevel* PersistentLevel;
	BYTE unnamed[0x148];
	UGameInstance* OwningGameInstance;
	//...
};

struct UWorldProxy
{
	UWorld* World;
};

//math caluate

struct FMatrix {
	float M[4][4];
};

struct GameScreen
{
	uint32_t windowWidth;
	uint32_t windowHeight;
	uint32_t positionX;
	uint32_t positionY;
};

std::tuple<float, float> WorldToScreen(FVector* world, FMinimalViewInfo& camera);

HWND FindTopGameWindow();