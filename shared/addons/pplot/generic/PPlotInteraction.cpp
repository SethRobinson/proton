/***************************************************************************
 *                                                                         *
 *   Copyright notice:                                                     *
 *                                                                         *
 *   This is free Pier ware. You may do whatever you want with this code.  *
 *   You may cont(r)act me by email: pierphil@xs4all.nl                    *
 *                                                                         *
 ***************************************************************************/

#include "PPlotInteraction.h"

#include <stdio.h>
#include <math.h>

using namespace std;

const float kHitDistance = (float)10.0;//pixels

float pmax (float inX1, float inX2) {
  if (inX1>inX2) {
    return inX1;
  }
  return inX2;
}
float pmin (float inX1, float inX2) {
  if (inX1<inX2) {
    return inX1;
  }
  return inX2;
}

PMouseEvent::PMouseEvent (int inX, int inY, EType inType, int inModifierKeys):
  PModifierKeys(inModifierKeys),
  mX(inX),
  mY(inY),
  mType(inType)
{
}

PKeyEvent::PKeyEvent (EKey inKey, int inRepeatCount, int inModifierKeys, char inChar):
  PModifierKeys (inModifierKeys),
  mKey (inKey),
  mChar (inChar),
  mRepeatCount (inRepeatCount)
{
}

PPlotInteraction::PPlotInteraction (PPlot &inPPlot):
  mPPlot (inPPlot),
  mIsEnabled (true)
{
}


PZoomInteraction::PZoomInteraction (PPlot &inPPlot):
  PPlotInteraction (inPPlot),
  mDragging (false),
  mZoomMode (kZoom_Region)
{
  inPPlot.mPostDrawerList.push_back (this);
}

bool PZoomInteraction::HandleMouseEvent (const PMouseEvent &inEvent) {
  if (!mDragging) {
    if (inEvent.IsMouseDown ()) {
      /*      if (inEvent.IsOnlyShiftKeyDown ()) {
	DoZoomOut ();
	return true;
	}*/
      if (inEvent.HasModifierKeys ()) {
	return false;
      }
      mDragging = true;
      mX1 = mX2 = inEvent.mX;
      mY1 = mY2 = inEvent.mY;
      return true;
    }
  }
  else {
    if (inEvent.IsMouseUp ()) {
      // here we should zoom

      if (mX1 == mX2 && mY1 == mY2) {
	mDragging = false;
	DoZoomOut ();
	return true;
	//	return false;// emtpy area				   
      }

      DoZoomIn ();
      mDragging = false;
      return true;
    }
    if (inEvent.IsMouseMove ()) {
      mX2 = inEvent.mX;
      mY2 = inEvent.mY;
      return true;
    }
  }
  return false;
}

bool PZoomInteraction::HandleKeyEvent (const PKeyEvent &inEvent) {

  if (inEvent.IsOnlyControlKeyDown () && inEvent.IsChar ()) {
    switch (inEvent.GetChar ()) {
    case 'r':
      mZoomMode = kZoom_Region;
      return true;
      break;
    case 'x':
      mZoomMode = kZoom_X;
      return true;
      break;
    case 'y':
      mZoomMode = kZoom_Y;
      return true;
      break;
    }
  }
  return false;
};

