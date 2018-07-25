/***************************************************************************
 *                                                                         *
 *   Copyright notice:                                                     *
 *                                                                         *
 *   This is free Pier ware. You may do whatever you want with this code.  *
 *   You may cont(r)act me by email: pierphil@xs4all.nl                    *
 *                                                                         *
 ***************************************************************************/

#ifndef __PPLOT_H__
#define __PPLOT_H__

#pragma warning (disable: 4786)

#include <vector>
using std::vector;
#include <string>
using std::string;
#include <map>
using std::map;

typedef vector<float> RealPlotData;

class PStyle {
public:
  PStyle (): mFontSize(10), mPenWidth (1){};
  int mFontSize;
  string mFont;
  int mPenWidth;
  string mPenStyle;
  map<string,string> mVar; 
};

class CalculatedDataBase {
public:
    virtual float GetValue (long inIndex) const = 0;
    virtual long GetSize () const = 0;
};

// data
class PlotDataBase {
 public:
  virtual ~PlotDataBase ();
  virtual const RealPlotData * GetRealPlotData () const = 0;
  virtual const CalculatedDataBase * GetCalculatedData () const {return 0;}
  long GetSize () const;
  float GetValue (long inIndex) const;

  virtual bool CalculateRange (float &outMin, float &outMax);
};
typedef vector<PlotDataBase *> PlotDataList;

class PlotDataPointer: public PlotDataBase {
public:
  PlotDataPointer (const PlotDataBase *inPlotData):mPlotData (inPlotData){};// does not own them

  virtual const RealPlotData * GetRealPlotData () const {return mPlotData->GetRealPlotData ();};
  virtual const CalculatedDataBase * GetCalculatedData () const {return mPlotData->GetCalculatedData ();}
private:
  const PlotDataBase *mPlotData;
};

// default data class
class PlotData: public RealPlotData, public PlotDataBase {
 public:
  virtual const RealPlotData * GetRealPlotData () const {return this;};
};

class CalculatedData: public CalculatedDataBase {
public:
    CalculatedData (float inMin, float inDelta, long inSize):
      mMin (inMin), mDelta (inDelta), mSize (inSize) {}
    virtual float GetValue (long inIndex) const { return mMin + inIndex * mDelta; }
    virtual long GetSize () const { return mSize; }

    float mMin;
    float mDelta;
    long mSize;
};

class CalculatedPlotData: public PlotDataBase {
public:
    CalculatedPlotData (CalculatedDataBase* inCalculatedData):
      mCalculatedData (inCalculatedData) {}
    ~CalculatedPlotData () {delete mCalculatedData;}

    virtual const RealPlotData * GetRealPlotData () const {return 0;}
    virtual const CalculatedDataBase * GetCalculatedData () const {return mCalculatedData;}

    CalculatedDataBase* mCalculatedData;
};

class DummyData: public PlotDataBase {
public:
  DummyData (long inSize=0);

  virtual const RealPlotData * GetRealPlotData () const {return &mRealPlotData;};
private:
  RealPlotData mRealPlotData;
};

class StringData: public PlotDataBase {
public:
  void AddItem (const char *inString);

  const vector<string> * GetStringData () const {return &mStringData;};

  virtual const RealPlotData * GetRealPlotData () const {return &mRealPlotData;};
private:
  RealPlotData mRealPlotData;
  vector<string> mStringData;
};


float SafeLog (float inFloat, float inBase, float inFac=1);
float SafeExp (float inFloat, float inBase, float inFac=1);

class PRect {
 public:
  PRect ():mX(0),mY(0),mW(0),mH(0){};
  long mX;
  long mY;
  long mW;
  long mH;
};

class PMargins {
 public:
  PMargins ():mLeft (0), mRight(0), mTop (0), mBottom (0){};
  PMargins (long inLeft, long inRight, long inTop, long inBottom):mLeft (inLeft), mRight (inRight), mTop(inTop), mBottom (inBottom) {};
  long mLeft;
  long mRight;
  long mTop;
  long mBottom;
};

class PColor {
 public:
  PColor (): mR(0), mG(0), mB(0){};
  PColor (int inR, int inG, int inB): mR(inR), mG(inG), mB(inB){};
  unsigned char mR;
  unsigned char mG;
  unsigned char mB;
};

class LegendData {
public:
  LegendData (): mShow (true) {};
  string mName;
  PColor mColor;
  bool mShow;

