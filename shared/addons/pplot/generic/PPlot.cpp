/***************************************************************************
 *                                                                         *
 *   Copyright notice:                                                     *
 *                                                                         *
 *   This is free Pier ware. You may do whatever you want with this code,  *
 *    except that you should not remove this copyright notice.             *
 *                                                                         *
 ***************************************************************************/

#include "PPlot.h"

#include <algorithm>
#include <stdio.h>
#include <math.h>
// --- #include <stdlib.h>
#include <assert.h>
using namespace std;

static PPlot *sCurrentPPlot=0;

const float kFloatSmall = 1e-20f;
const float kLogMin = 1e-10f;// min argument for log10 function
const float kExpMax = 1e10f;// max argument for pow10 function
const float kLogMinClipValue = 1e-10f;// pragmatism to avoid problems with small values in log plot
const float kEps = 1e-4f;
const float kRelMajorTickSize = 0.02f;
const float kRelMinorTickSize = 0.01f;
const int kMinMinorTickScreenSize = 1;// minor ticks should not become smaller than this
const float kMaxMajorTickSizeInFontHeight = 0.5f;// not larger than half the font height
const float kLittleIncrease = 1.0001f;
const float kLittleDecrease = 0.9999f;
const float kTickValueVeryBig = 1.0e4;// switch to scientific format
const float kTickValueVerySmall = (float)1.0e-3;
const float kMajorTickXInitialFac = 2.0f;
const float kMajorTickYInitialFac = 3.0f;
const PMargins kDefaultMargins = PMargins (40,20,5,42);

const float PPlot::kRangeVerySmall = (float)1.0e-3;         // also in ZoomInteraction

template <class T> const T & PMin (const T &a, const T &b) {
  return b< a ? b: a;
}
template <class T> const T & PMax (const T &a, const T &b) {
  return b> a ? b: a;
}

float SafeLog (float inFloat, float inBase, float inFac) {
  if (inFloat<kLogMin) {
    inFloat = kLogMin;
  }
  return inFac*log10 (inFloat)/log10(inBase);
}

float SafeExp (float inFloat, float inBase, float inFac) {
  if (inFloat>kExpMax) {
    inFloat = kExpMax;
  }
  return pow(inBase, inFloat/inFac);
}

long PlotDataBase::GetSize () const { 
  if (GetRealPlotData ()) {
    return GetRealPlotData ()->size ();
  }
  if (GetCalculatedData ()) {
    return GetCalculatedData ()->GetSize ();
  }
  return 0;
}

float PlotDataBase::GetValue (long inIndex) const {
  if (GetRealPlotData ()) {
    return (*GetRealPlotData ())[inIndex];
  }
  if (GetCalculatedData ()) {
    return GetCalculatedData ()->GetValue (inIndex);
  }
  return 0;
}

bool PlotDataBase::CalculateRange (float &outXMin, float &outXMax) {
  const RealPlotData *theData = GetRealPlotData ();
  if (theData && theData->size () >0) {
    std::vector<float>::const_iterator imin = std::min_element (theData->begin (), theData->end ());
    std::vector<float>::const_iterator imax = std::max_element (theData->begin (), theData->end ());
    outXMin = *imin;
    outXMax = *imax;
    return true;
  }
  else {
    const CalculatedDataBase *theCalculated = GetCalculatedData ();
    if (theCalculated) {
        outXMin = theCalculated->GetValue (0);
        outXMax = theCalculated->GetValue (theCalculated->GetSize () - 1);
        return true;
    }
  }

  return false;
}

DummyData::DummyData (long inSize) {
  for (int theI=0;theI<inSize;theI++) {
    mRealPlotData.push_back (theI);// simple ascending data
  }
}

void StringData::AddItem (const char *inString) {
  mStringData.push_back (inString);
  mRealPlotData.push_back (mStringData.size ()-1);
}

void LegendData::SetDefaultColor (int inPlotIndex) {
  mColor = GetDefaultColor (inPlotIndex);
}

void LegendData::SetDefaultValues (int inPlotIndex) {
    SetDefaultColor (inPlotIndex);
  char theBuf[32];
  sprintf (theBuf, "plot %d", inPlotIndex);
  mName = theBuf;
}



bool PlotDataSelection::IsSelected (long inIndex) const {
  if (size ()<=inIndex) {
    return false;
  }
  return (*this)[inIndex]>0;
}

long PlotDataSelection::GetSelectedCount () const {
  long theCount = 0;
  for (int theI=0;theI<size (); theI++) {
    if (IsSelected (theI)) {
      theCount++;
    }
  }
  return theCount;
}


int PPlot::Round (float inFloat) {
  return (int)floor (inFloat+0.5f);
}

PColor LegendData::GetDefaultColor (int inPlotIndex) {

  PColor theC;
  switch (inPlotIndex%7) {
  case 0:
    theC.mR = 255;
    theC.mG = 0;
    theC.mB = 0;
    break;
  case 1:
    theC.mR = 0;
    theC.mG = 0;
    theC.mB = 255;
    break;
  case 2:
    theC.mR = 0;
    theC.mG = 255;
    theC.mB = 0;
    break;
  case 3:
    theC.mR = 0;
    theC.mG = 255;
    theC.mB = 255;
    break;
  case 4:
    theC.mR = 255;
    theC.mG = 0;
    theC.mB = 255;
    break;
  case 5:
    theC.mR = 255;
    theC.mG = 255;
    theC.mB = 0;
    break;
  default:
    // black
    break;
  }
  return theC;
}



float TickInfo::RoundSpan (float inSpan) {
  // round it to something producing readable tick labels
  // write it in the form inSpan = a*SafeExp10 (b)

  if (inSpan<=0) {
    // error
    return (float)-1.234567;
  }

  int thePow = 0;
  float theSpan = inSpan;
  if (inSpan>1) {
    while (theSpan>10) {
      theSpan/=10;
      if (theSpan == inSpan) { // not a number
          return (float)-1.234567;
      }
      thePow++;
    }
  }
  else {
    while (theSpan<1) {
      theSpan*=10;
      thePow--;      
    }
  }
  int theRoundedFirstDigit = PPlot::Round (theSpan);
  int thePreferredFirstDigit = 1;
  switch (theRoundedFirstDigit) {
  case 1:
    thePreferredFirstDigit = 1;
    break;
  case 2:
  case 3:
  case 4:
    thePreferredFirstDigit = 2;
    break;
  case 5:
  case 6:
  case 7:
  case 8:
  case 9:
    thePreferredFirstDigit = 5;
    break;
  case 10:
    thePreferredFirstDigit = 1;
    thePow++;
    break;
  default:
    // error
    return (float)-1.234567;
    break;
  }
  float theRes = thePreferredFirstDigit*pow (10, (double)thePow);
  return theRes;
}

void TickInfo::MakeFormatString (float inValue, string &outFormatString) {
  if (inValue<0) {
    inValue = - inValue;
  }
  if (inValue > kTickValueVeryBig || inValue < kTickValueVerySmall) {
    outFormatString = "%.1e";
  }
  else {

    int thePrecision = 0;
    if (inValue<1) {
      float theSpan = inValue;
      while (theSpan<1) {
	thePrecision++;
	theSpan *=10;
      }
    }

    char theBuf[128] = "%.0f";
    theBuf[2] = '0'+thePrecision;

    outFormatString = theBuf;
  }
}


PlotDataBase::~PlotDataBase (){
};


PlotDataContainer::PlotDataContainer (){
}
PlotDataContainer::~PlotDataContainer (){
  ClearData ();
}

PlotDataBase * PlotDataContainer::GetXData (int inIndex) {
    if (inIndex < 0 || inIndex >= mXDataList.size ()) {
        return 0;
    }
    return mXDataList[inIndex];
}

PlotDataBase * PlotDataContainer::GetYData (int inIndex) {
    if (inIndex < 0 || inIndex >= mYDataList.size ()) {
        return 0;
    }
    return mYDataList[inIndex];
}

LegendData * PlotDataContainer::GetLegendData (int inIndex) {
    if (inIndex < 0 || inIndex >= mLegendDataList.size ()) {
        return 0;
    }
    return mLegendDataList[inIndex];
}

DataDrawerBase * PlotDataContainer::GetDataDrawer (int inIndex) {
    if (inIndex < 0 || inIndex >= mDataDrawerList.size ()) {
        return 0;
    }
    return mDataDrawerList[inIndex];
}

PlotDataSelection * PlotDataContainer::GetPlotDataSelection (int inIndex) {
    if (inIndex < 0 || inIndex >= mPlotDataSelectionList.size ()) {
        return 0;
    }
    return mPlotDataSelectionList[inIndex];
}

const PlotDataBase * PlotDataContainer::GetConstXData (int inIndex) const {
    if (inIndex < 0 || inIndex >= mXDataList.size ()) {
        return 0;
    }
    return mXDataList[inIndex];
}

const PlotDataBase * PlotDataContainer::GetConstYData (int inIndex) const {
    if (inIndex < 0 || inIndex >= mYDataList.size ()) {
        return 0;
    }
    return mYDataList[inIndex];
}

const LegendData * PlotDataContainer::GetConstLegendData (int inIndex) const {
    if (inIndex < 0 || inIndex >= mLegendDataList.size ()) {
        return 0;
    }
    return mLegendDataList[inIndex];
}

const DataDrawerBase * PlotDataContainer::GetConstDataDrawer (int inIndex) const {
    if (inIndex < 0 || inIndex >= mDataDrawerList.size ()) {
        return 0;
    }
    return mDataDrawerList[inIndex];
}

const PlotDataSelection * PlotDataContainer::GetConstPlotDataSelection (int inIndex) const {
    if (inIndex < 0 || inIndex >= mPlotDataSelectionList.size ()) {
        return 0;
    }
    return mPlotDataSelectionList[inIndex];
}