bool PZoomInteraction::Draw (Painter &inPainter) {
  if (mDragging) {
    inPainter.SetLineColor (255, 0, 0);

    float theX1 = mX1;
    float theX2 = mX2;
    float theY1 = mY1;
    float theY2 = mY2;

    bool theDrawInverse = true;

    switch (mZoomMode) {
    case kZoom_Region:
      //      theDrawInverse = false;
      break;
    case kZoom_X:
      theY1 = mPPlot.mMargins.mTop;
      theY2 = inPainter.GetHeight ()-mPPlot.mMargins.mBottom;
      break;
    case kZoom_Y:
      theX1 = mPPlot.mMargins.mLeft;
      theX2 = inPainter.GetWidth ()-mPPlot.mMargins.mRight;
      break;
    }

    // draw rectangle
    inPainter.DrawLine (theX1, theY1, theX2, theY1);
    inPainter.DrawLine (theX2, theY1, theX2, theY2);
    inPainter.DrawLine (theX2, theY2, theX1, theY2);
    inPainter.DrawLine (theX1, theY2, theX1, theY1);

    if (theDrawInverse) {
      float theX = pmin (theX1, theX2); 
      float theY = pmin (theY1, theY2); 
      float theW = fabs (theX1-theX2);
      float theH = fabs (theY1-theY2);
      inPainter.InvertRect (theX, theY, theW, theH);
    }
  }
  return true;
  
}

bool PZoomInteraction::CheckRange (float inFloat1, float inFloat2) {
    if (fabs(inFloat1-inFloat2) < 1e-5) {
        return false;
    }
    int abs1 = (int) log10(fabs (inFloat1));
    int abs2 = (int) log10(fabs (inFloat2));
    if ( abs1 == abs2) {
        float theVal1 = (inFloat1 / pow (10, (double)abs1));
        float theVal2 = (inFloat2 / pow (10, (double)abs2));
        float theValDif = fabs(theVal1  - theVal2);
        if (theValDif < 1e-5) {
            return false;
        }
    }
    return true;
}

// hmmm... copied from PPlot.cpp
void PZoomInteraction::DoZoomIn (float inX1, float inX2, float inY1, float inY2) {
    if (!CheckRange (inX1, inX2)) {
        return;
    }
    if (!CheckRange (inY1, inY2)) {
        return;
    }
    // also use the following criterium that is used in PPlot::CalculateTickInfo to 
    // avoid strange zoom in / zoom out behaviour
    float theYRange = fabs (inY1 - inY2);
    float theYMax = (inY1 > inY2) ? inY1 : inY2;
    if (fabs (theYRange / theYMax) < PPlot::kRangeVerySmall) {
        return;
    }

    StoreCurrentAxisSetup ();

    if (IsZoomRegion () || IsZoomX ()) {
      mPPlot.mXAxisSetup.SetAutoScale (false);
      mPPlot.mXAxisSetup.mMin = pmin (inX1, inX2);
      mPPlot.mXAxisSetup.mMax = pmax (inX1, inX2);
    }
    if (IsZoomRegion () || IsZoomY ()) {
      mPPlot.mYAxisSetup.SetAutoScale (false);
      mPPlot.mYAxisSetup.mMin = pmin (inY1, inY2);
      mPPlot.mYAxisSetup.mMax = pmax (inY1, inY2);
    }

    return;
}

void PZoomInteraction::StoreCurrentAxisSetup () {
  PAxisInfo theInfo;// store the current axis setup

  theInfo.mXAxisSetup = mPPlot.mXAxisSetup;
  theInfo.mYAxisSetup = mPPlot.mYAxisSetup;

  mZoomHistory.push (theInfo);
}

void PZoomInteraction::DoZoomIn () {
  float theX1 = mPPlot.mXTrafo->TransformBack (mX1);
  float theX2 = mPPlot.mXTrafo->TransformBack (mX2);
  float theY1 = mPPlot.mYTrafo->TransformBack (mY1);
  float theY2 = mPPlot.mYTrafo->TransformBack (mY2);

  DoZoomIn (theX1, theX2, theY1, theY2);
};

// arguments allow us to zoom out in X direction but change the Y-axis
void PZoomInteraction::DoZoomOut (float inY1, float inY2) {
  if (mZoomHistory.size () == 0) {
    return;
  }
  PAxisInfo theInfo = mZoomHistory.top ();
  mZoomHistory.pop ();

  mPPlot.mXAxisSetup = theInfo.mXAxisSetup;
  if (inY1 != -1) {
    mPPlot.mYAxisSetup.mMin = pmin (inY1, inY2);
    mPPlot.mYAxisSetup.mMax = pmax (inY1, inY2);
  } else {
    mPPlot.mYAxisSetup = theInfo.mYAxisSetup;
  }
}

