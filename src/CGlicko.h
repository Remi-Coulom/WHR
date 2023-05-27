/////////////////////////////////////////////////////////////////////////////
//
// CGlicko.h
//
// Rémi Coulom
//
// April, 2008
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CGlicko_Declared
#define CGlicko_Declared

#include "CIncrementalPredictor.h"
#include <vector>

namespace whr {

 class CGameCollection;

 class CGlicko: public CIncrementalPredictor // glicko
 {
  private: ///////////////////////////////////////////////////////////////////
   float MaxVariance;
   float DailyVariance;

   std::vector<float> vRating;
   std::vector<float> vVariance;
   std::vector<int> vDay;

   void UpdateVariance(int Player, int Day);
   void UpdateAfterGame(int Player,
                        int Opponent,
                        int fWin,
                        float &NewRating,
                        float &NewVariance);
 
  public: ////////////////////////////////////////////////////////////////////
   CGlicko(const CGameCollection &gc);
 
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
   float GetVariance(int Player) {return vVariance[Player];}
 };

}

#endif
