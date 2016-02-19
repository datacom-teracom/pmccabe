


/************************************************************************\
   LabelledNode declaration
\************************************************************************/

class Shaper;

#define LabelledNodeParent IlvListLabel
#define LabelledNodeParentPtr IlvListLabel*
class LabelledNode: public LabelledNodeParent {
private:
protected:
   char* label_;
   IlvColor* border_;
   IlvShort bwidth_;
   XeNodePtr xeNode_;
   ShapeType shapeType_;
   Shaper* shaper_;
   
   virtual void drawShape(IlvDisplay* dpy, IlvPort* dst,
			    IlvPalette* palette, IlvTransformer* t, IlvRect& r)
;

   virtual void baseBoundingBox(IlvRect& r, IlvTransformer* t=0);
public:
   LabelledNode(XeNodePtr xenode, IlvDisplay* dpy, const IlvPoint& p, char* lbl
,
	  IlvShort bwidth, IlvPalette* palette = nil);
   ~LabelledNode() { ; }
   void label(char*);
   char* label() { return (label_); }
   void borderColor(char*);
   
   void shape(ShapeType type);
   ShapeType shapeType() { return shapeType_; }
   
   IlvShort borderWidth() { return bwidth_; }
   void borderWidth(IlvShort w) { bwidth_ =  w; }
   
   XeNodePtr xeNode() { return xeNode_; }
   void xeNode(XeNodePtr n) { xeNode_ = n; }
   
   virtual void draw(IlvPort* dst, IlvTransformer* t, IlvRegion* rect=0);
   virtual void boundingBox(IlvRect& r, IlvTransformer* t=0);
   virtual IlvBoolean contains(const IlvPoint& p,
			       const IlvPoint& tp, IlvTransformer* t);
   virtual void computeRegion(IlvRegion& r, IlvTransformer* t=0);   

   static LabelledNode* makeShapedNode(XeNodePtr node,
				       IlvDisplay* dpy,
				       const ShapeType type,
				       const IlvPoint& p,
				       char* label,
				       IlvShort bwidth);
   
   DeclareTypeInfo();
};

typedef enum { FixedPosition = 0x0, RelativePosition = 0x1 } ArcEndType;

/************************************************************************\
   LabelledArc declaration and implementation
\************************************************************************/


typedef IlvLinkImage ArcRootClass;

class TextList;

#define RECURSIVE_BEZIER_ARCS 1
// IlvDisplay::drawArrow(dst, transform, start, end, atPos) requires
// atPos in the range [0.0,1.0].   0.0 means draw the arrow at the
// beginning of line, 1.0 means draw at end...

const float ArrowAtStart = 0.0;
const float ArrowAtEnd = 1.0;

const int NodeBorderAllowance = 4;

#define LabelledArcParent IlvLinkImage
#define LabelledArcParentPtr IlvLinkImage*
class LabelledArc: public LabelledArcParent {
protected:
   XeArcPtr arc;
   char* label;
   Boolean labelVisible;

   char* labelColorString;
   IlvColor* labelColor;
   
   TextList* tlist;
#if RECURSIVE_BEZIER_ARCS
   IlvPoint _points[7];
   IlvShort _nPoints;
   IlvPoint _labelOrigin;
#else
   IlvPoint _points[4];
   IlvInt _nPoints;
   IlvPoint _labelOrigin;
#endif
   
   IlvBoolean arrowContains(const IlvPoint& p,
			    const IlvPoint& tp, IlvTransformer*t=0);
   
   virtual IlvBoolean arcContains(const IlvPoint& p,
				  const IlvPoint& tp, IlvTransformer*t=0)
   {
      IlvBoolean isIn;
      isIn = IlvPointInLine(p, _points[0], _points[1]);
      if (!isIn) {
	 isIn = arrowContains(p, tp, t);
      }
      return isIn;
   }
   
   virtual IlvBoolean labelContains(const IlvPoint& p,
				    const IlvPoint& tp, IlvTransformer*t=0)
   {
      IlvRect r;
      labelBoundingBox(r, t);
      IlvBoolean isIn;
      if (t) {
	 isIn = r.contains(tp);
      } else {
	 isIn = r.contains(p);
      }
      return isIn;
   }
   
   // calculate the origination point for the label, using the
   // indicated transformation
   virtual void getLabelOrigin(IlvPoint& origin, IlvTransformer* t=0)
   {
      IlvRect r;
      arcBoundingBox(r, t);
      origin.move(r.centerx(), r.centery());
   }

