/////////////////////////////////////////////////////////////////////////////
//
// CTrueSkill.cpp
// ftp://ftp.research.microsoft.com/pub/tr/TR-2006-80.pdf
// This algorithm is patented by Microsoft
//
// Rémi Coulom
//
// April, 2008
//
/////////////////////////////////////////////////////////////////////////////
#include "CTrueSkill.h"
#include "CGameCollection.h"

#include <cmath>
#include <iostream>
#include <iomanip>

using namespace whr;

/////////////////////////////////////////////////////////////////////////////
// Constructor
/////////////////////////////////////////////////////////////////////////////
CTrueSkill::CTrueSkill(const CGameCollection &gc):
 Beta2(1.0f),
 InitialVariance(0.7f),
 AdditionalVariance(0.0f),
 vRating(gc.GetPlayers()),
 vVariance(gc.GetPlayers())
{
 AddParameter("Beta2", Beta2);
 AddParameter("InitialVariance", InitialVariance);
 AddParameter("AdditionalVariance", AdditionalVariance);
 Reset();
}

/////////////////////////////////////////////////////////////////////////////
// Reset ratings
/////////////////////////////////////////////////////////////////////////////
void CTrueSkill::Reset()
{
 for (int i = int(vRating.size()); --i >= 0;)
 {
  vRating[i] = 0;
  vVariance[i] = InitialVariance;
 }
}

/////////////////////////////////////////////////////////////////////////////
// Add one game, and update ratings
/////////////////////////////////////////////////////////////////////////////
void CTrueSkill::AddGame(const CGame &game)
{
 int Winner;
 int Loser;

 if (game.GetResult())
 {
  Winner = game.GetBlack();
  Loser = game.GetWhite();
 }
 else
 {
  Loser = game.GetBlack();
  Winner = game.GetWhite();
 }

 vVariance[Winner] += AdditionalVariance;
 vVariance[Loser] += AdditionalVariance;

 float c2 = 2 * Beta2 + vVariance[Winner] + vVariance[Loser];
 float c = std::sqrt(c2);
 float Delta = (vRating[Winner] - vRating[Loser]) / c;
 float Delta2 = Delta / std::sqrt(2.0f);
 float v = 2.0f * std::exp(-Delta2 * Delta2) / ((1.0f + float(erf(Delta2))) * std::sqrt(2.0f * 3.141592653589f));
 float w = v * (v + Delta);
 float vdivc = v / c;
 float wdivc2 = w / c2;

#if 0
 std::cerr << "c2 = " << c2 << '\n';
 std::cerr << "Delta = " << Delta << '\n';
 std::cerr << "v = " << v << '\n';
 std::cerr << "w = " << w << '\n';
 std::cerr << "vdivc = " << vdivc << '\n';
 std::cerr << "wdivc2 = " << wdivc2 << '\n';

 if (std::isnan(v))
  exit(1);
#endif

 vRating[Winner] += vVariance[Winner] * vdivc;
 vRating[Loser] -= vVariance[Loser] * vdivc;
 vVariance[Winner] *= (1 - vVariance[Winner] * wdivc2);
 vVariance[Loser] *= (1 - vVariance[Loser] * wdivc2);
}

/////////////////////////////////////////////////////////////////////////////
// Predict outcome of one game (probability that black wins)
/////////////////////////////////////////////////////////////////////////////
float CTrueSkill::Predict(int Handicap,
                          int Komi,
                          int Black,
                          int White,
                          int Day)
{
 float c2 = 2 * Beta2 + vVariance[Black] + vVariance[White];
 float c = std::sqrt(c2);
 float Delta = (vRating[Black] - vRating[White]) / c;
 return 0.5f * (float(erf(Delta/std::sqrt(2.0f))) + 1.0f);
}
