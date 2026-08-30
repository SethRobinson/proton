//  ***************************************************************
//  GL1ShaderShimUndef - Creation date: 08/30/2026
//  -------------------------------------------------------------
//  Robinson Technologies - Check license.txt for license info.
//  ***************************************************************

//Undoes GL1ShaderShim.h's remapping for translation units that implement the
//pipeline and must call the REAL gl functions (ShaderPipeline.cpp).  This is
//the only opt-out mechanism: under MSVC /Yu the precompiled header replaces
//everything up to the PlatformPrecomp.h include, so a .cpp cannot prevent the
//shim macros from existing - it can only remove them again afterwards.
//Include immediately after PlatformPrecomp.h.  No include guard on purpose.

#ifdef glMatrixMode
	#undef glMatrixMode
	#undef glPushMatrix
	#undef glPopMatrix
	#undef glLoadIdentity
	#undef glTranslatef
	#undef glRotatef
	#undef glScalef
	#undef glOrtho
	#undef glLoadMatrixf
	#undef glMultMatrixf
	#undef glGetFloatv
	#undef glOrthof
	#undef glColor4x
	#undef glEnableClientState
	#undef glDisableClientState
	#undef glVertexPointer
	#undef glTexCoordPointer
	#undef glColorPointer
	#undef glNormalPointer
	#undef glDrawArrays
	#undef glDrawElements
	#undef glEnable
	#undef glDisable
	#undef glClipPlane
	#undef glClipPlanef
	#undef glLightf
	#undef glLightfv
	#undef glShadeModel
#endif