  void SetDefaultColor (int inPlotIndex);
  void SetDefaultValues (int inPlotIndex);
  static PColor GetDefaultColor (int inPlotIndex);
  PStyle mStyle;
};

typedef vector<LegendData *> LegendDataList;

class PlotDataSelection: public vector<int> {
public:
  PlotDataSelection (long inSize=0):vector<int>(inSize){};

  bool IsSelected (long inIndex) const;
  long GetSelectedCount () const;
};

typedef vector<PlotDataSelection *> PlotDataSelectionList;

class Painter {
 public:

  virtual void DrawLine (float inX1, float inY1, float inX2, float inY2)=0;
  virtual void FillRect (int inX, int inY, int inW, int inH)=0;
  virtual void InvertRect (int inX, int inY, int inW, int inH)=0;
  virtual void SetClipRect (int inX, int inY, int inW, int inH)=0;
  virtual long GetWidth () const=0;
  virtual long GetHeight () const=0;
  virtual void SetLineColor (int inR, int inG, int inB)=0;
  virtual void SetFillColor (int inR, int inG, int inB)=0;
  virtual long CalculateTextDrawSize (const char *inString)=0;
  virtual long GetFontHeight () const =0;
  virtual void DrawText (int inX, int inY, const char *inString)=0;
  virtual void DrawRotatedText (int inX, int inY, float inDegrees, const char *inString)=0;
  virtual void SetStyle (const PStyle &inStyle){};
};

class DummyPainter: public Painter {
 public:
  virtual void DrawLine (float inX1, float inY1, float inX2, float inY2){};
  virtual void FillRect (int inX, int inY, int inW, int inH){};
  virtual void InvertRect (int inX, int inY, int inW, int inH){};
  virtual void SetClipRect (int inX, int inY, int inW, int inH){};
  virtual long GetWidth () const {return 100;};
  virtual long GetHeight () const {return 100;};
  virtual void SetLineColor (int inR, int inG, int inB){};
  virtual void SetFillColor (int inR, int inG, int inB){};
  virtual long CalculateTextDrawSize (const char *inString){return strlen (inString);};
  virtual long GetFontHeight () const {return 10;};
  virtual void DrawText (int inX, int inY, const char *inString){};
  virtual void DrawRotatedText (int inX, int inY, float inDegrees, const char *inString){};
};

class Trafo;
class AxisSetup;

class DataDrawerBase {
 public:
  DataDrawerBase (): mXTrafo (0), mYTrafo (0), mDrawFast (false), mPlotCount (1), mPlotIndex (0) {};
  virtual ~DataDrawerBase (){};

  void SetXTrafo (Trafo *inTrafo) {mXTrafo = inTrafo;};
  void SetYTrafo (Trafo *inTrafo) {mYTrafo = inTrafo;};
  void SetDrawFast (bool inDrawFast) {mDrawFast = inDrawFast;}
  void SetPlotCount (int inPlotCount) {mPlotCount = inPlotCount;}
  void SetPlotIndex (int inPlotIndex) {mPlotIndex = inPlotIndex;}

  virtual bool DrawData (const PlotDataBase &inXData, const PlotDataBase &inYData, const PlotDataSelection &inPlotDataSelection, const AxisSetup &inXAxisSetup, const PRect &inRect, Painter &inPainter) const =0;

  virtual DataDrawerBase* Clone () const = 0;
 protected:
  Trafo *mXTrafo;
  Trafo *mYTrafo;
  bool  mDrawFast;
  int   mPlotCount;
  int   mPlotIndex;
};

typedef vector<DataDrawerBase *> DataDrawerList;

class LineDataDrawer: public DataDrawerBase {
 public:
  LineDataDrawer ():mDrawLine (true), mDrawPoint (false){};
  virtual bool DrawData (const PlotDataBase &inXData, const PlotDataBase &inYData, const PlotDataSelection &inPlotDataSelection, const AxisSetup &inXAxisSetup, const PRect &inRect, Painter &inPainter) const;

  virtual DataDrawerBase* Clone () const;
  virtual bool DrawPoint (int inScreenX, int inScreenY, const PRect &inRect, Painter &inPainter) const;
  virtual bool DrawSelection (int inScreenX, int inScreenY, const PRect &inRect, Painter &inPainter) const;

  bool mDrawLine;
  bool mDrawPoint;
  PStyle mStyle;
};

class DotDataDrawer: public LineDataDrawer {
 public:
     DotDataDrawer () { mDrawLine = false; mDrawPoint = true;};

  virtual bool DrawPoint (int inScreenX, int inScreenY, const PRect &inRect, Painter &inPainter) const;
};