PSelectionInteraction::PSelectionInteraction (PPlot &inPPlot):
  PPlotInteraction (inPPlot),
  mCommand (kNone),
  mListener (0)
{
  inPPlot.mPostCalculatorList.push_back (this);
}


bool PSelectionInteraction::HandleKeyEvent (const PKeyEvent &inEvent) {

  mCommand = kNone;
  if (inEvent.IsOnlyControlKeyDown () && inEvent.IsChar () && inEvent.GetChar () == 'a') {
//    mCalculate = true;
    mCommand = kSelectAll;
    mKeyEvent = inEvent;
  }
  return mCommand != kNone;
};

void PSelectionInteraction::SelectAll (PlotDataBase *inYData, PlotDataSelection *inPlotDataSelection) {
  for (long theI=0;theI<inPlotDataSelection->size ();theI++) {
    (*inPlotDataSelection)[theI] = true;
  }
}


bool PSelectionInteraction::HandleMouseEvent (const PMouseEvent &inEvent) {

  mCommand = kNone;
  if (inEvent.IsMouseDown ()) {
    if (!(inEvent.IsOnlyControlKeyDown () || inEvent.IsOnlyShiftKeyDown ())) {
      return false;
    }
    fprintf (stderr, "selection\n");
//    mCalculate = true;
    if (inEvent.IsOnlyControlKeyDown ()) {
      mCommand = kPointwiseSelection;
    }
    else if (inEvent.IsOnlyShiftKeyDown ()) {
      mCommand = kGlobalSelection;
    }
    mMouseEvent = inEvent;
    return true;
  }
  return false;
}

bool PSelectionInteraction::Calculate (Painter &inPainter, PPlot& inPPlot) {

  if (mCommand == kNone) {
    return true;
  }

  PlotDataContainer &theContainer = inPPlot.mPlotDataContainer;
  long thePlotCount = theContainer.GetPlotCount ();
  float theDist = -1;

  for (long theI=0;theI<thePlotCount;theI++) {
    PlotDataBase *theXData = theContainer.GetXData (theI);
    PlotDataBase *theYData = theContainer.GetYData (theI);
    DataDrawerBase *theDataDrawer = theContainer.GetDataDrawer (theI);
    PlotDataSelection *thePlotDataSelection = theContainer.GetPlotDataSelection (theI);



    long theNearestPointIndex;
    float theLocalDist = CalculateDistanceToPlot (theXData, theYData, theNearestPointIndex);

    fprintf (stderr, "dist %f\n", theLocalDist);

    bool theHit = theLocalDist < kHitDistance;

    if (mCommand == kPointwiseSelection) {
      HandlePointwiseInteraction (theHit, theNearestPointIndex, theDataDrawer, thePlotDataSelection);
    }
    else if (mCommand == kGlobalSelection){
      HandleGlobalInteraction (theHit, theNearestPointIndex, theDataDrawer, thePlotDataSelection);
    }
    else if (mCommand == kSelectAll) {
      SelectAll (theYData, thePlotDataSelection);
    }

    if (theHit) {
      fprintf (stderr, "hit/n");
    }
  }
  if (mListener) {
    mListener->HandlePSelectionInteraction ();
  }

  mCommand = kNone;

  return true;
}

void PSelectionInteraction::HandleGlobalInteraction (bool inHit, long inNearestPointIndex, DataDrawerBase *inDataDrawer, PlotDataSelection *inPlotDataSelection) {
    LineDataDrawer *theLineDataDrawer = dynamic_cast<LineDataDrawer *>(inDataDrawer);
    if (theLineDataDrawer) {
//      theLineDataDrawer->mDrawPoint = inHit;
    }
    if (inPlotDataSelection->size ()>0) {
      for (int theI=0;theI<inPlotDataSelection->size ();theI++) {
        (*inPlotDataSelection)[theI] = inHit;
      }
    }
}