   virtual void arrowPoints(IlvPoint* points, IlvTransformer* t=0)
   {
      // This should really calculate the points then apply the transformer...
      IlvPoint from = _points[0], to = _points[1];
      if (t) {
	 t->apply(from);
	 t->apply(to);
      }
      IlvComputeArrow(from, to, ArrowAtEnd, points);
   }
   
   virtual void arrowBoundingBox(IlvRect& r, IlvTransformer* t=0)
   {
      IlvPoint pts[3];
      arrowPoints(pts, t);
      r.resize(0,0);
      r.move(pts[0]);
      r.add(pts[1]);
      r.add(pts[2]);
   }
   
   virtual void arcBoundingBox(IlvRect& r, IlvTransformer* t=0)
   {
      r.move(_points[0]);
      r.resize(0,0);
      r.add(_points[1]);
      if (t) t->apply(r);
      IlvRect arrowBox;
      arrowBoundingBox(arrowBox, t);
      if (arrowBox.w() != 0 && arrowBox.h() != 0) {
	 r.add(arrowBox);
      }
   }
   // find the label's origin relative to the non-transformed bounding
   // box, then calculate the label's bounding box using the
   // indicated transformation...
   virtual void labelBoundingBox(IlvRect& r, IlvTransformer* t=0);

   virtual void drawArrow(IlvPort* dst, IlvTransformer* t=0, IlvRegion* = 0)
   {
      IlvPoint start = _points[0], end = _points[1];
      if (t) {
	 t->apply(start);
	 t->apply(end);
      }
      getDisplay()->drawArrow(dst, getPalette(), start, end, ArrowAtEnd);
   }
   
   virtual void drawArc(IlvPort* dst, IlvTransformer* t=0, IlvRegion* rect=0);
      
   virtual void drawLabel(IlvPort* dst, IlvTransformer* t, IlvRegion* clip=0);
   
   virtual void computePoints(IlvGraphic* f, IlvGraphic* t);
   
public:
   LabelledArc(XeArcPtr owner, IlvDisplay* dpy, char* lbl,
	       UIPtr tail, UIPtr head, Boolean showLabel = True);
   ~LabelledArc();

   XeArcPtr owner() { return arc; }
   
   char* getLabel() { return label; }
   void  setLabel(char* s);

   char* getLabelColor() { return labelColorString; }
   void setLabelColor(char* c);
   
   Boolean getLabelVisible() { return labelVisible; }
   void setLabelVisible(Boolean v);

   virtual IlvBoolean contains(const IlvPoint& p,
			       const IlvPoint& tp, IlvTransformer*t =0);
   virtual void draw(IlvPort* dst, IlvTransformer* t=0, IlvRegion* rect=0);
   virtual void boundingBox(IlvRect&, IlvTransformer*t =0);
   virtual void computeRegion(IlvRegion& r, IlvTransformer* t);
   
   DeclareTypeInfo();
   
};

/************************************************************************\
 sibling computation code -- perhaps this should be a nested class of arcs
\************************************************************************/

class SiblingList {
protected:
   XeArcPtr* arcs;
   int nSlots;
   int nArcs;

   void listAppend(XeArcPtr sib);
   void listRemove(XeArcPtr sib);
public:
   SiblingList() { nSlots = 0; nArcs = 0; arcs = nil; }
   ~SiblingList() { delete arcs; }
   
   int count() { return nArcs; }
   int max() { return nSlots; }
   
   void insert(XeArcPtr sib);
   void remove(XeArcPtr sib);
   int rank(XeArcPtr);

   IlvBoolean isSibling(XeArcPtr sib)
   {
      IlvBoolean isSib = IlvFalse;
      for(int i = 0; i < nArcs; i++) {
	 if (arcs[i] == sib) {
	    isSib = IlvTrue;
	    break;
	 }
      }
      return isSib;
   }
};


#if RECURSIVE_BEZIER_ARCS
const IlvFloat RankedArcArrowPosition = ArrowAtEnd;
#else
const IlvFloat RankedArcArrowPosition = 0.6;
#endif

#define RankedArcParent LabelledArc
#define RankedArcParentPtr LabelledArc*
class RankedArc: public RankedArcParent {
protected:
   
   IlvGraphic* _selfNode;
#if !RECURSIVE_BEZIER_ARCS
   EllipseInfo* _eInfo;
#endif
   int currentRank;