void PlotDataContainer::RemoveElement (int inIndex) {
    if (!(inIndex < mXDataList.size () && inIndex < mYDataList.size () && 
        inIndex < mLegendDataList.size () && inIndex < mDataDrawerList.size ())) {
        // Invalid index
        return;
    }

    PlotDataList::iterator theXI = mXDataList.begin () + inIndex;
    PlotDataList::iterator theYI = mYDataList.begin () + inIndex;
    LegendDataList::iterator theLI = mLegendDataList.begin () + inIndex;
    DataDrawerList::iterator theDI = mDataDrawerList.begin () + inIndex;
    PlotDataSelectionList::iterator thePI = mPlotDataSelectionList.begin () + inIndex;

    delete *theXI;
    delete *theYI;
    delete *theLI;
    delete *theDI;
    delete *thePI;

    mXDataList.erase (theXI);
    mYDataList.erase (theYI);
    mLegendDataList.erase (theLI);
    mDataDrawerList.erase (theDI);
    mPlotDataSelectionList.erase (thePI);
}

void PlotDataContainer::ClearData () {
  PlotDataList::iterator theXI = mXDataList.begin ();
  PlotDataList::iterator theYI = mYDataList.begin ();
  LegendDataList::iterator theLI = mLegendDataList.begin ();
  DataDrawerList::iterator theDI = mDataDrawerList.begin ();
  PlotDataSelectionList::iterator thePI = mPlotDataSelectionList.begin ();

  for (;theXI!=mXDataList.end () && theYI!=mYDataList.end () && theLI!=mLegendDataList.end () && theDI != mDataDrawerList.end () && thePI != mPlotDataSelectionList.end ();) {
    PlotDataBase *theX = *theXI;
    PlotDataBase *theY = *theYI;
    LegendData *theL = *theLI;
    DataDrawerBase *theD = *theDI;
    PlotDataSelection *theP = *thePI;

    delete theX;
    delete theY;
    delete theL;
    delete theD;
    delete theP;

    theXI++;
    theYI++;
    theLI++;
    theDI++;
    thePI++;
  }
  mXDataList.clear ();
  mYDataList.clear ();
  mLegendDataList.clear ();
  mDataDrawerList.clear ();
  mPlotDataSelectionList.clear ();
}

void PlotDataContainer::AddXYPlot (PlotDataBase *inXData, PlotDataBase *inYData, LegendData *inLegendData,
								   DataDrawerBase *inDataDrawer, PlotDataSelection *inPlotDataSelection) 
{
  if (!inYData || (!inYData->GetRealPlotData () && !inYData->GetCalculatedData ())) {
    return;
  }
  PlotDataBase *theXData = inXData;
  if (!theXData) {
    theXData = new DummyData (inYData->GetSize ());
  }
  mXDataList.push_back (theXData);
  mYDataList.push_back (inYData);

  LegendData *theLegendData = inLegendData;
  if (!theLegendData) {
    theLegendData = new LegendData ();
    theLegendData->SetDefaultValues (mLegendDataList.size ());
  }
  mLegendDataList.push_back (theLegendData);

  DataDrawerBase *theDataDrawer = inDataDrawer;
  if (!theDataDrawer) {
    theDataDrawer = new LineDataDrawer ();
  }
  mDataDrawerList.push_back (theDataDrawer);

  PlotDataSelection *thePlotDataSelection = inPlotDataSelection;
  if (!thePlotDataSelection) {
    thePlotDataSelection = new PlotDataSelection ();
  }
  else {
    thePlotDataSelection->resize (inYData->GetSize ());
  }
  mPlotDataSelectionList.push_back (thePlotDataSelection);
}

void PlotDataContainer::SetXYPlot (int inIndex, PlotDataBase *inXData, PlotDataBase *inYData, LegendData *inLegendData, DataDrawerBase *inDataDrawer, PlotDataSelection *inPlotDataSelection) {
    if (!inYData || !inYData->GetRealPlotData ()) {
        return;
    }
    if (!CheckState ()) {
      return;
    }
    long thePlotCount = GetPlotCount ();
    if (inIndex<0||inIndex>thePlotCount) {
      return;
    }
    PlotDataBase *theXData = inXData;
    int theSize = theXData->GetSize ();

    if (!theXData) {
        theXData = new DummyData (inYData->GetRealPlotData ()->size ());
    }
    LegendData *theLegendData = inLegendData;
    DataDrawerBase *theDataDrawer = inDataDrawer;
    if (!theLegendData) {
        theLegendData = new LegendData ();
        if (inIndex >= 0 && inIndex < mYDataList.size () ) {
            *theLegendData = *mLegendDataList[inIndex];   // copy old values...
        } else {
            theLegendData->SetDefaultValues (mLegendDataList.size ());
        }
    }
    if (!theDataDrawer) {
        theDataDrawer = new LineDataDrawer ();
    }
    PlotDataSelection *thePlotDataSelection = inPlotDataSelection;
    if (!thePlotDataSelection) {
      thePlotDataSelection = new PlotDataSelection (inYData->GetSize ());
  //    thePlotDataSelection = new PlotDataSelection ();
    }
    if (inIndex >= 0 && inIndex < mYDataList.size () ) {
        delete mXDataList[inIndex];
        delete mYDataList[inIndex];
        delete mLegendDataList[inIndex];
        delete mDataDrawerList[inIndex];
        delete mPlotDataSelectionList[inIndex];

        mXDataList[inIndex] = theXData;
        mYDataList[inIndex] = inYData;
        mLegendDataList[inIndex] = theLegendData;
        mDataDrawerList[inIndex] = theDataDrawer;
        mPlotDataSelectionList[inIndex] = thePlotDataSelection;
    } else { // add at end
        mXDataList.push_back (theXData);
        mYDataList.push_back (inYData);
        mLegendDataList.push_back (theLegendData);
        mDataDrawerList.push_back (theDataDrawer);
        mPlotDataSelectionList.push_back (thePlotDataSelection);
    }
}

bool PlotDataContainer::SetDataDrawer (int inIndex, DataDrawerBase* inDataDrawer) {
    if (inIndex < 0 || inIndex >= mYDataList.size () ) {
        return false;
    }
    DataDrawerBase* theDataDrawer = inDataDrawer;
    if (!inDataDrawer) {
        theDataDrawer = new LineDataDrawer;
    }
    delete mDataDrawerList[inIndex];
    mDataDrawerList[inIndex] = theDataDrawer;
    return true;
}

int PlotDataContainer::GetPlotIndexByName (const string &inName) const {

  if (CheckState ()) {
    for (int theI=0;theI<mLegendDataList.size ();theI++) {
      LegendData *theLegendData = mLegendDataList[theI];
      if (theLegendData->mName == inName) {
        return theI;
      }
    }
  }
  return -1;
}


bool PlotDataContainer::CalculateXRange (float &outXMin, float &outXMax) const {
  bool theFirst = true;
  outXMin = 0;
  outXMax = 0;
  for (PlotDataList::const_iterator theI=mXDataList.begin();theI!=mXDataList.end ();theI++) {
    PlotDataBase *theXDataBase = *theI;
    if (!theXDataBase) {
      return false;
    }
    if (theXDataBase->GetSize () == 0) {
        continue;
    }
    float theXMin;
    float theXMax;
    if (!theXDataBase->CalculateRange (theXMin, theXMax)) {
      return false;
    }
    if (theXMax < theXMin) {
        return false;
    }
    if (theFirst) {
      outXMin = theXMin;
      outXMax = theXMax;
      theFirst = false;
    }
    if (theXMax>outXMax) {
      outXMax = theXMax;
    }
    if (theXMin<outXMin) {
      outXMin = theXMin;
    }
  }
  if (outXMin == 0 && outXMax == 0) {
      return false;
  }
  return true;
}

bool PlotDataContainer::CalculateYRange (float inXMin, float inXMax, float &outYMin, float &outYMax) const {
  outYMin = 0;
  outYMax = 0;
  bool theFirst = true;

  for (int theI=0; theI<GetPlotCount (); theI++) {
    const PlotDataBase *theXDataBase = GetConstXData (theI);
    const PlotDataBase *theYDataBase = GetConstYData (theI);
    if (!theXDataBase || !theYDataBase) {
      return false;
    }
    float theYMin;
    float theYMax;
    if (!CalculateYRangePlot (inXMin, inXMax, *theXDataBase, *theYDataBase, theYMin,theYMax)) {
      return false;
    }
    if (theFirst) {
      outYMin = theYMin;
      outYMax = theYMax;
      theFirst = false;
    }
    if (theYMin<outYMin) {
      outYMin = theYMin;
    }
    if (theYMax>outYMax) {
      outYMax = theYMax;
    }
    
  }
  return true;
}

bool PlotDataContainer::CalculateYRangePlot (float inXMin, float inXMax, const PlotDataBase &inXData, const PlotDataBase &inYData, float &outYMin, float &outYMax) const {
    outYMin = 0;
    outYMax = 0;
    bool theEnteredXRange = false;
    bool initialized = false;
    
    if (inXData.GetSize () != inYData.GetSize ()) {
        return false;
    }

    for (long theI = 0; theI < inXData.GetSize (); theI++) {
        float theX = inXData.GetValue (theI);
        float theY = inYData.GetValue (theI);
        
        if (theX>=inXMin && theX <= inXMax) {
            if (!initialized) {
                initialized = true;
                outYMin = theY;
                outYMax = theY;
            } else {
                if (theY<outYMin) {
                    outYMin = theY;
                }
                if (theY>outYMax) {
                    outYMax = theY;
                }
            }
        }
    }
    return true;
}

bool PlotDataContainer::CheckState () const {
  long theSize1 = mXDataList.size ();
  long theSize2 = mYDataList.size ();
  long theSize3 = mLegendDataList.size ();
  long theSize4 = mDataDrawerList.size ();
  long theSize5 = mPlotDataSelectionList.size ();
  if (theSize1!=theSize2 || theSize1!=theSize3 || theSize1!=theSize4 || theSize1!=theSize5) {
    return false;
  }
  return true;
}