class BarDataDrawer: public DataDrawerBase {
 public:
   BarDataDrawer (bool inDrawOnlyLastPoint = false):mDrawOnlyLastPoint (inDrawOnlyLastPoint){};
  virtual bool DrawData (const PlotDataBase &inXData, const PlotDataBase &inYData, const PlotDataSelection &inPlotDataSelection, const AxisSetup &inXAxisSetup, const PRect &inRect, Painter &inPainter) const;
  virtual DataDrawerBase* Clone () const;

 protected:
  bool mDrawOnlyLastPoint;// special mode
  virtual bool DrawOnlyLastPoint (const PlotDataBase &inXData, const PlotDataBase &inYData, const PlotDataSelection &inPlotDataSelection, const AxisSetup &inXAxisSetup, const PRect &inRect, Painter &inPainter) const;
};


class PlotDataContainer {
 public:
  PlotDataContainer ();
  ~PlotDataContainer ();

  void RemoveElement (int inIndex);
  void ClearData ();

  void AddXYPlot (PlotDataBase *inXData, PlotDataBase *inYData, LegendData *inLegendData=0, DataDrawerBase *inDataDrawer=0, PlotDataSelection *inPlotDataSelection=0);//takes ownership
  void SetXYPlot (int inIndex, PlotDataBase *inXData, PlotDataBase *inYData, LegendData *inLegendData=0, DataDrawerBase *inDataDrawer=0, PlotDataSelection *inPlotDataSelection=0);//takes ownership

  int GetPlotCount () const {return mYDataList.size ();};

  PlotDataBase * GetXData (int inIndex);
  PlotDataBase * GetYData (int inIndex);
  LegendData * GetLegendData (int inIndex);
  DataDrawerBase * GetDataDrawer (int inIndex);
  PlotDataSelection * GetPlotDataSelection (int inIndex);
  bool  SetDataDrawer (int inIndex, DataDrawerBase* inDataDrawer);  // takes ownership

  int GetPlotIndexByName (const string &inName) const;// negative value: not found

  const PlotDataBase * GetConstXData (int inIndex) const;
  const PlotDataBase * GetConstYData (int inIndex) const;
  const LegendData * GetConstLegendData (int inIndex) const;
  const DataDrawerBase * GetConstDataDrawer (int inIndex) const;
  const PlotDataSelection * GetConstPlotDataSelection (int inIndex) const;

  bool CalculateXRange (float &outXMin, float &outXMax) const;
  bool CalculateYRange (float inXMin, float inXMax, float &outYMin, float &outYMax) const;
  bool CalculateYRangePlot (float inXMin, float inXMax, const PlotDataBase &inXData, const PlotDataBase &inYData, float &outYMin, float &outYMax) const;

 protected:
  bool CheckState () const;
  PlotDataList mXDataList;
  PlotDataList mYDataList;
  LegendDataList mLegendDataList;
  DataDrawerList mDataDrawerList;
  PlotDataSelectionList mPlotDataSelectionList;
};

class GridInfo {
public:
    GridInfo (const bool inXGridOn = false, const bool inYGridOn = false) : mXGridOn (inXGridOn), mYGridOn (inYGridOn) {};
    
    bool    mXGridOn;
    bool    mYGridOn;
};

class TickInfo {
 public:
  TickInfo ():mAutoTick (true), mAutoTickSize (true), mTickDivision(1), mMajorTickSpan(1), mMajorTickScreenSize (1), mMinorTickScreenSize (1), mFormatString ("%.0f"), mTicksOn (true) {};


  static float RoundSpan (float inSpan);

  static void MakeFormatString (float inValue, string &outFormatString);

  bool mAutoTick;
  bool mAutoTickSize;
  bool mTicksOn;
  
  int mTickDivision;
  float mMajorTickSpan; // in plot units
  int mMajorTickScreenSize;
  int mMinorTickScreenSize;
  string mFormatString;
  PStyle mStyle;
};

class AxisSetup {

 public:
  AxisSetup (): mMin(0),mMax(0), mAutoScaleMin(true), mAutoScaleMax (true), mAscending (true), mLogScale(false), mCrossOrigin(true), mMaxDecades(-1), mLogFactor (1), mLogBase (10) {};

  void SetMin (float inMin) {mMin = inMin;};
  void SetMax (float inMax) {mMax = inMax;};
  void SetAutoScale (bool inBool) {mAutoScaleMin = mAutoScaleMax = inBool;};
  bool IsAutoScale () const {return mAutoScaleMin && mAutoScaleMax;};

