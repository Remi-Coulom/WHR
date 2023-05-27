/////////////////////////////////////////////////////////////////////////////
//
// CDecayedHistory.h
//
// Rémi Coulom
//
// April, 2008
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CDecayedHistory_Declared
#define CDecayedHistory_Declared

#include "CIncrementalPredictor.h"
#include "CGame.h"
#include <vector>

namespace whr {

 class CGameCollection;

 class CDecayedHistory: public CIncrementalPredictor // dh
 {
  private: ///////////////////////////////////////////////////////////////////
   float HalfLife;
   float Prior;
   float Period;

   std::vector<float> vGamma;
   std::vector<CGame> vgame;
   std::vector<float> vWeight;
   std::vector<double> vNumerator;
   std::vector<double> vDenominator;
   std::vector<std::vector<int> > vvPlayerGame;
   int Today;
   int Counter;

   void UpdateWeights(int Day);

   void PlayerMM(int Player);
   void MM();

   void PlayerNewton(int Player);
   void Newton();

  public: ////////////////////////////////////////////////////////////////////
   CDecayedHistory(const CGameCollection &gc);
 
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
