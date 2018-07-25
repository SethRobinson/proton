/***************************************************************************
 *                                                                         *
 *   Copyright notice:                                                     *
 *                                                                         *
 *   This is free Pier ware. You may do whatever you want with this code.  *
 *   You may cont(r)act me by email: pierphil@xs4all.nl                    *
 *                                                                         *
 ***************************************************************************/

#include "P3DPlot.h"

#include<math.h>

P3DPlot::P3DPlot ():mData (0), mXCount (0), mYCount (0) {
}
P3DPlot::~P3DPlot () {
  delete mData;
}

void P3DPlot::SetData (PlotData *inData, long inXCount, long inYCount) {
  long theSize = inData->size ();
  if (inXCount ==0) {
    float theA = sqrt (float(theSize));
    long theRoundA = theA + 0.5;
    if (theRoundA*theRoundA != theSize) {
      delete inData;
      return;
    }
    delete mData;
    mData = inData;
    mXCount = mYCount = theRoundA;
  }
  
}

bool P3DPlot::MakeModel (Carpenter &inCarpenter) {

  mCarpenter = &inCarpenter;
  inCarpenter.SetSurfaceNormal (PVertex (1,0,0));
    inCarpenter.SetColor (255,0,0);
    inCarpenter.MakeLine (PVertex(0,0,0), PVertex (1,0,0));
  inCarpenter.SetSurfaceNormal (PVertex (0,1,0));
    inCarpenter.SetColor (0,255,0);
    inCarpenter.MakeLine (PVertex(0,0,0), PVertex (0,1,0));
  inCarpenter.SetSurfaceNormal (PVertex (0,0,1));
    inCarpenter.SetColor (0,0,255);
    inCarpenter.MakeLine (PVertex(0,0,0), PVertex (0,0,1));

    inCarpenter.SetColor (255,0,0);

    if (!mData) {
      return false;
    }
    long theIndex = 0;
    float theXScale = 1.0/mXCount;
    float theYScale = 1.0/mYCount;
    float theMaxZ = 0;

    int theI;
    for (theI=0;theI<mData->size();theI++) {
      float theValue = (*mData)[theI];	
      if (theValue>theMaxZ) {
	theMaxZ = theValue;
      }
    }    
    float theZScale = 1.0/theMaxZ;
    for (theI=0;theI<mXCount;theI++) {
      for (int theJ=0;theJ<mYCount;theJ++) {
	float theValue = (*mData)[theIndex];	
	float theX=theI*theXScale-0.5;
	float theY=theJ*theYScale-0.5;
	float theZ = theValue*theZScale;
	float theW = 0.8/mXCount;
	if (theZ>0.001) {
	  MakeBar (theX, theY, theZ, theW);
	}
	theIndex++;
      }
    }
    inCarpenter.Finish ();
    return true;
}


void P3DPlot::MakeBar (float inX, float inY, float inZ, float inSize) {
  // x
  vector<PVertex> theVs;
  float theS = inSize/2;
  theVs.push_back (PVertex (0,-theS,0));
  theVs.push_back (PVertex (0,+theS,0));
  theVs.push_back (PVertex (0,+theS,inZ));
  theVs.push_back (PVertex (0,-theS,inZ));

  mCarpenter->SetSurfaceNormal (PVertex (1,0,0));
  MakePolygonWithOffset (theVs, PVertex (inX+theS,inY,0));
  mCarpenter->SetSurfaceNormal (PVertex (-1,0,0));
  MakePolygonWithOffset (theVs, PVertex (inX-theS,inY,0));

  //y 
  theVs.clear ();
  theVs.push_back (PVertex (-theS,0,0));
  theVs.push_back (PVertex (+theS,0,0));
  theVs.push_back (PVertex (+theS,0,inZ));
  theVs.push_back (PVertex (-theS,0,inZ));

  mCarpenter->SetSurfaceNormal (PVertex (0,1,0));
  MakePolygonWithOffset (theVs, PVertex (inX,inY+theS,0));
  mCarpenter->SetSurfaceNormal (PVertex (0,-1,0));
  MakePolygonWithOffset (theVs, PVertex (inX,inY-theS,0));

  // top
  theVs.clear ();
  theVs.push_back (PVertex (inX-theS,inY-theS,inZ));
  theVs.push_back (PVertex (inX-theS,inY+theS,inZ));
  theVs.push_back (PVertex (inX+theS,inY+theS,inZ));
  theVs.push_back (PVertex (inX+theS,inY-theS,inZ));
  mCarpenter->SetSurfaceNormal (PVertex (0,0,1));
  mCarpenter->MakePolygon (theVs);
  
}


void P3DPlot::MakePolygonWithOffset (const vector<PVertex> &inVertices, const PVertex &inOffset) {
  vector<PVertex> theVertices;
  for (int theI=0;theI<inVertices.size ();theI++) {
    PVertex theV = inVertices[theI];
    theV.mX += inOffset.mX;
    theV.mY += inOffset.mY;
    theV.mZ += inOffset.mZ;
    theVertices.push_back (theV);
  }
  mCarpenter->MakePolygon (theVertices);
}


void MakeExamplePlot1 (P3DPlot &ioPPlot) {
  const long kDim = 32;

  PlotData *theData = new PlotData ();

  long theIndex = 0;
  float theFac = 1;
  for (int theI=0;theI<kDim;theI++) {
    for (int theJ=0;theJ<kDim;theJ++) {
      theData->push_back ((theI+theJ)*theFac);
    }
  }
  ioPPlot.SetData (theData);
}

void MakeExamplePlot2 (P3DPlot &ioPPlot) {
  const long kDim = 32;

  PlotData *theData = new PlotData ();

  long theIndex = 0;
  float theFac = 1;
  for (int theI=0;theI<kDim;theI++) {
    for (int theJ=0;theJ<kDim;theJ++) {
      float theVal = rand ();
      if (theJ>theI) {
	theVal = 0;
      }
      theData->push_back (theVal);
    }
  }
  ioPPlot.SetData (theData);
}
