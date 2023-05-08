#include "pch.h"
#include "UEHack.h"
#include "Utils.h"
#include "Render.h"
#include "imgui.h"
#include "atlbase.h"

#include <map>
#include <thread>
#include <vector>

uint64_t GWorldVA = reinterpret_cast<uint64_t>(GetModuleHandleW(0)) + 0x46B3640;
uint64_t NamePoolDataVA = reinterpret_cast<uint64_t>(GetModuleHandleW(0)) + 0x4561180;
uint64_t GUObjectArrayVA = reinterpret_cast<uint64_t>(GetModuleHandleW(0)) + 0x4579718;

FNameEntry* DisplayFromFName(FName* namePrivate)
{
	FNamePool* PNamePoolData = reinterpret_cast<FNamePool*>(NamePoolDataVA);
	uint32_t Block = namePrivate->ComparisonIndex >> 16;
	uint32_t Offset = namePrivate->ComparisonIndex & 0xFFFF;

	return reinterpret_cast<FNameEntry*>(PNamePoolData->Entries.Blocks[Block] + alignof(FNameEntry) * Offset);
}

std::string GetClassNameFromObj(UObjectBase* Object)
{
	FNamePool* PNamePoolData = reinterpret_cast<FNamePool*>(NamePoolDataVA);
	uint32_t Block = Object->ClassPrivate->NamePrivate.ComparisonIndex >> 16;
	uint32_t Offset = Object->ClassPrivate->NamePrivate.ComparisonIndex & 0xFFFF;

	auto classEntry = reinterpret_cast<FNameEntry*>(PNamePoolData->Entries.Blocks[Block] + alignof(FNameEntry) * Offset);
	std::string className;
	if (classEntry->Header.bIsWide)
	{
		className = ATL::CW2A(std::wstring(classEntry->WideName, classEntry->Header.Len).c_str());
	}
	else
	{
		className.assign(classEntry->AnsiName, classEntry->Header.Len);
	}

	return className;
}

void DisplayObjectInfo(UObjectBase* Object)
{
	printf("Object address: 0x%p\n", Object);
	if (Object->ClassPrivate)
	{
		auto classEntry = DisplayFromFName(&Object->ClassPrivate->NamePrivate);
		std::string classStr;
		if (classEntry->Header.bIsWide)
		{
			classStr = ATL::CW2A(std::wstring(classEntry->WideName, classEntry->Header.Len).c_str());
		}
		else
		{
			classStr.assign(classEntry->AnsiName, classEntry->Header.Len);
		}
		std::cout << "class: " << classStr << std::endl;
	}
	
	std::string outerStr;
	for (auto outer = Object->OuterPrivate; outer; outer = outer->OuterPrivate)
	{
		auto outerEntry = DisplayFromFName(&outer->NamePrivate);
		if (outerEntry->Header.bIsWide)
		{
			outerStr = std::string(ATL::CW2A(std::wstring(outerEntry->WideName, outerEntry->Header.Len).c_str())) + "." + outerStr;
		}
		else
		{
			outerStr = std::string(outerEntry->AnsiName, outerEntry->Header.Len) + "." + outerStr;
		}
	}
	if (!outerStr.empty())
	{
		outerStr += ".";
	}
	auto objEntry = DisplayFromFName(&Object->NamePrivate);
	if (objEntry->Header.bIsWide)
	{
		outerStr += std::string(ATL::CW2A(std::wstring(objEntry->WideName, objEntry->Header.Len).c_str()));
	}
	else
	{
		outerStr += std::string(objEntry->AnsiName, objEntry->Header.Len);
	}

	std::cout << "name: " << outerStr << std::endl << std::endl;
}

void DumpUEObjects()
{
	FILE* file = nullptr;
	auto err = freopen_s(&file, "log.txt", "w", stdout);

	FUObjectArray* PGUObjectArray = reinterpret_cast<FUObjectArray*>(GUObjectArrayVA);
	
	FChunkedFixedUObjectArray ObjObjects = PGUObjectArray->ObjObjects;

	for (int Index = 0; Index < ObjObjects.NumElements; Index++)
	{
		int32_t ChunkIndex = Index / 65536;
		int32_t WithinChunkIndex = Index % 65536;
		if (ChunkIndex >= ObjObjects.NumChunks)	break;

		FUObjectItem* Chunk = ObjObjects.Objects[ChunkIndex];
		if (!Chunk)		continue;

		FUObjectItem ObjItemPtrs = Chunk[WithinChunkIndex];
		if (!ObjItemPtrs.Object)	continue;

		DisplayObjectInfo(ObjItemPtrs.Object);
	}
}


void ShowUEActorsRealtime()
{
	Render::Init();

	UWorldProxy* PGWorld = reinterpret_cast<UWorldProxy*>(GWorldVA);
	auto level = PGWorld->World->PersistentLevel;
	auto localPlayers = PGWorld->World->OwningGameInstance->LocalPlayers;
	if (localPlayers.ArrayNum != 1)
	{
		return;
	}

	std::map<bool, std::vector<AActor*>> actors;
	while (true)
	{
		actors.clear();

		for (unsigned Index = 0; Index < level->Actors.ArrayNum; Index++)
		{
			auto actor = level->Actors.Data[Index];
			if (actor)
			{
				auto name = GetClassNameFromObj(&actor->baseobj);
				if (name == "PlayerPawn_C" && reinterpret_cast<AShooterCharacter*>(actor)->Health > 0)
				{
					actors[true].push_back(actor);
				}
				else if (name == "BotPawn_C" && reinterpret_cast<AShooterCharacter*>(actor)->Health > 0)
				{
					actors[false].push_back(actor);					
				}
			}
		}

		if (actors[true].size() == 1)
		{
			reinterpret_cast<AShooterCharacter*>(actors[true][0])->Health = float(99999.0); //нч╣п
			for (const auto& enemy : actors[false])
			{
				auto [x, y] = WorldToScreen(&enemy->RootComponent->RelativeLocation, localPlayers.Data[0]->PlayerController->PlayerCameraManager->CameraCachePrivate.POV);
				float distance = actors[true][0]->RootComponent->RelativeLocation.distance(enemy->RootComponent->RelativeLocation);
				Render::NewFrame();
				Render::DrawBox(ImColor(1.f, 0.f, 0.f, 1.f), x - (35000 / distance), y - (45000 / distance), 45000 / distance, 110000 / distance);
				Render::ShowFrame();
			}
		}
		
		std::this_thread::sleep_for(std::chrono::milliseconds(16));
	}

	Render::Release();
}