float LinTrafo::Transform (float inValue) const {
  return inValue * mSlope + mOffset;
}
float LinTrafo::TransformBack (float inValue) const {
    if (mSlope != 0) {
        return (inValue - mOffset) / mSlope;
    } else {
        return 0;
    }
}


float LogTrafo::Transform (float inValue) const{
  if (inValue<kLogMinClipValue) {
    inValue = kLogMinClipValue;
  }
  return SafeLog (inValue, mBase, mFactor)*mSlope+mOffset;
}
float LogTrafo::TransformBack (float inValue) const {
    if (mSlope != 0) {
        return SafeExp( (inValue - mOffset)/mSlope, mBase, mFactor);
    } else {
        return 0;
    }
}


bool LinTickIterator::Init () {
  if (!mAxisSetup) {
    return false;
  }

  float theMin = mAxisSetup->mMin;
  float theMajorTickSpan = mAxisSetup->mTickInfo.mMajorTickSpan;
  int theDiv = mAxisSetup->mTickInfo.mTickDivision;
  mDelta = theMajorTickSpan/theDiv;
  mCount = ceil (theMin/mDelta);
  mCurrentTick = mCount*mDelta;

  mFormatString = mAxisSetup->mTickInfo.mFormatString;

  return true;

}

bool LinTickIterator::GetNextTick (float &outTick, bool &outIsMajorTick, string &outFormatString) {
  if (!mAxisSetup) {
    return false;
  }
  if (mCurrentTick>mAxisSetup->mMax*kLittleIncrease) {
    return false;
  }
  outTick = mCurrentTick;
  outIsMajorTick = (mCount%mAxisSetup->mTickInfo.mTickDivision == 0);
  outFormatString = mFormatString;

  mCurrentTick += mDelta;
  mCount++;
  return true;
}

bool LinTickIterator::InitFromRanges (float inParRange, float inOrthoScreenRange, float inDivGuess, TickInfo &ioTickInfo) const {
  if (inDivGuess <= kFloatSmall) {
    return false;
  }
  float thePreferredSpan = TickInfo::RoundSpan (inParRange/inDivGuess);
  if (thePreferredSpan < 0) {
    return false;
  }

  float thePreferredNrOfTicks = inParRange/thePreferredSpan;
  if (thePreferredNrOfTicks <1) {
    ioTickInfo.mMajorTickSpan = inParRange;
  }
  else {
    ioTickInfo.mMajorTickSpan = thePreferredSpan;
  }

  ioTickInfo.mTickDivision = 5;
  if (ioTickInfo.mAutoTickSize) {
      ioTickInfo.mMinorTickScreenSize = PMax (kMinMinorTickScreenSize, PPlot::Round (inOrthoScreenRange*kRelMinorTickSize));
      ioTickInfo.mMajorTickScreenSize = PMax (ioTickInfo.mMinorTickScreenSize+1, PPlot::Round (inOrthoScreenRange*kRelMajorTickSize));
  }

  TickInfo::MakeFormatString (ioTickInfo.mMajorTickSpan, ioTickInfo.mFormatString);
  return true;
}

bool LogTickIterator::Init () {
  if (!mAxisSetup) {
    return false;
  }

  float theMin = mAxisSetup->mMin;
  //  float theMax = mAxisSetup->mMax;
  float theMajorTickSpan = mAxisSetup->mTickInfo.mMajorTickSpan;
  int theDiv = mAxisSetup->mTickInfo.mTickDivision;
  mDelta = theMajorTickSpan/theDiv;
  float theBase = mAxisSetup->mLogBase;
  long theLogFac =  1;//mAxisSetup->mLogFactor;
  long thePowMin = (long)floor(SafeLog(theMin, theBase, theLogFac));
  mCurrentTick = SafeExp (thePowMin, theBase, theLogFac);
  mCount = 0;

  // walk to the first tick

  if (theMin<=0) {
    return false;
  // error
  }
  else {
    // walk forward
    float theNext = mCurrentTick+mDelta*SafeExp (thePowMin, theBase, theLogFac);;
    while (theNext<=theMin*kLittleDecrease) {
      mCurrentTick = theNext;
      theNext += mDelta*SafeExp (thePowMin,theBase, theLogFac);
      mCount++;
    }
  }
  return true;
}

bool LogTickIterator::InitFromRanges (float inParRange, float inOrthoScreenRange, float inDivGuess, TickInfo &ioTickInfo) const {
  if (inDivGuess<=kFloatSmall) {
    return false;
  }
  /*
  float thePreferredSpan = TickInfo::RoundSpan (inParRange/inDivGuess);
  float thePreferredNrOfTicks = inParRange/thePreferredSpan;
  if (thePreferredNrOfTicks <1) {
    ioTickInfo.mMajorTickSpan = inParRange;
  }
  else {
    ioTickInfo.mMajorTickSpan = thePreferredSpan;
  }
  */
  float theBase = mAxisSetup->mLogBase;
  ioTickInfo.mMajorTickSpan = theBase-1;// relative

  ioTickInfo.mTickDivision = PPlot::Round (ioTickInfo.mMajorTickSpan);
  ioTickInfo.mMinorTickScreenSize = PMax (kMinMinorTickScreenSize, PPlot::Round (inOrthoScreenRange*kRelMinorTickSize));
  ioTickInfo.mMajorTickScreenSize = PMax (ioTickInfo.mMinorTickScreenSize+1, PPlot::Round (inOrthoScreenRange*kRelMajorTickSize));

  ioTickInfo.mFormatString = "%.1e";
  return true;
}

bool LogTickIterator::GetNextTick (float &outTick, bool &outIsMajorTick, string &outFormatString) {
  if (!mAxisSetup) {
    return false;
  }
  if (mCurrentTick>mAxisSetup->mMax*kLittleIncrease) {
    return false;
  }
  outTick = mCurrentTick;
  outIsMajorTick = (mCount%mAxisSetup->mTickInfo.mTickDivision == 0);
  TickInfo::MakeFormatString (outTick, outFormatString);
  float theBase = mAxisSetup->mLogBase;
  float theLogFac = 1;//mAxisSetup->mLogFactor;
  float theLogNow = SafeLog(mCurrentTick, theBase, theLogFac);
  int thePowNow = (int)floor(theLogNow);
  outIsMajorTick = false;
  if (fabs (theLogNow-thePowNow)<kEps) {
    outIsMajorTick = true;
  }

  if (mAxisSetup->mLogFactor>1) {
    char theBuf[128];
    sprintf (theBuf, "%d", thePowNow*20);
    outFormatString = theBuf;
  }

  mCurrentTick += mDelta*SafeExp (thePowNow, theBase, theLogFac);
  mCount++;

  return true;
}

bool LogTickIterator::AdjustRange (float &ioMin, float &ioMax) const {

  float theBase = mAxisSetup->mLogBase;
  long theLogFac = 1;//mAxisSetup->mLogFactor;
  if (mAxisSetup->mMaxDecades > 0) {
    ioMin = ioMax/SafeExp (mAxisSetup->mMaxDecades, theBase, theLogFac);
  }
  if (ioMin == 0 && ioMax == 0) {
      ioMin = kLogMinClipValue;
      ioMax = 1.0f;
  }
  if (ioMin <= 0 || ioMax<=0) {
    return false;
  }
  ioMin = RoundDown (ioMin*kLittleIncrease);
  ioMax = RoundUp (ioMax*kLittleDecrease);

  if (ioMin<kLogMinClipValue) {
    ioMin = kLogMinClipValue;
  }
  if (mAxisSetup->mMaxDecades > 0) {
    ioMin = ioMax/SafeExp (mAxisSetup->mMaxDecades, theBase, theLogFac);
  }
  return true;
}

float LogTickIterator::RoundUp (float inFloat) const {
  float theBase = mAxisSetup->mLogBase;
  float theLogFac = 1;//mAxisSetup->mLogFactor;
  int thePow = (int)ceil(SafeLog(inFloat, theBase, theLogFac));
  return pow (theBase, thePow);
}

float LogTickIterator::RoundDown (float inFloat) const {
  float theBase = mAxisSetup->mLogBase;
  long theLogFac = 1;//mAxisSetup->mLogFactor;
  int thePow = (int)floor(SafeLog(inFloat,theBase, theLogFac));
  return pow (theBase, thePow);
}

bool NamedTickIterator::GetNextTick (float &outTick, bool &outIsMajorTick, string &outFormatString) {
  if (LinTickIterator::GetNextTick (outTick, outIsMajorTick, outFormatString)) {
    int theIndex = PPlot::Round (outTick);
    if (theIndex>=0 && theIndex < (int)mStringList.size ()) {
      outFormatString = mStringList[theIndex];
      return true;
    }
  }
  return false;
}

bool NamedTickIterator::InitFromRanges (float inParRange, float inOrthoScreenRange, float inDivGuess, TickInfo &outTickInfo) const {
  if (LinTickIterator::InitFromRanges (inParRange, inOrthoScreenRange, inDivGuess, outTickInfo)) {
    outTickInfo.mTickDivision = 1;
    return true;
  }
  return false;
}

