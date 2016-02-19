


/************************************************************************\
   Grapher declaration
\************************************************************************/


#define GrapherParent IlvGrapher
#define GrapherParentPtr IlvGrapher*
class Grapher: public GrapherParent {
protected:
   XeGraphPtr xeg;
   static IlvDrawSelection* select(IlvManager* mgr, IlvGraphic* object);
public:
   Grapher(XeGraphPtr g,
	   IlvDisplay*     display,
	   int            layers    = 2,
	   IlvBoolean      useacc    = IlvTrue,
	   unsigned short maxInList = IlvMaxObjectsInList,
	   unsigned short maxInNode = IlvMaxObjectsInList)
      : GrapherParent(display, layers, useacc, maxInList, maxInNode)
   {
      xeg = g;
      setMakeSelection(Grapher::select);
   }
   ~Grapher() { }
   
   XeGraphPtr xegraph() { return xeg; }
};


