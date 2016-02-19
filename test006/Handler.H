


/************************************************************************\
   classes used by GraphHandler
\************************************************************************/

class ActionHandler;          /* public IlvManagerViewInteractor */

   class MotionHandler;       /* public ActionHandler */
      class Selector;            /* public MotionHandler */
         class SweepSelector;      /* public Selector      */
         class SubgraphSelector;   /* public Selector      */
      class ArcHandler;          /* public MotionHandler */
         class ArcMaker;           /* public ArcHandler    */
         class ArcChanger;         /* public ArcHandler    */
      class NodeMaker;           /* public MotionHandler */

   class ItemMover;           /* public ActionHandler */
      class NodeMover;          /* public ItemMover     */
      class SelectionMover;     /* public ItemMover     */


   class MenuHandler;         /* public ActionHandler */

/************************************************************************\
   GraphHandler declaration and implementation
\************************************************************************/


#define GraphHandlerParent IlvManagerViewInteractor
#define GraphHandlerParentPtr IlvManagerViewInteractor*
class GraphHandler: public GraphHandlerParent {
protected:
   IlvGraphic* _target;
   
   SweepSelector* _sweepSelector;
   SubgraphSelector* _subgraphSelector;
   Selector* _toggleSelector;
   Selector* _selector;

   NodeMover* _nodeMover;
   SelectionMover* _selectionMover;

   ArcMaker* _arcMaker;
   ArcChanger* _arcChanger;
   NodeMaker* _nodeMaker;

   MenuHandler* _menuHandler;
  
public:
   GraphHandler(GrapherPtr graph, GraphViewPtr view);
   ~GraphHandler();
   
   virtual void handleEvent(IlvEvent& event);
   virtual void doIt(IlvGraphic* graphic,const IlvPoint&);
};


#define ActionHandlerParent IlvManagerViewInteractor
#define ActionHandlerParentPtr IlvManagerViewInteractor*
class ActionHandler: public ActionHandlerParent
{
protected:
   GraphHandler* _caller;
   IlvGraphic* _target;
   
   void returnControl()
   {
      if (_caller) {
	 getManager()->setInteractor(_caller, getView());
      } else {
	 getManager()->removeInteractor(getView());
      }
   }

   ActionHandler(GraphHandler* caller, GrapherPtr manager, IlvView* view)
      : ActionHandlerParent(manager, view), _caller(caller), _target(nil)
   {
   }
   
public:
   void target(IlvGraphic* t) { _target = t; }
   IlvGraphic* target() { return _target; }
   IlvBoolean targetted() { return (_target) ? IlvTrue : IlvFalse; }
   
   virtual void handleEvent(IlvEvent& event);

   virtual void init() {
      ActionHandlerParent::init();
      target(nil);
   }
   
   void takeCharge(IlvEvent& event, IlvGraphic* under = nil)
   {
      getManager()->setInteractor(this);
      target(under);
      handleEvent(event);
   }
   
   void abort() {
      ActionHandlerParent::abort();
      target(nil);
   }
};


#define MotionHandlerParent ActionHandler
#define MotionHandlerParentPtr ActionHandler*
class MotionHandler: public MotionHandlerParent
{
protected:
   // for rubber-banding
   IlvPoint _first;
   IlvPoint _current;
   
   virtual void initializeData(IlvEvent& event)
   {
      _first.move(event.x(), event.y());
   }
   
   virtual void updateData(IlvEvent& event, IlvView*, IlvManager*)
   {
      _current.move(event.x(), event.y());
   }
   
   virtual void startMotion(IlvEvent& event) {
      initializeData(event);
      drawGhost();
      updateMotion(event);
   }
   virtual void updateMotion(IlvEvent& event) {
      drawGhost();
      updateData(event, getView(), getManager());
      ensureVisible(IlvPoint(event.x(), event.y()));
      drawGhost();
   }
   virtual void endMotion(IlvEvent& event) {
      updateData(event, getView(), getManager());
      drawGhost();
      doIt();
      returnControl();
   }

   virtual void drawGhost() { }
   virtual void doIt() { }
   
   MotionHandler(GraphHandler* caller, GrapherPtr manager, IlvView* view)
      : MotionHandlerParent(caller, manager, view)
   {
   }
   
public:
   
   virtual IlvBoolean isInitialEvent(IlvEvent&) { return IlvTrue; }
   virtual IlvBoolean isOkInitialTarget(IlvGraphic*) { return IlvTrue; }
   virtual IlvBoolean isOkFinalTarget(IlvGraphic*) { return IlvTrue; }
   virtual void handleEvent(IlvEvent& event);
   
};