bool PainterTester::Draw (Painter &inPainter) {

  const char * theString = "The quick brown fox...";
  int theWidth = inPainter.CalculateTextDrawSize (theString);
  int theOffset = theWidth/10;

  // a horizontal lines
  int theHAscent_x = theOffset+2*inPainter.GetFontHeight ();
  int theHAscent_y = 10;
  int theHAscent_w = theWidth;
  inPainter.DrawLine (theHAscent_x, theHAscent_y, theHAscent_x+theHAscent_w, theHAscent_y);

  int theHDescent_x = theHAscent_x;
  int theHDescent_y = theHAscent_y+inPainter.GetFontHeight ();
  int theHDescent_w = theHAscent_w;
  inPainter.DrawLine (theHDescent_x, theHDescent_y, theHDescent_x+theHDescent_w, theHDescent_y);
  
  // a vertical lines
  int theVAscent_x = theOffset;
  int theVAscent_y = theHAscent_y+theWidth;
  int theVAscent_h = -theWidth;
  inPainter.DrawLine (theVAscent_x, theVAscent_y, theVAscent_x, theVAscent_y+theVAscent_h);

  int theVDescent_x = theVAscent_x+inPainter.GetFontHeight ();
  int theVDescent_y = theVAscent_y;
  int theVDescent_h = theVAscent_h;
  inPainter.DrawLine (theVDescent_x, theVDescent_y, theVDescent_x, theVDescent_y+theVDescent_h);

  // Draw vertical text, followed by horizontal.
  inPainter.DrawRotatedText (theVDescent_x, theVDescent_y, -90, theString);
  inPainter.DrawText (theHDescent_x, theHDescent_y, theString);

  return true;
}

PPlot::PPlot ():
  mXTrafo (&mXLinTrafo),
  mYTrafo (&mYLinTrafo),
  mXTickIterator (&mXLinTickIterator),
  mYTickIterator (&mYLinTickIterator),
  mPPlotDrawer (0),
  mOwnsPPlotDrawer (true),
  mHasAnyModifyingCalculatorBeenActive (false)
{
   mMargins = kDefaultMargins;
   mYAxisSetup.mAscending = false;
}

PPlot::~PPlot () {
  if (mOwnsPPlotDrawer) {
    delete mPPlotDrawer;
  }
  mPPlotDrawer = 0;
}

bool PPlot::Draw (Painter &inPainter) {
  PRect theRect;
  theRect.mX = mMargins.mLeft;
  theRect.mY = mMargins.mTop;
  theRect.mW = inPainter.GetWidth () - mMargins.mLeft - mMargins.mRight;
  theRect.mH = inPainter.GetHeight () - mMargins.mTop - mMargins.mBottom;

  if (mPPlotDrawer) {
    mPPlotDrawer->Prepare (inPainter, *this);
    return mPPlotDrawer->Draw (inPainter);
  }
  if (!mPlotDataContainer.GetPlotCount ()) {
    return true;
  }

  if (!ConfigureSelf ()) {
    return false;
  }
  bool theShouldRepeat = true;
  long theRepeatCount = 0;

  while (theShouldRepeat && theRepeatCount<2) {
    theRepeatCount++;

    if (!ValidateData ()) {
      return false;
    }

    if (!CalculateAxisRanges ()) {
      return false;
    }

    if (!this->CheckRange (mXAxisSetup)) {
      return false;
    }

    if (!this->CheckRange (mYAxisSetup)) {
      return false;
    }

    if (!CalculateTickInfo (theRect, inPainter)) {
      return false;
    }

    if (!CalculateXTransformation (theRect)) {
      return false;
    }

    if (!CalculateYTransformation (theRect)) {
      return false;
    }
    if (theRepeatCount>1) {
      break;
    }
    // hooks for some final calculations
    bool theShouldRepeat = false;
    for (PCalculator::tList::iterator theModifyingC=mModifyingCalculatorList.begin ();theModifyingC!=mModifyingCalculatorList.end();theModifyingC++) {
      PCalculator *theModifyingCalculator = *theModifyingC;
      if (theModifyingCalculator->ShouldCalculate ()) {
        theShouldRepeat = true;
        theModifyingCalculator->Calculate (inPainter, *this);
        mHasAnyModifyingCalculatorBeenActive = true;
      }
    }
//    theShouldRepeat = mModifyingCalculatorList.size ()>0;
  }

  // hooks for some final calculations
  for (PCalculator::tList::iterator thePostC=mPostCalculatorList.begin ();thePostC!=mPostCalculatorList.end();thePostC++) {
    PCalculator *thePostCalculator = *thePostC;
    thePostCalculator->Calculate (inPainter, *this);
  }

  for (PDrawer::tList::iterator thePre1=mPreDrawerList.begin();thePre1!=mPreDrawerList.end ();thePre1++) {
    PDrawer *thePreDrawer = *thePre1;
    thePreDrawer->Prepare (inPainter, *this);
  }

  // Drawing !

  inPainter.SetLineColor (0,0,0);
  inPainter.SetClipRect (0, 0, inPainter.GetWidth (), inPainter.GetHeight ());

  // draw entire background, including the margins (for scrolling...)
  PRect fullRect;
  fullRect.mX = 0;
  fullRect.mY = 0;
  fullRect.mW = inPainter.GetWidth ();
  fullRect.mH = inPainter.GetHeight ();
  if (!DrawPlotBackground (fullRect, inPainter)) {
    return false;
  }

  for (PDrawer::tList::iterator thePre=mPreDrawerList.begin ();thePre!=mPreDrawerList.end();thePre++) {
    PDrawer *thePreDrawer = *thePre;
    thePreDrawer->Draw (inPainter);
  } 


  if (!DrawGridXAxis (theRect, inPainter)) {
    return false;
  }
  
  if (!DrawGridYAxis (theRect, inPainter)) {
    return false;
  }
  
  if (!DrawXAxis (theRect, inPainter)) {
    return false;
  }

  if (!DrawYAxis (theRect, inPainter)) {
    return false;
  }

  if (!DrawLegend (theRect, inPainter)) {
    return false;
  }

  // clip the plotregion while drawing plots
  inPainter.SetClipRect (theRect.mX, theRect.mY, theRect.mW, theRect.mH);


  for (int theI=0;theI<mPlotDataContainer.GetPlotCount ();theI++) {
    if (!DrawPlot (theI, theRect, inPainter)) {
      return false;
    }
  }

  for (PDrawer::tList::iterator thePost=mPostDrawerList.begin ();thePost!=mPostDrawerList.end();thePost++) {
    PDrawer *thePostDrawer = *thePost;
    thePostDrawer->Draw (inPainter);
  } 

  return true;
}

void PPlot::SetPPlotDrawer (PDrawer *inPDrawer) {
  if (mOwnsPPlotDrawer) {
    delete mPPlotDrawer;// delete (if any)
  }
  mOwnsPPlotDrawer = true;
  mPPlotDrawer = inPDrawer;  
}

void PPlot::SetPPlotDrawer (PDrawer &inPDrawer) {
  mOwnsPPlotDrawer = false;
  mPPlotDrawer = &inPDrawer;
}

bool PPlot::DrawPlotBackground (const PRect &inRect, Painter &inPainter) const {
  inPainter.SetStyle (mPlotBackground.mStyle);

  if (!mPlotBackground.mTransparent) {
    PColor theC = mPlotBackground.mPlotRegionBackColor;
    inPainter.SetFillColor (theC.mR, theC.mG, theC.mB);
    inPainter.FillRect (inRect.mX, inRect.mY, inRect.mW, inRect.mH);
  }
  string theTitle = mPlotBackground.mTitle;
  if (theTitle.size ()>0) {
    int theW = inPainter.CalculateTextDrawSize (theTitle.c_str ());
    int theX = Round (inRect.mX + (inRect.mW-theW)*0.5);
    int theY = inRect.mY - inPainter.GetFontHeight ()+mMargins.mTop;
    inPainter.DrawText (theX, theY, theTitle.c_str ());
  }
  return true;
}

bool PPlot::DrawGridXAxis (const PRect &inRect, Painter &inPainter) const {
  inPainter.SetStyle (mXAxisSetup.mStyle);

  // ticks
  inPainter.SetStyle (mXAxisSetup.mTickInfo.mStyle);
  if (!mXTickIterator->Init ()) {
    return false;
  }

  float theX;
  bool theIsMajorTick;
  string theFormatString;

  inPainter.SetFillColor (200,200,200);
  inPainter.SetLineColor (200,200,200);

  // draw gridlines
  if (mGridInfo.mXGridOn) {
      while (mXTickIterator->GetNextTick (theX, theIsMajorTick, theFormatString)) {

          if (theIsMajorTick && mGridInfo.mXGridOn) {
              float theScreenX = mXTrafo->Transform(theX);
              inPainter.DrawLine (theScreenX, inRect.mY, theScreenX, inRect.mY + inRect.mH);
          }
      }
  }

  inPainter.SetFillColor (0,0,0);
  inPainter.SetLineColor (0,0,0);
  return true;
}

bool PPlot::DrawGridYAxis (const PRect &inRect, Painter &inPainter) const {
    inPainter.SetStyle (mYAxisSetup.mStyle);
    
    // ticks
    inPainter.SetStyle (mYAxisSetup.mTickInfo.mStyle);
    if (!mYTickIterator->Init ()) {
        return false;
    }
    
    float theY;
    bool theIsMajorTick;
    string theFormatString;
    PRect theTickRect;

    inPainter.SetFillColor (200,200,200);
    inPainter.SetLineColor (200,200,200);
    
    // draw gridlines
    if (mYAxisSetup.mTickInfo.mTicksOn) {
        while (mYTickIterator->GetNextTick (theY, theIsMajorTick, theFormatString)) {

            if (theIsMajorTick && mGridInfo.mYGridOn) {
                float theScreenY = mYTrafo->Transform(theY);
                inPainter.DrawLine (inRect.mX, theScreenY, inRect.mX + inRect.mW, theScreenY);
            }
        }
    }

    inPainter.SetFillColor (0,0,0);
    inPainter.SetLineColor (0,0,0);
    return true;
}

