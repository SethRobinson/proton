/***************************************************************************
 *                                                                         *
 *   Copyright notice:                                                     *
 *                                                                         *
 *   This is free Pier ware. You may do whatever you want with this code.  *
 *   You may cont(r)act me by email: pierphil@xs4all.nl                    *
 *                                                                         *
 ***************************************************************************/

#ifndef __P3DPLOT_H__
#define __P3DPLOT_H__

#include <vector>
using std::vector;
#include <string>
using std::string;

#include "../generic/PPlot.h"

class PVertex {
 public:
  PVertex (float inX=0, float inY=0, float inZ=0): mX(inX), mY(inY), mZ(inZ){}; 
  float mX;
  float mY;
  float mZ;
};

class Carpenter {
 public:

    virtual void MakeLine (const PVertex &inVertex1, const PVertex &inVertex2)=0;
    virtual void MakePolygon (const vector<PVertex> &inVertices)=0;
    virtual void SetColor (int inR, int inG, int inB)=0;
    virtual void SetSurfaceNormal (const PVertex &inVertex)=0;
    virtual void Finish ()=0;
};


class P3DPlot {
 public:
  P3DPlot ();
  virtual ~P3DPlot ();

  virtual bool MakeModel (Carpenter &inCarpenter);
  void SetData (PlotData *inData, long inXCount=0, long inYCount=0);// takes ownership
 protected:
    void MakeBar (float inX, float inY, float inZ, float inSize);
    void MakePolygonWithOffset (const vector<PVertex> &inVertices, const PVertex &inOffset);

    Carpenter *mCarpenter;

  PlotData *mData;
  long mXCount;
  long mYCount;
};

void MakeExamplePlot1 (P3DPlot &ioPPlot);
void MakeExamplePlot2 (P3DPlot &ioPPlot);

#endif