void PSelectionInteraction::HandlePointwiseInteraction (bool inHit, long inNearestPointIndex, DataDrawerBase *inDataDrawer, PlotDataSelection *inPlotDataSelection) {
  if (!inHit) {
    return;
  }
  if (inPlotDataSelection->size ()>inNearestPointIndex) {
      bool theWasHit = (*inPlotDataSelection)[inNearestPointIndex] !=0;
      (*inPlotDataSelection)[inNearestPointIndex] = !theWasHit;
      /*
    for (int theI=0;theI<inPlotDataSelection->size ();theI++) {
      bool theWasHit = (*inPlotDataSelection)[theI] !=0;
      bool theIsHit = false;
      if (theI==inNearestPointIndex) {
        theIsHit = !theWasHit;
      }
      (*inPlotDataSelection)[theI] = theIsHit;
      }*/
  }
}

float PSelectionInteraction::CalculateDistanceToPlot (const PlotDataBase *inXData, const PlotDataBase *inYData, long &outNearestPointIndex) {

//  const RealPlotData *theXData = inXData->GetRealPlotData ();
//  const RealPlotData *theYData = inYData->GetRealPlotData ();
  outNearestPointIndex = -1;
  float theDist = -1;
  for (int theI=0;theI<inXData->GetSize ();theI++) {
//    float theX = (*theXData)[theI];
//    float theY = (*theYData)[theI];
    float theX = inXData->GetValue (theI);
    float theY = inYData->GetValue (theI);
    // transform to pixels
    theX = mPPlot.mXTrafo->Transform (theX)-mMouseEvent.mX;
    theY = mPPlot.mYTrafo->Transform (theY)-mMouseEvent.mY;

    float theTmp = theX*theX+theY*theY;

    if (theTmp<theDist||theDist<0) {
      theDist = theTmp;
      outNearestPointIndex = theI;
    }
  }

  return sqrt (theDist);
}


PlotDataIncrementerBounds::PlotDataIncrementerBounds ():
  mLowerBoundEnabled (false),
  mLowerBound (0),
  mUpperBoundEnabled (false),
  mUpperBound (0)
{
}

bool PlotDataIncrementerBounds::CheckBounds (float inValue) const {
  if (mLowerBoundEnabled && inValue<mLowerBound) {
    return false;
  }
  if (mUpperBoundEnabled && inValue>mUpperBound) {
    return false;
  }
  return true;
}


bool PlotDataIncrementer::Increment (const vector<float> &inIncrementList, vector<float *> &inData, const PlotDataIncrementerBounds &inGlobalBounds, const vector<PlotDataIncrementerBounds> &inBoundList) const {
  bool theDontChange = true;

  if (!Impl_Increment (inIncrementList, inData, inGlobalBounds, inBoundList, theDontChange)) {
    return false;
  }
  theDontChange = false;
  Impl_Increment (inIncrementList, inData, inGlobalBounds, inBoundList, theDontChange);
  return true;
}

bool PlotDataIncrementer::Impl_Increment (const vector<float> &inIncrementList, vector<float *> &inData, const PlotDataIncrementerBounds &inGlobalBounds, const vector<PlotDataIncrementerBounds> &inBoundList, bool inDontChange) const {

  if (inBoundList.size ()>0 && inBoundList.size () != inData.size ()) {
    return false;
  }
  if (inIncrementList.size () != inData.size ()) {
    return false;
  }

  for (int theI=0;theI<inData.size ();theI++) {
    float *theValue = inData[theI];
    float theIncrement = inIncrementList[theI];
    float theIncrementedValue = *theValue+theIncrement;
    if (!inGlobalBounds.CheckBounds (theIncrementedValue)) {
      return false;
    }
    if (inBoundList.size ()>0) {
      if (!inBoundList[theI].CheckBounds (theIncrementedValue)) {
        return false;
      }
    }
    if (!inDontChange) {
      *theValue = theIncrementedValue;
    }
  }
  return true;
}