bool PPlot::DrawXAxis (const PRect &inRect, Painter &inPainter) const {
  inPainter.SetStyle (mXAxisSetup.mStyle);

  float theX1 = inRect.mX;
  float theY1;
  float theTargetY = 0;
  if (!mXAxisSetup.mCrossOrigin) {
    if (mYAxisSetup.mAscending) {
      theTargetY = mYAxisSetup.mMax;
    } else {
      theTargetY = mYAxisSetup.mMin;
    }
  }
  theY1 = mYTrafo->Transform (theTargetY);

  // x-axis
  float theX2 = theX1+inRect.mW;
  float theY2 = theY1;
  inPainter.DrawLine (theX1, theY1, theX2, theY2);

  // ticks
  inPainter.SetStyle (mXAxisSetup.mTickInfo.mStyle);
  if (!mXTickIterator->Init ()) {
    return false;
  }

  float theX;
  bool theIsMajorTick;
  string theFormatString;

  int theYMax = 0;
  PRect theTickRect;
  PRect theRect = inRect;

  if (mXAxisSetup.mTickInfo.mTicksOn) {
      while (mXTickIterator->GetNextTick (theX, theIsMajorTick, theFormatString)) {
          if (!DrawXTick (theX, theY1, theIsMajorTick, theFormatString, inPainter, theTickRect)) {
              return false;
          }
          
          if (theTickRect.mY+theTickRect.mH>theYMax) {
              theYMax = theTickRect.mY+theTickRect.mH;
          }
      }
  }

  if (theYMax>theRect.mY+theRect.mH) {
    theRect.mH = theYMax-theRect.mY;
  }

  inPainter.SetStyle (mXAxisSetup.mStyle);
  string theLabel = mXAxisSetup.mLabel;
  if (theLabel.size ()>0) {
    int theW = inPainter.CalculateTextDrawSize (theLabel.c_str ());
    int theX = theRect.mX + (theRect.mW-theW)/2;
    int theY = theRect.mY+theRect.mH+inPainter.GetFontHeight ();
    inPainter.DrawText (theX, theY, theLabel.c_str ());
  }
  return true;
}

bool PPlot::DrawXTick (float inX, int inScreenY, bool inMajor, const string &inFormatString, Painter &inPainter, PRect &outRect) const{
  char theBuf[128];
  int theTickSize;
  float theScreenX = mXTrafo->Transform(inX);
  outRect.mX = theScreenX;
  outRect.mY = inScreenY;
  outRect.mW = 0;
  if (inMajor) {
    theTickSize = mXAxisSetup.mTickInfo.mMajorTickScreenSize;
    sprintf (theBuf, inFormatString.c_str (), inX);

    outRect.mH = inPainter.GetFontHeight ()+theTickSize + mXAxisSetup.mTickInfo.mMinorTickScreenSize;;
    inPainter.DrawText (theScreenX, inScreenY+outRect.mH, theBuf);
  }
  else {
    theTickSize = mXAxisSetup.mTickInfo.mMinorTickScreenSize;
    outRect.mH = theTickSize;
  }

  inPainter.DrawLine (theScreenX, inScreenY,theScreenX, inScreenY+theTickSize);  
  return true;
}

bool PPlot::DrawYAxis (const PRect &inRect, Painter &inPainter) const {
  inPainter.SetStyle (mYAxisSetup.mStyle);
  float theX1;
  PRect theRect = inRect;
  float theTargetX = 0;
  if (!mYAxisSetup.mCrossOrigin) {
    if (mXAxisSetup.mAscending) {
      theTargetX = mXAxisSetup.mMin;
    }
    else {
      theTargetX = mXAxisSetup.mMax;
    }
  }
  theX1 = mXTrafo->Transform (theTargetX);

  int theY1 = inRect.mY;
  float theX2 = theX1;
  int theY2 = theY1+theRect.mH;

  // draw y axis
  inPainter.DrawLine (theX1, theY1, theX2, theY2);

  // ticks
  inPainter.SetStyle (mYAxisSetup.mTickInfo.mStyle);
  if (!mYTickIterator->Init ()) {
    return false;
  }

  float theY;
  bool theIsMajorTick;
  string theFormatString;
  PRect theTickRect;

  if (mYAxisSetup.mTickInfo.mTicksOn) {
      while (mYTickIterator->GetNextTick (theY, theIsMajorTick, theFormatString)) {
          if (!DrawYTick (theY, theX1, theIsMajorTick, theFormatString, inPainter, theTickRect)) {
              return false;
          }

          if (theTickRect.mX < theRect.mX) {
              theRect.mX = theTickRect.mX;
          }
      }
  }
  
  // draw label
  inPainter.SetStyle (mYAxisSetup.mStyle);
  string theLabel = mYAxisSetup.mLabel;
  if (theLabel.size ()>0) {
    int theW = inPainter.CalculateTextDrawSize (theLabel.c_str ());
    int theX = theRect.mX;
    int theY = theRect.mY + theRect.mH - (theRect.mH-theW)/2;
    inPainter.DrawRotatedText (theX, theY, -90, theLabel.c_str ());
  }  

  return true;
}

bool PPlot::DrawYTick (float inY, int inScreenX, bool inMajor, const string &inFormatString, Painter &inPainter, PRect &outRect) const {
  char theBuf[128];
  int theTickSize;
  float theScreenY = mYTrafo->Transform(inY);
  outRect.mX = inScreenX;
  outRect.mY = theScreenY;
  outRect.mW = 0;// not used
  outRect.mH = 0;// not used
  if (inMajor) {
    theTickSize = mYAxisSetup.mTickInfo.mMajorTickScreenSize;
    sprintf (theBuf, inFormatString.c_str (), inY);
    int theStringWidth = inPainter.CalculateTextDrawSize (theBuf);
    outRect.mX -= (theStringWidth+theTickSize+mYAxisSetup.mTickInfo.mMinorTickScreenSize);
    int theHalfFontHeight = inPainter.GetFontHeight ()/2;// for sort of vertical centralizing
    inPainter.DrawText (outRect.mX, theScreenY+theHalfFontHeight, theBuf);

  }
  else {
    theTickSize = mYAxisSetup.mTickInfo.mMinorTickScreenSize;
    outRect.mX -= theTickSize;
  }

  inPainter.DrawLine (inScreenX, theScreenY, inScreenX-theTickSize, theScreenY);  
  return true;
}

bool PPlot::DrawLegend (const PRect &inRect, Painter &inPainter) const {
    const int kXoffsetLegend(20);
    
    for (int theI=0; theI<mPlotDataContainer.GetPlotCount (); theI++) {
        PColor theC;
        string theText;
        const LegendData *theLegendData = mPlotDataContainer.GetConstLegendData (theI);
        if (theLegendData) {
            inPainter.SetStyle (theLegendData->mStyle);
            theC = theLegendData->mColor;
            if (theLegendData->mShow) {
              theText = theLegendData->mName;
            }
        }
        inPainter.SetLineColor (theC.mR, theC.mG, theC.mB);

        // cut legend if it doesn't fit in plot
        int theSize (0);
        if (inPainter.CalculateTextDrawSize (theText.c_str ()) >= inRect.mW - kXoffsetLegend) {
            theText.insert(0, "...");
            while (inPainter.CalculateTextDrawSize (theText.c_str ()) >= inRect.mW - kXoffsetLegend) {
                theSize = theText.size ();
                // display dots and at least 3 characters
                if (theSize >= 9) {
                    theText.erase (3, 3);
                }
                else if (theSize >= 7) {
                    theText.erase (3, theSize - 6);
                } else {
                    // keep dots only
                    theText.erase (3, theSize - 3);
                    break;
                }
            }
        }
        
        int theHeight = inPainter.GetFontHeight ();
        int theX = inRect.mX + kXoffsetLegend;
        int theY = inRect.mY + theI*(theHeight*2)+theHeight;
        inPainter.DrawText (theX, theY, theText.c_str ());
    }
    return true;
}

float GetMaxFromRange (const PlotDataBase &inData, long inStartIndex, long inEndIndex) {
    float max = 0;
    float fabsMax = 0;
    for (long theI = inStartIndex; theI <= inEndIndex; theI++) {
        if (theI == inStartIndex) {
            max = inData.GetValue (theI);
            fabsMax = fabs (max);
        }
        else {
            float data = inData.GetValue (theI);
            if (fabs (data) > fabsMax) {
                max = data;
                fabsMax = fabs (data);
            }
        }
    }

    return max;
}

void FindRange (const PlotDataBase &inData, float inMin, float inMax, long& outStartIndex, long& outEndIndex) {
    outStartIndex = 0;
    while (outStartIndex < inData.GetSize () && inData.GetValue (outStartIndex) <= inMin) {
        outStartIndex++;
    }

    if (outStartIndex == inData.GetSize ()) {
        outStartIndex = inData.GetSize () - 1;
        outEndIndex = outStartIndex;
        assert (outStartIndex>-1);
        return;
    }

    // We want the value at outStartIndex smaller than or equal to inMin
    if (outStartIndex > 0) {
        outStartIndex--;
    }

    outEndIndex = outStartIndex;
    while (outEndIndex < inData.GetSize () && inData.GetValue (outEndIndex) < inMax) {
        outEndIndex++;
    }

    if (outEndIndex == inData.GetSize ()) {
        outEndIndex--;
    }
    assert (outStartIndex>-1);
}