#define ArcHandlerParent MotionHandler
#define ArcHandlerParentPtr MotionHandler*
class ArcHandler: public ArcHandlerParent
{
protected:

   IlvCursor* originalCursor;
   IlvCursor* initialTargetCursor;
   IlvCursor* noTargetCursor;
   IlvCursor* finalTargetCursor;
   
   void indicateInitialTarget() {
      IlvDisplay* display = getView()->getDisplay();
      originalCursor = display->defaultCursor();
      display->setCursor(getView(), initialTargetCursor);
      // setCursor(originalCursor);
   }
   void indicateNoTarget() {
      getView()->getDisplay()->setCursor(getView(), noTargetCursor);
      // setCursor(originalCursor);
   }
   void indicateFinalTarget() {
      getView()->getDisplay()->setCursor(getView(), finalTargetCursor);
      // setCursor(originalCursor);
   }
   void indicatePreviousCursor() {
      getView()->getDisplay()->setCursor(getView(), originalCursor);
   }
   
   virtual void drawGhost()
   {
      if ((_current.x() != _first.x()) ||
	  (_current.y() != _first.y())) {
	 IlvPoint tf = _first;
	 IlvTransformer* t = getTransformer();
	 if (t) {
	    t->apply(tf);
	 }

	 getManager()->getDisplay()->drawLine(getView(),
					      getManager()->getPalette(),
					      tf, _current);
      }
   }
   
   virtual IlvBoolean isInitialEvent(IlvEvent& event)
   {
      return (IlvBoolean) (event.modifiers() & IlvCtrlModifier);
   }

   virtual IlvBoolean isOkInitialTarget(IlvGraphic* t)
   {
      return (t) ? IlvTrue : IlvFalse;
   }
   virtual IlvBoolean isOkFinalTarget(IlvGraphic* t)
   {
      return (t) ? IlvTrue : IlvFalse;
   }

   void initCursors();
   void deleteCursors();
   
   
   /* protected constructor... cannot instantiate one of these */
   ArcHandler(GraphHandler* caller, GrapherPtr manager, IlvView* view)
      : ArcHandlerParent(caller, manager, view)
   {
      initCursors();
   }
   ~ArcHandler()
   {
      deleteCursors();
   }

public:
   
};


#define ArcMakerParent ArcHandler
#define ArcMakerParentPtr ArcHandler*
class ArcMaker: public ArcMakerParent
{
protected:
   IlvBoolean exitedTarget;
   IlvGraphic* finalTarget;

   virtual void initializeData(IlvEvent& event)
   {
	 IlvRect bbox;
	 target()->boundingBox(bbox);
	 _first.move(bbox.centerx(), bbox.centery());
	 _current.move(event.x(), event.y());

	 exitedTarget = IlvFalse;
	 finalTarget = nil;
   }
   virtual void startMotion(IlvEvent& event) {
      if (!targetted()) {
	 returnControl();
      } else {
	 initializeData(event);
	 drawGhost();
	 indicateInitialTarget();
	 updateMotion(event);
      }
   }

   virtual void updateMotion(IlvEvent& event)
   {
      drawGhost();
      IlvPoint p(event.x(), event.y());
      IlvManager* mgr = getManager();
      IlvView* view = getView();
      updateData(event, view, mgr);
      if (!exitedTarget) {
	 if (mgr->lastContains(p, view) != target()) {
	    exitedTarget = IlvTrue;
	 }
      }
      if (exitedTarget) {
	 IlvGraphic* newTarget = mgr->lastContains(p, view);
	 if (finalTarget) {
	    if (newTarget != finalTarget) {
	       if (isOkFinalTarget(newTarget)) {
		  finalTarget = newTarget;
	       } else {
		  finalTarget = nil;
	       }
	    }
	 } else { // no previous final target
	    if (isOkFinalTarget(newTarget)) {
	       finalTarget = newTarget;
	    }
	 }
      }
      if (finalTarget) {
	 indicateFinalTarget();
      } else if (exitedTarget) {
	 indicateNoTarget();
      } // else leave as it was...

      ensureVisible(IlvPoint(event.x(), event.y()));
      drawGhost();
   }

