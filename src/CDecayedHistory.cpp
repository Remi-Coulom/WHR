/////////////////////////////////////////////////////////////////////////////
//
// CDecayedHistory.cpp
//
// Rémi Coulom
//
// April, 2008
//
/////////////////////////////////////////////////////////////////////////////
#include "CDecayedHistory.h"
#include "CGameCollection.h"

#include <cmath>

using namespace whr;

/////////////////////////////////////////////////////////////////////////////
// Constructor
/////////////////////////////////////////////////////////////////////////////
CDecayedHistory::CDecayedHistory(const CGameCollection &gc):
 HalfLife(100.0),
 Prior(0.5),
 Period(1000),
 vGamma(gc.GetPlayers()),
 vWeight(gc.GetGames()),
 vNumerator(gc.GetPlayers()),
 vDenominator(gc.GetPlayers())
{
 vgame.reserve(gc.GetGames());
 Reset();
 AddParameter("Half-life", HalfLife);
 AddParameter("Prior", Prior);
 AddParameter("Period", Period);
}

/////////////////////////////////////////////////////////////////////////////
// Reset
/////////////////////////////////////////////////////////////////////////////
void CDecayedHistory::Reset()
{
 Today = 0;
 std::fill(vGamma.begin(), vGamma.end(), 1.0f);
 std::fill(vWeight.begin(), vWeight.end(), 1.0f);
 vgame.clear();
 vvPlayerGame.clear();
 vvPlayerGame.resize(vGamma.size());
 Counter = 0;
}

/////////////////////////////////////////////////////////////////////////////
// Update game weights
/////////////////////////////////////////////////////////////////////////////
void CDecayedHistory::UpdateWeights(int Day)
{
 if (Day == Today || HalfLife == 0)
  return;
 Today = Day;

 for (int i = int(vgame.size()); --i >= 0;)
 {
  int OldDay = vgame[i].GetDay();
  float Weight = std::exp(float(OldDay - Today) / HalfLife);
  while(1)
  {
   vWeight[i] = Weight;
   if (i == 0 || vgame[i - 1].GetDay() != OldDay)
    break;
   i--;
  }
 }
}

/////////////////////////////////////////////////////////////////////////////
// One step of minorization-maximization over all players
/////////////////////////////////////////////////////////////////////////////
void CDecayedHistory::MM()
{
 for (int i = int(vGamma.size()); --i >= 0;)
 {
  vNumerator[i] = Prior;
  vDenominator[i] = 2.0 * Prior / (1.0 + vGamma[i]);
 }

 for (int i = int(vgame.size()); --i >= 0;)
 {
  int Winner;
  int Loser;
  
  if (vgame[i].GetResult())
  {
   Winner = vgame[i].GetBlack();
   Loser = vgame[i].GetWhite();
  }
  else
  {
   Winner = vgame[i].GetWhite();
   Loser = vgame[i].GetBlack();
  }

  vNumerator[Winner] += vWeight[i];
  double x = vWeight[i] / (vGamma[Winner] + vGamma[Loser]);
  vDenominator[Winner] += x;
  vDenominator[Loser] += x;
 }

 for (int i = int(vGamma.size()); --i >= 0;)
  vGamma[i] = float(vNumerator[i] / vDenominator[i]);
}

/////////////////////////////////////////////////////////////////////////////
// MM for only one player
/////////////////////////////////////////////////////////////////////////////
void CDecayedHistory::PlayerMM(int Player)
{
 double Numerator = Prior;
 double Denominator = 2.0 * Prior / (1.0 + vGamma[Player]);
 const std::vector<int> &v = vvPlayerGame[Player];

 for (int j = int(v.size()); --j >= 0;)
 {
  int i = v[j];
  const CGame &game = vgame[i];

  if (game.GetResult())
  {
   if (game.GetBlack() == Player)
    Numerator += vWeight[i];
  }
  else
  {
   if (game.GetWhite() == Player)
    Numerator += vWeight[i];
  }

  double x = vWeight[i] / (vGamma[game.GetBlack()] + vGamma[game.GetWhite()]);
  Denominator += x;
 }

 vGamma[Player] = float(Numerator / Denominator);
}

/////////////////////////////////////////////////////////////////////////////
// Add one game, and update ratings
/////////////////////////////////////////////////////////////////////////////
void CDecayedHistory::AddGame(const CGame &game)
{
 int Game = int(vgame.size());
 vgame.push_back(game);

 vvPlayerGame[game.GetBlack()].push_back(Game);
 vvPlayerGame[game.GetWhite()].push_back(Game);

 UpdateWeights(game.GetDay());

 PlayerNewton(game.GetWhite());
 PlayerNewton(game.GetBlack());
}

/////////////////////////////////////////////////////////////////////////////
// Predict outcome of one game (probability that black wins)
/////////////////////////////////////////////////////////////////////////////
float CDecayedHistory::Predict(int Handicap,
                               int Komi,
                               int Black,
                               int White,
                               int Day)
{
 if (vvPlayerGame[Black].size() == 0 &&
     vvPlayerGame[White].size() == 0)
  return 0.5;

 if (--Counter < 0)
 {
  Counter = int(Period);
  Newton();
 }
 PlayerNewton(Black);
 PlayerNewton(White);
 return vGamma[Black] / (vGamma[Black] + vGamma[White]);
}

/////////////////////////////////////////////////////////////////////////////
// Optimize one player (Newton's method)
/////////////////////////////////////////////////////////////////////////////
void CDecayedHistory::PlayerNewton(int Player)
{
 float Gamma = vGamma[Player];

 float G;
 float H;

 {
  float inv = 1.0f / (Gamma + 1.0f);
  G = Prior * (1.0f - 2.0f * Gamma * inv);
  H = -2.0f * Prior * Gamma * inv * inv - 0.1f;
 }

 const std::vector<int> &v = vvPlayerGame[Player];

 for (int j = int(v.size()); --j >= 0;)
 {
  float ThisG = 0.0;
  float ThisH = 0.0;

  int i = v[j];
  const CGame &game = vgame[i];

  float OpponentGamma;
  if (game.GetBlack() == Player)
  {
   OpponentGamma = vGamma[game.GetWhite()];
   if (game.GetResult())
    ThisG += 1.0f;
  }
  else
  {
   OpponentGamma = vGamma[game.GetBlack()];
   if (!game.GetResult())
    ThisG += 1.0f;
  }

  float inv = 1.0f / (Gamma + OpponentGamma);
  ThisG -= Gamma * inv;
  ThisH -= Gamma * OpponentGamma * inv * inv;

  G += vWeight[i] * ThisG;
  H += vWeight[i] * ThisH;
 }

 vGamma[Player] *= std::exp(-G / H);
}

/////////////////////////////////////////////////////////////////////////////
// Optimize all players
/////////////////////////////////////////////////////////////////////////////
void CDecayedHistory::Newton()
{
 for (int i = int(vGamma.size()); --i >= 0;)
  PlayerNewton(i);
}

/////////////////////////////////////////////////////////////////////////////
// Player rating
/////////////////////////////////////////////////////////////////////////////
float CDecayedHistory::GetRating(int Player)
{
 return std::log(vGamma[Player]);
}

/////////////////////////////////////////////////////////////////////////////
// Player variance
/////////////////////////////////////////////////////////////////////////////
float CDecayedHistory::GetVariance(int Player)
{
 return 0.0;
}
