/////////////////////////////////////////////////////////////////////////////
//
// CEGF.h
//
// Rémi Coulom
//
// January, 2008
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CEGF_Declared
#define CEGF_Declared

#include "CIncrementalPredictor.h"
#include <vector>

namespace whr {

 class CGameCollection;

 class CEGF: public CIncrementalPredictor // egf
 {
  private: ///////////////////////////////////////////////////////////////////
   std::vector<float> vRating;
 
  public: ////////////////////////////////////////////////////////////////////
   CEGF(const CGameCollection &gc);
 
   //
   // Virtual functions
   //
   void AddGame(const CGame &game);
   float Predict(int Handicap,
                 int Komi,
                 int Black,
                 int White,
                 int Day);
   float GetRating(int Player) {return vRating[Player];}
   float GetVariance(int Player) {return 0.0;}
 };

}

#endif