PEditInteraction::PEditInteraction (PPlot &inPPlot):
  PPlotInteraction (inPPlot),
  mCalculate (false),
  mListener (0)
{
  inPPlot.mModifyingCalculatorList.push_back (this);
}

bool PEditInteraction::HandleKeyEvent (const PKeyEvent &inEvent) {

  mCalculate = Impl_HandleKeyEvent (inEvent);
  mKeyEvent = inEvent;
  return mCalculate;
};

bool PEditInteraction::Calculate (Painter &inPainter, PPlot& inPPlot) {
  if (!mCalculate) {
    return true;
  }

  Impl_Calculate (inPainter, inPPlot);

  if (mListener) {
    mListener->HandlePEditInteraction ();
  }
  mCalculate = false;
  return true;
}



PVerticalCursorInteraction::PVerticalCursorInteraction (PPlot &inPPlot):
  PEditInteraction (inPPlot)
{
}

bool PVerticalCursorInteraction::Impl_HandleKeyEvent (const PKeyEvent &inEvent) {

  if (inEvent.IsArrowDown () || inEvent.IsArrowUp ()) {
    return true;
  }
  return false;
};

bool PVerticalCursorInteraction::Impl_Calculate (Painter &inPainter, PPlot& inPPlot) {

  PlotDataContainer &theContainer = inPPlot.mPlotDataContainer;
  long thePlotCount = theContainer.GetPlotCount ();
  for (long theI=0;theI<thePlotCount;theI++) {
    PlotDataBase *theXData = theContainer.GetXData (theI);
    PlotDataBase *theYData = theContainer.GetYData (theI);
    DataDrawerBase *theDataDrawer = theContainer.GetDataDrawer (theI);
    PlotDataSelection *thePlotDataSelection = theContainer.GetPlotDataSelection (theI);

    if (mKeyEvent.IsArrowUp () || mKeyEvent.IsArrowDown () ) {
      HandleVerticalCursorKey (thePlotDataSelection, theYData);
    }
  }
  return true;
}

void PVerticalCursorInteraction::HandleVerticalCursorKey (const PlotDataSelection *inPlotDataSelection, PlotDataBase *inYData) {
  class PlotData *theYData = dynamic_cast<PlotData *>(inYData);
  if (!theYData) {
    return;
  }
  vector<float> theIncrementList (inPlotDataSelection->GetSelectedCount ());
  vector<float *> theSelectedData (inPlotDataSelection->GetSelectedCount ());
  float theDelta = 1;// pixels
  if (mKeyEvent.IsArrowDown ()) {
    theDelta *= -1;
  }
  if (mKeyEvent.IsOnlyControlKeyDown ()) {
    theDelta *= 10;
  }
  long theIndex = 0;
  for (int theI=0;theI<theYData->GetSize ();theI++) {
    if (inPlotDataSelection->IsSelected (theI)) {
      float *theNow = &((*theYData)[theI]);
      float theNowPixels = mPPlot.mYTrafo->Transform (*theNow);
      float theNow2 = mPPlot.mYTrafo->TransformBack (theNowPixels);
      theNowPixels -= theDelta;
      float theShiftedNow = mPPlot.mYTrafo->TransformBack (theNowPixels);
      float theDeltaData = theShiftedNow-*theNow;
      theIncrementList[theIndex] = theDeltaData;
      theSelectedData[theIndex] = theNow;
//      float theNew = theNow + theDelta;
//      (*theYData)[theI] = theNew;
      theIndex++;
    }
  }
  PlotDataIncrementer theIncremter;
  vector<PlotDataIncrementerBounds> theDummyList;
  theIncremter.Increment (theIncrementList, theSelectedData, mGlobalBounds, theDummyList);
}