  float mMin;
  float mMax;
  bool mAutoScaleMin;
  bool mAutoScaleMax;
  bool mAscending;  // not Ascending: Descending
  bool mLogScale;
  bool mCrossOrigin;
  long mMaxDecades;// property for auto logscale
  long mLogFactor;// to make db possible with logscale
  float mLogBase;

  string mLabel;
  PStyle mStyle;

  TickInfo mTickInfo;

 private:
};

class Trafo {
 public:
  virtual ~Trafo (){};
  virtual float Transform (float inValue) const=0;
  virtual float TransformBack (float inValue) const = 0;
};
class LinTrafo: public Trafo {
 public:
  LinTrafo ():mOffset (0), mSlope(0){};

  virtual float Transform (float inValue) const;
  virtual float TransformBack (float inValue) const;

  float mOffset;
  float mSlope;
};

class LogTrafo: public Trafo {
 public:
  LogTrafo ():mOffset (0), mSlope(0), mBase (10), mFactor (1){};
  virtual float Transform (float inValue) const;
  virtual float TransformBack (float inValue) const;

  float mOffset;
  float mSlope;
  float mBase;
  float mFactor;
};

class TickIterator {
public:
  TickIterator ():mAxisSetup (0){};
  virtual ~TickIterator () {};

  virtual bool Init ()=0;
  virtual bool GetNextTick (float &outTick, bool &outIsMajorTick, string &outFormatString)=0;

  virtual bool InitFromRanges (float inParRange, float inOrthoScreenRange, float inDivGuess, TickInfo &outTickInfo) const=0;
  virtual bool AdjustRange (float &ioMin, float &ioMax) const{return true;};

  void SetAxisSetup (const AxisSetup *inAxisSetup) {mAxisSetup = inAxisSetup;};
protected:
  const AxisSetup *mAxisSetup;
};

class LinTickIterator: public TickIterator {
public:
  LinTickIterator ():mCurrentTick (0), mDelta (0){}
  virtual bool Init ();
  virtual bool GetNextTick (float &outTick, bool &outIsMajorTick, string &outFormatString);

  bool InitFromRanges (float inParRange, float inOrthoScreenRange, float inDivGuess, TickInfo &outTickInfo) const;
protected:
  float mCurrentTick;
  long mCount;
  float mDelta;
  string mFormatString;
};

class LogTickIterator: public TickIterator {
public:
  LogTickIterator ():mCurrentTick (0), mDelta (0){}
  virtual bool Init ();
  virtual bool GetNextTick (float &outTick, bool &outIsMajorTick, string &outFormatString);

  bool InitFromRanges (float inParRange, float inOrthoScreenRange, float inDivGuess, TickInfo &outTickInfo) const;
  virtual bool AdjustRange (float &ioMin, float &ioMax) const;
  float RoundUp (float inFloat) const;
  float RoundDown (float inFloat) const;
protected:
  float mCurrentTick;
  long mCount;
  float mDelta;
};

class NamedTickIterator: public LinTickIterator {
public:
  NamedTickIterator (){}

  void SetStringList (const vector<string> &inStringList) {mStringList = inStringList;};

  //  virtual bool Init ();
  virtual bool GetNextTick (float &outTick, bool &outIsMajorTick, string &outFormatString);

  bool InitFromRanges (float inParRange, float inOrthoScreenRange, float inDivGuess, TickInfo &outTickInfo) const;
protected:
  vector<string> mStringList;
};

class PlotBackground {
 public:
  PlotBackground ():mTransparent (true), mPlotRegionBackColor (255,255,255) {};
  bool mTransparent;
  PColor mPlotRegionBackColor;
  string mTitle;
  PStyle mStyle;
};

class PPlot;

class PDrawer {
 public:
  typedef vector<PDrawer *> tList;

  virtual ~PDrawer (){};
  virtual bool Prepare (Painter &inPainter, PPlot& inPPlot) {return true;};
  virtual bool Draw (Painter &inPainter)=0;
};

class PCalculator {// base class to do additional calculations on a PPlot
 public:
  typedef vector<PCalculator *> tList;

  virtual ~PCalculator (){};

  virtual bool ShouldCalculate () const {return true;};
  virtual bool Calculate (Painter &inPainter, PPlot& inPPlot) {return true;};
};


class PainterTester: public PDrawer {
 public:
    virtual bool Draw (Painter &inPainter);
};

