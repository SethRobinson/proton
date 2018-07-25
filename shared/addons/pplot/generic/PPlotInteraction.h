/***************************************************************************
 *                                                                         *
 *   Copyright notice:                                                     *
 *                                                                         *
 *   This is free Pier ware. You may do whatever you want with this code.  *
 *   You may cont(r)act me by email: pierphil@xs4all.nl                    *
 *                                                                         *
 ***************************************************************************/

#ifndef __PPLOTINTERACTION_H__
#define __PPLOTINTERACTION_H__

#include "PPlot.h"

#pragma warning (disable: 4786)

#include <vector>
using std::vector;
#include <string>
using std::string;
#include <stack>
using std::stack;

class PModifierKeys {
public:
  enum {
    kShift=1,
    kControl=2,
    kAlt=4
  };
  PModifierKeys (int inModifierKeys=0):mModifierKeys (inModifierKeys) {};


  bool IsShiftKeyDown () const {return mModifierKeys & kShift;};
  bool IsControlKeyDown () const {return mModifierKeys & kControl;};
  bool IsAltKeyDown () const {return mModifierKeys & kAlt;};
  bool IsOnlyShiftKeyDown () const {return mModifierKeys == kShift;};
  bool IsOnlyControlKeyDown () const {return mModifierKeys == kControl;};
  bool IsOnlyAltKeyDown () const {return mModifierKeys == kAlt;};

  bool HasModifierKeys () const {return mModifierKeys != 0;};

  void SetModifierKeys (int inModifierKeys) {mModifierKeys = inModifierKeys;};
private:
  int mModifierKeys;// values like kShift | kAlt
};

class PMouseEvent: public PModifierKeys {
public:
  enum EType {
    kNone,
    kDown,
    kUp,
    kMove
  };

  PMouseEvent (int inX=0, int inY=0, EType inType=kNone, int inModifierKeys=0);

  int mX;
  int mY;


  EType mType;

  bool IsNone () const {return mType == kNone;};
  bool IsMouseDown () const {return mType == kDown;};
  bool IsMouseUp () const {return mType == kUp;};
  bool IsMouseMove () const {return mType == kMove;};
};

class PKeyEvent: public PModifierKeys {
public:

  enum EKey {
    kNone,
    kArrowUp,
    kArrowDown,
    kArrowLeft,
    kArrowRight,
    kDelete,
    kChar
  };

  PKeyEvent (EKey inKey=kNone, int inRepeatCount=0, int inModifierKeys=0, char inChar=0);


  bool IsNone () const {return mKey == kNone;};
  bool IsArrowUp () const {return mKey == kArrowUp;};
  bool IsArrowDown () const {return mKey == kArrowDown;};
  bool IsArrowLeft () const {return mKey == kArrowLeft;};
  bool IsArrowRight () const {return mKey == kArrowRight;};
  bool IsDelete () const {return mKey == kDelete;};
  bool IsChar () const {return mKey == kChar;};

  int GetRepeatCount () const {return mRepeatCount;};
  char GetChar () const {return mChar;};
protected:
  EKey mKey;
  char mChar;

  int mRepeatCount;
};


class PPlotInteraction {
 public:

  typedef vector<PPlotInteraction *>tList;

  PPlotInteraction (PPlot &inPPlot);

  virtual bool HandleMouseEvent (const PMouseEvent &inEvent)=0;
  virtual bool HandleKeyEvent (const PKeyEvent &inEvent) {return false;};

  void SetEnabled (bool inBool) {mIsEnabled = inBool;};
  bool IsEnabled () const {return mIsEnabled;};
protected:
  PPlot &mPPlot;
  bool mIsEnabled;
};

class PAxisInfo {
public:
  typedef vector<AxisSetup> tList;
  AxisSetup mXAxisSetup;
  AxisSetup mYAxisSetup;
};

class PZoomInteraction: public PPlotInteraction, public PDrawer {
public:

  enum EZoomMode {
    kZoom_Region,
    kZoom_X,
    kZoom_Y
  };

  PZoomInteraction (PPlot &inPPlot);

  virtual bool HandleMouseEvent (const PMouseEvent &inEvent);
  virtual bool HandleKeyEvent (const PKeyEvent &inEvent);

  void DoZoomIn (float inX1, float inX2, float inY1, float inY2);
  void DoZoomOut (float inY1 = -1, float inY2 = -1);
  bool CanZoomOut () { return !mZoomHistory.empty (); };
  int  GetZoomStackSize () { return mZoomHistory.size (); };

  stack<PAxisInfo> mZoomHistory;
  EZoomMode mZoomMode;

  bool IsZoomRegion () const {return mZoomMode == kZoom_Region;};
  bool IsZoomX () const {return mZoomMode == kZoom_X;};
  bool IsZoomY () const {return mZoomMode == kZoom_Y;};
protected:
  void StoreCurrentAxisSetup ();
  virtual bool Draw (Painter &inPainter);
  bool CheckRange (float inFloat1, float inFloat2);

  void DoZoomIn ();

  bool mDragging;
  int mX1;
  int mY1;
  int mX2;
  int mY2;

};

class PlotDataIncrementerBounds {
public:
  PlotDataIncrementerBounds ();

  bool CheckBounds (float inValue) const;

  bool mLowerBoundEnabled;
  float mLowerBound;
  bool mUpperBoundEnabled;
  float mUpperBound;
};

class PlotDataIncrementer {
public:

