/////////////////////////////////////////////////////////////////////////////
//
// CIncrementalWHR.cpp
//
// Rémi Coulom
//
// September, 2007
//
/////////////////////////////////////////////////////////////////////////////
#include "CIncrementalWHR.h"

using namespace whr;

/////////////////////////////////////////////////////////////////////////////
// Constructor
/////////////////////////////////////////////////////////////////////////////
CIncrementalWHR::CIncrementalWHR(const CGameCollection &gc):
 whr(gc),
 Period(1000)
{
 AddParameter("LinkStrength", whr.LinkStrength);
 AddParameter("InitialPriorWins", whr.InitialPriorWins);
 AddParameter("Period", Period);
}

/////////////////////////////////////////////////////////////////////////////
// Reset
/////////////////////////////////////////////////////////////////////////////
void CIncrementalWHR::Reset()
{
 whr.Reset();
 Counter = 0;
}

/////////////////////////////////////////////////////////////////////////////
// AddGame
/////////////////////////////////////////////////////////////////////////////
void CIncrementalWHR::AddGame(const CGame &game)
{
 whr.AddGame(game);
 whr.ComputeHessian(game.GetBlack(), 0);
 whr.Newton(game.GetBlack());
 whr.ComputeHessian(game.GetWhite(), 0);
 whr.Newton(game.GetWhite());

 if ((whr.GetGames() & 0xff) == 0)
  whr.NewtonKomiHandicap();
}

/////////////////////////////////////////////////////////////////////////////
// Predict
/////////////////////////////////////////////////////////////////////////////
float CIncrementalWHR::Predict(int Handicap,
                               int Komi,
                               int Black,
                               int White,
                               int Day)
{
 if (--Counter < 0)
 {
  Counter = int(Period);
  whr.OneIterationOverPlayers(0);
 }
 return whr.Predict(Handicap, Komi, Black, White, Day);
}

/////////////////////////////////////////////////////////////////////////////
// GetRating
/////////////////////////////////////////////////////////////////////////////
float CIncrementalWHR::GetRating(int Player)
{
 return 0.0;
}

/////////////////////////////////////////////////////////////////////////////
// GetVariance
/////////////////////////////////////////////////////////////////////////////
float CIncrementalWHR::GetVariance(int Player)
{
 return 0.0;
}