class PPlot: public PDrawer {
 public:
  PPlot ();
  virtual ~PPlot ();

  virtual bool Draw (Painter &inPainter);

  PlotDataContainer mPlotDataContainer;
  AxisSetup mXAxisSetup;
  AxisSetup mYAxisSetup;
  GridInfo  mGridInfo;
  PMargins mMargins;// [pixels]
  PlotBackground mPlotBackground;

  void SetPPlotDrawer (PDrawer *inPDrawer);// taker ownership. Used to bypass normal Draw function, i.e., set Draw function by composition.
  void SetPPlotDrawer (PDrawer &inPDrawer);// same as above: does not take ownership

  bool mHasAnyModifyingCalculatorBeenActive;
  PCalculator::tList mModifyingCalculatorList;
  PCalculator::tList mPostCalculatorList;
  PDrawer::tList mPreDrawerList;
  PDrawer::tList mPostDrawerList;

  TickIterator *mXTickIterator;
  TickIterator *mYTickIterator;

  virtual bool CalculateXTransformation (const PRect &inRect);
  virtual bool CalculateYTransformation (const PRect &inRect);
  virtual bool DrawGridXAxis (const PRect &inRect, Painter &inPainter) const;
  virtual bool DrawGridYAxis (const PRect &inRect, Painter &inPainter) const;
  virtual bool DrawXAxis (const PRect &inRect, Painter &inPainter) const;
  virtual bool DrawYAxis (const PRect &inRect, Painter &inPainter) const;
  virtual bool CalculateTickInfo (const PRect &inRect, Painter &inPainter);

  Trafo *mXTrafo;
  Trafo *mYTrafo;

  static int Round (float inFloat);
  static const float kRangeVerySmall;
 protected:
  PPlot (const PPlot&);
  PPlot& operator=(const PPlot&);

  static bool CalculateLogTransformation (int inBegin, int inEnd, const AxisSetup& inAxisSetup, LogTrafo& outTrafo);
  static bool CalculateLinTransformation (int inBegin, int inEnd, const AxisSetup& inAxisSetup, LinTrafo& outTrafo);

  virtual bool DrawPlotBackground (const PRect &inRect, Painter &inPainter) const;
  virtual bool DrawXTick (float inX, int inScreenY, bool inMajor, const string &inFormatString, Painter &inPainter, PRect &outRect) const;
  virtual bool DrawYTick (float inY, int inScreenX, bool inMajor, const string &inFormatString, Painter &inPainter, PRect &outRect) const;
  virtual bool DrawLegend (const PRect &inRect, Painter &inPainter) const;
  virtual bool DrawPlot (int inIndex, const PRect &inRect, Painter &inPainter) const;
  virtual bool ConfigureSelf ();// change here implementations of interfaces
  virtual bool ValidateData ();// check preconditions here things like x is ascending
  virtual bool CalculateAxisRanges ();
  virtual bool CheckRange (const AxisSetup &inAxisSetup) const;

  void SetTickSizes (int inFontHeight, TickInfo &ioTickInfo);

  // trafo's between plot coordinates and screen coordinates.
  LinTrafo mXLinTrafo;
  LinTrafo mYLinTrafo;
  LogTrafo mXLogTrafo;
  LogTrafo mYLogTrafo;

  LinTickIterator mXLinTickIterator;
  LinTickIterator mYLinTickIterator;
  LogTickIterator mXLogTickIterator;
  LogTickIterator mYLogTickIterator;
  NamedTickIterator mXNamedTickIterator;

  PDrawer * mPPlotDrawer;
  bool mOwnsPPlotDrawer;
};

bool MakeExamplePlot (int inExample, PPlot &ioPPlot);
void MakeExamplePlot1 (PPlot &ioPPlot);
void MakeExamplePlot2 (PPlot &ioPPlot);
void MakeExamplePlot3 (PPlot &ioPPlot);
void MakeExamplePlot4 (PPlot &ioPPlot);
void MakeExamplePlot5 (PPlot &ioPPlot);
void MakeExamplePlot6 (PPlot &ioPPlot);
void MakeExamplePlot7 (PPlot &ioPPlot);
void MakeExamplePlot8 (PPlot &ioPPlot);
void MakePainterTester (PPlot &ioPPlot);

void MakeCopy (const PPlot &inPPlot, PPlot &outPPlot);

// following functions can be used to interface with scripts
void SetCurrentPPlot (PPlot *inPPlot);
PPlot & GetCurrentPPlot ();

#endif
