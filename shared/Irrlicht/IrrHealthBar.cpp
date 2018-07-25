#include "PlatformPrecomp.h"
#include "IrrHealthBar.h"

IrrHealthBar::IrrHealthBar()
{
	m_bb = NULL;
}

IrrHealthBar::~IrrHealthBar()
{
}

bool IrrHealthBar::Init( scene::ISceneNode *pParent, core::vector3df vOffset, core::dimension2df vSize, float healthPercent )
{
	m_vSize = vSize;

	m_bb = GetIrrlichtManager()->GetScene()->addBillboardSceneNode(pParent, m_vSize, vOffset);
	m_bb->setMaterialFlag(video::EMF_LIGHTING, false);
	//bill->setMaterialTexture(0, driver->getTexture("../../media/portal1.bmp"));
	m_bb->setMaterialType(video::EMT_TRANSPARENT_ADD_COLOR);
	//bill->addAnimator(anim);

	m_bb->setColor(video::SColor(200,255,0,0),  video::SColor(100,155,0,0));
	return true;
}

void IrrHealthBar::SetHealthTarget( float target )
{
	assert(m_bb);
	
	core::dimension2df vSize = m_vSize;

//	LogMsg("Setting health to %.2f", target);
	vSize.Width *= target;
	if (vSize.Width == 0)
	{
		m_bb->setVisible(false);
	} else
	{
		m_bb->setVisible(true);

	}
	m_bb->setSize(vSize);

}

void IrrHealthBar::Update()
{

}

void IrrHealthBar::SetVisible( bool bNew )
{
	if (m_bb)
	{
		m_bb->setVisible(bNew);
	}
}