PDeleteInteraction::PDeleteInteraction (PPlot &inPPlot):
  PEditInteraction (inPPlot)
{
}

bool PDeleteInteraction::Impl_HandleKeyEvent (const PKeyEvent &inEvent) {

  if (inEvent.IsDelete ()) {
    return true;
  }
  return false;
};

bool PDeleteInteraction::Impl_Calculate (Painter &inPainter, PPlot& inPPlot) {

  PlotDataContainer &theContainer = inPPlot.mPlotDataContainer;
  long thePlotCount = theContainer.GetPlotCount ();
  for (long theI=0;theI<thePlotCount;theI++) {
    PlotDataBase *theXData = theContainer.GetXData (theI);
    PlotDataBase *theYData = theContainer.GetYData (theI);
    DataDrawerBase *theDataDrawer = theContainer.GetDataDrawer (theI);
    PlotDataSelection *thePlotDataSelection = theContainer.GetPlotDataSelection (theI);

    if (mKeyEvent.IsDelete () ) {
      HandleDeleteKey (theXData, theYData, thePlotDataSelection);
    }
  }
  return true;
}

#include <algorithm>
using namespace std;

template<class T> bool Erase (const vector<int> &inEraseList, vector <T> &ioVec) {
  vector<int> theSortedList = inEraseList;
  sort (theSortedList.begin (), theSortedList.end ());
  reverse (theSortedList.begin (), theSortedList.end ());
  unique (theSortedList.begin (), theSortedList.end ());// remove duplicates
  for (vector<int>::iterator theI=theSortedList.begin();theI!=theSortedList.end ();theI++) {
    int theEraseIndex = *theI;
    //    vector <T> ::iterator theX;
    //    theX = ioVec[theEraseIndex];
    //    T *theX = &(ioVec[theEraseIndex]);
    ioVec.erase (ioVec.begin ()+theEraseIndex);
  }

  return true;
}


void PDeleteInteraction::HandleDeleteKey (PlotDataBase *inXData, PlotDataBase *inYData, PlotDataSelection *inPlotDataSelection) {
  class PlotData *theXData = dynamic_cast<PlotData *>(inXData);
  if (!theXData) {
    return;
  }
  class PlotData *theYData = dynamic_cast<PlotData *>(inYData);
  if (!theYData) {
    return;
  }
  vector<int> theDeleteList (inPlotDataSelection->GetSelectedCount ());
  long theIndex = 0;
  for (int theI=0;theI<theYData->GetSize ();theI++) {
    if (inPlotDataSelection->IsSelected (theI)) {
      theDeleteList[theIndex] = theI;
      theIndex++;
    }
  }
  Erase (theDeleteList, *theXData);
  Erase (theDeleteList, *theYData);
  Erase (theDeleteList, *inPlotDataSelection);
}

PCrosshairInteraction::PCrosshairInteraction (PPlot &inPPlot):
  PPlotInteraction (inPPlot),
  mActive (false),
  mX (0),
  mListener (0)
{
  inPPlot.mPostDrawerList.push_back (this);
}

bool PCrosshairInteraction::HandleMouseEvent (const PMouseEvent &inEvent) {
  if (!mActive) {
    if (inEvent.IsMouseDown ()) {
      if (inEvent.IsShiftKeyDown () && inEvent.IsControlKeyDown ()) {
	mActive = true;
	mX = inEvent.mX;
	return true;
      }
    }
  }
  else {
    if (inEvent.IsMouseUp ()) {
      mActive = false;
      return true;
    }
    if (inEvent.IsMouseMove ()) {
      mX = inEvent.mX;
      return true;
    }
  }
  return false;  
}

