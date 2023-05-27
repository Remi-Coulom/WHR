/////////////////////////////////////////////////////////////////////////////
//
// CGlicko.cpp
// according to http://math.bu.edu/people/mg/glicko/glicko.doc/glicko.html
//
// Rémi Coulom
//
// April, 2008
//
/////////////////////////////////////////////////////////////////////////////
#include "CGlicko.h"
#include "CGameCollection.h"

#include <cmath>

using namespace whr;

/////////////////////////////////////////////////////////////////////////////
// Some constants
/////////////////////////////////////////////////////////////////////////////
const float q = std::log(10.0f) / 400.0f;
const float gconst = 3.0f * q * q / (3.141592653589f * 3.141592653589f);

/////////////////////////////////////////////////////////////////////////////
// Constructor
/////////////////////////////////////////////////////////////////////////////
CGlicko::CGlicko(const CGameCollection &gc):
 MaxVariance(350 * 350),
 DailyVariance(MaxVariance / 3000.0f),
 vRating(gc.GetPlayers()),
 vVariance(gc.GetPlayers()),
 vDay(gc.GetPlayers())
{
 AddParameter("MaxVariance", MaxVariance);
 AddParameter("DailyVariance", DailyVariance);
 Reset();
}

/////////////////////////////////////////////////////////////////////////////
// Reset
/////////////////////////////////////////////////////////////////////////////
void CGlicko::Reset()
{
 std::fill(vRating.begin(), vRating.end(), 0.0f);
 std::fill(vVariance.begin(), vVariance.end(), MaxVariance);
 std::fill(vDay.begin(), vDay.end(), 0);
}

/////////////////////////////////////////////////////////////////////////////
// Update variance because of time
/////////////////////////////////////////////////////////////////////////////
void CGlicko::UpdateVariance(int Player, int Day)
{
 vVariance[Player] += float(Day - vDay[Player]) * DailyVariance;
 vDay[Player] = Day;
 if (vVariance[Player] > MaxVariance)
  vVariance[Player] = MaxVariance;
}

/////////////////////////////////////////////////////////////////////////////
// Update after game
// fWin: 1 if Player won, 0 if Opponent won
/////////////////////////////////////////////////////////////////////////////
void CGlicko::UpdateAfterGame(int Player,
                              int Opponent,
                              int fWin,
                              float &NewRating,
                              float &NewVariance)
{
 float g = 1.0f / std::sqrt(1.0f + gconst * vVariance[Opponent]);
 float E = 1.0f / (1.0f + std::pow(10.0f,
                   g * (vRating[Opponent] - vRating[Player]) / 400.0f));
 float dinv2 = q * q * g * g * E * (1 - E);

 NewVariance = 1.0f / (1.0f / vVariance[Player] + dinv2);
 NewRating = vRating[Player] + q * NewVariance * g * (float(fWin) - E);
}

/////////////////////////////////////////////////////////////////////////////
// Add one game, and update ratings
/////////////////////////////////////////////////////////////////////////////
void CGlicko::AddGame(const CGame &game)
{
 const int Black = game.GetBlack();
 const int White = game.GetWhite();

 //
 // Step 1: increase variance if time passed
 //
 UpdateVariance(Black, game.GetDay());
 UpdateVariance(White, game.GetDay());

 //
 // Step 2: update rating and variance as a consequence of this game
 //
 float BlackRating;
 float BlackVariance;
 UpdateAfterGame(Black,
                 White,
                 game.GetResult(),
                 BlackRating,
                 BlackVariance);

 float WhiteRating;
 float WhiteVariance;
 UpdateAfterGame(White,
                 Black,
                 1 - game.GetResult(),
                 WhiteRating,
                 WhiteVariance);

 vRating[Black] = BlackRating;
 vVariance[Black] = BlackVariance;
 vRating[White] = WhiteRating;
 vVariance[White] = WhiteVariance;
}

/////////////////////////////////////////////////////////////////////////////
// Predict outcome of one game (probability that black wins)
/////////////////////////////////////////////////////////////////////////////
float CGlicko::Predict(int Handicap,
                       int Komi,
                       int Black,
                       int White,
                       int Day)
{
 float D = vRating[White] - vRating[Black];
 float p = 1.0f / (1.0f + std::pow(10.0f, D / 400.0f));
 return p;
}