   // Compute the control points for our curve based on rank and
   // current position of head and tail of arc
   virtual void computePoints(IlvGraphic* f, IlvGraphic* t);
   
   virtual void arrowPoints(IlvPoint* points, IlvTransformer* t=0) {
      // This should really calculate the points then apply the transformer...
      IlvPoint from, to;
      IlvFloat arrowPos = ArrowAtEnd;
      if (currentRank == 0 && !selfish()) {
	 RankedArcParent::arrowPoints(points, t);
	 return;
      }
      if (selfish()) {
#if RECURSIVE_BEZIER_ARCS
	 arrowPos = RankedArcArrowPosition;
	 from = _points[5];
	 to = _points[6];
#else
	 arrowPos = RankedArcArrowPosition;
	 from = _eInfo->startArrow();
	 to = _eInfo->endArrow();
#endif
      } else {
	 from = _points[2];
	 to = _points[3];
      }
      if (t) {
	 t->apply(from);
	 t->apply(to);
      }
      IlvComputeArrow(from, to, arrowPos, points);
   }
   
   int computeRank() {
      return owner()->siblings() ? owner()->siblings()->rank(owner()): 0;
   }
   
   IlvBoolean selfish() { return _selfNode ? IlvTrue : IlvFalse; }
   
   virtual IlvBoolean arcContains(const IlvPoint& p,
				  const IlvPoint& tp, IlvTransformer*t=0)
   {
      IlvBoolean isIn = IlvFalse;
      currentRank = computeRank();
      IlvGraphic* tail = getFrom();
      IlvGraphic* head = getTo();
      computePoints(tail, head);
      if (currentRank == 0 && !selfish()) {
	 isIn = RankedArcParent::arcContains(p, tp, t);
      } else {
	 if (t) {
#if RECURSIVE_BEZIER_ARCS
	    IlvPoint points[8];
#else
	    IlvPoint points[4];
#endif
	    for (int i = 0; i < _nPoints; i++) {
	       points[i] = _points[i];
	       t->apply(points[i]);
	    }
	    // check the "transformed point" supplied by Views manager...
#if RECURSIVE_BEZIER_ARCS
	    isIn = IlvPointInSpline(tp, _nPoints, points);
#else
	    if (selfish()) {
	       IlvRect r = _eInfo->rect();
	       t->apply(r);
	       isIn = PointOnArc(tp, r, _eInfo->startAngle(),
				 _eInfo->deltaAngle(), /* tolerance */ 2);
	    } else {
	       isIn = IlvPointInSpline(tp, _nPoints, points);
	    }
#endif
	 } else {
#if RECURSIVE_BEZIER_ARCS
	    isIn = IlvPointInSpline(p, _nPoints, _points);
#else
	    if (selfish()) {
	       isIn = PointOnArc(p, _eInfo->rect(), _eInfo->startAngle(),
				 _eInfo->deltaAngle(), /* tolerance */ 2);
	    } else {
	       isIn = IlvPointInSpline(p, _nPoints, _points);
	    }
#endif
	 }
	 if (!isIn) {
	    isIn = arrowContains(p, tp, t);
	 }
      }
      return isIn;
   }
   
   // calculate the origination point for the label, using the
   // indicated transformation
   virtual void getLabelOrigin(IlvPoint& origin, IlvTransformer* t=0)
   {
      currentRank = computeRank();
      IlvGraphic* tail = getFrom();
      IlvGraphic* head = getTo();
      computePoints(tail, head);
      if (currentRank == 0 && !selfish()) {
	 RankedArcParent::getLabelOrigin(origin, t);
      } else {
	 origin = _labelOrigin;
	 if (t) {
	    t->apply(origin);
	 }
      }
   }

