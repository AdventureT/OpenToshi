#include "pch.h"
#include "AExampleClass.h"
#include "Memory/AMemory.h"
#include "Toshi/Xui/TXUI.h"
#include "Toshi/Memory/TMemory.h"

#include <Toshi/File/TTSF.h>
#include <Toshi/Render/TAssetInit.h>
#include <Toshi/Core/TError.h>

using namespace Toshi;

AExampleClass::AExampleClass()
{
	TOSHI_INFO("Called AExampleClass");

	TRandom rnd;
	TOSHI_INFO("=================TRandom Test=================");
	TOSHI_INFO("1) Random TINT (9000, 10000): {0}", rnd.GetInt(9000, 10000));
	TOSHI_INFO("2) Random TINT (9000, 10000): {0}", rnd.GetInt(9000, 10000));
	TOSHI_INFO("3) Random TINT (9000, 10000): {0}", rnd.GetInt(9000, 10000));
	TOSHI_INFO("4) Random TINT (9000, 10000): {0}", rnd.GetInt(9000, 10000));
	TOSHI_INFO("5) Random TFLOAT: {0}", rnd.GetFloat());
	TOSHI_INFO("6) Random TFLOAT: {0}", rnd.GetFloat());
	TOSHI_INFO("7) Random TFLOAT: {0}", rnd.GetFloat());
	TOSHI_INFO("8) Random TFLOAT: {0}", rnd.GetFloat());
	TOSHI_INFO("9) Random TINT (500): {0}", rnd.GetInt(500));
	TOSHI_INFO("10) Random TINT (500): {0}", rnd.GetInt(500));
	TOSHI_INFO("11) Random TINT (500): {0}", rnd.GetInt(500));
	TOSHI_INFO("12) Random TINT (500): {0}", rnd.GetInt(500));
	TOSHI_INFO("13) Random TINT: {0}", rnd.GetInt());
	TOSHI_INFO("14) Random TINT: {0}", rnd.GetInt());
	TOSHI_INFO("15) Random TINT: {0}", rnd.GetInt());
	TOSHI_INFO("16) Random TINT: {0}", rnd.GetInt());
	TOSHI_INFO("==============================================");

	{
		TFile* file = TFile::Create("C:\\Program Files (x86)\\Steam\\steamapps\\common\\de Blob\\Data\\BlobChar\\AssetPack.trb");

		if (file)
		{
			TCHAR sos[0x800]{};
			file->Seek(5, TFile::TSEEK_SET);
			TINT read = file->Read(&sos, sizeof(sos));

			TOSHI_INFO("File size: {0} bytes", file->GetSize());
			file->Seek(5, TFile::TSEEK_SET);
			TOSHI_INFO("Seeked to offset: {0}", file->Tell());
			file->Destroy();
		}
	}

	{
		TFile* file = TFile::Create("C:\\Program Files (x86)\\Steam\\userdata\\180297931\\532320\\remote\\SaveData.dat.old");

		if (file)
		{
			TUINT8* buf = (TUINT8*)TMalloc(file->GetSize()); //-12);
			//file->Seek(12, TFile::TSEEK_CUR);
			file->Read(buf, file->GetSize()); //-12);

			// This is only for testing purposes
			*(TINT*)(&buf[8]) = 0; // Setting the CRC to 0 so the new CRC doesn't take the old one and CRCs it

			TUINT32 crc = TUtil::CRC32(buf, file->GetSize());

			TFree(buf);
			file->Destroy();
		}
	}

	TFile* file = TFile::Create("Data\\XUI\\DE\\common.trb");

	if (file)
	{
		static TTRB trb;
		trb.Load(file);
		TAssetInit::InitAssets(trb, TTRUE, TFALSE);
	}


	/*TError* error = TError::GetSingleton();
	error->AddError("Material \'%s\' Couldnt find texture \'%s\'", "test", "debloblol.tga");
	error->AddError("couldnt create resource \'%s\' because we have reached our max (%d)", "Resource1", 5);
	const TCHAR* error1 = error->GetError(0);
	const TCHAR* error2 = error->GetError(1);
	const TCHAR* error3 = error->GetError(2);*/

	/*new Toshi::TLogFile();

	Toshi::TLogFile::GetSingleton()->Log(Toshi::TLogFile::TYPE_Info, "Toshi", "Kernel", "Creating AExampleClass %s", "Yes we are...");*/

	/*TFile* file = TFile::Create("C:\\Program Files (x86)\\Steam\\steamapps\\common\\de Blob\\Data\\BlobChar\\AssetPack.trb");

	if (file)
	{
		TTRB trb;
		trb.Load(file);
		TAssetInit::InitAssets(trb, TTRUE, TFALSE);
	}*/

	/*
	TFile* file = TFile::Create("C:\\Program Files (x86)\\Steam\\steamapps\\common\\de Blob\\Data\\XUI\\DE\\common.trb", TFile::OpenFlags_Read);
	
	if (file)
	{
		file->Seek(0x6E1 + 2, TFile::TSEEK_SET);

		TUINT8 buf[0x30];
		file->Read(buf, 0x30);
		TUINT8* buf2 = buf;
		XURXUISceneData* element = new XURXUISceneData();
		TXUIResource resource;
		element->Load(resource, buf2);
	}
	*/
	/*
	if (error == TTRB::ERROR_OK)
	{
		TXUIResource xuiResource;
		xuiResource.ReadHeader((TUINT8*)trb.GetSymbolAddress("txui"));
		TINT size = xuiResource.GetTotalSize((TUINT8*)trb.GetSymbolAddress("txui"));
		TOSHI_INFO(size);
	}
	
	trb.Load("C:\\Program Files (x86)\\Steam\\steamapps\\common\\de Blob\\Data\\BlobChar\\AssetPack.trb");
	trb.GetSymbolAddress("SkeletonHeader");
	trb.Load("C:\\Users\\nepel\\Desktop\\BKG_CONCEPT01_NTSC_ENG.TTL");
	*/

	TSystemManager::GetSingletonSafe()->GetScheduler()->CreateTask(TGetClass(ARootTask))->Create();
}

AExampleClass::~AExampleClass()
{
	TOSHI_INFO("Called ~AExampleClass");
}
