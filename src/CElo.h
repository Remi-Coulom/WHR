/////////////////////////////////////////////////////////////////////////////
//
// CElo.h
//
// Rémi Coulom
//
// January, 2008
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CElo_Declared
#define CElo_Declared

#include "CIncrementalPredictor.h"
#include <vector>

namespace whr {

 class CGameCollection;

 class CElo: public CIncrementalPredictor // elo
 {
  private: ///////////////////////////////////////////////////////////////////
   std::vector<float> vRating;
   float k;
 
  public: ////////////////////////////////////////////////////////////////////
   CElo(const CGameCollection &gc);
 
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
   float GetRating(int Player) {return vRating[Player];}
   float GetVariance(int Player) {return 0.0;}
 };

}

#endif