bool LineDataDrawer::DrawData (const PlotDataBase &inXData, const PlotDataBase &inYData, const PlotDataSelection &inPlotDataSelection, const AxisSetup &inXAxisSetup, const PRect &inRect, Painter &inPainter) const {
  if (!mXTrafo || !mYTrafo) {
    return false;
  }
  if ((inXData.GetSize () == 0) || (inYData.GetSize () == 0)) {
      return false;
  }
  long theXSize = inXData.GetSize ();
  long theYSize = inYData.GetSize ();
  if (theXSize>theYSize) {
    return false;
  }
  inPainter.SetStyle (mStyle);
  float thePrevX = 0;
  float thePrevY = 0;
  bool theFirst = true;
  float theTraX, theTraY;

  long theStart = 0;
  long theEnd = inXData.GetSize () - 1;
  int theStride = 1;
  if (mDrawFast) {
      FindRange (inXData, inXAxisSetup.mMin, inXAxisSetup.mMax, theStart, theEnd);

      theStride = (theEnd - theStart + 1) / inPainter.GetWidth ();
      if (theStride == 0) {
          theStride = 1;
      }
  }


  for (int theI = theStart; theI <= theEnd; theI+=theStride) {

    theTraX = mXTrafo->Transform (inXData.GetValue (theI));
    if (theStride > 1) {
      long theLast = theI + theStride - 1;
      if (theLast>theEnd) {
        theLast = theEnd;
      }
        theTraY = mYTrafo->Transform (GetMaxFromRange (inYData, theI, theLast));
    }
    else {
        theTraY = mYTrafo->Transform (inYData.GetValue (theI));
    }

    if (!theFirst && mDrawLine) {
      inPainter.DrawLine (thePrevX, thePrevY, theTraX, theTraY);
    }
    else {
      theFirst = false;
    }
    bool theDrawPoint = mDrawPoint;

    if (theDrawPoint && !DrawPoint (theTraX, theTraY, inRect, inPainter)) {
      return false;
    }
    if (inPlotDataSelection.IsSelected (theI) && !DrawSelection (theTraX, theTraY, inRect, inPainter)) {
      return false;
    }
    thePrevX = theTraX;
    thePrevY = theTraY;
  }
  return true;
}

DataDrawerBase* LineDataDrawer::Clone () const {
    return new LineDataDrawer (*this);
}

bool LineDataDrawer::DrawPoint (int inScreenX, int inScreenY, const PRect &inRect, Painter &inPainter) const {
  inPainter.DrawLine (inScreenX-5, inScreenY+5, inScreenX+5, inScreenY-5);
  inPainter.DrawLine (inScreenX-5, inScreenY-5, inScreenX+5, inScreenY+5);
  return true;
}

bool LineDataDrawer::DrawSelection (int inScreenX, int inScreenY, const PRect &inRect, Painter &inPainter) const {
//  inPainter.DrawLine (inScreenX-5, inScreenY+5, inScreenX+5, inScreenY-5);
//  inPainter.DrawLine (inScreenX-5, inScreenY-5, inScreenX+5, inScreenY+5);
  inPainter.FillRect (inScreenX-5, inScreenY-5, 10, 10);
  return true;
}

bool DotDataDrawer::DrawPoint (int inScreenX, int inScreenY, const PRect &inRect, Painter &inPainter) const {
  inPainter.DrawLine (inScreenX, inScreenY, inScreenX + 1, inScreenY);
  return true;
}

bool BarDataDrawer::DrawData (const PlotDataBase &inXData, const PlotDataBase &inYData, const PlotDataSelection &inPlotDataSelection, const AxisSetup &inXAxisSetup, const PRect &inRect, Painter &inPainter) const {
  if (!mXTrafo || !mYTrafo) {
    return false;
  }
  if (inXData.GetSize ()>inYData.GetSize ()) {
    return false;
  }
  if (!mPlotCount) {
      return false;
  }
  if (mDrawOnlyLastPoint) {
    return DrawOnlyLastPoint (inXData, inYData, inPlotDataSelection, inXAxisSetup, inRect, inPainter);
  }

  int theTraX, theTraY;
  int theTraY0 = mYTrafo->Transform (0);

  int theLeft, theTop, theWidth, theHeight;

  theWidth = inRect.mW/inXData.GetSize ();
  
  for (long theI=0;theI<inXData.GetSize ();theI++) {
    theTraX = mXTrafo->Transform (inXData.GetValue (theI));
    theTraY = mYTrafo->Transform (inYData.GetValue (theI));

    theLeft = theTraX-theWidth/2;
    theTop = theTraY;
    theHeight = theTraY0-theTop;

    inPainter.FillRect (theLeft, theTop, theWidth, theHeight);
  }
  return true;
}


bool BarDataDrawer::DrawOnlyLastPoint (const PlotDataBase &inXData, const PlotDataBase &inYData, const PlotDataSelection &inPlotDataSelection, const AxisSetup &inXAxisSetup, const PRect &inRect, Painter &inPainter) const {

  PRect theRect = inRect;
  theRect.mW = inRect.mW / mPlotCount;
  theRect.mX = inRect.mX + mPlotIndex * theRect.mW;

  int theTraX, theTraY;
  int theTraY0 = mYTrafo->Transform (0);

  int theLeft, theTop, theWidth, theHeight;

  theWidth = theRect.mW;
  
  // only draw last point:
  long theI = inXData.GetSize () - 1;
  if (theI >= 0) {
    theTraX = mXTrafo->Transform (inXData.GetValue (theI));
    theTraY = mYTrafo->Transform (inYData.GetValue (theI));

    theLeft = theRect.mX;
    theTop = theTraY;
    theHeight = theTraY0-theTop;

    inPainter.FillRect (theLeft, theTop, theWidth, theHeight);
  }
  return true;
}


DataDrawerBase* BarDataDrawer::Clone () const {
    return new BarDataDrawer (*this);
}

bool PPlot::DrawPlot (int inIndex, const PRect &inRect, Painter &inPainter) const {

  if (inIndex>=mPlotDataContainer.GetPlotCount ()) {
    return false;
  }

  const PlotDataBase *theXData = mPlotDataContainer.GetConstXData (inIndex);
  const PlotDataBase *theYData = mPlotDataContainer.GetConstYData (inIndex);
  if (!theXData || !theYData) {
    return false;
  }
  const LegendData *theLegendData = mPlotDataContainer.GetConstLegendData (inIndex);
  PColor theC;
  if (theLegendData) {
    theC = theLegendData->mColor;
  }
  inPainter.SetLineColor (theC.mR, theC.mG, theC.mB);
  inPainter.SetFillColor (theC.mR, theC.mG, theC.mB);

  const DataDrawerBase *theD = mPlotDataContainer.GetConstDataDrawer (inIndex);
  if (!theD) {
    return false;
  }
  const PlotDataSelection *thePlotDataSelection = mPlotDataContainer.GetConstPlotDataSelection (inIndex);
  if (!thePlotDataSelection) {
    return false;
  }

  return theD->DrawData (*theXData, *theYData, *thePlotDataSelection, mXAxisSetup, inRect, inPainter);
 }

bool PPlot::ConfigureSelf () {
  long thePlotCount = mPlotDataContainer.GetPlotCount ();
  if (thePlotCount == 0) {
    return false;
  }
  if (mXAxisSetup.mLogScale) {
    mXTickIterator = &mXLogTickIterator;
    mXTrafo = &mXLogTrafo;
    mYAxisSetup.mCrossOrigin = false;
  }
  else {
    const PlotDataBase *theGlue = mPlotDataContainer.GetConstXData (0);
    const StringData *theStringXData = dynamic_cast<const StringData *>(theGlue);
    if (theStringXData != 0) {
      mXTickIterator = &mXNamedTickIterator;
      mXNamedTickIterator.SetStringList (*(theStringXData->GetStringData ()));
    }
    else {
      mXTickIterator = &mXLinTickIterator;
    }
    mXTrafo = &mXLinTrafo;
  }
  if (mYAxisSetup.mLogScale) {
    mYTickIterator = &mYLogTickIterator;
    mYTrafo = &mYLogTrafo;
    mXAxisSetup.mCrossOrigin = false;
  }
  else {
    mYTickIterator = &mYLinTickIterator;
    mYTrafo = &mYLinTrafo;
  }
  mXTickIterator->SetAxisSetup (&mXAxisSetup);
  mYTickIterator->SetAxisSetup (&mYAxisSetup);

  // set trafo's for data drawers
  for (int theI=0; theI<mPlotDataContainer.GetPlotCount ();theI++) {
    DataDrawerBase *theD =  (mPlotDataContainer.GetDataDrawer (theI));
    if (theD) {
      theD->SetXTrafo (mXTrafo);
      theD->SetYTrafo (mYTrafo);
      theD->SetPlotCount (mPlotDataContainer.GetPlotCount ());
      theD->SetPlotIndex (theI);
    }
  }

  return true;
}

bool PPlot::ValidateData () {
    return true;

  // check x data ascending
  for (int theI=0; theI<mPlotDataContainer.GetPlotCount ();theI++) {
    const RealPlotData *theX = dynamic_cast <const RealPlotData *> (mPlotDataContainer.GetConstXData (theI));
    if (theX && theX->size ()>0) {
      float thePrev = (*theX)[0];
      for (RealPlotData::const_iterator theJ=theX->begin ();theJ!=theX->end ();theJ++) {
        float theNext = *theJ;
        if (theNext<thePrev) {
          return false;
        }
      }
    }
  }
  return true;
}

bool PPlot::CalculateAxisRanges () {

  float theXMin;
  float theXMax;

  mPlotDataContainer.CalculateXRange (theXMin, theXMax);
  if (mXAxisSetup.mAutoScaleMin || mXAxisSetup.mAutoScaleMax) {

    if (mXAxisSetup.mAutoScaleMin) {
      mXAxisSetup.mMin = theXMin;
      if (mXAxisSetup.mLogScale && (theXMin < kLogMinClipValue) ) {
          mXAxisSetup.mMin = kLogMinClipValue;
      }
    }

    if (mXAxisSetup.mAutoScaleMax) {
      mXAxisSetup.mMax = theXMax;
    }

    if (!mXTickIterator->AdjustRange (mXAxisSetup.mMin, mXAxisSetup.mMax)) {
      return false;
    }
  }

  if (mYAxisSetup.mAutoScaleMin || mYAxisSetup.mAutoScaleMax) {
    float theYMin;
    float theYMax;

    mPlotDataContainer.CalculateYRange (mXAxisSetup.mMin, mXAxisSetup.mMax,
					theYMin, theYMax);

    if (mYAxisSetup.mAutoScaleMin) {
      mYAxisSetup.mMin = theYMin;
      if (mYAxisSetup.mLogScale && (theYMin < kLogMinClipValue) ) {
          mYAxisSetup.mMin = kLogMinClipValue;
      }
    }
    if (mYAxisSetup.mAutoScaleMax) {
      mYAxisSetup.mMax = theYMax;
    }

    if (!mYTickIterator->AdjustRange (mYAxisSetup.mMin, mYAxisSetup.mMax)) {
      return false;
    }
  }
  
  return true;
}