   virtual void endMotion(IlvEvent& event)
   {
      updateData(event, getView(), getManager());
      indicatePreviousCursor();
      drawGhost();
      
      if (targetted() && exitedTarget && (finalTarget != nil)) {
	 doIt();
      }
      returnControl();
   }

   virtual void doIt()
   {
      XeNodePtr tail = ((LabelledNodePtr) target())->xeNode();
      XeNodePtr head = ((LabelledNodePtr) finalTarget)->xeNode();
      XeGraphPtr graph = ((GrapherPtr)getManager())->xegraph();
      
      // This will need to be replaced with a call to XeGraph::notify(ArcCreate
)
      XeArcPtr arc = new XeArc(head, tail, "New Arc", True);
   }

   virtual IlvBoolean isOkInitialTarget(IlvGraphic* t)
   {
      return (t != nil) && ((GrapherPtr)getManager())->isNode(t);
   }
   virtual IlvBoolean isOkFinalTarget(IlvGraphic* t)
   {
      return isOkInitialTarget(t);
   }
   
public:
   ArcMaker(GraphHandler* caller, GrapherPtr manager, IlvView* view)
      : ArcMakerParent(caller, manager, view)
   {
   }
};


#define ArcChangerParent ArcHandler
#define ArcChangerParentPtr ArcHandler*
class ArcChanger: public ArcChangerParent
{
protected:
   IlvGraphic* finalTarget;
   
   virtual IlvBoolean isOkInitialTarget(IlvGraphic* t)
   {
      return (t != nil) && ((GrapherPtr)getManager())->isLink(t);
   }
   virtual IlvBoolean isOkFinalTarget(IlvGraphic* t)
   {
      return (t != nil) && ((GrapherPtr)getManager())->isNode(t);
   }
   
   virtual void initializeData(IlvEvent& event) {
      // decide which end was the initial target.  For now,
      // only allow change of head
      IlvLinkImage* arc = (IlvLinkImage*) target();
      IlvGraphic* node = arc->getFrom();
      target(node);
      IlvRect bbox;
      node->boundingBox(bbox);
      _first.move(bbox.centerx(), bbox.centery());
      _current.move(event.x(), event.y());
      finalTarget = nil;
   }
   
   virtual void startMotion(IlvEvent& event) {
      if (!targetted()) {
	 returnControl();
      } else {
	 initializeData(event);
	 drawGhost();

	 // this will change to indicate appropriate target.
	 indicateNoTarget();
	 updateMotion(event);
      }
   }

   virtual void updateMotion(IlvEvent& event)
   {
      drawGhost();
      IlvPoint p(event.x(), event.y());
      IlvManager* mgr = getManager();
      IlvView* view = getView();
      updateData(event, view, mgr);
      IlvGraphic* newTarget = mgr->lastContains(p, view);
      if (finalTarget) {
	 if (newTarget != finalTarget) {
	    if (isOkFinalTarget(newTarget)) {
	       finalTarget = newTarget;
	    } else {
	       finalTarget = nil;
	    }
	 }
      } else { // no previous final target
	 if (isOkFinalTarget(newTarget)) {
	    finalTarget = newTarget;
	 }
      }
      if (finalTarget) {
	 indicateFinalTarget();
      } else {
	 indicateNoTarget();
      }

      ensureVisible(IlvPoint(event.x(), event.y()));
      drawGhost();
   }

   virtual void endMotion(IlvEvent& event)
   {
      updateData(event, getView(), getManager());
      indicatePreviousCursor();
      drawGhost();

      if (targetted() && (finalTarget != nil)) {
	 doIt();
      }
      returnControl();
   }
   virtual void doIt();

public:
   ArcChanger(GraphHandler* caller, GrapherPtr manager, IlvView* view)
      : ArcChangerParent(caller, manager, view)
   {
   }
};



#define NodeMakerParent MotionHandler
#define NodeMakerParentPtr MotionHandler*
class NodeMaker: public NodeMakerParent
{
protected:
   IlvRect _xor_rectangle;
   
   virtual void drawGhost()
   {
      if (_xor_rectangle.w()) {
	 getManager()->getDisplay()->drawRectangle(getView(),
						   getManager()->getPalette(),
						   _xor_rectangle);
      }
   }
   virtual IlvBoolean isInitialEvent(IlvEvent& event)
   {
      return (IlvBoolean) (event.modifiers() & IlvCtrlModifier);
   }

   virtual IlvBoolean isOkInitialTarget(IlvGraphic* t)
   {
      return (t) ? IlvFalse : IlvTrue;
   }
   
