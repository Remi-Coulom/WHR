/////////////////////////////////////////////////////////////////////////////
//
// CIncrementalWHR.h
//
// Rémi Coulom
//
// September, 2007
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CIncrementalWHR_Declared
#define CIncrementalWHR_Declared

#include "CIncrementalPredictor.h"
#include "CWHR.h"

namespace whr {

 class CIncrementalWHR: public CIncrementalPredictor // whr
 {
  private: ///////////////////////////////////////////////////////////////////
   CWHR whr;
   int Counter;
   float Period;
 
  public: ////////////////////////////////////////////////////////////////////
   CIncrementalWHR(const CGameCollection &gc);
   CWHR &GetTR() {return whr;}
 
   //
   // Virtual functions
   //
   void Reset();
   void AddGame(const CGame &game);
   float Predict(int Handicap,
                 int Komi,
                 int Black,
                 int White,
                 int Day);
   float GetRating(int Player);
   float GetVariance(int Player);
 };

}

#endif