bool PPlot::CheckRange (const AxisSetup &inAxisSetup) const {
  if (inAxisSetup.mLogScale) {
    if (inAxisSetup.mMin < kLogMinClipValue) {
      return false;
    }

  }
  return true;
}

bool PPlot::CalculateTickInfo (const PRect &inRect, Painter &inPainter) {
  float theXRange = mXAxisSetup.mMax - mXAxisSetup.mMin;
  float theYRange = mYAxisSetup.mMax - mYAxisSetup.mMin;

  if (theXRange <= 0 || theYRange < 0) {
    return false;
  }

  if ((mYAxisSetup.mMax != 0 && fabs (theYRange / mYAxisSetup.mMax) < kRangeVerySmall) ||
    theYRange == 0) {
    float delta = 0.1f;
    if (mYAxisSetup.mMax != 0) {
        delta *= fabs(mYAxisSetup.mMax);
    }
    
    mYAxisSetup.mMax += delta;
    mYAxisSetup.mMin -= delta;
    theYRange = mYAxisSetup.mMax - mYAxisSetup.mMin;
  }

  if (mXAxisSetup.mTickInfo.mAutoTick) {
    int theTextWidth = inPainter.CalculateTextDrawSize ("12345");
    float theDivGuess = inRect.mW/(kMajorTickXInitialFac*theTextWidth);
    if (!mXTickIterator->InitFromRanges (theXRange, inRect.mH, theDivGuess, mXAxisSetup.mTickInfo)) {
      return false;
    }
  }
  if (mYAxisSetup.mTickInfo.mAutoTick) {
    float theTextHeight = inPainter.GetFontHeight ();
    float theDivGuess = inRect.mH/(kMajorTickYInitialFac*theTextHeight);
    if (!mYTickIterator->InitFromRanges (theYRange, inRect.mW, theDivGuess, mYAxisSetup.mTickInfo)) {
      return false;
    }
  }

  SetTickSizes (inPainter.GetFontHeight (), mXAxisSetup.mTickInfo);
  SetTickSizes (inPainter.GetFontHeight (), mYAxisSetup.mTickInfo);

  return true;
}

void PPlot::SetTickSizes (int inFontHeight, TickInfo &ioTickInfo) {
  if (ioTickInfo.mAutoTickSize) {
    float theFac = kRelMinorTickSize/kRelMajorTickSize;
    float theMax = Round (inFontHeight*kMaxMajorTickSizeInFontHeight);
    if (ioTickInfo.mMajorTickScreenSize>theMax) {
      ioTickInfo.mMajorTickScreenSize = theMax;
    }
    ioTickInfo.mMinorTickScreenSize = Round (ioTickInfo.mMajorTickScreenSize*theFac);
  }
}


bool PPlot::CalculateLogTransformation (int inBegin, int inEnd, const AxisSetup& inAxisSetup, LogTrafo& outTrafo) {

    float theBase = inAxisSetup.mLogBase;
    long theLogFac = 1;//inAxisSetup.mLogFactor;
    float theDataRange = SafeLog (inAxisSetup.mMax, theBase, theLogFac) - SafeLog(inAxisSetup.mMin, theBase, theLogFac);
    if (theDataRange < kFloatSmall) {
      return false;
    }
    float theTargetRange = inEnd - inBegin;
    float theScale = theTargetRange / theDataRange;

    if (inAxisSetup.mAscending ) {
        outTrafo.mOffset = inBegin - SafeLog(inAxisSetup.mMin,theBase, theLogFac) * theScale;
    } else {
        outTrafo.mOffset = inBegin + theTargetRange + SafeLog(inAxisSetup.mMin, theBase, theLogFac)* theScale;
    }
    outTrafo.mSlope = -theScale;
    outTrafo.mBase = theBase;
//    outTrafo.mFactor = inAxisSetup.mLogFactor;

    if (inAxisSetup.mAscending) {
        outTrafo.mSlope *= -1;
    }
    return true;
}

bool PPlot::CalculateLinTransformation (int inBegin, int inEnd, const AxisSetup& inAxisSetup, LinTrafo& outTrafo) {
    float theDataRange = inAxisSetup.mMax - inAxisSetup.mMin;
    if (theDataRange < kFloatSmall) {
      return false;
    }
    float theTargetRange = inEnd - inBegin;
    float theScale = theTargetRange / theDataRange;

    if (inAxisSetup.mAscending) {
        outTrafo.mOffset = inBegin - inAxisSetup.mMin * theScale;;
    } else {
        outTrafo.mOffset = inBegin + theTargetRange + inAxisSetup.mMin * theScale;;
    }
    outTrafo.mSlope = -theScale;

    if (inAxisSetup.mAscending) {
        outTrafo.mSlope *= -1;
    }
    return true;
}

bool PPlot::CalculateXTransformation (const PRect &inRect) {
  if (mXAxisSetup.mLogScale) {
    return CalculateLogTransformation (inRect.mX, inRect.mX + inRect.mW, mXAxisSetup, mXLogTrafo);
  }
  else {
    return CalculateLinTransformation (inRect.mX, inRect.mX + inRect.mW, mXAxisSetup, mXLinTrafo);
  }
  return true;
}

bool PPlot::CalculateYTransformation (const PRect &inRect) {
  if (mYAxisSetup.mLogScale) {
    return CalculateLogTransformation (inRect.mY, inRect.mY + inRect.mH, mYAxisSetup, mYLogTrafo);
  }
  else {
    return CalculateLinTransformation (inRect.mY, inRect.mY + inRect.mH, mYAxisSetup, mYLinTrafo);
  }
  return true;
}


bool MakeExamplePlot (int inExample, PPlot &ioPPlot) {
  switch (inExample) {
  case 1:
    MakeExamplePlot1 (ioPPlot);
    return true;
    break;
  case 2:
    MakeExamplePlot2 (ioPPlot);
    return true;
    break;
  case 3:
    MakeExamplePlot3 (ioPPlot);
    return true;
    break;
  case 4:
    MakeExamplePlot4 (ioPPlot);
    return true;
    break;
  case 5:
    MakeExamplePlot5 (ioPPlot);
    return true;
    break;
  case 6:
    MakeExamplePlot6 (ioPPlot);
    return true;
    break;
  case 7:
    MakeExamplePlot7 (ioPPlot);
    return true;
    break;
  case 8:
    MakeExamplePlot8 (ioPPlot);
    return true;
    break;
  }
  return false;
}

void MakeExamplePlot1 (PPlot &ioPPlot) {

  int theI;
  PlotData *theX1 = new PlotData ();
  PlotData *theY1 = new PlotData ();
  float theFac = (float)1.0/(100*100*100);
  for (theI=-100;theI<=100;theI++) {
    theX1->push_back (theI+50);
    theY1->push_back (theFac*theI*theI*theI);
  }
  LineDataDrawer *theDataDrawer1 = new LineDataDrawer ();
  theDataDrawer1->mStyle.mPenWidth = 3;
  ioPPlot.mPlotDataContainer.AddXYPlot (theX1, theY1, 0, theDataDrawer1);
  ioPPlot.mPlotBackground.mTitle = "Bar";
  ioPPlot.mPlotBackground.mStyle.mFontSize = 20;
  ioPPlot.mMargins.mTop = 60;
  ioPPlot.mXAxisSetup.mLabel = "gnu (Foo)";
  ioPPlot.mYAxisSetup.mLabel = "Space (m^3)";
  ioPPlot.mYAxisSetup.mStyle.mFontSize = 9;
  ioPPlot.mYAxisSetup.mTickInfo.mStyle.mFontSize = 5;
  ioPPlot.mYAxisSetup.mStyle.mPenWidth = 2;
  ioPPlot.mXAxisSetup.mStyle = ioPPlot.mYAxisSetup.mStyle;
  ioPPlot.mXAxisSetup.mTickInfo.mStyle = ioPPlot.mYAxisSetup.mTickInfo.mStyle;

  PlotData *theX2 = new PlotData ();
  PlotData *theY2 = new PlotData ();
  theFac = (float)2.0/100;
  for (theI=-100;theI<=100;theI++) {
    theX2->push_back (theI);
    theY2->push_back (-theFac*theI);
  }
  ioPPlot.mPlotDataContainer.AddXYPlot (theX2, theY2);
  LegendData *theLegendData2 = ioPPlot.mPlotDataContainer.GetLegendData (1);
  theLegendData2->mStyle.mFontSize = 9;

  PlotData *theX3 = new PlotData ();
  PlotData *theY3 = new PlotData ();
  for (theI=-100;theI<=100;theI++) {
    theY3->push_back (0.01*theI);
    theX3->push_back (0.01*theI*theI-30);
  }
  ioPPlot.mPlotDataContainer.AddXYPlot (theX3, theY3);
}
 
