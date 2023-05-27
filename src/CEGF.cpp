/////////////////////////////////////////////////////////////////////////////
//
// CEGF.cpp
// according to http://gemma.ujf.cas.cz/~cieply/GO/gormain.html
//
// Rémi Coulom
//
// January, 2008
//
/////////////////////////////////////////////////////////////////////////////
#include "CEGF.h"
#include "CGameCollection.h"

#include <cmath>

using namespace whr;

/////////////////////////////////////////////////////////////////////////////
// Algorithm parameters
/////////////////////////////////////////////////////////////////////////////
static float EGF_InitialRating = 1400.0f;
static float EGF_MinRating = 100.0f;

static float EGF_e = 0.014f;

static float EGF_a(float Rating)
{
 if (Rating > 2700.0f)
  return 70.0f;
 else
  return 200.0f - (Rating - 100.0f) / 20.0f;
}

static float EGF_con(float Rating)
{
 if (Rating >= 2700.0f)
  return 10.0f;
 else
 {
  static float tcon[28] = {116, 110, 105, 100, 95, 90, 85, 80, 75, 70, 65, 60, 55, 51, 47, 43, 39, 35, 31, 27, 24, 21, 18, 15, 13, 11, 10, 10};
  float x = (Rating - 100.0f) / 100.0f;
  int Index = int(x);
  float Remainder = x - float(Index);
  return tcon[Index] + Remainder * (tcon[Index + 1] - tcon[Index]);
 }
}

/////////////////////////////////////////////////////////////////////////////
// Constructor: set a negative rating to indicate a player who never played
/////////////////////////////////////////////////////////////////////////////
CEGF::CEGF(const CGameCollection &gc):
 vRating(gc.GetPlayers())
{
 for (int i = gc.GetPlayers(); --i >= 0;)
  vRating[i] = -1;
}

/////////////////////////////////////////////////////////////////////////////
// Predict the outcome of one game (probability that Black wins)
// ??? ignore komi and handicap: we use this algorithm for even games only
/////////////////////////////////////////////////////////////////////////////
float CEGF::Predict(int Handicap,
                    int Komi,
                    int Black,
                    int White,
                    int Day)
{
 if (vRating[Black] > vRating[White])
  return 1.0f;
 else
  return 0.0f;
}

/////////////////////////////////////////////////////////////////////////////
// Add one game, and update ratings
/////////////////////////////////////////////////////////////////////////////
void CEGF::AddGame(const CGame &game)
{
 int Black = game.GetBlack();
 int White = game.GetWhite();

 //
 // Rating initialization 
 //
 if (vRating[Black] < 0)
  vRating[Black] = vRating[White];
 if (vRating[White] < 0)
  vRating[White] = vRating[Black];
 if (vRating[Black] < 0)
  vRating[Black] = EGF_InitialRating;
 if (vRating[White] < 0)
  vRating[White] = EGF_InitialRating;

 //
 // find which player has the lower rating
 //
 int A;
 int B;
 float Result; // 1: A wins, 0: B wins

 if (vRating[White] < vRating[Black])
 {
  A = White;
  B = Black;
  Result = float(game.GetResult());
 }
 else
 {
  A = Black;
  B = White;
  Result = float(1 - game.GetResult());
 }

 //
 // Compute winning expectancies
 //
 float D = vRating[B] - vRating[A];
 float SEA = 1.0f / (std::exp(D/EGF_a(vRating[A])) + 1.0f);
 float SEB = 1.0f - SEA - EGF_e;

 //
 // Update ratings accordingly
 //
 vRating[A] += EGF_con(vRating[A]) * (Result - SEA);
 vRating[B] += EGF_con(vRating[B]) * (1 - Result - SEB);
 if (vRating[A] < EGF_MinRating)
  vRating[A] = EGF_MinRating;
 if (vRating[B] < EGF_MinRating)
  vRating[B] = EGF_MinRating;
}
