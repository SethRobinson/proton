#include "PlatformPrecomp.h"
#include "RenderScissorComponent.h"
#include "BaseApp.h"


RenderScissorComponent::RenderScissorComponent()
{
	SetName("RenderScissor");
	m_bOldScissorEnabled= false;
}

RenderScissorComponent::~RenderScissorComponent()
{
}

void RenderScissorComponent::OnAdd(Entity *pEnt)
{
	EntityComponent::OnAdd(pEnt);

	//register to get updated every frame

	GetParent()->OnFilterAdd();
	GetParent()->GetFunction("FilterOnRender")->sig_function.connect(1, boost::bind(&RenderScissorComponent::FilterOnRender, this, _1));
	GetParent()->GetFunction("PostOnRender")->sig_function.connect(1, boost::bind(&RenderScissorComponent::PostOnRender, this, _1));

	m_pPos2d = &GetParent()->GetVar("pos2d")->GetVector2();
	m_pSize2d = &GetParent()->GetVar("size2d")->GetVector2();
	m_pAlignment = &GetParent()->GetVar("alignment")->GetUINT32();
	m_pScissorMode = &GetParent()->GetVarWithDefault("ScissorMode", uint32(POSITION_FROM_SIZE))->GetUINT32();

	//our own vars
}

void RenderScissorComponent::OnRemove()
{
	GetParent()->OnFilterRemove();
	EntityComponent::OnRemove();
}


//Imagine you have rect area located on a 480X320 screen, but you rotate the screen to be 320X480, and you want that
//rect area rotated along with it, so it's still in the same relative position? That's what this does, needed it
//to properly handle what a GL cliprect would be after rotating, as that's one of the few things that rotating GL
//coords doesn't handle

CL_Rectf RotateRect(CL_Rect r, float angleDegrees, CL_Vec2f srcScreenSize)
{
	//Clanlib is missing functions for CL_Mat2 and CL_Vec2f.. so I'll do it this way..

	CL_Mat4f mat = CL_Mat4f::rotate(CL_Angle(-angleDegrees, cl_degrees), 0,0,1);

	switch ((int)angleDegrees)
	{
	case 90:
		mat = mat * CL_Mat4f::translate(0, srcScreenSize.x, 0);
		break;
	case 180:
		mat = mat * CL_Mat4f::translate(srcScreenSize.x, srcScreenSize.y, 0);
		break;
	case 270:
		mat = mat * CL_Mat4f::translate(srcScreenSize.y, 0, 0);
		break;

	case 0:

		break;
	default:
		assert(!"Unsupported angle.. uh..");
	}
	
	//apply
	CL_Vec3f v;

	v = mat.get_transformed_point(CL_Vec3f((float)r.left, (float)r.top, 0.0f));
	r.left = (int)v.x;
	r.top = (int)v.y;

	v = mat.get_transformed_point(CL_Vec3f((float)r.right, (float)r.bottom, 0.0f));
	r.right = (int)v.x;
	r.bottom = (int)v.y;

	r.normalize();
	return r;
}

void RenderScissorComponent::FilterOnRender(VariantList *pVList)
{

	CHECK_GL_ERROR()
	g_globalBatcher.Flush();

	GLboolean b = false;
	
	//Note: This fails when using the webOS emulation libs on Windows.. but works on the real device
	glGetBooleanv(GL_SCISSOR_TEST, &b);
	
#if defined(WIN32) && defined(RT_WEBOS) && defined(_DEBUG)
	//clear the error out so it doesn't flood our log
	glGetError();
#endif
	
	
#ifdef PLATFORM_ANDROID
//	m_bOldScissorEnabled = 1; //force scissors to work regardless of what the above check says, fixes Nexus 7.
	//might break other stuff though, who knows
	//update: hmm, it does break stuff on some devices with Growtopia.  So we better not do this

	m_bOldScissorEnabled = b != 0;
#else
	m_bOldScissorEnabled = b != 0;
#endif
	

	CHECK_GL_ERROR()
	if (m_bOldScissorEnabled)
	{
		//warning: Untested code...
		GLint nums[4];
		glGetIntegerv(GL_SCISSOR_BOX, &nums[0]);
		m_oldScissorPos = CL_Vec2f((float)nums[0],(float) nums[1]);
		m_oldScissorSize = CL_Vec2f((float)nums[2],(float) nums[3]);
		CHECK_GL_ERROR()
	}

	CL_Vec2f vFinalPos = pVList->m_variant[0].GetVector2()+*m_pPos2d;
	//vFinalPos -= GetAlignmentOffset(*m_pSize2d, eAlignment(*m_pAlignment));	

	CL_Rectf clipRect(vFinalPos.x, vFinalPos.y, vFinalPos.x+m_pSize2d->x, vFinalPos.y+m_pSize2d->y);
		
	//well, turns out we need to always do this on iOS, as this accounts for the screen being rotated as well, not just
	//the scaling issue

//	if (NeedToUseFakeScreenSize())
	{
		//Oh shit-sticks. We're stretching our content and using a fake screensize.  We'll need to convert
		//our glScissor rect to match the real gl surface size.
	
		float angle = OrientationToDegrees(GetOrientation());
		while (angle < 0)
		{
			angle+= 360;
		}
		rtRectf r = rtRectf(clipRect.left, clipRect.top, clipRect.right, clipRect.bottom);
		r = ConvertFakeScreenRectToReal(r);
		clipRect = CL_Rectf(r.left, r.top, r.right, r.bottom);

		float primaryX = (float)GetPrimaryGLX();
		float primaryY = (float)GetPrimaryGLY();
		
        if (GetBaseApp()->GetManualRotationMode())
        {
            if (InLandscapeGUIMode())
            {
                swap(primaryX, primaryY);
            }
            clipRect = RotateRect(clipRect, angle, CL_Vec2f(primaryX, primaryY));
        }
		
	}

	//remember, glScissors x/y is the LOWER LEFT of the rect, not upper left. (and lower left is 0,0)
	glScissor((GLint)clipRect.left, (GLint)GetPrimaryGLY()-((GLint)clipRect.top+(GLint)clipRect.get_height()), (GLint)clipRect.get_width(),(GLint) clipRect.get_height());
	glEnable(GL_SCISSOR_TEST);

}

void RenderScissorComponent::PostOnRender(VariantList *pVList)
{
	g_globalBatcher.Flush();
	if (m_bOldScissorEnabled)
	{
		//leave it enabled.. setup the box to how it was
		glScissor((GLint)m_oldScissorPos.x, (GLint)m_oldScissorPos.y,(GLsizei) m_oldScissorSize.x, (GLsizei)m_oldScissorSize.y);
	} else
	{
		glDisable(GL_SCISSOR_TEST);
	}


}