void MakeExamplePlot2 (PPlot &ioPPlot) {

  int theI;
  PlotData *theX1 = new PlotData ();
  PlotData *theY1 = new PlotData ();
  float theFac = (float)1.0/(100*100*100);
  for (theI=0;theI<=100;theI++) {
    theX1->push_back (theI);
    theY1->push_back (theFac*theI*theI*theI);
  }
  ioPPlot.mPlotDataContainer.AddXYPlot (theX1, theY1, 0);

  PlotData *theX2 = new PlotData ();
  PlotData *theY2 = new PlotData ();
  theFac = (float)2.0/100;
  for (theI=0;theI<=100;theI++) {
    theX2->push_back (theI);
    theY2->push_back (-theFac*theI);
  }
  ioPPlot.mPlotDataContainer.AddXYPlot (theX2, theY2, 0);

  ioPPlot.mPlotBackground.mTitle = "no autoscale";
  ioPPlot.mPlotBackground.mStyle.mFontSize = 15;
  ioPPlot.mPlotBackground.mTransparent = false;
  ioPPlot.mPlotBackground.mPlotRegionBackColor = PColor (200,200,200);
  ioPPlot.mMargins.mTop = 60;
  ioPPlot.mMargins.mRight = 30;
  ioPPlot.mXAxisSetup.mLabel = "Tg (X)";
  ioPPlot.mXAxisSetup.SetAutoScale (false);
  ioPPlot.mXAxisSetup.mMin = 10;
  ioPPlot.mXAxisSetup.mMax = 60;
  ioPPlot.mYAxisSetup.SetAutoScale (false);
  ioPPlot.mYAxisSetup.mMin = -0.5;
  ioPPlot.mYAxisSetup.mMax = 0.5;

  ioPPlot.mXAxisSetup.mCrossOrigin = false;
  ioPPlot.mYAxisSetup.mCrossOrigin = false;
  ioPPlot.mXAxisSetup.mAscending = false;
  ioPPlot.mYAxisSetup.mAscending = true;
}

void MakeExamplePlot3 (PPlot &ioPPlot) {

  PlotData *theX1 = new PlotData ();
  PlotData *theY1 = new PlotData ();
  float theFac = 1.0f/(100*100*100);
  for (int theI=0;theI<=100;theI++) {
    //    theX1->push_back (theI*0.001);
    theX1->push_back (theI);
    theY1->push_back (theFac*theI*theI*theI);
  }
  ioPPlot.mPlotDataContainer.AddXYPlot (0, theY1, 0);

  ioPPlot.mPlotBackground.mTitle = "narrow margins";
  ioPPlot.mPlotBackground.mTransparent = false;
  ioPPlot.mPlotBackground.mPlotRegionBackColor = PColor (200,200,200);
  ioPPlot.mMargins.mTop = 20;
  ioPPlot.mMargins.mRight = 10;
  ioPPlot.mMargins.mLeft = 10;
  ioPPlot.mMargins.mBottom = 10;
}

void MakeExamplePlot4 (PPlot &ioPPlot) {

  PlotData *theX1 = new PlotData ();
  PlotData *theY1 = new PlotData ();
  float theFac = 100.0f/(100*100*100);
  for (int theI=0;theI<=100;theI++) {
    theX1->push_back (0.0001+theI*0.001);
    theY1->push_back (0.01+theFac*theI*theI*theI);
  }
  LegendData *theLegend = new LegendData ();
  theLegend->mName = "foo";
  theLegend->mColor = PColor (100,100,200);
  ioPPlot.mPlotDataContainer.AddXYPlot (theX1, theY1, theLegend);
  ioPPlot.mXAxisSetup.mLogScale = true;
  ioPPlot.mYAxisSetup.mLogScale = true;
  ioPPlot.mYAxisSetup.mLogBase = 2;
  ioPPlot.mMargins.mLeft = 50;
  ioPPlot.mMargins.mTop = 20;

  ioPPlot.mGridInfo.mXGridOn = true;
  ioPPlot.mGridInfo.mYGridOn = true;
}

void MakeExamplePlot5 (PPlot &ioPPlot) {

  const char * kLables[12] = {"jan","feb", "mar", "apr", "may", "jun", "jul", "aug", "sep", "oct", "nov", "dec"};
  const float kData[12] = {1,2,3,4,5,6,7,8,9,0,1,2};

  StringData *theX1 = new StringData ();
  PlotData *theY1 = new PlotData ();
  for (int theI=0;theI<12;theI++) {
    theX1->AddItem (kLables[theI]);
    theY1->push_back (kData[theI]);
  }
  LegendData *theLegend = new LegendData ();
  theLegend->mName = "bar";
  theLegend->mColor = PColor (100,100,200);
  BarDataDrawer *theDataDrawer = new BarDataDrawer ();
  ioPPlot.mPlotDataContainer.AddXYPlot (theX1, theY1, theLegend, theDataDrawer);
  ioPPlot.mMargins.mLeft = 50;
  ioPPlot.mMargins.mTop = 20;
}

void MakeExamplePlot6 (PPlot &ioPPlot)
 {

  int theI;

  ioPPlot.mPlotBackground.mTitle = "line styles";
  PlotData *theX1 = new PlotData ();
  PlotData *theY1 = new PlotData ();
  float theFac = 100.0f/(100*100*100);
  for (theI=0;theI<=10;theI++) {
    theX1->push_back (0.0001+theI*0.001);
    theY1->push_back (0.01+theFac*theI*theI);
  }
  LineDataDrawer *theDataDrawer1 = new LineDataDrawer ();
  theDataDrawer1->mDrawPoint = true;
  theDataDrawer1->mStyle.mPenWidth = 3;
  ioPPlot.mPlotDataContainer.AddXYPlot (theX1, theY1, 0, theDataDrawer1);

  PlotData *theX2 = new PlotData ();
  PlotData *theY2 = new PlotData ();
  for (theI=0;theI<=10;theI++) {
    theX2->push_back (0.0001+theI*0.001);
    theY2->push_back (0.2-theFac*theI*theI);
  }
  LineDataDrawer *theDataDrawer2 = new LineDataDrawer ();
  theDataDrawer2->mDrawPoint = true;
  theDataDrawer2->mDrawLine = false;
  ioPPlot.mPlotDataContainer.AddXYPlot (theX2, theY2, 0, theDataDrawer2);

  ioPPlot.mMargins.mLeft = 50;
  ioPPlot.mMargins.mTop = 20;
}

void MakeExamplePlot7 (PPlot &ioPPlot) {

  PlotData *theX1 = new PlotData ();
  PlotData *theY1 = new PlotData ();
  float theFac = 100.0f/(100*100*100);
  for (int theI=0;theI<=100;theI++) {
    theX1->push_back (0.0001+theI*0.001);
    theY1->push_back (0.01+theFac*theI*theI*theI);
  }
  LegendData *theLegend = new LegendData ();
  theLegend->mName = "foo";
  theLegend->mColor = PColor (100,100,200);
  ioPPlot.mPlotDataContainer.AddXYPlot (theX1, theY1, theLegend);
  ioPPlot.mYAxisSetup.mLogScale = true;
  ioPPlot.mYAxisSetup.mLogFactor = 20;
  ioPPlot.mMargins.mLeft = 50;
  ioPPlot.mMargins.mTop = 20;

  ioPPlot.mGridInfo.mXGridOn = true;
  ioPPlot.mGridInfo.mYGridOn = true;
}

void MakeExamplePlot8 (PPlot &ioPPlot) {

  ioPPlot.mPlotBackground.mTitle = "data selection and editing";
  PlotData *theX1 = new PlotData ();
  PlotData *theY1 = new PlotData ();
  float theFac = 100.0f/(100*100*100);
  for (int theI=0;theI<=10;theI++) {
    theX1->push_back (0.001*theI);
    theY1->push_back (0.01+theFac*theI*theI*theI);
  }
  LegendData *theLegend = new LegendData ();
  theLegend->mName = "foo";
  theLegend->mColor = PColor (100,100,200);

  PlotDataSelection *thePlotDataSelection = new PlotDataSelection ();
  ioPPlot.mPlotDataContainer.AddXYPlot (theX1, theY1, theLegend, 0, thePlotDataSelection);
  ioPPlot.mMargins.mLeft = 50;
  ioPPlot.mMargins.mTop = 50;

}
 
void MakePainterTester (PPlot &ioPPlot) {
  ioPPlot.SetPPlotDrawer (new PainterTester ());
}

void SetCurrentPPlot (PPlot *inPPlot) {
  sCurrentPPlot = inPPlot;
}
#include <assert.h>
PPlot & GetCurrentPPlot () {
  fprintf (stderr, "getplot\n");
  if (sCurrentPPlot) {
    return *sCurrentPPlot;
  }
  assert (0);
  fprintf (stderr, "aargh\n");

  return *sCurrentPPlot;// this should not happen
}



void MakeCopy (const PPlot &inPPlot, PPlot &outPPlot) {
  // copy settings
  outPPlot.mGridInfo = inPPlot.mGridInfo;
  outPPlot.mMargins = inPPlot.mMargins;
  outPPlot.mXAxisSetup = inPPlot.mXAxisSetup;
  outPPlot.mYAxisSetup = inPPlot.mYAxisSetup;
  outPPlot.mPlotBackground = inPPlot.mPlotBackground;

  // now the data
  for (int theI=0;theI<inPPlot.mPlotDataContainer.GetPlotCount ();theI++) {
    const PlotDataBase *theXData = inPPlot.mPlotDataContainer.GetConstXData (theI);
    const PlotDataBase *theYData = inPPlot.mPlotDataContainer.GetConstYData (theI);
    const LegendData *theLegendData = inPPlot.mPlotDataContainer.GetConstLegendData (theI);
    const DataDrawerBase* theDrawer = inPPlot.mPlotDataContainer.GetConstDataDrawer (theI);

    PlotDataBase *theNewXData = new PlotDataPointer (theXData);
    PlotDataBase *theNewYData = new PlotDataPointer (theYData);
    LegendData *theNewLegendData = new LegendData ();
    *theNewLegendData = *theLegendData;
    DataDrawerBase* theNewDrawer = theDrawer->Clone ();

    outPPlot.mPlotDataContainer.AddXYPlot (theNewXData, theNewYData, theNewLegendData, theNewDrawer);

  }

}