bool PCrosshairInteraction::Draw (Painter &inPainter) {
  if (mActive) {
    float theX1 = mX;
    float theY1 = mPPlot.mMargins.mTop;
    float theX2 = mX;
    float theY2 = inPainter.GetHeight () - mPPlot.mMargins.mBottom;

    inPainter.SetLineColor (0, 0, 0);
    inPainter.DrawLine (theX1, theY1, theX2, theY2);

    PlotDataContainer &theContainer = mPPlot.mPlotDataContainer;
    long thePlotCount = theContainer.GetPlotCount ();

    for (long theI=0;theI<thePlotCount;theI++) {
      PlotDataBase *theXData = theContainer.GetXData (theI);
      PlotDataBase *theYData = theContainer.GetYData (theI);
      LegendData *theLegendData = theContainer.GetLegendData (theI);

      float theY;
      if (GetCrossPoint (theXData, theYData, theY)) {
        if (mListener) {
          float theXTarget = mPPlot.mXTrafo->TransformBack (mX);
          float theYTarget = mPPlot.mYTrafo->TransformBack (theY);
          mListener->HandleCrosshair (theI, thePlotCount, theXTarget, theYTarget);
        }
        theX1 = mPPlot.mMargins.mLeft;
        theX2 = inPainter.GetWidth ()-mPPlot.mMargins.mLeft;
        theY1 = theY2 = theY;
        PColor theC = theLegendData->mColor;
        inPainter.SetLineColor (theC.mR, theC.mG, theC.mB);
        inPainter.DrawLine (theX1, theY1, theX2, theY2);
      }
    }
  }
  return true;
}

bool PCrosshairInteraction::GetCrossPoint (const PlotDataBase *inXData, const PlotDataBase *inYData, float &outY) {
  if (inXData->GetSize ()==0 ){
    return false;
  }
  float theXTarget = mPPlot.mXTrafo->TransformBack (mX);
  bool theFirstIsLess = inXData->GetValue (0) < theXTarget;
  for (int theI=0;theI<inXData->GetSize ();theI++) {
    float theX = inXData->GetValue (theI);
    float theY = inYData->GetValue (theI);
    bool theCurrentIsLess = theX < theXTarget;

    if (theCurrentIsLess != theFirstIsLess) {
      outY = mPPlot.mYTrafo->Transform (theY);// transform to pixels
      return true;
    }
  }
  return false;
}


bool InteractionContainer::HandleMouseEvent (const PMouseEvent &inEvent) {
  for (int theI=0;theI<mList.size ();theI++) {
    PPlotInteraction *theInteraction = mList[theI];
    if (theInteraction->IsEnabled () && theInteraction->HandleMouseEvent (inEvent)) {
      return true;
    }
  }
  return false;
}

bool InteractionContainer::HandleKeyEvent (const PKeyEvent &inEvent) {
  for (int theI=0;theI<mList.size ();theI++) {
    PPlotInteraction *theInteraction = mList[theI];
    if (theInteraction->IsEnabled () && theInteraction->HandleKeyEvent (inEvent)) {
      return true;
    }
  }
  return false;
}


DefaultInteractionContainer::DefaultInteractionContainer (PPlot &inPPlot):
  mZoomInteraction (inPPlot),
  mSelectionInteraction (inPPlot),
  mVerticalCursorInteraction (inPPlot),
  mDeleteInteraction (inPPlot),
  mCrosshairInteraction (inPPlot)
{
  AddInteraction (mZoomInteraction);
  AddInteraction (mSelectionInteraction);
  AddInteraction (mVerticalCursorInteraction);
  AddInteraction (mDeleteInteraction);
  AddInteraction (mCrosshairInteraction);
/*
  mVerticalCursorInteraction.mGlobalBounds.mLowerBoundEnabled = true;
  mVerticalCursorInteraction.mGlobalBounds.mLowerBound = 0;
  mVerticalCursorInteraction.mGlobalBounds.mUpperBoundEnabled = true;
  mVerticalCursorInteraction.mGlobalBounds.mUpperBound = 10;*/
}