   virtual void initializeData(IlvEvent& event)
   {
      _xor_rectangle.move(event.x(), event.y());
      _xor_rectangle.resize(100, 40);
   }
   virtual void updateData(IlvEvent& event, IlvView* view, IlvManager* mgr)
   {
      NodeMakerParent::updateData(event, view, mgr);
      _xor_rectangle.move(event.x(), event.y());
   }
   virtual void doIt();

public:
   NodeMaker(GraphHandler* caller, GrapherPtr manager, IlvView* view)
      : NodeMakerParent(caller, manager, view)
   {
   }
};

const int ObjectLayer = 0;
const int HighlightLayer = 1;

typedef enum {
   NewSelection,
   AddSelection,
   ToggleSelection
} SelectionType;


#define SelectorParent MotionHandler
#define SelectorParentPtr MotionHandler*
class Selector: public SelectorParent
{
protected:
   // used to determine actions in various modes
   SelectionType _type;
   IlvPoint _p;
   IlvPoint _tp;
   IlvRegion _region;

   // for manipulating an individual object
   IlvBoolean _wasSelected;

   // for outlining individual graphical objects
   Outliner* _outliner;

   void deselect() {
      getManager()->deSelectAll();
   }
   
   virtual void updateData(IlvEvent& event, IlvView* view, IlvManager* mgr)
   {
      SelectorParent::updateData(event, view, mgr);
      _p.move(event.x(), event.y());
      _tp = _p;
      IlvTransformer* xform = getTransformer();
      if (xform) {
	 xform->apply(_tp);
      }
   }

   virtual IlvBoolean isInitialEvent(IlvEvent& event)
   {
      return (IlvBoolean) ((!event.modifiers()) ||
			   (event.modifiers() & IlvCtrlModifier));
   }

   virtual IlvBoolean isOkInitialTarget(IlvGraphic* t)
   {
      return (IlvBoolean)(t != nil);
      // && ((GrapherPtr) getManager())->isNode(target()));
   }

 public:
    Selector(GraphHandler* caller, GrapherPtr manager, IlvView* view)
      : SelectorParent(caller, manager, view)
       , _type(NewSelection)
       , _wasSelected(IlvFalse)
       , _outliner(nil)
   {
   }
   
   virtual void init();
   
   virtual void startMotion(IlvEvent& event);
   virtual void updateMotion(IlvEvent& event);
   virtual void endMotion(IlvEvent& event);

};


#define SweepSelectorParent Selector
#define SweepSelectorParentPtr Selector*
class SweepSelector: public SweepSelectorParent
{
protected:
   // for sweeping out rectangles for selection
   IlvRect _xor_rectangle;
   IlvPos _firstx;
   IlvPos _firsty;

   void updateData(IlvEvent& event, IlvView* view, IlvManager* mgr)
   {
      SweepSelectorParent::updateData(event, view, mgr);
      _xor_rectangle.move(IlvMin(_firstx, event.x()),
			  IlvMin(_firsty, event.y()));
      _xor_rectangle.resize((IlvDim)(IlvMax(_firstx, event.x()) -
				     _xor_rectangle.x()),
			    (IlvDim)(IlvMax(_firsty, event.y()) -
				     _xor_rectangle.y()));
   }

   virtual void startMotion(IlvEvent& event) {
      if (targetted()) {
	 returnControl();
      } else {
	 _firstx = event.x();
	 _firsty = event.y();
	 _xor_rectangle.w(0);
#if 0
	 deselect();
#endif
	 updateMotion(event);
      }
   }

   virtual void updateMotion(IlvEvent& event) {
      if (_xor_rectangle.w()) drawGhost();

      updateData(event, getView(), getManager());
      ensureVisible(IlvPoint(event.x(), event.y()));
      drawGhost();
   }

   virtual void endMotion(IlvEvent& event) {
      deselect();
      updateData(event, getView(), getManager());
      if (_xor_rectangle.w()) {
	 drawGhost();
	 doIt();
	 _xor_rectangle.w(0);
      }
      returnControl();
   }

   virtual void doIt();

   virtual void drawGhost()
   {
      if (_xor_rectangle.w()) {
	 getManager()->getDisplay()->drawRectangle(getView(),
						   getManager()->getPalette(),
						   _xor_rectangle);
      }
   }

   IlvBoolean isInitialEvent(IlvEvent& event)
   {
      return (IlvBoolean) !event.modifiers();
   }