   virtual void arcBoundingBox(IlvRect& r, IlvTransformer* t=0)
   {
      currentRank = computeRank();
      IlvGraphic* tail = getFrom();
      IlvGraphic* head = getTo();
      computePoints(tail, head);
      if (currentRank == 0 && !selfish()) {
	 RankedArcParent::arcBoundingBox(r, t);
      } else {
#if RECURSIVE_BEZIER_ARCS
	 r.move(_points[0]);
	 r.resize(0,0);
	 r.add(_points[1]);
	 r.add(_points[2]);
	 r.add(_points[3]);
	 if (selfish()) { // only valid for self arcs...
	    r.add(_points[4]);
	    r.add(_points[5]);
	    r.add(_points[6]);
	 }
#else
	 if (selfish()) {
	    IlvComputeArcBBox(_eInfo->rect(),
			      _eInfo->startAngle(),
			      _eInfo->deltaAngle(),
			      r);
	 } else {
	    r.move(_points[0]);
	    r.resize(0,0);
	    r.add(_points[1]);
	    r.add(_points[2]);
	    r.add(_points[3]);
	 }
#endif
	 IlvRect arrowBox;
	 arrowBoundingBox(arrowBox, 0);
	 r.add(arrowBox);
	 if (t) t->apply(r);
      }
   }

   virtual void drawArc(IlvPort* dst, IlvTransformer* t=0, IlvRegion* rect=0)
   {
      currentRank = computeRank();
      IlvGraphic* tail = getFrom();
      IlvGraphic* head = getTo();
      computePoints(tail, head);
      if (currentRank == 0 && !selfish()) {
	 RankedArcParent::drawArc(dst, t, rect);
      } else {
	 // how do we draw the arrow???
	 drawArrow(dst, t, rect);
	 drawRankedArc(currentRank, dst, t, rect);
      }
   }

   virtual void drawRankedArc(int rank, IlvPort* dst, IlvTransformer* t=0, IlvR
egion* rect=0)
   {
#if RECURSIVE_BEZIER_ARCS
      if (selfish() || rank > 0) {
	 if (!t) {
	    getDisplay()->drawBezier(dst, getPalette(), _nPoints, _points);
	 } else {
	    IlvPoint points[8];
	    for (int i = 0; i < _nPoints; i++) {
	       points[i] = _points[i];
	       t->apply(points[i]);
	    }
	    getDisplay()->drawBezier(dst, getPalette(), _nPoints, points);
	 }
      } else {
	 // shouldn't reach this point...
	 RankedArcParent::drawArc(dst, t, rect);
      }
#else
      if (selfish()) {
	 IlvRect r = _eInfo->rect();
	 if (t) t->apply(r);
	 getDisplay()->drawArc(dst, getPalette(), r,
			       (IlvFloat) _eInfo->startAngle(),
			       (IlvFloat) _eInfo->deltaAngle());
      } else if (rank > 0) {
	 if (t) {
	    IlvPoint points[4];
	    for (int i = 0; i < _nPoints; i++) {
	       points[i] = _points[i];
	       t->apply(points[i]);
	    }
	    getDisplay()->drawBezier(dst, getPalette(), _nPoints, points);
	 } else {
	    getDisplay()->drawBezier(dst, getPalette(), _nPoints, _points);
	 }
      } else {
	 // shouldn't reach this point...
	 RankedArcParent::drawArc(dst, t, rect);
      }
#endif
   }
   
   virtual void drawArrow(IlvPort* dst, IlvTransformer* t=0, IlvRegion* r=0)
   {
      IlvBoolean selfArc = selfish();
      if (!selfArc && currentRank == 0) {
	 RankedArcParent::drawArrow(dst, t, r);
      } else {
	 IlvPoint start, end;
	 IlvFloat arrowPos = ArrowAtEnd;
#if RECURSIVE_BEZIER_ARCS
	 if (selfArc) {
	    start = _points[5];
	    end = _points[6];
	    arrowPos = RankedArcArrowPosition;
	 } else {
	    start = _points[2];
	    end = _points[3];
	 }
#else
	 if (selfArc) {
	    start = _eInfo->startArrow();
	    end = _eInfo->endArrow();
	    arrowPos = RankedArcArrowPosition;
	 } else {
	    // we use the control points of our spline to determine direction
	    // and "slope" of arrow's center...
	    start = _points[2];
	    end = _points[3];
	 }
#endif
	 if (t) {
	    t->apply(start);
	    t->apply(end);
	 }
	 getDisplay()->drawArrow(dst, getPalette(), start, end, arrowPos);
      }
   }
      
   
public:
   RankedArc(XeArcPtr owner, IlvDisplay* dpy, char* lbl,
	     UIPtr tail, UIPtr head, Boolean showLabel = True);
   ~RankedArc() {
      _nPoints = 0;
#if !RECURSIVE_BEZIER_ARCS
      if (_selfNode)
	 delete _eInfo;
#endif
   }
      
   DeclareTypeInfo();
   
};



