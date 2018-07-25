#include "PlatformPrecomp.h"
#include "UnpackArchiveComponent.h"
#include "util/GLESUtils.h"
#include "Entity/EntityUtils.h"
#include "BaseApp.h"

UnpackArchiveComponent::UnpackArchiveComponent()
{
	SetName("UnpackArchive");
}

UnpackArchiveComponent::~UnpackArchiveComponent()
{
}

void UnpackArchiveComponent::OnAdd(Entity *pEnt)
{
	EntityComponent::OnAdd(pEnt);

	m_pPos2d = &GetParent()->GetVar("pos2d")->GetVector2();

	/*
	m_pSize2d = &GetParent()->GetVar("size2d")->GetVector2();
	m_pScale = &GetParent()->GetShared()->GetVarWithDefault("scale", Variant(1.0f))->GetFloat();
	m_pRotation = &GetParent()->GetVar("rotation")->GetFloat();  //in degrees

	m_pColor = &GetParent()->GetShared()->GetVarWithDefault("color", Variant(MAKE_RGBA(255,255,255,255)))->GetUINT32();
	m_pColorMod = &GetParent()->GetShared()->GetVarWithDefault("colorMod", Variant(MAKE_RGBA(255,255,255,255)))->GetUINT32();
	m_pAlpha = &GetParent()->GetShared()->GetVarWithDefault("alpha", Variant(1.0f))->GetFloat();
	m_pAlignment = &GetParent()->GetVar("alignment")->GetUINT32();
	*/
	m_pSrcFileName = &GetVar("sourceFileName")->GetString();
	m_pDestDirectory = &GetVar("destDirectory")->GetString();
	m_pDeleteSourceOnFinish = &GetVar("deleteSourceOnFinish")->GetUINT32();
	m_pLimitToSingleSubdir = &GetVar("limitToSingleSubdir")->GetUINT32(); //only useful for Dink smallwood's DMODs probably

	
	//register ourselves to render if the parent does
	GetParent()->GetFunction("OnUpdate")->sig_function.connect(1, boost::bind(&UnpackArchiveComponent::OnUpdate, this, _1));
}

void UnpackArchiveComponent::OnRemove()
{
	m_tarHandler.Kill();

	if (*m_pDeleteSourceOnFinish == 1 && !m_pSrcFileName->empty())
	{
		RemoveFile(*m_pSrcFileName, false);
	}
	EntityComponent::OnRemove();
}

void UnpackArchiveComponent::OnUpdate(VariantList *pVList)
{
	if (m_pSrcFileName->empty()) return;

	switch (m_tarHandler.GetState())
	{
		case TarHandler::STATE_NONE:
			//need to start it up
			m_tarHandler.SetLimitOutputToSingleSubDir(*m_pLimitToSingleSubdir != 0);
#ifdef _DEBUG
	//		LogMsg("Opening Tar %s", m_pSrcFileName->c_str());
#endif
			if (m_tarHandler.OpenFile(*m_pSrcFileName, *m_pDestDirectory) == false)
			{
				VariantList vList(this, uint32(m_tarHandler.GetError()));
				GetFunction("OnError")->sig_function(&vList);
			}
			break;

		case TarHandler::STATE_BZIPPING:

			if (m_tarHandler.ProcessChunk())
			{
				//still unzipping
                VariantList vList(this, uint32(m_tarHandler.GetBytesWritten()),uint32(m_tarHandler.GetTotalBytes()));
				GetFunction("OnStatusUpdate")->sig_function(&vList);
			}

			if (m_tarHandler.GetState() == TarHandler::STATE_DONE)
			{
                VariantList vList(this, uint32(m_tarHandler.GetBytesWritten()),uint32(m_tarHandler.GetTotalBytes()));
				GetVar("firstDirCreated")->Set(m_tarHandler.GetFirstDirCreated()); //just in case they want this..
				GetFunction("OnFinish")->sig_function(&vList);
			}

			if (m_tarHandler.GetState() == TarHandler::STATE_ERROR)
			{
                VariantList vList(this, uint32(m_tarHandler.GetError()));
				GetFunction("OnError")->sig_function(&vList);
			}
			break;
	}
	
}