   IlvBoolean isOkInitialTarget(IlvGraphic* t)
   {
      return (IlvBoolean)(t == nil);
   }
   
public:
    SweepSelector(GraphHandler* caller, GrapherPtr manager, IlvView* view)
      : SweepSelectorParent(caller, manager, view)
   {
   }
   
};


#define SubgraphSelectorParent Selector
#define SubgraphSelectorParentPtr Selector*
class SubgraphSelector: public SubgraphSelectorParent
{
protected:
   // for selecting subgraph -- _target holds topmost node...
   IlvUInt _nSubgraphNodes;
   LabelledNodePtr* _subgraph;
   Outliner** _subgraphOutliners;
   
   virtual void startMotion(IlvEvent& event);

   virtual void updateMotion(IlvEvent& event);
   virtual void endMotion(IlvEvent& event);
   IlvBoolean isInitialEvent(IlvEvent& event)
   {
      return (IlvBoolean) (event.modifiers() & IlvShiftModifier);
   }
   
   IlvBoolean isOkInitialTarget(IlvGraphic* t)
   {
      return (IlvBoolean)(t && ((GrapherPtr) getManager())->isNode(target()));
   }
   
public:
    SubgraphSelector(GraphHandler* caller, GrapherPtr manager, IlvView* view)
      : SubgraphSelectorParent(caller, manager, view)
   {
   }
};


#define ItemMoverParent ActionHandler
#define ItemMoverParentPtr ActionHandler*
class ItemMover: public ItemMoverParent
{
protected:
   virtual void startMove(IlvEvent&) { }
   virtual void updateMove(IlvEvent&) { }
   virtual void endMove(IlvEvent&) { }

   virtual IlvBoolean isOkInitialTarget(IlvGraphic* t) {
      return (t != nil) && ((GrapherPtr) getManager())->isNode(t);
   }
public:
   ItemMover(GraphHandler* caller, GrapherPtr manager, IlvView* view)
      : ItemMoverParent(caller, manager, view)
   {
   }
   
   void handleEvent(IlvEvent& event);

};


#define NodeMoverParent ItemMover
#define NodeMoverParentPtr ItemMover*
class NodeMover: public NodeMoverParent
{
protected:
   IlvPos _deltax, _deltay;

   void startMove(IlvEvent& event) {
      IlvRect bbox;
      target()->boundingBox(bbox, getTransformer());
      _deltax = event.x() - bbox.x();
      _deltay = event.y() - bbox.y();
   }
   virtual void updateMove(IlvEvent& event) {
      IlvRect bbox;
      target()->boundingBox(bbox, getTransformer());
      IlvPoint newOrigin(event.x() - _deltax,
			 event.y() - _deltay);
      if(getTransformer()) getTransformer()->inverse(newOrigin);
      target()->boundingBox(bbox);
      IlvDeltaPoint dp((IlvDeltaPos)(newOrigin.x()-bbox.x()),
		       (IlvDeltaPos)(newOrigin.y()-bbox.y()));
      doIt(target(), dp);
   }
   virtual void endMove(IlvEvent&) {
      target(nil);
   }
   virtual void doIt(IlvGraphic* graphic,const IlvDeltaPoint& p);
public:
   NodeMover(GraphHandler* caller, GrapherPtr manager, IlvView* view)
      : NodeMoverParent(caller, manager, view)
   {
   }
};


#define SelectionMoverParent NodeMover
#define SelectionMoverParentPtr NodeMover*
class SelectionMover: public SelectionMoverParent
{
protected:   
   virtual IlvBoolean isOkInitialTarget(IlvGraphic* t) {
      return (IlvBoolean) (t && getManager()->isSelected(t) &&
			   ((GrapherPtr)getManager())->isNode(t));
   }
   virtual void doIt(IlvGraphic* graphic, const IlvDeltaPoint& p);
public:
   SelectionMover(GraphHandler* caller, GrapherPtr manager, IlvView* view)
      : SelectionMoverParent(caller, manager, view)
   {
   }
};


#define MenuHandlerParent ActionHandler
#define MenuHandlerParentPtr ActionHandler*
class MenuHandler : public MenuHandlerParent
{
public:
   MenuHandler(GraphHandler* caller, GrapherPtr manager, IlvView* view)
      : MenuHandlerParent(caller, manager, view)
   {
   }
   void handleEvent(IlvEvent& event);
};

extern void PopupMenu(XegPopupMenuPtr menu, Widget w, IlvEvent* event);





