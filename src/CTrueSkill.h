/////////////////////////////////////////////////////////////////////////////
//
// CTrueSkill.h
//
// Rémi Coulom
//
// April, 2008
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CTrueSkill_Declared
#define CTrueSkill_Declared

#include "CIncrementalPredictor.h"
#include <vector>

namespace whr {

 class CGameCollection;

 class CTrueSkill: public CIncrementalPredictor // trueskill
 {
  private: ///////////////////////////////////////////////////////////////////
   float Beta2;
   float InitialVariance;
   float AdditionalVariance;

   std::vector<float> vRating;
   std::vector<float> vVariance;

   static float v(float t);
   static float w(float t);

  public: ////////////////////////////////////////////////////////////////////
   CTrueSkill(const CGameCollection &gc);
 
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