  // all are none are incremented
  bool Increment (const vector<float> &inIncrementList, vector<float *> &inData, const PlotDataIncrementerBounds &inGlobalBounds, const vector<PlotDataIncrementerBounds> &inBoundList) const;

protected:
  bool Impl_Increment (const vector<float> &inIncrementList, vector<float *> &inData, const PlotDataIncrementerBounds &inGlobalBounds, const vector<PlotDataIncrementerBounds> &inBoundList, bool inDontChange) const;
};

class PSelectionInteractionListener {
public:
  virtual void HandlePSelectionInteraction ()=0;
};

class PSelectionInteraction: public PPlotInteraction, public PCalculator {
public:

  enum ECommand {
    kNone,
    kPointwiseSelection,
    kGlobalSelection,
    kSelectAll
  };

  PSelectionInteraction (PPlot &inPPlot);

  virtual bool HandleKeyEvent (const PKeyEvent &inEvent);
  virtual bool HandleMouseEvent (const PMouseEvent &inEvent);
  virtual bool Calculate (Painter &inPainter, PPlot& inPPlot);

  void SetCommand (ECommand inCommand, const PKeyEvent &inKeyEvent, const PMouseEvent &inMouseEvent);
  void SetListener (PSelectionInteractionListener *inListener) {mListener = inListener;};
protected:
  PSelectionInteractionListener *mListener;

  void HandleGlobalInteraction (bool inHit, long inNearestPointIndex, DataDrawerBase *inDataDrawer, PlotDataSelection *inPlotDataSelection);
  void HandlePointwiseInteraction (bool inHit, long inNearestPointIndex, DataDrawerBase *inDataDrawer, PlotDataSelection *inPlotDataSelection);
  float CalculateDistanceToPlot (const PlotDataBase *inXData, const PlotDataBase *inYData, long &outNearestPointIndex);
  void SelectAll (PlotDataBase *inYData, PlotDataSelection *inPlotDataSelection);
//  int mX;
//  int mY;
  ECommand mCommand;
  PMouseEvent mMouseEvent;
  PKeyEvent mKeyEvent;
};


class PKeySelectionInteraction: public PPlotInteraction, public PCalculator {
public:
  PKeySelectionInteraction (PPlot &inPPlot);

  virtual bool Calculate (Painter &inPainter, PPlot& inPPlot);

protected:

  bool mCalculate;
};

class PEditInteractionListener {
public:
  virtual void HandlePEditInteraction ()=0;
};

class PEditInteraction: public PPlotInteraction, public PCalculator {
public:
  PEditInteraction (PPlot &inPPlot);

  virtual bool HandleMouseEvent (const PMouseEvent &inEvent) {return false;};
  virtual bool HandleKeyEvent (const PKeyEvent &inEvent);
  virtual bool ShouldCalculate () const {return mCalculate;};
  virtual bool Calculate (Painter &inPainter, PPlot& inPPlot);

  virtual bool Impl_HandleKeyEvent (const PKeyEvent &inEvent)=0;
  virtual bool Impl_Calculate (Painter &inPainter, PPlot& inPPlot)=0;
  void SetListener (PEditInteractionListener *inListener) {mListener = inListener;};
protected:
  PEditInteractionListener *mListener;
  PKeyEvent mKeyEvent;
private:
  bool mCalculate;
};

class PVerticalCursorInteraction: public PEditInteraction {
public:

  PVerticalCursorInteraction (PPlot &inPPlot);

  virtual bool Impl_HandleKeyEvent (const PKeyEvent &inEvent);
  virtual bool Impl_Calculate (Painter &inPainter, PPlot& inPPlot);

  PlotDataIncrementerBounds mGlobalBounds;
protected:
  void HandleVerticalCursorKey (const PlotDataSelection *inPlotDataSelection, PlotDataBase *inYData);
};

class PDeleteInteraction: public PEditInteraction {
public:

  PDeleteInteraction (PPlot &inPPlot);

  virtual bool Impl_HandleKeyEvent (const PKeyEvent &inEvent);
  virtual bool Impl_Calculate (Painter &inPainter, PPlot& inPPlot);

protected:
  void HandleDeleteKey (PlotDataBase *inXData, PlotDataBase *inYData, PlotDataSelection *inPlotDataSelection);
};

class PCrosshairInteractionListener {
public:
  virtual void HandleCrosshair (int inIndex, int inPlotCount, float inX, float inY)=0;
};

class PCrosshairInteraction: public PPlotInteraction, public PDrawer  {
public:
  PCrosshairInteraction (PPlot &inPPlot);

  void SetListener (PCrosshairInteractionListener *inListener) {mListener = inListener;};
protected:
  virtual bool HandleMouseEvent (const PMouseEvent &inEvent);
  virtual bool Draw (Painter &inPainter);

  bool GetCrossPoint (const PlotDataBase *inXData, const PlotDataBase *inYData, float &outY);
  bool mActive;
  int mX;
  PCrosshairInteractionListener *mListener;
};

class InteractionContainer {
public:
  InteractionContainer (){};
  virtual ~InteractionContainer (){};

  bool HandleMouseEvent (const PMouseEvent &inEvent);
  bool HandleKeyEvent (const PKeyEvent &inEvent);

  void AddInteraction (PPlotInteraction &inInteraction){mList.push_back(&inInteraction);};

protected:
  PPlotInteraction::tList mList;
};

class DefaultInteractionContainer: public InteractionContainer {
public:
  DefaultInteractionContainer (PPlot &inPPlot);

  PZoomInteraction mZoomInteraction;
  PSelectionInteraction mSelectionInteraction;
  PVerticalCursorInteraction mVerticalCursorInteraction;
  PDeleteInteraction mDeleteInteraction;
  PCrosshairInteraction mCrosshairInteraction;
};

#endif
